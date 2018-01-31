#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_LEDBackpack.h>

Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

boolean isContinuous = false;
boolean isSequential = false;

const int measurementLimit = 20;
const int maxScale = 8;

const int soundSensorPin = 17; // A0
const int soundRedZone = 5;
const int soundYellowZone = 2;
const int soundWindowWidth = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int soundSample;

const int sequentialButtonPin = 0; // D3
int sequentialButtonState;
int sequentialButtonPreviousState = LOW;

const int continuousButtonPin = 12; // D6
int continuousButtonState;
int continuousButtonPreviousState = LOW;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 50;   // the debounce time; increase if the output flickers

static const uint8_t PROGMEM
    smile[] =
        {B00111100,
         B01000010,
         B10100101,
         B10000001,
         B10100101,
         B10011001,
         B01000010,
         B00111100},
    neutral[] =
        {B00111100,
         B01000010,
         B10100101,
         B10000001,
         B10111101,
         B10000001,
         B01000010,
         B00111100},
    frown[] =
        {B00111100,
         B01000010,
         B10100101,
         B10000001,
         B10011001,
         B10100101,
         B01000010,
         B00111100};

void setup()
{
  Serial.begin(230400);
  // I2C matrix address
  matrix.begin(0x70);
  // Auto-gain, switches automatically between 1x and 16x
  tsl.enableAutoRange(true);
  // Changing the integration time gives you better sensor resolution (402ms = 16-bit data)
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS); /* fast but low resolution */
  // Setup activation button
  pinMode(sequentialButtonPin, INPUT);
  pinMode(soundSensorPin, INPUT);
}

void loop()
{
  debouncedListener(
      sequentialButtonPin,
      sequentialButtonState,
      sequentialButtonPreviousState,
      isSequential);

  debouncedListener(
      continuousButtonPin,
      continuousButtonState,
      continuousButtonPreviousState,
      isContinuous);

  if (isSequential)
  {
    scrolledText("light");
    measureAndDrawLight();
    delay(3000);

    matrix.clear();
    matrix.writeDisplay();

    scrolledText("sound");

    matrix.clear();
    matrix.writeDisplay();

    int i = 0;
    int cumulativeSoundMeasurements;
    while (i < 300)
    {
      measureAndDrawSound(cumulativeSoundMeasurements);
      i++;
    }
    int averageSoundMeasurement = cumulativeSoundMeasurements / 300;
    Serial.println(averageSoundMeasurement);
    matrix.clear();
    matrix.writeDisplay();

    isSequential = false;
  }

  if (isContinuous)
  {
    measureAndDrawCombined();
  }
}

void debouncedListener(const int &pin, int &state, int &previousState, boolean &Bool)
{
  // read the state of the switch into a local variable:
  int reading = digitalRead(pin);

  // If the switch changed, due to noise or pressing:
  if (reading != previousState)
  {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    if (reading != state)
    {
      state = reading;

      if (state == HIGH)
      {
        Bool = !Bool;
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the previousSate:
  previousState = reading;
}

void scrolledText(String text)
{
  matrix.clear();
  matrix.setTextWrap(false);
  matrix.setTextSize(1);
  matrix.setTextColor(LED_GREEN);

  // int8_t === type of unsigned integer of length 8 bits
  for (int8_t x = 7; x >= -37; x--)
  {
    matrix.clear();
    matrix.setCursor(x, 0);
    matrix.print(text);
    matrix.writeDisplay();
    delay(75);
  }
}

void measureAndDrawLight()
{
  int total = 0;

  for (int i = 1; i <= measurementLimit; i++)
  {
    // Get a new sensor event
    sensors_event_t event;
    tsl.getEvent(&event);

    if (event.light)
    {
      matrix.clear();

      if (event.light < 50)
      {
        matrix.fillRect(0, 7, 8, 1, LED_RED);
      }
      if (event.light < 100)
      {
        matrix.fillRect(0, 6, 8, 1, LED_RED);
      }
      if (event.light < 200)
      {
        matrix.fillRect(0, 5, 8, 1, LED_YELLOW);
      }

      // everything inbetween is fine
      matrix.fillRect(0, 3, 8, 2, LED_GREEN);

      if (event.light > 500)
      {
        matrix.fillRect(0, 2, 8, 1, LED_YELLOW);
      }
      if (event.light > 700)
      {
        matrix.fillRect(0, 1, 8, 1, LED_RED);
      }
      if (event.light > 1000)
      {
        matrix.fillRect(0, 0, 8, 1, LED_RED);
      }
      matrix.writeDisplay();
      total = total + event.light;
    }

    delay(200);

    if (i == measurementLimit)
    {
      matrix.clear();
      int average = total / measurementLimit;

      if (average <= 50)
      {
        matrix.drawBitmap(0, 0, frown, 8, 8, LED_RED);
      }
      if (average > 50 && average <= 200)
      {
        matrix.drawBitmap(0, 0, neutral, 8, 8, LED_YELLOW);
      }
      if (average > 200 && average <= 500)
      {
        matrix.drawBitmap(0, 0, smile, 8, 8, LED_GREEN);
      }
      if (average > 500 && average <= 600)
      {
        matrix.drawBitmap(0, 0, neutral, 8, 8, LED_YELLOW);
      }
      if (average > 600)
      {
        matrix.drawBitmap(0, 0, frown, 8, 8, LED_RED);
      }
      matrix.writeDisplay();
    }
  }
}

void measureAndDrawSound(int &total)
{
  unsigned long startMillis = millis(); // Start of soundSample window
  unsigned int peakToPeak = 0;          // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  while (millis() - startMillis < soundWindowWidth)
  {
    soundSample = analogRead(soundSensorPin);
    if (soundSample < 1024) // toss out spurious readings
    {
      if (soundSample > signalMax)
      {
        signalMax = soundSample; // save just the max levels
      }
      else if (soundSample < signalMin)
      {
        signalMin = soundSample; // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;
  total = total + peakToPeak;

  // map 1v p-p level to the max scale of the display
  int displayPeak = map(peakToPeak, 0, 1300, 0, maxScale);

  // Update the display:
  for (int i = 0; i < 7; i++) // shift the display left
  {
    matrix.displaybuffer[i] = matrix.displaybuffer[i + 1];
  }

  // draw the new soundSample
  for (int i = 0; i <= maxScale; i++)
  {
    if (i >= displayPeak) // blank these pixels
    {
      matrix.drawPixel(i, 7, 0);
    }
    else if (i < soundYellowZone)
    {
      matrix.drawPixel(i, 7, LED_GREEN);
    }
    else if (i < soundRedZone && i >= soundYellowZone)
    {
      matrix.drawPixel(i, 7, LED_YELLOW);
    }
    else
    {
      matrix.drawPixel(i, 7, LED_RED);
    }
  }
  matrix.writeDisplay();
}

void measureAndDrawCombined()
{
  Serial.println("combined");
  delay(200);
}

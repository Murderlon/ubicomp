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

unsigned int lightSample;

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
    int x = 0;
    int totalLight = 0;
    while (x < 300)
    {
      measureAndDrawLight(totalLight, 0, 7);
      delay(3);
      x++;
    }
    int averageLightMeasurement = totalLight / 300;
    conditionallyDrawLightConclusion(averageLightMeasurement);

    scrolledText("sound");
    int i = 0;
    int totalSound = 0;
    while (i < 300)
    {
      measureAndDrawSound(totalSound, 0, 7);
      delay(3);
      i++;
    }
    int averageSoundMeasurement = totalSound / 300;
    conditionallyDrawSoundConclusion(averageSoundMeasurement);

    matrix.clear();
    matrix.writeDisplay();
    isSequential = false;
  }

  if (isContinuous)
  {
    measureAndDrawCombined();
  }

  if (!isContinuous && !isSequential)
  {
    matrix.clear();
    matrix.writeDisplay();
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
  matrix.setRotation(1);
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

void measureAndDrawLight(int &total, int displayMin, int displayMax)
{
  matrix.setRotation(4);
  unsigned long startMillis = millis(); // Start of soundSample window

  while (millis() - startMillis < soundWindowWidth)
  {
    sensors_event_t event;
    tsl.getEvent(&event);
    lightSample = event.light;
    total = total + event.light;
  }

  // calculate range based on domain
  int displayPeakBright = map(lightSample, 0, 400, 4, 8);
  int displayPeakDark = map(lightSample, 0, 400, 0, 4);

  // Update the display:
  for (int i = displayMin; i < displayMax; i++) // shift the display left
  {
    matrix.displaybuffer[i] = matrix.displaybuffer[i + 1];
  }

  for (int i = 4; i <= 8; i++)
  {
    if (i >= displayPeakBright) // blank these pixels
    {
      matrix.drawPixel(i, 7, 0);
    }
    else if (i < 5)
    {
      matrix.drawPixel(i, 7, LED_GREEN);
    }
    else if (i < 7 && i >= 5)
    {
      matrix.drawPixel(i, 7, LED_YELLOW);
    }
    else
    {
      matrix.drawPixel(i, 7, LED_RED);
    }

    matrix.drawPixel(3, 7, LED_GREEN);
    matrix.drawPixel(4, 7, LED_GREEN);
  }

  for (int i = 4; i > 0; i--)
  {
    if (i <= displayPeakDark) // blank these pixels
    {
      matrix.drawPixel(i, 7, 0);
    }
    else if (i > 3)
    {
      matrix.drawPixel(i, 7, LED_GREEN);
    }
    else if (i < 4 && i >= 1)
    {
      matrix.drawPixel(i, 7, LED_YELLOW);
    }
    else
    {
      matrix.drawPixel(i, 7, LED_RED);
    }

    matrix.drawPixel(3, 7, LED_GREEN);
    matrix.drawPixel(4, 7, LED_GREEN);
  }
  matrix.writeDisplay();
}

void measureAndDrawSound(int &total, int displayMin, int displayMax)
{
  matrix.setRotation(4);
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

  // calculate range based on domain
  int displayPeak = map(peakToPeak, 0, 1300, 0, maxScale);

  // Update the display:
  for (int i = displayMin; i < displayMax; i++) // shift the display left
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
  unsigned long startMillis = millis(); // Start of soundSample window
  unsigned int peakToPeak = 0;          // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  matrix.setRotation(1);

  // Get a new sensor event
  sensors_event_t event;
  tsl.getEvent(&event);

  if (event.light)
  {
    matrix.clear();

    if (event.light < 50)
    {
      matrix.fillRect(0, 7, 3, 1, LED_RED);
    }
    if (event.light < 100)
    {
      matrix.fillRect(0, 6, 3, 1, LED_RED);
    }
    if (event.light < 200)
    {
      matrix.fillRect(0, 5, 3, 1, LED_YELLOW);
    }

    // everything inbetween is fine
    matrix.fillRect(0, 3, 3, 2, LED_GREEN);

    if (event.light > 500)
    {
      matrix.fillRect(0, 2, 3, 1, LED_YELLOW);
    }
    if (event.light > 700)
    {
      matrix.fillRect(0, 1, 3, 1, LED_RED);
    }
    if (event.light > 1000)
    {
      matrix.fillRect(0, 0, 3, 1, LED_RED);
    }

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

    // calculate range based on domain
    int displayPeak = map(peakToPeak, 0, 1300, 0, maxScale);

    for (int i = 0; i <= maxScale; i++)
    {
      if (i >= displayPeak) // blank these pixels
      {
        matrix.fillRect(5, i, 7, 1, 0);
      }
      else if (i < soundYellowZone)
      {
        matrix.fillRect(5, i, 7, 1, LED_GREEN);
      }
      else if (i < soundRedZone && i >= soundYellowZone)
      {
        matrix.fillRect(5, i, 7, 1, LED_YELLOW);
      }
      else
      {
        matrix.fillRect(5, i, 7, 1, LED_RED);
      }
    }

    matrix.writeDisplay();
    delay(3);
  }
}

void conditionallyDrawLightConclusion(int average)
{
  matrix.clear();
  matrix.setRotation(1);

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
  delay(3000);
}

void conditionallyDrawSoundConclusion(int average)
{
  matrix.clear();
  matrix.setRotation(1);

  if (average > 300 && average <= 500)
  {
    matrix.drawBitmap(0, 0, smile, 8, 8, LED_GREEN);
  }
  if (average > 500 && average <= 700)
  {
    matrix.drawBitmap(0, 0, neutral, 8, 8, LED_YELLOW);
  }
  if (average > 700)
  {
    matrix.drawBitmap(0, 0, frown, 8, 8, LED_RED);
  }

  matrix.writeDisplay();
  delay(3000);
}

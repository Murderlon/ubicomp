# Ubiquitous Computing

> A smart box prototype for the productive flex worker.

Modern day workers [spend more time in the office](http://www.motherjones.com/politics/2011/06/speed-up-american-workers-long-hours/), but if they’re not working smarter, they aren’t doing much for their employers. This might sound like a simple insight, but companies that understand what drives productivity get the most out of their talent and, as a result, produce better products, offer greater service and, ultimately, generate higher revenue.

From Carol Jones & Kelly Gordon's research _Efficient Lighting Design and Office Worker Productivity_: "Satisfaction with the office lighting influenced other areas of preference. People who were more satisfied with their lighting [...] said they were happier, more comfortable and more satisfied with their environment and their work."

So if lighting is a proven stimulator (or unstimulator), why are we neglecting it's effects? The same can actually be said about sound.

Julian Treasure, chairman of [The Sound Agency](https://www.thesoundagency.com/), states, “Noise is a major threat to our health and productivity – but until now we have been largely unconscious of its effects because of our obsession with how things look. By addressing noise concerns, we can transform the productivity and well-being of office workers, patients in hospitals and children in schools, among many others.”

Isn't it about time we strive for a more positively influenced healthy worker?

## The office

Walking around CLEVER°FRANKE's office you will notice very different atmospheres in lighting and sound depending on the surroundings and density of people. What could be a (subconscious) productivity sweet spot in the morning could be the opposite in the afternoon. The entire second floor has a glass ceiling which can introduce all kinds of lighting scenarios and there are regularly stand-up meetings, discussions, and 'watercooler' conversations going on.

![Clever Franke office](images/cf-office.png)

## Prototype

The smart box as seen below tends to solve these problems mentioned above, by cleary indicating the (subconscious) lighting and sound conditions around you, on demand at a simple touch. The LED matrix will indicate both lighting and sound as barcharts and end with a positive, neutral, or sad emoji. It will go inactive afterwards te prevent any distraction.

If the conditions are questionable, the flex worker can move him/her self to a more appropriate working spot. He/she can then check again by simply touching the box again.

<img src="images/prototype.png" alt="prototype" width=400 />

### Key Components

#### NodeMCU ESP8266

<img src="images/nodemcu.jpg" alt="NodeMCU" width=200 />

#### Bicolor LED Square Pixel Matrix

<img src="images/bi-color-matrix.jpeg" alt="LED square matrix" width=200 />

#### TSL2561 Digital Luminosity/Lux/Light Sensor

<img src="images/luminosity-sensor.jpg" alt="Luminosity sensor" width=200 />

#### Electret Microphone

<img src="images/mic-sensor.jpg" alt="Electret Microphone" width=200 />

#### Square Force-Sensitive Resistor

<img src="images/force-sensor.jpg" alt="Square Force-Sensitive Resistor" width=200 />

## References

Harrison, G. (ND). What Does Productivity Sound Like?. Retrieved from https://www.wired.com/insights/2013/08/what-does-productivity-sound-like/

Pacific Northwest National Laboratory (ND). Efficient Lighting Design and Office Worker Productivity [PDF file].
Retrieved from http://aceee.org/files/proceedings/2004/data/papers/SS04_Panel3_Paper10.pdf

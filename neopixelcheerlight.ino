// Delete and readd this using the Particle IDE.
#include "ThingSpeak/ThingSpeak.h"

// Delete and readd this using the Particle IDE.
#include "neopixel/neopixel.h"

/*
  CheerLights
  
  Reads the latest CheerLights color on ThingSpeak, and sets a common anode RGB LED on digital pins 5, 6, and 9.
  On Spark core, the built in RGB LED is used
  Visit http://www.cheerlights.com for more info.

  ThingSpeak ( https://www.thingspeak.com ) is a free IoT service for prototyping
  systems that collect, analyze, and react to their environments.

  Documentation for the ThingSpeak Communication Library for Arduino is in the extras/documentation folder where the library was installed.
  See the accompaning licence file for licensing information.
*/

#define Pixels 300

TCPClient client;

STARTUP(WiFi.selectAntenna(ANT_EXTERNAL)); // selects the u.FL antenna. Comment out if you are not using one.

/*
  This is the ThingSpeak channel number for CheerLights
  https://thingspeak.com/channels/1417.  Field 1 contains a string with
  the latest CheerLights color.
*/
unsigned long cheerLightsChannelNumber = 1417;

//Adafruit_NeoPixel strip = Adafruit_NeoPixel(#LEDs, pin);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(Pixels, 4);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(Pixels, 3);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(Pixels, 2);
Adafruit_NeoPixel strip4 = Adafruit_NeoPixel(Pixels, 5);

unsigned long previousMillis = 0;
unsigned long previousTwinkleMillis = 0;
unsigned long previousColorUpdateMillis = 0;
const long Twinkleinterval = 10;
const long interval = 5000;
const long colorUpdateInterval = 25;

int colorRed = 0;
int colorBlue = 0;
int colorGreen = 0;
float intensity[Pixels];
float fadeRate[Pixels];

String currentColor;

int ledColors [Pixels][3];
int currentLED = 0;
bool doColorUpdate = true;
bool colorUpdating = false;

Thread* updateThread;


void setup() {


  ThingSpeak.begin(client);

    strip.begin();
    strip2.begin();
    strip3.begin();
    strip4.begin();
    strip.show();
    strip2.show();
    strip3.show();
    strip4.show();

    // Adjust brightness 1-255 to suit your environment
    strip.setBrightness(255);
    strip2.setBrightness(255);
    strip3.setBrightness(255);
    strip4.setBrightness(255);
    
    for(uint16_t n=0; n<strip.numPixels(); n++) {  
        intensity[n] = 0.0;
        fadeRate[n] = 0.0;
        
        ledColors[n][0] = 0;
        ledColors[n][1] = 0;
        ledColors[n][2] = 0;
    }
    
    updateThread = new Thread("update", update);
}

void loop() {
  
    unsigned long currentMillis = millis();
  
    if (currentMillis - previousMillis >= interval) {
      
        previousMillis = currentMillis;
      
        // Read the latest value from field 1 of channel 1417
        String color = ThingSpeak.readStringField(cheerLightsChannelNumber, 1);
        
        if (color != currentColor && !colorUpdating) {
            setColor(color);
            
            doColorUpdate = true;
            colorUpdating = true;
            
            currentColor = color;
        }
    }
}

os_thread_return_t update() {
    while(true) {
        unsigned long currentMillis = millis();
        
        if (currentMillis - previousTwinkleMillis >= Twinkleinterval) {
            previousTwinkleMillis = currentMillis;
          //  String testColor = "blue";
          //  setColor(testColor);
            twinkleLEDs();      
        }
      
        if (currentMillis - previousColorUpdateMillis >= colorUpdateInterval && doColorUpdate) {
            if(currentLED < strip.numPixels()) {
                previousColorUpdateMillis = currentMillis;
                
                ledColors[currentLED][0] = colorRed;
                ledColors[currentLED][1] = colorGreen;
                ledColors[currentLED][2] = colorBlue;
                
                currentLED++;
                
                colorUpdating = true;
            } else {
                currentLED = 0;
                doColorUpdate = false;
                colorUpdating = false;
            }
        }
    }
}


// List of CheerLights color names
String colorName[] = {"none","red","pink","green","blue","cyan","white","warmwhite","oldlace","purple","magenta","yellow","orange"};

// Map of RGB values for each of the Cheerlight color names
int colorRGB[][3] = {     0,  0,  0, // "none"
                        150,  0,  0, // "red"
                        255,150,150, // "pink" //100,8,58, 255,20,147,
                          0,150,  0, // "green"
                          0,  64, 128, // "blue"
                         0,150,125,// 0, 255,255, // "cyan",
                        150,150,75,//255, 255,255, // "white",
                       150,150,50,//255,222,173,// 255, 245, 230, // "warmwhite",
                        150,150,50,//255, 245, 230, // "oldlace",
                       150,0,150,// 255,20,147,//128,  0, 25, // "purple",
                       255,20,147,// 100,0,100,//255,  0, 255, // "magenta",
                        150,100,0,//255, 255,  0, // "yellow",
                        150, 45,  0};//255, 140,  0}; // "orange"};


void setColor(String color)
{
  // Look through the list of colors to find the one that was requested
  for(int iColor = 0; iColor <= 12; iColor++)
  {
    if(color == colorName[iColor])
    {
    // When it matches, look up the RGB values for that color in the table,
    // and write the red, green, and blue values.
    colorRed = colorRGB[iColor][0];
    colorGreen = colorRGB[iColor][1];
    colorBlue = colorRGB[iColor][2];
   //   colorWipe(strip.Color(colorRGB[iColor][0], colorRGB[iColor][1], colorRGB[iColor][2]),1);
      return;
    }
  }
}

/*
void colorWipe(uint32_t c, uint8_t wait) {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);

        strip.show();
       
        delay(wait);
    }
}
*/

void twinkleLEDs() {
    for(uint16_t n=0; n<strip.numPixels(); n++) {  
        intensity[n] = intensity[n]*fadeRate[n];
        if(intensity[n]<0.01){
            intensity[n] = random(50,1000)/1000.0;
            fadeRate[n] = random(800,990)/1000.0;
        }
        int r = ledColors[n][0];
        int g = ledColors[n][1];
        int b = ledColors[n][2];
        
        strip.setPixelColor(n,r*intensity[n],g*intensity[n],b*intensity[n]);
        strip2.setPixelColor(n,r*intensity[n],g*intensity[n],b*intensity[n]);
        strip3.setPixelColor(n,r*intensity[n],g*intensity[n],b*intensity[n]);
        strip4.setPixelColor(n,r*intensity[n],g*intensity[n],b*intensity[n]);
    } 

        strip.show();
        strip2.show();
        strip3.show();
        strip4.show();
}


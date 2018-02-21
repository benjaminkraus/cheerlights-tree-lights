// This #include statement was automatically added by the Particle IDE.
#include <neopixel.h>

// This #include statement was automatically added by the Particle IDE.
#include <ThingSpeak.h>

/*
  CheerLights
  
  Reads the latest CheerLights color on ThingSpeak and updates a strip of NeoPixels.
  Visit http://www.cheerlights.com for more info.
  
  ThingSpeak ( https://www.thingspeak.com ) is a free IoT service for prototyping
  systems that collect, analyze, and react to their environments.
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the extras/documentation folder where the library was installed.
  See the accompaning licence file for licensing information.
*/

#define Pixels 120

TCPClient client;

/*
  This is the ThingSpeak channel number for CheerLights
  https://thingspeak.com/channels/1417.  Field 1 contains a string with
  the latest CheerLights color.
*/
unsigned long cheerLightsChannelNumber = 1417;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(Pixels, D0);

unsigned long previousMillis = 0;
unsigned long previousTwinkleMillis = 0;
unsigned long previousColorUpdateMillis = 0;
const long Twinkleinterval = 10;
const long interval = 5000;
const long colorUpdateInterval = 250;

int colorRed = 0;
int colorBlue = 0;
int colorGreen = 0;
float intensity[Pixels];
float fadeRate[Pixels];

String currentColor;
String thingSpeakColor;
String manualColor;
bool newManualColor = false;

int ledColors [Pixels][3];
int currentLED = 0;
bool doColorUpdate = true;
bool colorUpdating = false;

Thread* updateThread;

void setup() {
    ThingSpeak.begin(client);
    
    strip.begin();
    strip.show();

    // Adjust brightness 1-255 to suit your environment
    strip.setBrightness(255);
    
    for(uint16_t n=0; n<strip.numPixels(); n++) {
        intensity[n] = 0.0;
        fadeRate[n] = 0.0;
        
        ledColors[n][0] = 0;
        ledColors[n][1] = 0;
        ledColors[n][2] = 0;
    }
    
    updateThread = new Thread("update", update);
    
    // Register variables that store the current colors.
    Particle.variable("currentColor", currentColor);
    Particle.variable("thingColor", thingSpeakColor);
    Particle.variable("manualColor", manualColor);
    
    // Register function to update the manual color.
    Particle.function("setColor", setManualColor);
}

void loop() {
    unsigned long currentMillis = millis();
    
    if (newManualColor && !colorUpdating) {
        newManualColor = false;
        updateColor(manualColor);
    } else if (!colorUpdating && (currentMillis - previousMillis >= interval)) {
        previousMillis = currentMillis;
        
        // Read the latest value from field 1 of channel 1417
        String color = ThingSpeak.readStringField(cheerLightsChannelNumber, 1);
        
        if (color != thingSpeakColor && !colorUpdating) {
            thingSpeakColor = color;
            Particle.publish("new-thingspeak-color", thingSpeakColor, 60, PRIVATE);
            updateColor(color);
        }
    }
}

int setManualColor(String color) {
    manualColor = color;
    newManualColor = true;
    Particle.publish("new-manual-color", manualColor, 60, PRIVATE);
    return 0;
}

os_thread_return_t update() {
    while(true) {
        unsigned long currentMillis = millis();
        
        if (currentMillis - previousTwinkleMillis >= Twinkleinterval) {
            previousTwinkleMillis = currentMillis;
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
int colorRGB[][3] = {     0,  0,  0, // none:        0,  0,  0
                        255,  0,  0, // red:       255,  0,  0
                        255, 64, 64, // pink:      255,192,203
                          0, 64,  0, // green:       0,128,  0
                          0,  0,255, // blue:        0,  0,255
                          0,255,192, // cyan:        0,255,255
                        255,160,128, // white:     255,255,255
                        255,160, 32, // warmwhite: 255,222,173
                        255,160, 64, // oldlace:   253,245,230
                        128,  0,128, // purple:    128,  0,128
                        255,  0, 32, // magenta:   255,  0,255
                        255,128,  0, // yellow:    255,255,  0
                        255, 32,  0};// orange:    255,165,  0

void updateColor(String color) {
    if (color != currentColor) {
        if (setColor(color)) {
            doColorUpdate = true;
            colorUpdating = true;
            currentColor = color;
            Particle.publish("updating-color", currentColor, 60, PRIVATE);
        }
    }
}

bool setColor(String color)
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
            return true;
        }
    }
    return false; // No matching color found.
}

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
    }
    
    strip.show();
}

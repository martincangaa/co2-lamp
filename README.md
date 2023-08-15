# Co2 Lamp

A minimal, simple and good looking lamp, based on ESP8266 and MHz19b Co2 sensor. 

This lamp is good way to introduce yourself into the PCBs and 3d printing world, while at the same time you build a quite decorative and cute lamp.
PS. This is my first project with arduino and my second repository on GitHub.


https://user-images.githubusercontent.com/91851271/232823344-01003170-eaa2-429d-a9bf-3f29856314a1.mp4



### Features:
 * Co2 readings and brightness controll on App. 
 * Co2 color mode on the lamp itself (button interaction). 
 * Scheduling is an option. 

### Components

- **MHz19B**
- **esp8266 NodeMCU**
- **SK6812 RGB LED Flex-strip**
- **Touch sensor module TTP223**
- **Capacitor 10 nF**
- **LD3007ms**


### Building
The base of this was printed out of wood filament and the rest of the lamp was done with PLA (the .stl files are down in this project) 
Once you have everything printed, you'll only need to polish some of the parts with a sandpaper and it'll be done and ready to go. 
 
Then comes the electrical stuff, components which are on the list below. For the wiring you have two options:

1. Option. You can choose to use simple cables by soldering them and then slam them into the base of the lamp. (The connections for this are in a .pdf called schematic) 

2. Option. The other way to do this is to order a PCB, this is a more organized way of packaging everything and at the same time gives your project a more professional looking (To order the PCB there is .zip with all the gerber files which you will have to use to order your PCB trough sites such as jlcpcb.com "it is not an ad", you don't have to unzip it most providers require the full zip). 

Update, I notice the board has a serious design problem, it works fine but I should have used a Diod right bafor the 10uf capacitor.

![IMG_2766](https://user-images.githubusercontent.com/91851271/232711891-a06405e1-0338-41cf-b4b2-486d14af7c11.jpg)

### Software
I've used the Arduino cloud service in order to get the mqtt runing

![Untitled Diagram](https://user-images.githubusercontent.com/91851271/232711106-da8e662e-794d-4c92-abea-56df0ff7d983.jpg)

> Logic diagram from the lamp functions.


This is the main code but, you will need some other files in order to make it run (You can find them in the code folder)
```c++
#include "thingProperties.h"
#include "SoftwareSerial.h"       // Remove if using HardwareSerial or non-uno compatible device
#include "OneButton.h"            // https://github.com/mathertel/OneButton
#include "Adafruit_NeoPixel.h"    // https://github.com/adafruit/Adafruit_NeoPixel *use version 1.8.4 to avoid compatibility  issues with arduino IoT*
#include "MHZ19.h"                // https://github.com/WifWaf/MH-Z19

#define INPUT_PIN D2
#define LED_PIN D1
#define NUM_PIXELS 30 // remember to change this value depending on the numbers of leds that you have
#define LED_DELAY 20

// uint8_t LED_BRIGHTNESS = 60; // from 0 to 255 *if you are powering the led strip trough arduino be aware of the LEd consummption*

#define RX_PIN D7                                         // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN D8                                         // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                     // Device serial baudrate (should not be changed)


MHZ19 myMHZ19;                                             // Constructor for library

SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // (Uno example) create device to MH-Z19 serial

typedef enum {
  LAMP_OFF,  // set lamp "OFF".
  LAMP_ON,   // set lamp "ON"
  MODE_CO2  // blink LED "FAST"
} 
MyActions;

Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_RGBW + NEO_KHZ800);

OneButton button(INPUT_PIN, false, false);

MyActions nextAction = LAMP_OFF; // lamp off when starting

unsigned long getDataTimer = 0;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  // delay(1500);

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
  */

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  mySerial.begin(BAUDRATE);                               // (Uno example) device to MH-Z19 serial start
  
  myMHZ19.begin(mySerial);                                // *Serial(Stream) reference must be passed to library begin().

  myMHZ19.autoCalibration();

  // enable the standard led on pin 13.
  pinMode(LED_PIN, OUTPUT);      // sets the digital pin as output.

  pixels.begin(); // configure NeoPixel pin for output.
  pixels.show(); // turn off all leds.

  uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 120);

  // link the Press function to be called on a click event.   
  button.attachLongPressStart(PressFunction);

  // link the Click function to be called on a doubleclick event.   
  button.attachClick(ClickFunction);

  // set 80 msec. debouncing time. Default is 50 msec.
  // button.setDebounceTicks(80);

  button.setPressTicks(900);
  
}

void loop() {
  
  ArduinoCloud.update();

  if (millis() - getDataTimer >= 2000) {
    CDO = myMHZ19.getCO2();

    Serial1.print("CO2 (ppm): ");
    Serial1.println(CDO);

    getDataTimer = millis();

  }

  // keep watching the push button:
  button.tick(); 

  if ( (nextAction == LAMP_OFF) && (calendar.isActive()) ) {
    // turn lamp off
      GoBed();

  } else if ( (nextAction == LAMP_ON) || (calendar.isActive()) ) {
    // turn lamp on
      WakeUp();

  } else if (nextAction == MODE_CO2) {
    // start mode CO2
      CO2Mode();
  }
}
/*
  Since LEDCONTROLL is READ_WRITE variable, onLEDCONTROLLChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onLEDCONTROLLChange()  {
  uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 120);
  
  if(lED_CONTROLL.getSwitch()){
    nextAction = LAMP_ON;
  }
  else{
    nextAction = LAMP_OFF;
  }
  // Add your code here to act upon LEDCONTROLL change
}

// this function will be called when the button was held for 1 second.
void PressFunction() {
  if (nextAction == LAMP_OFF)
    nextAction = LAMP_ON;
  else
    nextAction = LAMP_OFF;
} // PressFunction


// this function will be called when the button was pressed 1 time.
void ClickFunction() {
  if (nextAction == LAMP_ON) {
    nextAction = MODE_CO2;

  } else if (nextAction == MODE_CO2) {
    nextAction = LAMP_ON;

  } 
} // ClickFunction

// lamp turns on
void WakeUp(){
    uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 120);
    for(int i=0; i<NUM_PIXELS; i++){
        pixels.setPixelColor(i, pixels.Color(40, 160, 0, 180));
        pixels.setBrightness(LED_BRIGHTNESS);
        pixels.show();
        delay(LED_DELAY);
    }
}


//lamp turns off
void GoBed(){
    uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 120);
    for(int i=NUM_PIXELS; i>=0; i--){
        pixels.setPixelColor(i, pixels.Color(0, 0, 0, 0));
        pixels.setBrightness(LED_BRIGHTNESS);
        pixels.show();
        delay(LED_DELAY);
    }
}   

void CO2Mode()  {
  // Minimum interval between CO2 reads  
  // Read CO2 from sensor
  CDO = myMHZ19.getCO2();
  
  if (400 < CDO < 600){
      uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 120);
      for(int i=0; i<NUM_PIXELS; i++){
          pixels.setPixelColor(i, pixels.Color(29, 176, 78, 0));
          pixels.setBrightness(LED_BRIGHTNESS);
          pixels.show();
          delay(LED_DELAY);
      }
  } else if (600 < CDO < 1000){
      uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 120);
      for(int i=0; i<NUM_PIXELS; i++){
         pixels.setPixelColor(i, pixels.Color(152, 201, 57, 0));
         pixels.setBrightness(LED_BRIGHTNESS);
         pixels.show();
         delay(LED_DELAY);
      }   
  } else if (1000 < CDO < 1500){
      uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 120);
      for(int i=0; i<NUM_PIXELS; i++){
         pixels.setPixelColor(i, pixels.Color(251, 196, 16, 0));
         pixels.setBrightness(LED_BRIGHTNESS);
         pixels.show();
         delay(LED_DELAY);
      }   
  } else if (1500 < CDO < 2000){
      uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 120);
      for(int i=0; i<NUM_PIXELS; i++){
          pixels.setPixelColor(i, pixels.Color(242, 146, 36, 0));
          pixels.setBrightness(LED_BRIGHTNESS);
          pixels.show();
          delay(LED_DELAY);
      }   
  } else {
      uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 120);
      for(int i=0; i<NUM_PIXELS; i++){
          pixels.setPixelColor(i, pixels.Color(255,140,140, 0));
          pixels.setBrightness(LED_BRIGHTNESS);
          pixels.show();
          delay(LED_DELAY);
      }   
  }
}

/*
  Since Calendar is READ_WRITE variable, onCalendarChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onCalendarChange()  {
  // Add your code here to act upon Calendar change
}
```
### License

[MIT](https://choosealicense.com/licenses/mit/)

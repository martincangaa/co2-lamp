#include "arduino_secrets.h"
#include "thingProperties.h"
#include "SoftwareSerial.h"       // Remove if using HardwareSerial or non-uno compatible device
#include "OneButton.h"            // https://github.com/mathertel/OneButton
#include "Adafruit_NeoPixel.h"    // https://github.com/adafruit/Adafruit_NeoPixel *use version 1.8.4 to avoid compatibility issues with arduino IoT*!!!
#include "MHZ19.h"                // https://github.com/WifWaf/MH-Z19 *search MH-Z19 by Jonathan Dempsey in the library manager*

// The model of the board being used is the nodeMCU 1.0 (ESP-12E Module) (esp2866)

#define INPUT_PIN D2              // change the input pin to the one the button is atached to
#define LED_PIN D1                // change the input pin to the one the led strip is atached to
#define NUM_PIXELS 60             // remember to change this value depending on the numbers of leds that you have

// uint8_t LED_BRIGHTNESS = 60; // from 0 to 255 *if you are powering the led strip trough arduino be aware of the LEd consummption*

#define RX_PIN D7                                         // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN D8                                         // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                     // Device serial baudrate (should not be changed)

bool ITBUTTONSTATE = false; 
/* auxiliar variable to check the state of the button (if this one becomes true 
enables the if condition to change the state of the lamp run, then this returns to its initial state "false" )*/

MHZ19 myMHZ19;                                             // Constructor for library

SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // (Uno example) create device to MH-Z19 serial

typedef enum {
  LAMP_OFF,  // set lamp "OFF"
  LAMP_OFF_FROM_CO2, // set lamp "OFF" from co2 mode
  LAMP_ON,   // set lamp "ON"
  LAMP_ON_FROM_CO2, // set lamp "ON" from co2 mode
  MODE_CO2  // co2 mode 
} 
MyActions;

MyActions nextAction; // naming the variable for my acctions

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_RGBW + NEO_KHZ800); // telling library which kind of leds are being used

OneButton button(INPUT_PIN, false, false);

unsigned long getDataTimer = 0;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);

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

  mySerial.begin(BAUDRATE);                               
  
  myMHZ19.begin(mySerial);        // *Serial(Stream) reference must be passed to library begin().

  myMHZ19.autoCalibration();

  pinMode(LED_PIN, OUTPUT); // sets the digital pin as output.

  pixels.begin(); // configure NeoPixel pin for output.
  pixels.show(); // turn off all leds.

  uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 255); // map the brightness value from 0 to 100 to 0 to 255 for IoT cloud
  
  // link the longPress function to be called on a longpress event.
  button.attachLongPressStart(pressFunction);

  // link the double click function to be called on a doubleclick event. 
  button.attachDoubleClick(doubleClickFunction);
 
  nextAction = LAMP_OFF;
}

void loop() {
  
  button.tick(); // checks the button state
  
  ArduinoCloud.update(); // updates the cloud
  
  if (millis() - getDataTimer >= 20000) {
    CDO = myMHZ19.getCO2();

    getDataTimer = millis();
    button.tick();
  } 

  // conditions to change the state of the lamp

  if ((ITBUTTONSTATE == true && nextAction == LAMP_ON) || calendar.isActive()) {
    // turn lamp on
    wakeUp();
    ITBUTTONSTATE = false;
  
  } else if (ITBUTTONSTATE == true && nextAction == LAMP_OFF && calendar.isActive() == false) {
    // turn lamp off
    goBed();
    ITBUTTONSTATE = false;

  } else if (ITBUTTONSTATE == true && nextAction == MODE_CO2) {
    // start Co2 mode
    co2Mode();
    ITBUTTONSTATE = false;

  } else if (ITBUTTONSTATE == true && nextAction == LAMP_ON_FROM_CO2) {
    // Wake up from CO2 mode  
    wakeUpFromCo2();
    ITBUTTONSTATE = false;

  } else if (ITBUTTONSTATE == true && nextAction == LAMP_OFF_FROM_CO2) {
    // Go bed from CO2 mode
    goBedFromCo2();
    ITBUTTONSTATE = false;
  }

}

/*
  Since LEDCONTROLL is READ_WRITE variable, onLEDCONTROLLChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onLEDCONTROLLChange() {
  uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 255);

  if(lED_CONTROLL.getSwitch() && (nextAction == LAMP_OFF || nextAction == LAMP_OFF_FROM_CO2)) {
    nextAction = LAMP_ON;
    ITBUTTONSTATE = true;
  }
  else if(lED_CONTROLL.getSwitch() == false && (nextAction == LAMP_ON || nextAction == LAMP_ON_FROM_CO2) && calendar.isActive() == false) {
    nextAction = LAMP_OFF;
    ITBUTTONSTATE = true;
  }
}

// this function will be called when the button had been clicked 2 times.
void doubleClickFunction() {

  if (nextAction == LAMP_ON || nextAction == LAMP_ON_FROM_CO2) {
    nextAction = MODE_CO2;
    ITBUTTONSTATE = true;
  }
  else if (nextAction == MODE_CO2) {
    nextAction = LAMP_ON_FROM_CO2;
    ITBUTTONSTATE = true;
  }
} // doubleClickFunction

// this function will be called when the button had been pressed 1 time.
void pressFunction() {

  if (nextAction == LAMP_OFF || nextAction == LAMP_OFF_FROM_CO2){
    nextAction = LAMP_ON;
    ITBUTTONSTATE = true;
  } 
  else if (nextAction == LAMP_ON || nextAction == LAMP_ON_FROM_CO2) {
    nextAction = LAMP_OFF;
    ITBUTTONSTATE = true;
  }
  else if (nextAction == MODE_CO2) {
    nextAction = LAMP_OFF_FROM_CO2;
    ITBUTTONSTATE = true;
  }
} // pressFunction

// lamp turns on
void wakeUp() {

  uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 255);

  for(int i = 0; i <= 180; i++) {
    for(int j = 0; j < pixels.numPixels(); j++) {
      pixels.setPixelColor(j, pixels.Color(i * 2/9, i * 17/12, 0, i * 17/12)); // 180 * values = 40, 195, 0, 255 (green, red, blue, white)
      pixels.setBrightness(LED_BRIGHTNESS);
    }
    pixels.show();
    delay(10);  // Adjust the fading speed here
  }
}

void wakeUpFromCo2() {

  if (CDO <= 1000){
    wakeUpColorFromCo2(17/12, 1/9, 1/9, 0); // 180 * values = 255, 20, 20, 0 (green, red, blue, white)

  } else if (CDO <= 2000){
    wakeUpColorFromCo2(13/36, 17/12, 0, 0); // 180 * values = 255, 255, 10, 0 (green, red, blue, white)

  } else if (CDO > 2000){
    wakeUpColorFromCo2(0, 17/12, 0, 0); // 180 * values = 0, 255, 0, 0 (green, red, blue, white)

  }
}


//lamp turns off
void goBed() {

  uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 255);
    
  for(int i = 180; i >= 0; i--) {
    for(int j = 0; j < pixels.numPixels(); j++) {
      pixels.setPixelColor(j, pixels.Color(i * 2/9, i * 17/12, 0, i * 17/12)); // 180 * values = 40, 195, 0, 255 (green, red, blue, white)
      pixels.setBrightness(LED_BRIGHTNESS);
    }
    pixels.show();
    delay(10);  // Adjust the fading speed here
  }
}

void goBedFromCo2() {

  if (CDO <= 1000){
    goBedColorFromCo2(17/12, 17/12, 1/18, 0); // 180 * values = 255, 255, 10, 0 (green, red, blue, white)

  } else if (CDO <= 2000){
    goBedColorFromCo2(13/36, 17/12, 0, 0); // 180 * values = 255, 255, 10, 0 (green, red, blue, white)

  } else if (CDO > 2000){
    goBedColorFromCo2(0, 17/12, 0, 0); // 180 * values = 0, 255, 0, 0 (green, red, blue, white)

  }
}

void co2Mode() { 
  // Read CO2 from sensor
  Serial.println(CDO);

  if (CDO <= 1000){
    setColorCo2(17/12, 1/9, 1/9, 0); // 180 * values = 255, 20, 20, 0 (green, red, blue, white)

  } else if (CDO <= 2000){
    setColorCo2(13/36, 17/12, 0, 0); // 180 * values = 255, 255, 10, 0 (green, red, blue, white)

  } else if (CDO > 2000){
    setColorCo2(0, 17/12, 0, 0); // 180 * values = 0, 255, 0, 0 (green, red, blue, white)

  }
}

void setColorCo2(int8_t g, int8_t r ,int8_t b, int8_t w) {

  uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 255);

  for(int i = 180; i >= 0; i--) {
    for(int j = 0; j < pixels.numPixels(); j++) {
      pixels.setPixelColor(j, pixels.Color(i * 2/9, i * 17/12, 0, i * 17/12)); // 180 * values = 40, 195, 0, 255 (green, red, blue, white)
      pixels.setBrightness(LED_BRIGHTNESS);
    }
    pixels.show();
    delay(10);  // Adjust the fading speed here
  }

  for(int i = 0; i <= 180; i++) {

    for(int j = 0; j < pixels.numPixels(); j++) {
      pixels.setPixelColor(j, pixels.Color(i * g, i * r, i * b, i * w));
      pixels.setBrightness(LED_BRIGHTNESS);
    }
    pixels.show();
    delay(10);  // Adjust the fading speed here
  }
}

void wakeUpColorFromCo2(int8_t g, int8_t r ,int8_t b, int8_t w) {

  uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 255);
  
  for(int i = 180; i >= 0; i--) {
    for(int j = 0; j < pixels.numPixels(); j++) {
      pixels.setPixelColor(j, pixels.Color(i * g, i * r, i * b, i * w));
      pixels.setBrightness(LED_BRIGHTNESS);
    }
    pixels.show();
    delay(10);  // Adjust the fading speed here
  }

  for(int i = 0; i <= 180; i++) {
    for(int j = 0; j < pixels.numPixels(); j++) {
      pixels.setPixelColor(j, pixels.Color(i * 2/9, i * 17/12, 0, i * 17/12)); // 180 * values = 40, 195, 0, 255 (green, red, blue, white)
      pixels.setBrightness(LED_BRIGHTNESS);
    }
    pixels.show();
    delay(10);  // Adjust the fading speed here
  }
}

void goBedColorFromCo2(int8_t g, int8_t r ,int8_t b, int8_t w) {

  uint8_t LED_BRIGHTNESS = map(lED_CONTROLL.getBrightness(), 0, 100, 0, 255);
  
  for(int i = 180; i >= 0; i--) {
    for(int j = 0; j < pixels.numPixels(); j++) {
      pixels.setPixelColor(j, pixels.Color(i * g, i * r, i * b, i * w));
      pixels.setBrightness(LED_BRIGHTNESS);
    }
    pixels.show();
    delay(10);  // Adjust the fading speed here
  }
}
void onCalendarChange() {}    // Arduino cloud makes the use of this function mandatory in order to use its scheduler, ps doesn't do anything

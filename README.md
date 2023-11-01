# CO2 Lamp

  

A minimal and probably overkill lamp, based on the microcontroller ESP8266 and MHZ-19B Co2 sensor that allow us to have controll over the Co2 levels in our room, kitchen...

This lamp is good way to introduce yourself into the Arduino and 3d printing world, while at the same time you build a quite decorative and cute lamp.

PS. This is my first project with arduino.

https://user-images.githubusercontent.com/91851271/232823344-01003170-eaa2-429d-a9bf-3f29856314a1.mp4

  
![IMG_5844](https://github.com/martincangaa/co2-lamp/assets/91851271/03bcff7d-72ba-4d37-b127-4adee2727eba)

  

### Features:

* Controllable brightnes on app

* Co2 readings through app (chart)
* Scheduling option (this allows us to tell the lamp when to turn on)

* Co2 mode integrated with the lamp that tell us the  Co2 levels based on its color


![Screenshot_1](https://github.com/martincangaa/co2-lamp/assets/91851271/113031d9-c4c6-459c-90dc-5f36d2a09c8a)
  

### Components

  

-  **[MHZ-19B](https://es.aliexpress.com/item/4000212024923.html?channel=twinner)**

-  **[ESP8266 NodeMCU 1.0](https://es.aliexpress.com/item/1005005704719013.html?channel=twinner)**

-  **[SK6812 RGBWW LED Flex-strip](https://es.aliexpress.com/item/1005006044418344.html?channel=twinner)**

-  **[Touch sensor module TTP223](https://es.aliexpress.com/item/4000540638639.html?channel=twinner)**

-  **[LD3007ms fan](https://es.aliexpress.com/item/4001026649515.html?channel=twinner)** (Optional)
- **[USB-C board](https://es.aliexpress.com/item/1005006047462864.html?channel=twinner)**

  
  

## Body and electronics 

The base of the lamp was printed out of wood filament (recomendable) and the rest of the lamp was made out PLA (the .stl files are down in this project)

Once you have everything printed, you'll only need to polish the parts with some sandpaper and it'll be ready to go.

Then comes the electrical stuff, components which are on the upper list (components). For the wiring you have two options:

1. Option. You can choose to solder the components through wires following the next circuit (this is the cheapest and most reliable way to do it):

  2. Option. Order a PCB (expensive and overengineered option don do it) you can download the gerber files in the folder of the ropository desing files

![IMG_2766](https://user-images.githubusercontent.com/91851271/232711891-a06405e1-0338-41cf-b4b2-486d14af7c11.jpg)

  

## Software

The lamp uses Arduino Cloud MQTT service in order to controll the lamp trough internet and you will need some [libraries](https://www.arduino.cc/reference/en/libraries/) (Think of them as mini-programs that you can include in your own Arduino projects to make things work the way you want) in order to be able to compile the program.

|Library|Author  |
|--|--|
|MHZ-19 | [Jonathan Dempsey](https://github.com/WifWaf/MH-Z19)
OneButton | [Matthias Hertel](https://github.com/mathertel/OneButton)
Neopixel (must use version 1.8.4)| [Adafruit](https://github.com/WifWaf/MH-Z19)
> Also you will have to install the arduino IoT libraries (When you try to verify the program arduino will tell you which libraries you'll need)

![Diagrama en blanco](https://github.com/martincangaa/co2-lamp/assets/91851271/fce6eae1-30f2-4cd4-b458-6db0eff0fba5)


  > Logic diagram from the lamp functions.

### CO2 color code:

In  `CO2 MODE` the lamp will give display the given colours based on the number of ppm (parts per million) of CO2 in the room:



|             |RGBW                          |Co2 levels                      |
|-------------|-------------------------------------------|-------------------|
|Green        |`255, 20, 20, 0 (green, red, blue, white)`|CO2 <= 1000ppm|
|Yellow       |`255, 255, 10, 0 (green, red, blue, white)`|1000ppm < CO2 <= 2000ppm|
|Red          |`0, 255, 0, 0 (green, red, blue, white)`|CO2 > 2000ppm         |

  
  



### License
[MIT](https://choosealicense.com/licenses/mit/)

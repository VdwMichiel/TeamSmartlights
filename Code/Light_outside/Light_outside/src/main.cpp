#include <Arduino.h>

#include <Wire.h>
#include "DFRobot_VEML7700.h"

#define power_veml 17

#define uS_TO_S 1000000
#define TIME_TO_SLEEP_START_VEML 30 //aantal seconden in deep sleep voor opstarten VEML
#define TIME_TO_SLEEP 300 //aantal seconden in deep sleep voor elke loop
 
DFRobot_VEML7700 als;

float mean_lux;
float lux;
float old_lux;

int lux_interval(int waarde){
  if (waarde<500){
    return 1;
  }
  return 2;
}
 
void setup()
{
  Serial.begin(9600);
  als.begin();


  //Meten wat beginwaarde is bij opstarten
  for(int i = 0; i<10;i++){
    als.getALSLux(old_lux);
  }
  old_lux = old_lux/10;
}
 
 /* 
 Om de zoveel tijd 1 of meerdere metingen
 meerdere metingen om foutieve resultaten te kunnen wegfilteren met gemiddelde ofzo
 Indien groter dan een bepaalde waarde, doorsturen voor alarm

 Hoe doorsturen? --> welk protocol
 */

void loop()
{
  //PIN HIGH mosfet
  digitalWrite(power_veml, HIGH);

  //deep sleep voor VEML op te starten
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_START_VEML*uS_TO_S);
  esp_deep_sleep_start();


  float lux_total=0;

  for(int i = 0; i<10;i++){
    als.getALSLux(lux);
    lux_total += lux;
    delay(200);
    //Serial.print("Lux:");
    //Serial.print(lux);
    //Serial.println(" lx");
  }

  mean_lux = lux_total/10;

  if (lux_interval(mean_lux) != lux_interval(old_lux) && lux_interval(mean_lux) == 2){ //De extra voorwaarde van mean lux = 2 dient om enkel signaal te sturen als lichten uit moeten
    //stuur bericht naar alarm om te zeggen dat de lichten aan of uit moeten

    old_lux = mean_lux;
  }

  //PIN LOW mosfet
  digitalWrite(power_veml,LOW);

  //deep sleep 5 minuten
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP*uS_TO_S);
  esp_deep_sleep_start();

}


//http://www.esp32learning.com/code/an-esp32-and-veml7700-lux-sensor-example-using-the-arduino-ide.php

//SITE OM PIN HOOG TE HOUDEN TIJDENS DEEP SLEEP
//https://electronics.stackexchange.com/questions/350158/esp32-how-to-keep-a-pin-high-during-deep-sleep-rtc-gpio-pull-ups-are-too-weak


#include <Arduino.h>

#define sensor_LDR 36
#define mosfet 17

#define uS_TO_S 1000000
#define TIME_TO_SLEEP 300 //aantal seconden in deep sleep

int totalTime = 0;
int on_time = 0;
int off_time = 0;
int last_value = 100; //waarde waarbij licht uit staat om te starten
int totalHours = 0;
int extra_minutes = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {


  int value = analogRead(sensor_LDR);

  /*Indien de lamp aan is moet er geteld worden hoe lang de lamp aanstaat
  Er kan ook gewerkt worden met metingen om een bepaalde delay en dan tellen hoeveel delays boven een bepaalde waarde
  */
  
  if(value > last_value+1000){ // als opeens de waarde met meer dan 1000 toeneemt, dan timer starten
    on_time = millis();
    last_value = value;
  }

  if(value < last_value-1000){ //als opeens de waarde met meer dan 1000 afneemt, dan timer stoppen
    off_time = millis();
    last_value = value;
    totalTime += off_time - on_time;
  }

  totalHours = totalTime/3600000; //3 600 000 milliseconds in 1 hour
  extra_minutes = (totalTime%3600000)/60000; // kan nu getoond worden in ...uren ...minuten


  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP*uS_TO_S);
  esp_deep_sleep_start();


  //Kan nu getoond worden op een display of doorgestuurd worden naar het centrale onderdeel
  
}


/* 
MQTT https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
*/

//https://www.instructables.com/Interfacing-Photoresistor-With-ESP32/
//https://esp32io.com/tutorials/esp32-light-sensor

//SITE OM PIN HOOG TE HOUDEN TIJDENS DEEP SLEEP
//https://electronics.stackexchange.com/questions/350158/esp32-how-to-keep-a-pin-high-during-deep-sleep-rtc-gpio-pull-ups-are-too-weak

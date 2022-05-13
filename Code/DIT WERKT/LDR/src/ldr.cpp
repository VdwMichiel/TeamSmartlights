#include <Arduino.h>

#define sensor_LDR 36

#define power GPIO_NUM_21


#define uS_TO_S 1000000
#define TIME_TO_SLEEP 5 //aantal seconden in deep sleep

//RTC_DATA_ATTR int totalTime = 0;
//RTC_DATA_ATTR int on_time = 0;
//RTC_DATA_ATTR int off_time = 0;
//RTC_DATA_ATTR int last_value = 600; //waarde waarbij licht uit staat om te starten
RTC_DATA_ATTR int totalseconds = 0;
RTC_DATA_ATTR int totalHours = 0;
RTC_DATA_ATTR int extra_minutes = 0;
RTC_DATA_ATTR int extra_seconds = 0;

RTC_DATA_ATTR int aantal = 0;

////////////////////////////////// BLE /////////////////////////////////
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "7ff58734-b6c0-4008-92bd-ddeb01dbafed"
#define CHARACTERISTIC_UUID "bac2d100-a95e-468c-9f8e-057cd49116f5"

bool deviceConnected = false;
BLECharacteristic* pCharacteristic = NULL;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      BLEDevice::startAdvertising();
      deviceConnected = false;
    }
};

void setup_ble(){
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("in");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Start");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

//////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(sensor_LDR, INPUT);
  pinMode(power, OUTPUT);
  setup_ble();

}

void loop() {

  digitalWrite(power, HIGH);

  int value = analogRead(sensor_LDR);

  Serial.println(value);

  /*Indien de lamp aan is moet er geteld worden hoe lang de lamp aanstaat
  Er kan ook gewerkt worden met metingen om een bepaalde delay en dan tellen hoeveel delays boven een bepaalde waarde
  */
  
  if(value  >1500 ){ // als opeens de waarde met meer dan 1000 toeneemt, dan timer starten
    //on_time = millis();
    //last_value = value;
    aantal++;
    
  }


  Serial.print("Aantal intervallen: ");
  Serial.println(aantal);


  totalseconds = aantal*TIME_TO_SLEEP;
  totalHours = totalseconds/3600; //3 600 000 milliseconds in 1 hour
  extra_minutes = (totalseconds%3600)/60;
  extra_seconds = (totalseconds%3600)%60;

  Serial.print("Totaal aantal seconden: ");
  Serial.println(totalseconds);
  Serial.print(totalHours);
  Serial.print("u");
  Serial.print(extra_minutes);
  Serial.print("min");
  Serial.print(extra_seconds);
  Serial.println("sec");
                                           
  
  //Kan nu getoond worden op een display of doorgestuurd worden naar het centrale onderdeel
  if (deviceConnected) {
        pCharacteristic->setValue(std::to_string(totalseconds));
        pCharacteristic->notify();
        delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
  
    


  digitalWrite(power,LOW);

/*
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP*uS_TO_S);
  esp_deep_sleep_start();
*/
  delay(5000);
  
  
}


/* 
MQTT https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
*/

//https://www.instructables.com/Interfacing-Photoresistor-With-ESP32/
//https://esp32io.com/tutorials/esp32-light-sensor
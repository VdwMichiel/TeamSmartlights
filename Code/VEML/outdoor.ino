#include <Arduino.h>

#include <Wire.h>
#include <DFRobot_VEML7700.h>

#define power_veml GPIO_NUM_33

#define uS_TO_S 1000000
#define TIME_TO_SLEEP_START_VEML 10 //aantal seconden in deep sleep voor opstarten VEML
#define TIME_TO_SLEEP 10 //aantal seconden in deep sleep voor elke loop
 
DFRobot_VEML7700 als;

float mean_lux;
float lux;
RTC_DATA_ATTR float old_lux = 0;
bool alarm_bool = false;

int lux_interval(int waarde){
  if (waarde<150){
    return 1;
  }
  return 2;
}

////////////////////////////////// BLE /////////////////////////////////
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "841178f3-8c64-49f0-90fd-5089029cb323"
#define CHARACTERISTIC_UUID "bc3bfe05-1d81-4cc6-a1e5-ef2d61ed7ddc"

bool deviceConnected = false;
BLECharacteristic* pCharacteristic = NULL;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup_ble(){
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("out");
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
 
void setup()
{
  Serial.begin(9600);
  als.begin();
  pinMode(power_veml, OUTPUT);
  setup_ble();

/*
  //Meten wat beginwaarde is bij opstarten
  for(int i = 0; i<10;i++){
    als.getALSLux(lux);
    old_lux += lux;
    delay(200);
    Serial.println(lux);
  }
  old_lux = old_lux/10;
  */
}
 

void loop()
{
  //PIN HIGH mosfet
  digitalWrite(power_veml, HIGH);
  //gpio_hold_en(power_veml);
  
  //delay(5000);
  //deep sleep voor VEML op te starten
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_START_VEML*uS_TO_S);
  //esp_deep_sleep_start();


  float lux_total=0;

  for(int i = 0; i<11;i++){
    als.getALSLux(lux);
    delay(200);
    if(i > 0){
      lux_total += lux; 
      Serial.print("Lux:");
      Serial.print(lux);
      Serial.println(" lx");
    }
  }

  mean_lux = lux_total/10;
  Serial.println(mean_lux);

  if (alarm_bool){
    alarm_bool = false;
    if (deviceConnected) {
      pCharacteristic->setValue("reset");
      pCharacteristic->notify();
      delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
  }

  if (lux_interval(mean_lux) != lux_interval(old_lux) && lux_interval(mean_lux) == 2){ //De extra voorwaarde van mean lux = 2 dient om enkel signaal te sturen als lichten uit moeten
    //stuur bericht naar alarm om te zeggen dat de lichten aan of uit moeten
    alarm_bool=true;
    
    if (deviceConnected) {
      pCharacteristic->setValue("ALARM");
      pCharacteristic->notify();
      delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    
    Serial.println("ALARM ALARM ALARM");
  }
  old_lux = mean_lux;


  //PIN LOW mosfet
  digitalWrite(power_veml,LOW);
  //gpio_hold_en(power_veml);

  //deep sleep 5 minuten
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP*uS_TO_S);
  //esp_deep_sleep_start();

  delay(5000);

}

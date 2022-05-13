#include <Arduino.h>
#define timeSeconds 10
#define uS_TO_S_FACTOR 1000000  
#define TIME_TO_SLEEP  5
#define led  GPIO_NUM_27
#define motionSensor GPIO_NUM_35
#define led2 GPIO_NUM_26
bool interup = false;
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
int scanTime = 5; //In seconds
BLEScan* pBLEScan;

// The remote service we wish to connect to.
static BLEUUID serviceUUID("841178f3-8c64-49f0-90fd-5089029cb323");
static BLEUUID serviceUUID2("7ff58734-b6c0-4008-92bd-ddeb01dbafed");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("bc3bfe05-1d81-4cc6-a1e5-ef2d61ed7ddc");
static BLEUUID    charUUID2("bac2d100-a95e-468c-9f8e-057cd49116f5");
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static boolean doConnect2 = false;
static boolean connected2 = false;
static boolean doScan2 = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static BLERemoteCharacteristic* pRemoteCharacteristic2;
static BLEAdvertisedDevice* myDevice2;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}
static void notifyCallback2(
  BLERemoteCharacteristic* pBLERemoteCharacteristic2,
  uint8_t* pData2,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic2->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData2);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};
class MyClientCallback2 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient2) {
  }

  void onDisconnect(BLEClient* pclient2) {
    connected2 = false;
    Serial.println("onDisconnect");
  }
};
bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
        std::string value = pRemoteCharacteristic->readValue();
        Serial.print("The characteristic value was: ");
        Serial.println(value.c_str());
      }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
bool connectToServer2() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice2->getAddress().toString().c_str());
    
    BLEClient*  pClient2  = BLEDevice::createClient();
    Serial.println(" - Created client2");

    pClient2->setClientCallbacks(new MyClientCallback2());

    // Connect to the remove BLE Server.
    pClient2->connect(myDevice2);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server2");
    pClient2->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService2 = pClient2->getService(serviceUUID2);
    if (pRemoteService2 == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID2.toString().c_str());
      pClient2->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic2 = pRemoteService2->getCharacteristic(charUUID2);
    if (pRemoteCharacteristic2 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID2.toString().c_str());
      pClient2->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic2->canRead()) {
        std::string value2 = pRemoteCharacteristic2->readValue();
        Serial.print("The characteristic value was: ");
        Serial.println(value2.c_str());
      }

    if(pRemoteCharacteristic2->canNotify())
      pRemoteCharacteristic2->registerForNotify(notifyCallback2);

    connected2 = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
bool check1 = false;
bool check2 = false;
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID2)) {

      myDevice2 = new BLEAdvertisedDevice(advertisedDevice);
      doConnect2 = true;
      doScan2 = true;

    } // Found our server
    if (check1 == true & check2 == true){
      check1 = false;
      check2= false;
      BLEDevice::getScan()->stop();
    }
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void IRAM_ATTR detectsMovement() {
  interup = true;
}

void setup() {
  Serial.begin(115200);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  pinMode(led, OUTPUT);
  pinMode(led2,OUTPUT);
  digitalWrite(led, LOW);
  digitalWrite(led2,LOW);
  BLEDevice::init("");
  delay(5000);
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

bool veml = false;
bool ldr = false;
std::string prevLdr = "";

void loop() {
  delay(5000);
//  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
//  Serial.print("Devices found: ");
//  Serial.println(foundDevices.getCount());
//  Serial.println("Scan done!");
//  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
  if (doConnect2 == true) {
    if (connectToServer2()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect2 = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  std::string value;
  std::string value2;
  if (connected) {
    if(pRemoteCharacteristic->canRead()) {
        value = pRemoteCharacteristic->readValue();
        Serial.print("VEML: ");
        Serial.println(value.c_str());
      }
  }else{
    BLEDevice::getScan()->start(5,false);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }
  if (connected2) {
    if(pRemoteCharacteristic2->canRead()) {
        value2 = pRemoteCharacteristic2->readValue();
        Serial.print("LDR: ");
        Serial.println(value2.c_str());
        
      }
  }else{
    BLEDevice::getScan()->start(5,false);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }
  Serial.println("CHECKING FOR MOTION AGAIN");
  delay(5000);
  bool motion = false;
  if(interup == true){
    Serial.println("MOTION DETECTED!!!");
    motion = true;
    digitalWrite(led, HIGH);
    //gpio_hold_en(led);
    delay(1000);
    interup = false;
    //esp_deep_sleep_start();
  }else{
    Serial.println("NO MOTION DETECTED");
    digitalWrite(led2, HIGH);
    delay(1000);
    //gpio_hold_en(led2);
    //esp_deep_sleep_start();
  }
  if(motion != true && value2 != prevLdr){
    Serial.println("Turn off lights");
  }
  if(motion == true && value == "ALARM" && value2 != prevLdr){
    Serial.println("Turn off lights");
  }
  prevLdr = value2;
}

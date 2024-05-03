#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)  // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define LED_PIN LED_BUILTIN
#define SERVICE_UUID "d7204ebc-4bc5-4a6a-a7e5-7a86f998b7a9"

#define CHARACTERISTIC_UUID_RX "9be42785-94ae-4c1b-aa02-cd87c1c647c8"
#define CHARACTERISTIC_UUID_TX "59da6d93-99c4-48c0-8a6e-d02dfe235787"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

BLECharacteristic *pCharacteristic;
int HRval = 99;
bool deviceConnected = false;
String NewRxStr ="";


// This sets _base_ MAC address. The actual BLE MAC Address will be
// D0:90:AB:CD:EF:E2. To understand why, see
// https://docs.espressif.com/projects/esp-idf/en/v3.1.7/api-reference/system/base_mac_address.html#number-of-universally-administered-mac-address
uint8_t kBaseMACAddress[6] = {0xd0, 0x90, 0xab, 0xcd, 0xef, 0xa0};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("[MyServerCallbacks::onConnect] Client connected");
    deviceConnected = true;

  }

  void onDisconnect(BLEServer *pServer) {
    Serial.println(
        "[MyServerCallbacks::onConnect] Client disconnected - resuming advertising");
        deviceConnected = false;

    BLEDevice::startAdvertising();
  }
};


//reading first Ascii character
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    Serial.printf("[WriteCallbacks::onWrite] written length: %d\n",
                    rxValue.length());
    
      if (rxValue.length() > 0){
       String NewRxStr ="";
       for (int i = 0; i < rxValue.length(); i++) {
            NewRxStr = NewRxStr + rxValue[i];
      }
      Serial.print("[BLE] New Value::onRecieved: ");
      Serial.println(NewRxStr);
      
    }
  }
  
};

      

void setup() {
  Serial.begin(115200);

  if (esp_base_mac_addr_set(kBaseMACAddress) != ESP_OK) {
    Serial.println("[setup] Error setting MAC address");
  }
  // Set Bluetooth Name
  BLEDevice::init("HRM Sample Demo");

// Create BLE Server/Device 
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

// Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

   // Create a BLE  Notify Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX, 
                      BLECharacteristic::PROPERTY_NOTIFY   
                      );

  // BLE2902 needed to notify
 pCharacteristic->addDescriptor(new BLE2902());


   BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                              CHARACTERISTIC_UUID_RX, 
                                              BLECharacteristic::PROPERTY_WRITE
                                              );
   
    pCharacteristic->setCallbacks(new MyCallbacks());
 


// Data
  pCharacteristic->setValue(HRval);

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  
  pAdvertising->start();
}

void loop() {

  
if (deviceConnected) {
        //Send character (set -> notify)
        
        
    //Max HR Set potentiometer
    int HRval = analogRead(A0); // Range 0-4095 

   
      Serial.print("HR Val: ");
      Serial.print(HRval);
      Serial.println();

    //convert HRVal (int) to Characteristic
    char txString[8];
    dtostrf(HRval, 1, 2, txString);
    
    //Setting value to be sent 
    pCharacteristic->setValue(txString);
    
    // Notifying the client
    pCharacteristic->notify();

    Serial.println("Sent value: " + String(txString));
}
  delay(1000);
}


// NEOPIXEL: https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use
// Serial Library: https://www.electronicshub.org/esp32-bluetooth-tutorial/
// Device type: https://wiki.dfrobot.com/Beetle_ESP32_SKU_DFR0575
// Chipset ESP32-WROOM-DA Module or Firebeetle-ESP32


#include "BLEDevice.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LED_PIN D2
int LED_COUNT = 200; // max memory position, can be reduced programably
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)  // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


// The remote service we wish to connect to.
static BLEUUID serviceUUID("0000180d-0000-1000-8000-00805f9b34fb");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID(BLEUUID((uint16_t)0x2A37));
//0x2A37

static boolean doConnect = false;
static boolean connected = false;
static boolean notification = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

uint32_t LED_00 = strip.Color(0, 0, 0);           // off
uint32_t LED_Blue = strip.Color(0, 0, 255);       //Blue
uint32_t LED_Green = strip.Color(0, 255, 0);      //Green
uint32_t LED_Yellow = strip.Color(255, 255, 0);   //Yellow
uint32_t LED_Orange = strip.Color(255, 165, 0);   //Orange
uint32_t LED_Red = strip.Color(255, 0, 0);        // Red

int LEDPos;
int LEDBandZ1;
int LEDBandZ2;
int LEDBandZ3;
int LEDBandZ4;
int LEDBandZ5;

float HrRange;
int HrMaxVal;
float HrPercentage;
float HrInt;
int HrOLED = 000;
int HrPercenOLED =000;
String HrZoneOLED = "000";
int LEDMaxOLED = 000;
//String BLEStatOLED = "null";
String BleStep = "SEARCHING";

bool SerialDebugOn = false;

int Hr_Z_Lvl;
int Hr_Z0 = 1; // No HR
int Hr_Z1 = HrMaxVal * 0.3; // Blue
int Hr_Z2 = HrMaxVal * 0.5; // Green
int Hr_Z3 = HrMaxVal * 0.7; // Yellow
int Hr_Z4 = HrMaxVal * 0.8; // Orange
int Hr_Z5 = HrMaxVal * 0.9;  // Red


// new HR recieved
// =====================/

void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {

  // Get HR Value and update global Int
  HrInt = pData[1];
  }

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
    BleStep = "LINK FAIL";
    BleOledStatus(BleStep);                  // Update OLED
    HrInt = 0;                               // Set HR to Zero
    strip.fill(LED_00,0,0);                  //  Set pixel's color to black (off)
    strip.show();                            //  Update strip to match
    
   
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
  

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
 
        BleOledStatus(BleStep);    // Update OLED
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
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void BleOledStatus(String BleStep){

      display.clearDisplay();
       
      display.setTextSize(2);
      display.setCursor(10, 0);
      display.print("BT STATUS");
      
      display.drawLine(0,23,128,23,1); // Second Horisontal line
      
      display.setCursor(0, 30);
      display.setTextSize(2);      
      display.print(BleStep);
      display.display(); 
  
}



//===================================
// Main setup - run once after startup
//===================================

void setup() {

  HrOLED = 000;
  HrPercenOLED =000;
  HrZoneOLED = "0";

  strip.setBrightness(30);
  strip.begin();
  strip.show();            // Turn OFF all pixels ASAP

// Address 0x3C for 128x64, you might need to change this value (use an I2C scanner)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();  

  Serial.begin(115200);
  Serial.print ("Serial Debug on: ");
  Serial.println(SerialDebugOn);
  Serial.println("Starting BLE HRM Client ...");
    BleStep = "SEARCHING";
    BleOledStatus(BleStep);    // Update OLED
  
  Serial.println("Scanning for HRM devices for 60 Seconds ...");
  BLEDevice::init("");
 
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(60, false);

} // End of setup.



// =========================================//
// This is the Arduino main loop function.
// ========================================//

void loop() {

       
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

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    
    BleStep = "CONNECTED";
     
    if (notification == false) {     // Start
        Serial.println("Turning Notification On");  
        const uint8_t onPacket[] = {0x01, 0x0};
        pRemoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)onPacket, 2, true);
        notification = true;

        BleStep = "STARTING";
    }

    // we are not connected
       
    }else if(doScan){
      BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
       BleStep = "SCANNING";
       //BleOledStatus(BleStep);    // Update OLED
      
    }

    // prepare HRM zones and values
    //====================//

    //Max HR Set potentiometer
    int MaxHRSetValue = analogRead(A0); // Range 0-4095 -> 100-220
    HrMaxVal = 100 + (MaxHRSetValue/40.95)*1.2; 


    //Set LED Count potentiometer 1 - 200
    int MaxLEDSetValue = analogRead(A1); // Range 0-4095 -> 0-200
    LED_COUNT = 10+(MaxLEDSetValue/40.95)*2; 

    LEDMaxOLED = LED_COUNT;

   // setup HR zones as a percentage of Max HR
    Hr_Z0 = 1; // No HR
    Hr_Z1 = HrMaxVal * 0.3; // Blue
    Hr_Z2 = HrMaxVal * 0.5; // Green
    Hr_Z3 = HrMaxVal * 0.7; // Yellow
    Hr_Z4 = HrMaxVal * 0.8; // Orange
    Hr_Z5 = HrMaxVal * 0.9;  // Red
            
    HrPercentage = (HrInt/HrMaxVal)*100;
   
    LEDPos = (LED_COUNT*HrPercentage)/100;

  // Determine HR Zone Banding
    LEDBandZ1 = LED_COUNT*0.3;
    LEDBandZ2 = LED_COUNT*0.5;
    LEDBandZ3 = LED_COUNT*0.7;
    LEDBandZ4 = LED_COUNT*0.8;
    LEDBandZ5 = LED_COUNT*0.9;

    if (LEDPos <= Hr_Z0){                   //  Off
        strip.clear();
        strip.fill(LED_00,0,LEDPos);        //  Set pixel's color to off
        strip.show();                       //  Update strip to match
        Hr_Z_Lvl = 0;
    }

    else if ((HrInt >= Hr_Z0) && (HrInt <= Hr_Z1) ){
       strip.clear();
       strip.fill(LED_Blue,0,LEDPos);                                   //  Set pixel's color Blue
       strip.show();                                                    //  Update strip to match
       Hr_Z_Lvl = 1;
    }
    else if ((HrInt >= Hr_Z1) && (HrInt <= Hr_Z2) ){
      strip.clear();
      strip.fill(LED_Blue,0,LEDBandZ1);
      strip.fill(LED_Green,LEDBandZ1,(LEDPos + 1) - LEDBandZ1);        //  Set pixel's color BLUE
      strip.show();                                                    //  Update strip to match
      Hr_Z_Lvl = 2;
    }
    else if ((HrInt >= Hr_Z2) && (HrInt <= Hr_Z3) ){
      strip.clear();
      strip.fill(LED_Blue,0,LEDBandZ1);                                 // Set Blue Band
      strip.fill(LED_Green,LEDBandZ1,(LEDBandZ2+1)-LEDBandZ1);          // Set Green Band
      strip.fill(LED_Yellow,LEDBandZ2,(LEDPos + 1) - LEDBandZ2);        // Set pixel's color (in RAM)
      strip.show();                                                     // Update strip to match
      Hr_Z_Lvl = 3;
    }
    else if ((HrInt >= Hr_Z3) && (HrInt <= Hr_Z4) ){
      strip.clear();
      strip.fill(LED_Blue,0,LEDBandZ1);                                  // Set Blue Band
      strip.fill(LED_Green,LEDBandZ1,(LEDBandZ2+1)-LEDBandZ1);           // Set Green Band
      strip.fill(LED_Yellow,LEDBandZ2,(LEDBandZ3+1)-LEDBandZ2);          // Set Yellow Band
      strip.fill(LED_Orange,LEDBandZ3,(LEDPos + 1) - LEDBandZ3);         // Set pixel's color (in RAM)
      strip.show();                                                      // Update strip to match
      Hr_Z_Lvl = 4;
    }
    else if (HrInt >= Hr_Z4 ){
      strip.clear();
      strip.fill(LED_Blue,0,LEDBandZ1);                                  // Set Blue Band
      strip.fill(LED_Green,LEDBandZ1,(LEDBandZ2+1)-LEDBandZ1);           // Set Green Band
      strip.fill(LED_Yellow,LEDBandZ2,(LEDBandZ3+1)-LEDBandZ2);          // Set Yellow Band
      strip.fill(LED_Orange,LEDBandZ3,(LEDBandZ4+1)-LEDBandZ3);          // Set Orange Band
      strip.fill(LED_Red,LEDBandZ4,(LEDPos + 1) - LEDBandZ4);  
      strip.show();                                                      //  Update strip to match
      Hr_Z_Lvl = 5;
    }

      //update Serial
      //=============//

    if (SerialDebugOn = true){
      Serial.print("Heart Rate: ");
      Serial.print(HrInt);
      Serial.print("bpm");
      Serial.print(" | Max:");
      Serial.print(HrMaxVal); 
      Serial.print(" | Percent of Max :");
      Serial.print(HrPercentage);
      Serial.print("% | LED Pos:");
      Serial.print(LEDPos); 
      Serial.print(" | HR Lvl:"); 
      Serial.print(Hr_Z_Lvl); 
      Serial.print(" | Z1:"); 
      Serial.print(Hr_Z1); 
      Serial.print(" | Z2:"); 
      Serial.print(Hr_Z2); 
      Serial.print(" | Z3:"); 
      Serial.print(Hr_Z3); 
      Serial.print(" | Z4:"); 
      Serial.print(Hr_Z4); 
      Serial.print(" | Z5:"); 
      Serial.print(Hr_Z5); 
      Serial.println();
    
    }
       
     
      //Update OLED
      //=============//
      
      HrOLED = HrInt;
      HrZoneOLED = Hr_Z_Lvl;
      HrPercenOLED = HrPercentage;
  
      display.clearDisplay();
    
      //First Row
      // display HRM
      
      display.setTextSize(1);
      display.setCursor(0,0);
      display.print("HRM:");
      display.setTextSize(2);
      display.print(HrOLED);
       
      // display Zone
      display.setTextSize(1);
      display.setCursor(67,0);
      display.print("Zone:");
      display.setTextSize(2);
      display.print("Z");
      display.print(HrZoneOLED);

      display.drawLine(0,18,128,18,1); // first Horisonla line 

      //second row
      // display HR Percent
      display.setTextSize(1);
      display.setCursor(0, 22);
      display.print("HR%:");
      display.setTextSize(2);
      display.print(HrPercenOLED);

      // display Max HR
      display.setTextSize(1);
      display.setCursor(67, 22);
      display.print("MAX:");
      display.setTextSize(2);      
      display.print(HrMaxVal);
   
      display.drawLine(0,40,128,40,1); // Second Horisontal line

      //second row
      // LED Count
      display.setTextSize(1);
      display.setCursor(0, 45);
      display.print("LED:");
       display.setTextSize(2);
      display.print(LEDMaxOLED);

      display.drawLine(62,54,128,54,1); //3rd Partial horisontal Line
  
      // display Max HR
      display.setTextSize(1);
      display.setCursor(67, 45);
      display.print("BT STATUS");
      display.setCursor(70, 57);
       display.setTextSize(1);      
      display.print(BleStep);

      display.drawLine(62,0,62,64,1);  // Center split line
           
      display.display(); 
       
      delay(100); // Delay a second between loops.
      
      
      strip.show();
      
      
} // End of loop

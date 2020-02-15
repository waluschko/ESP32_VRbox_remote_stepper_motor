/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include <Arduino.h>
#include "BLEDevice.h"
//#include "BLEScan.h"
//#include "HIDKeys.h"

const int step = 21;
const int dir = 19;
//up joystick [0:0:x:0] pData[2]
uint8_t up1=0xf4, up2=0xe8, up3=0xdc;
// down joysrick [0:x:0:0] pData[2]
uint8_t down1=0xc, down2=0x18, down3=0x24;
// right joystick [0:0:x:0] pData[1]
uint8_t right1=0xc, right2=0x18, right3=0x24;
//left joystick [0:x:0:0] pData[1]
uint8_t left1=0xf4, left2=0xe8, left3=0xdc;



// The remote service we wish to connect to.
static BLEUUID serviceUUID((uint16_t) 0x1812);
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID((uint16_t) 0x2a4d);

static bool doConnect = false;
static bool connected = false;
static bool doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify)
{
//#ifdef DEBUG
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    for (size_t i = 0; i < length; i++)
    {
      Serial.printf("%2x", pData[i]);
    }
    Serial.println("");
    if (pData[2] == up3)
    {
      digitalWrite(dir, LOW);
      delayMicroseconds(1000);
      digitalWrite (step, HIGH);
      delayMicroseconds(400);
      digitalWrite(step, LOW);
      delayMicroseconds(400);
    }
    else if (pData[2] == down3)
    {
      digitalWrite(dir, HIGH);
      delayMicroseconds(1000);
      digitalWrite (step, HIGH);
      delayMicroseconds(400);
      digitalWrite(step, LOW);
      delayMicroseconds(400);
    }
    else
    {
      digitalWrite(step,LOW);
    }
/*#endif
    if(pData[0] == 0x0 && pData[2] != 0x0) {
      Serial.printf("%c", pData[2]);
    } else if(pData[0] == 0x02 && pData[2] != 0x0) {
      Serial.printf("%c", pData[2]);      
    }*/
}
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
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
    if(!pClient->connect(myDevice)) {  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
      return false;
    }
    Serial.println(" - Connected to server");

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
    do {
      pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
      if (pRemoteCharacteristic == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        pClient->disconnect();
        return false;
      }

      if(pRemoteCharacteristic->canNotify())
        break;
    } while(1);
    Serial.println(" - Found our characteristic");

    pRemoteCharacteristic->registerForNotify(notifyCallback);

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


void setup() {
  Serial.begin(460800);
  pinMode (step, OUTPUT);
  pinMode (dir, OUTPUT);
  //digitalWrite (step,0);
  //digitalWrite (dir,0);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.


// This is the Arduino main loop function.
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
    // do nothing, all data is handled in notifications callback
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }
  
  delay(1000); // Delay a second between loops.

} // End of loop
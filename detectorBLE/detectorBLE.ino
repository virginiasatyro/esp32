/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini


   Advertised Device: Name: Mi Band 3, Address: e1:87:ba:38:95:0d, manufacturer data: 5701004a612a17e6744ce1d05aa5a5b4eb4bb801e187ba38950d, serviceUUID: fee0-0000-1000-8000-00805f9b34fb 
   Advertised Device: Name: Amazfit Bip Watch, Address: e2:02:3b:58:bb:93, manufacturer data: 57010072beac9354894f46384d51e61ae22f7002e2023b58bb93, serviceUUID: fee0-0000-1000-8000-00805f9b34fb 
   Devices found: 2
   Scan done!

*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// --- Bluetooth ---
int scanTime = 5; //In seconds
BLEScan* pBLEScan;
String dispositivosAutorizados = "e1:87:ba:38:95:0d";
int nivelRSSI = -75;

// --- Setup ---
void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
 
}

// -- Funções auxiliares ---
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      String dispositivosEncontrados = advertisedDevice.getAddress().toString().c_str();
      if (dispositivosEncontrados == dispositivosAutorizados && advertisedDevice.getRSSI() > nivelRSSI){
        Serial.println("Dispositivo encontrado!");
        Serial.print("RSSI: ");
        Serial.println(advertisedDevice.getRSSI()); // nível de sinal - tendendo a zero -> mais próximo do dispositivo (nível mais alto)
      }
      //else{
      //  Serial.println("Dispositivo não encontrado!");
      //}
      // Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      // Serial.printf("Advertised Device: %s \n", advertisedDevice.getAddress().toString().c_str());
    }
};

// --- Scan Bluetooth BLE ---
void scanBLE(){
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  // Serial.print("Devices found: ");
  // Serial.println(foundDevices.getCount());
  // Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
}

void loop() {
  // put your main code here, to run repeatedly:
  scanBLE();
  delay(2000);
}

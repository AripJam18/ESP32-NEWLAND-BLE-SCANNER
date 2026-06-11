#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// Waktu pemindaian dalam detik
int scanTime = 5; 
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      // Tampilkan hasil scan ke Serial Monitor
      Serial.print("Perangkat Ditemukan: ");
      Serial.print(advertisedDevice.toString().c_str());
      Serial.print(" | RSSI: ");
      Serial.println(advertisedDevice.getRSSI());
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Memulai ESP32 BLE Scan...");

  // Inisialisasi perangkat BLE
  BLEDevice::init("");
  
  pBLEScan = BLEDevice::getScan(); // Buat objek scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // Scan aktif menggunakan lebih banyak daya tapi lebih cepat
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // Harus kurang dari atau sama dengan interval
}

void loop() {
  Serial.println("Memulai pemindaian...");
  
  // Mulai scan dan blokir program selama 'scanTime' detik
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  
  Serial.print("Jumlah perangkat ditemukan: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Pemindaian selesai!\n");
  
  pBLEScan->clearResults(); // Hapus hasil dari memori agar scan selanjutnya lancar
  delay(2000); // Jeda 2 detik sebelum scan ulang
}

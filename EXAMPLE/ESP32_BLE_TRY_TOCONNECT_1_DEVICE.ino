#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// Nama perangkat target Anda
const String targetDeviceName = "HR3000FH00077";

// Variabel status
BLEAdvertisedDevice* myDevice;
bool doConnect = false;
bool connected = false;
BLEScan* pBLEScan;

// Callback saat pemindaian menemukan perangkat
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("Memeriksa perangkat: ");
      Serial.println(advertisedDevice.getName().c_str());

      // Jika nama perangkat cocok dengan target
      if (advertisedDevice.getName() == targetDeviceName.c_str()) {
        Serial.println(">> Target ditemukan! Menghentikan pemindaian...");
        
        // Hentikan scan
        BLEDevice::getScan()->stop();
        
        // Simpan referensi perangkat dan ubah bendera koneksi
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
      }
    }
};

// Fungsi untuk melakukan koneksi ke perangkat target
bool connectToServer() {
    Serial.print("Mencoba menghubungkan ke alamat: ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient* pClient = BLEDevice::createClient();
    Serial.println(" - Klien BLE berhasil dibuat");

    // Hubungkan ke perangkat target
    if (!pClient->connect(myDevice)) {
        Serial.println(" - Gagal terhubung ke target.");
        return false;
    }
    Serial.println(" - BERHASIL TERHUBUNG!");
    
    // Opsional: Di sini Anda bisa mengambil Service dan Characteristic jika ingin membaca/mengirim data
    return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Memulai ESP32 BLE Client...");

  BLEDevice::init("");
  
  // Konfigurasi pemindaian
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
}

void loop() {
  // Jika perangkat ditemukan dan siap dihubungkan
  if (doConnect == true) {
    if (connectToServer()) {
      connected = true;
    } else {
      Serial.println("Gagal terhubung, akan mencoba memindai ulang...");
    }
    doConnect = false;
  }

  // Jika tidak terhubung dan tidak sedang mencoba menghubungkan, lakukan scan ulang
  if (!connected && !doConnect) {
    Serial.println("Mencari perangkat target...");
    pBLEScan->start(5, false); // Scan selama 5 detik
    delay(1000);
  }
  
  delay(1000); // Jeda loop utama
}

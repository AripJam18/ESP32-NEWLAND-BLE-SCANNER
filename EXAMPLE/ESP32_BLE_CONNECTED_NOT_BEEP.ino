#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// Nama target perangkat Anda
const String targetDeviceName = "HR3000FH00077";

// UUID Standar BLE HID
static BLEUUID hidServiceUUID("00001812-0000-1000-8000-00805f9b34fb");
static BLEUUID reportCharUUID("00002a4d-0000-1000-8000-00805f9b34fb");

BLEAdvertisedDevice* myDevice;
bool doConnect = false;
bool connected = false;
BLEScan* pBLEScan;
BLERemoteCharacteristic* pReportCharacteristic;

// PERBAIKAN: Array konversi HID Keycode ke karakter teks standar (US Keyboard ASCII)
const char keymap[] = {
  0,  0,  0,  0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
  'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\n', 27,  8,  9, ' ',
  '-', '=', '[', ']', 0, 0, ';', '\'', '`', ',', '.', '/', 0,  0,  0
};

// Fungsi Callback ketika barcode mendeteksi scan data (Notifikasi BLE)
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    
    // Protokol HID Keyboard mengirim data sepanjang 8 byte
    // Byte ke-0/2 adalah modifier (Shift/Ctrl), Byte ke-3 sampai 8 adalah keycode tombol
    if (length >= 3) {
      // PERBAIKAN: Mengambil keycode dari array pData (biasanya pada indeks ke-2 atau ke-3)
      uint8_t keycode = pData[2]; 
      
      if (keycode >= 4 && keycode <= 56) {
        char c = keymap[keycode];
        
        // Cek jika tombol Shift ditekan (modifier byte ke-0 bernilai 0x02 atau 0x20)
        if ((pData[0] == 0x02 || pData[0] == 0x20) && c >= 'a' && c <= 'z') {
          c -= 32; // Ubah menjadi huruf kapital
        }
        
        // Cetak karakter hasil scan ke Serial Monitor
        if (c == '\n') {
          Serial.println(); // Barcode selesai, buat baris baru
        } else {
          Serial.print(c);
        }
      }
    }
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.getName() == targetDeviceName.c_str()) {
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
      }
    }
};

// Fungsi melakukan koneksi dan mendaftarkan penangkap data
bool connectToServer() {
    Serial.print("Menghubungkan ke ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient* pClient = BLEDevice::createClient();
    if (!pClient->connect(myDevice)) return false;
    Serial.println(" - Terhubung ke perangkat!");

    // Mengambil layanan HID dari scanner
    BLERemoteService* pRemoteService = pClient->getService(hidServiceUUID);
    if (pRemoteService == nullptr) {
        Serial.println(" - Gagal menemukan HID Service.");
        pClient->disconnect();
        return false;
    }

    // Mengambil Karakteristik Report
    pReportCharacteristic = pRemoteService->getCharacteristic(reportCharUUID);
    if (pReportCharacteristic == nullptr) {
        Serial.println(" - Gagal menemukan Report Characteristic.");
        pClient->disconnect();
        return false;
    }

    // Aktifkan listener agar setiap scan data masuk
    if(pReportCharacteristic->canNotify()) {
        pReportCharacteristic->registerForNotify(notifyCallback);
        Serial.println(" - Siap menerima data hasil scan! Silahkan coba scan barcode.");
    }

    return true;
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
}

void loop() {
  if (doConnect == true) {
    if (connectToServer()) {
      connected = true;
    } else {
      Serial.println("Gagal terhubung, memindai ulang...");
    }
    doConnect = false;
  }

  if (!connected && !doConnect) {
    Serial.println("Mencari barcode scanner...");
    pBLEScan->start(5, false);
    delay(1000);
  }
  delay(10);
}

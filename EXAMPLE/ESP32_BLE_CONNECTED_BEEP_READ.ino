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

// Callback untuk menangani proses Security / Pairing
class MySecurityCallbacks : public BLESecurityCallbacks {
  uint32_t onPassKeyRequest() {
    Serial.println("PassKey Request");
    return 0;
  }
  void onPassKeyNotify(uint32_t pass_key) {
    Serial.print("PassKey Notify: ");
    Serial.println(pass_key);
  }
  bool onSecurityRequest() {
    Serial.println("Security Request");
    return true;
  }
  bool onConfirmPIN(uint32_t pin) {
    Serial.print("Confirm PIN: ");
    Serial.println(pin);
    return true;
  }
  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
    if (cmpl.success) {
      Serial.println(">> SECURITY/PAIRING BERHASIL! (Scanner seharusnya berbunyi Beep)");
    } else {
      Serial.print(">> Security/Pairing Gagal. Alasan: ");
      Serial.println(cmpl.fail_reason);
    }
  }
};

// Fungsi Callback ketika barcode mendeteksi scan data
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    
    // Protokol HID Keyboard mengirim data dalam bentuk byte array
    if (length >= 3) {
      // Tombol biasanya berada di indeks ke-2 atau ke-3 tergantung tipe HID Report-nya
      // Kita periksa byte ke-2 terlebih dahulu
      uint8_t keycode = pData[2]; 
      
      if (keycode >= 4 && keycode <= 56) {
        char c = keymap[keycode];
        
        // Cek jika tombol Shift ditekan (modifier byte ke-0)
        if ((pData[0] == 0x02 || pData[0] == 0x20) && c >= 'a' && c <= 'z') {
          c -= 32; 
        }
        
        if (c == '\n') {
          Serial.println(); 
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

// Fungsi melakukan koneksi
bool connectToServer() {
    Serial.print("Menghubungkan ke ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient* pClient = BLEDevice::createClient();
    if (!pClient->connect(myDevice)) return false;
    Serial.println(" - Terhubung secara BLE dasar.");

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

    // Aktifkan listener data
    if(pReportCharacteristic->canNotify()) {
        pReportCharacteristic->registerForNotify(notifyCallback);
        Serial.println(" - Karakteristik Notifikasi terdaftar. Menunggu proses pairing...");
    }

    return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Memulai ESP32 BLE HID Client...");
  
  BLEDevice::init("ESP32-HID-Host"); // Berikan nama host agar scanner mengenalnya

  // KONFIGURASI KEAMANAN (PENTING UNTUK HID DEVICE)
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_MITM);
  BLEDevice::setSecurityCallbacks(new MySecurityCallbacks());
  
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND); // Lakukan bonding agar tersimpan
  pSecurity->setCapability(ESP_IO_CAP_NONE);         // ESP32 tidak punya layar/tombol input PIN
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

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






#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Konfigurasi Dimensi Layar OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Nama target perangkat Anda
const String targetDeviceName = "HR3000FH00077";

// Variabel Buffer untuk menampung String teks Barcode
String barcodeBuffer = "";
String finalBarcodeData = "Menunggu Scan...";

// UUID Standar BLE HID
static BLEUUID hidServiceUUID("00001812-0000-1000-8000-00805f9b34fb");
static BLEUUID reportCharUUID("00002a4d-0000-1000-8000-00805f9b34fb");

BLEAdvertisedDevice* myDevice;
bool doConnect = false;
bool connected = false;
BLEScan* pBLEScan;
BLERemoteCharacteristic* pReportCharacteristic;

const char keymap[] = {
  0,  0,  0,  0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
  'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\n', 27,  8,  9, ' ',
  '-', '=', '[', ']', 0, 0, ';', '\'', '`', ',', '.', '/', 0,  0,  0
};

// Fungsi untuk memperbarui tampilan Layar OLED
void updateOLED(String statusAtas, String dataBawah) {
  display.clearDisplay();
  
  // --- BAGIAN ATAS (WARNA KUNING) ---
  display.setTextColor(SSD1306_WHITE); // Pada OLED 2 warna, baris atas otomatis jadi Kuning
  display.setTextSize(1);
  display.setCursor(0, 2);
  display.println(statusAtas);
  
  // Garis pembatas horizontal antara area kuning dan biru
  display.drawFastHLine(0, 18, 128, SSD1306_WHITE);
  
  // --- BAGIAN BAWAH (WARNA BIRU) ---
  display.setCursor(0, 24);
  display.setTextSize(1);
  display.println("HASIL SCAN:");
  
  // Menampilkan data barcode dengan ukuran teks sedikit lebih besar agar mudah dibaca
  display.setCursor(0, 36);
  display.setTextSize(1); 
  display.println(dataBawah);
  
  display.display();
}

class MySecurityCallbacks : public BLESecurityCallbacks {
  uint32_t onPassKeyRequest() { return 0; }
  void onPassKeyNotify(uint32_t pass_key) { }
  bool onSecurityRequest() { return true; }
  bool onConfirmPIN(uint32_t pin) { return true; }
  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
    if (cmpl.success) {
      Serial.println(">> SECURITY/PAIRING BERHASIL!");
      updateOLED(targetDeviceName, "READY TO SCAN");
    } else {
      Serial.print(">> Security/Pairing Gagal. Alasan: ");
      Serial.println(cmpl.fail_reason);
      updateOLED("PAIRING FAILED", "Coba Reset...");
    }
  }
};

// Callback ketika data tombol barcode masuk
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    
    if (length >= 3) {
      uint8_t keycode = pData[2]; 
      
      if (keycode >= 4 && keycode <= 56) {
        char c = keymap[keycode];
        
        if ((pData[0] == 0x02 || pData[0] == 0x20) && c >= 'a' && c <= 'z') {
          c -= 32; // Kapitalisasi huruf jika tombol Shift aktif
        }
        
        // JIKA MENERIMA KARAKTER ENTER (\n), BARCODE SELESAI
        if (c == '\n') {
          finalBarcodeData = barcodeBuffer; // Simpan data utuh
          Serial.print("Barcode Terbaca: ");
          Serial.println(finalBarcodeData);
          
          // Kirim teks barcode utuh ke area biru OLED
          updateOLED("DEVICE:" + targetDeviceName, finalBarcodeData);
          
          barcodeBuffer = ""; // Reset buffer untuk scan berikutnya
        } 
        // JIKA BUKAN ENTER, MASUKKAN KE BUFFER STRING
        else {
          barcodeBuffer += c; 
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

bool connectToServer() {
    updateOLED("CONNECTING...", "Menghubungkan...");
    Serial.print("Menghubungkan ke ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient* pClient = BLEDevice::createClient();
    if (!pClient->connect(myDevice)) return false;

    BLERemoteService* pRemoteService = pClient->getService(hidServiceUUID);
    if (pRemoteService == nullptr) {
        pClient->disconnect();
        return false;
    }

    pReportCharacteristic = pRemoteService->getCharacteristic(reportCharUUID);
    if (pReportCharacteristic == nullptr) {
        pClient->disconnect();
        return false;
    }

    if(pReportCharacteristic->canNotify()) {
        pReportCharacteristic->registerForNotify(notifyCallback);
        updateOLED("PAIRING...", "Menunggu Beep...");
    }

    return true;
}

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi Layar OLED via alamat I2C standar (biasanya 0x3C)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Gagal mendeteksi OLED SSD1306"));
    for(;;); // Kunci program jika OLED tidak terpasang
  }
  
  display.clearDisplay();
  updateOLED("BOOTING...", "Memulai BLE...");

  BLEDevice::init("ESP32-HID-Host");

  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_MITM);
  BLEDevice::setSecurityCallbacks(new MySecurityCallbacks());
  
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
  pSecurity->setCapability(ESP_IO_CAP_NONE);
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
      updateOLED("CONN FAILED", "Mencoba lagi...");
    }
    doConnect = false;
  }

  if (!connected && !doConnect) {
    updateOLED("SCANNING...", "Mencari Barcode");
    pBLEScan->start(5, false);
    delay(1000);
  }
  delay(10);
}

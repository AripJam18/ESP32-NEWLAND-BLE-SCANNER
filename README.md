# NEWLAND NLS-HR3000-BT Bluetooth HID dengan ESP32

## Overview

Dokumen ini menjelaskan cara menghubungkan barcode scanner **NEWLAND NLS-HR3000-BT** ke **ESP32** menggunakan **Bluetooth HID (Human Interface Device)**.

Pada konfigurasi ini ESP32 bertindak sebagai **BLE HID Host**, sedangkan scanner bertindak sebagai **BLE HID Keyboard** yang mengirimkan hasil scan dalam bentuk HID Keycode.

Metode ini telah diuji menggunakan:

* ESP32 WROOM32D
* Arduino IDE
* Library ESP32 BLE Arduino
* Scanner NEWLAND NLS-HR3000-BT

---

# Arsitektur Sistem

```text
+--------------------+
| NEWLAND HR3000-BT  |
| BLE HID Keyboard   |
+---------+----------+
          |
          | Bluetooth LE
          |
+---------v----------+
| ESP32              |
| BLE HID Host       |
+---------+----------+
          |
          |
          v
     Serial Monitor
```

---

# Persyaratan

## Hardware

* NEWLAND NLS-HR3000-BT
* ESP32 WROOM32D / ESP32 DevKit
* Kabel USB

## Software

* Arduino IDE
* ESP32 Board Package
* Library BLE bawaan ESP32

---

# Konfigurasi Scanner

## 1. Masuk Setup Mode

Scan barcode:

```text
#SETUP1
```

Fungsi:

```text
Enter Setup
```

---

## 2. Aktifkan Bluetooth HID

Scan barcode:

```text
@INTERF10
```

Fungsi:

```text
Bluetooth HID Mode
```

Pada mode ini scanner akan dikenali sebagai keyboard Bluetooth HID.

---

## 3. Keluar Setup Mode

Scan barcode:

```text
#SETUP0
```

Fungsi:

```text
Exit Setup
```

---

# Menghapus Pairing Lama

Jika scanner pernah dipasangkan ke perangkat lain:

* Android
* Laptop
* PC
* Tablet

Lakukan:

## Pada Scanner

Scan barcode:

```text
Clear Pairing Info on Scanner
```

## Pada Host Lama

Lakukan:

```text
Forget Device
```

atau

```text
Remove Device
```

---

# BLE Service yang Digunakan

## HID Service

UUID:

```text
00001812-0000-1000-8000-00805f9b34fb
```

Nama:

```text
Human Interface Device
```

---

## HID Report Characteristic

UUID:

```text
00002a4d-0000-1000-8000-00805f9b34fb
```

Nama:

```text
Report
```

Karakteristik ini digunakan scanner untuk mengirim data barcode.

---

# Cara Kerja Sistem

## Tahap 1

ESP32 melakukan scanning BLE.

```text
ESP32
 ↓
 Scan BLE Device
```

---

## Tahap 2

ESP32 menemukan scanner.

```text
HR3000FH00077
```

---

## Tahap 3

ESP32 melakukan koneksi BLE.

```text
ESP32
 ↓
 Connect
 ↓
 Pairing
 ↓
 Bonding
```

---

## Tahap 4

ESP32 subscribe ke HID Report Characteristic.

```text
0x2A4D
```

---

## Tahap 5

Scanner mengirim HID Keycode.

Contoh:

```text
0x1E
0x1F
0x20
```

---

## Tahap 6

ESP32 mengubah HID Keycode menjadi karakter ASCII.

Contoh:

```text
0x1E -> '1'
0x1F -> '2'
0x20 -> '3'
```

---

# Konfigurasi Security BLE

## Mengapa Security Diperlukan?

Perangkat HID Bluetooth biasanya memerlukan:

* Pairing
* Bonding
* Encryption

Tanpa konfigurasi security, koneksi akan gagal.

---

## Encryption

```cpp
BLEDevice::setEncryptionLevel(
    ESP_BLE_SEC_ENCRYPT_MITM);
```

---

## Bonding

```cpp
pSecurity->setAuthenticationMode(
    ESP_LE_AUTH_BOND);
```

---

## IO Capability

```cpp
pSecurity->setCapability(
    ESP_IO_CAP_NONE);
```

Digunakan karena ESP32 tidak memiliki keyboard atau display untuk memasukkan PIN.

---

# HID Keycode Mapping

Scanner tidak mengirim karakter ASCII secara langsung.

Scanner mengirim:

```text
HID Keycode
```

Contoh:

| Keycode | Karakter |
| ------- | -------- |
| 4       | a        |
| 5       | b        |
| 6       | c        |
| 30      | 1        |
| 31      | 2        |
| 32      | 3        |
| 39      | 0        |

Karena itu diperlukan tabel konversi:

```cpp
const char keymap[];
```

---

# Menangani Huruf Kapital

Huruf kapital dikirim menggunakan modifier byte.

Contoh:

```text
Modifier = SHIFT
Keycode = 4
```

Hasil:

```text
A
```

bukan:

```text
a
```

---

# Menangani Tombol Enter

Scanner biasanya mengirim ENTER setelah barcode selesai dibaca.

Keycode:

```text
40
```

Konversi:

```cpp
'\n'
```

Hasil:

```text
6920075771879

20251110YN-CP12
```

---

# Hal-Hal yang Perlu Diperhatikan

## 1. Nama Scanner Bisa Berubah

Contoh:

```text
HR3000FH00077
```

Disarankan menggunakan:

```text
MAC Address
```

agar lebih stabil.

---

## 2. Pairing Hanya ke Satu Host

Scanner dapat menyimpan informasi bonding.

Jika ingin berpindah host:

```text
Android
 ↓
 ESP32
```

atau

```text
ESP32
 ↓
 Laptop
```

maka:

1. Forget Device pada host lama.
2. Clear Pairing Info pada scanner.
3. Pair ulang.

---

## 3. Bonding Tersimpan

Setelah pairing berhasil:

```text
ESP32 <-> Scanner
```

bonding biasanya disimpan sehingga reconnect berikutnya lebih cepat.

---

# Troubleshooting

## Scanner Tidak Ditemukan

Periksa:

* Bluetooth HID sudah aktif
* Scanner menyala
* Jarak dekat dengan ESP32

---

## Koneksi Gagal

Periksa:

* Pairing lama sudah dihapus
* Scanner sudah di-clear pairing
* Security BLE sudah diaktifkan

---

## Tidak Ada Data Barcode

Periksa:

* UUID HID Service benar
* UUID Report Characteristic benar
* Notification sudah aktif

---

# Hasil Pengujian

## Barcode Numeric

Input:

```text
6920075771879
```

Output:

```text
6920075771879
```

---

## Barcode Alphanumeric

Input:

```text
20251110YN-CP12
```

Output:

```text
20251110YN-CP12
```

---

# Kesimpulan

NEWLAND NLS-HR3000-BT dapat dihubungkan langsung ke ESP32 menggunakan Bluetooth HID tanpa dongle tambahan.

Implementasi yang berhasil menggunakan:

* Bluetooth HID Mode
* BLE HID Service (0x1812)
* HID Report Characteristic (0x2A4D)
* BLE Pairing
* BLE Bonding
* HID Keycode to ASCII Conversion

Dengan konfigurasi ini ESP32 dapat menerima data barcode secara wireless dan real-time untuk aplikasi:

* Inventory
* Warehouse Management
* POS System
* Attendance System
* IoT Gateway
* Industrial Automation
* Smart Scale / Weighing System

# USER GUIDE

# Integrasi Barcode Scanner NEWLAND NLS-HR3000-BT dengan ESP32 melalui Bluetooth HID

---

**Document Number** : UG-HR3000BT-ESP32-001
**Revision** : 1.0
**Date** : June 2026
**Prepared By** : Engineering Department
**Project** : Wireless Barcode Acquisition using ESP32 BLE HID Host

---

# Document Revision History

| Revision | Date      | Description     | Author      |
| -------- | --------- | --------------- | ----------- |
| 1.0      | June 2026 | Initial Release | Engineering |

---

# Table of Contents

1. Introduction
2. System Overview
3. Hardware Requirements
4. Software Requirements
5. Bluetooth Operating Modes
6. Scanner Configuration
7. ESP32 Configuration
8. BLE HID Architecture
9. Pairing and Bonding Process
10. HID Report Processing
11. Example Application
12. Troubleshooting
13. Best Practices
14. Technical Specifications
15. Appendix

---

# 1. Introduction

## 1.1 Purpose

This document describes the procedure for integrating the NEWLAND NLS-HR3000-BT wireless barcode scanner with an ESP32 microcontroller using Bluetooth Low Energy (BLE) Human Interface Device (HID) communication.

The objective is to allow barcode data acquisition without additional USB dongles, serial adapters, or wired communication.

---

## 1.2 Scope

This guide covers:

* Scanner configuration
* Bluetooth HID operation
* ESP32 BLE implementation
* Pairing and bonding
* HID keycode decoding
* Barcode data reception
* Troubleshooting

---

# 2. System Overview

## 2.1 System Architecture

```text
+------------------------+
| NEWLAND HR3000-BT      |
| BLE HID Keyboard       |
+-----------+------------+
            |
            | BLE
            |
+-----------v------------+
| ESP32 WROOM32D         |
| BLE HID Host           |
+-----------+------------+
            |
            |
            v
      Application Layer
```

---

## 2.2 Data Flow

```text
Barcode
   ↓
Scanner Decode
   ↓
BLE HID Report
   ↓
ESP32 Receive
   ↓
ASCII Conversion
   ↓
Application
```

---

# 3. Hardware Requirements

## 3.1 Barcode Scanner

Supported Device:

* NEWLAND NLS-HR3000-BT

---

## 3.2 Controller

Supported Controllers:

* ESP32 WROOM32D
* ESP32 DevKit V1
* ESP32-WROVER
* ESP32-S3 (with BLE support)

---

## 3.3 Development Environment

Recommended:

* Arduino IDE 2.x
* ESP32 Board Package
* USB Data Cable

---

# 4. Software Requirements

## 4.1 ESP32 Board Package

Recommended Version:

```text
2.0.17
```

---

## 4.2 BLE Library

Library used:

```cpp
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
```

ESP32 BLE Arduino Library.

---

# 5. Bluetooth Operating Modes

The scanner supports multiple Bluetooth operating modes.

## 5.1 Bluetooth HID Mode

In this mode the scanner behaves as:

```text
Bluetooth Keyboard
```

Characteristics:

* BLE HID Device
* Requires Pairing
* Requires Bonding
* Sends HID Reports

This mode is required for ESP32 implementation described in this guide.

---

## 5.2 Bluetooth BLE Mode

Provides:

```text
Nordic UART Service
```

Characteristics:

* GATT Service
* Transparent Data Transfer
* Application Specific

This mode was tested but HID mode provided the most reliable operation with ESP32.

---

# 6. Scanner Configuration

## Step 1 – Enter Setup Mode

Scan:

```text
#SETUP1
```

Purpose:

```text
Enter Setup Configuration
```

---

## Step 2 – Enable Bluetooth HID

Scan:

```text
@INTERF10
```

Purpose:

```text
Bluetooth HID Mode
```

---

## Step 3 – Exit Setup Mode

Scan:

```text
#SETUP0
```

Purpose:

```text
Save Configuration and Exit
```

---

# 7. Clearing Existing Pairing Information

Before connecting to a new host:

1. Remove scanner from previous host.
2. Scan "Clear Pairing Info on Scanner".

Failure to do so may prevent new connections.

---

# 8. BLE HID Architecture

## 8.1 HID Service

UUID:

```text
00001812-0000-1000-8000-00805f9b34fb
```

Name:

```text
Human Interface Device
```

---

## 8.2 HID Report Characteristic

UUID:

```text
00002A4D-0000-1000-8000-00805F9B34FB
```

Used for barcode transmission.

---

# 9. Pairing and Bonding Process

## 9.1 Why Pairing is Required

Bluetooth HID devices require:

* Authentication
* Encryption
* Bonding

Without these security procedures the connection will be rejected.

---

## 9.2 Connection Sequence

```text
Scan Device
      ↓
Connect
      ↓
Security Request
      ↓
Pairing
      ↓
Bonding
      ↓
Subscribe Notification
      ↓
Receive Barcode
```

---

## 9.3 Successful Pairing

Expected Serial Output:

```text
SECURITY/PAIRING BERHASIL!
```

Scanner typically emits a confirmation beep.

---

# 10. HID Report Processing

## 10.1 Data Format

The scanner does not send ASCII text.

Instead it sends:

```text
HID Keycodes
```

Example:

```text
0x1E
0x1F
0x20
```

---

## 10.2 HID to ASCII Conversion

Example mapping:

| HID Code | Character |
| -------- | --------- |
| 30       | 1         |
| 31       | 2         |
| 32       | 3         |
| 33       | 4         |
| 34       | 5         |
| 35       | 6         |
| 36       | 7         |
| 37       | 8         |
| 38       | 9         |
| 39       | 0         |

---

## 10.3 Shift Handling

Uppercase letters require modifier processing.

Example:

```text
Modifier + Keycode
```

Result:

```text
A-Z
```

instead of:

```text
a-z
```

---

# 11. Example Application

Typical use cases include:

* Inventory System
* Warehouse Management
* POS Terminal
* Attendance System
* Weighbridge Automation
* Production Tracking
* Asset Management

---

# 12. Testing Results

## Test Case 1

Barcode:

```text
6920075771879
```

Result:

PASS

---

## Test Case 2

Barcode:

```text
20251110YN-CP12
```

Result:

PASS

---

## Test Case 3

Multiple Sequential Scans

Result:

PASS

---

# 13. Troubleshooting

## Scanner Not Found

Check:

* Scanner powered on
* HID mode enabled
* Distance within BLE range

---

## Pairing Failure

Check:

* Previous pairing removed
* Scanner pairing memory cleared
* BLE security enabled

---

## No Barcode Data Received

Check:

* Correct HID Service UUID
* Correct Report Characteristic UUID
* Notifications enabled

---

# 14. Best Practices

1. Use MAC Address instead of device name.
2. Clear scanner pairing before changing hosts.
3. Store bonded devices when possible.
4. Keep firmware versions documented.
5. Log received barcode data for diagnostics.

---

# 15. Technical Specifications

| Item                | Value                |
| ------------------- | -------------------- |
| Communication       | Bluetooth Low Energy |
| Profile             | HID                  |
| Service UUID        | 0x1812               |
| Characteristic UUID | 0x2A4D               |
| Pairing             | Required             |
| Bonding             | Required             |
| Encryption          | Recommended          |
| Data Type           | HID Report           |
| Output              | ASCII Barcode        |

---

# Appendix A – Tested Hardware

| Device                | Status |
| --------------------- | ------ |
| ESP32 WROOM32D        | PASS   |
| NEWLAND NLS-HR3000-BT | PASS   |

---

# Appendix B – Reference Code

Reference implementation:

```text
ESP32 BLE HID Client for NEWLAND NLS-HR3000-BT
```

Use the validated source code attached with this project documentation.

---

# End of Document

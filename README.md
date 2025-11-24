📘 RFID Attendance System with ESP32, OLED SH1106, and Telegram Notifications

This project is an RFID-based attendance system built using an ESP32, MFRC522 RFID reader, SH1106 OLED display, and Telegram Bot integration.
Students can tap their RFID cards to check in, and the system automatically sends attendance details (name, UID, timestamp, and status) to Telegram.

The system also supports managing student data via Telegram, including adding new students and adjusting entry times.

🚀 Features
✔️ Real-Time Attendance Logging

Reads RFID UID, matches it with stored student data, and displays the status on the OLED.

✔️ Telegram Bot Integration

Sends messages directly to a Telegram chat when a card is tapped.
Commands include:

/add [UID] [Name] → Add new student

/list → View student list

/jam HH:MM → Set allowed entry time

✔️ SH1106 OLED Display

Shows:

UID

Time

Attendance status (On Time / Late / Unknown Card)

✔️ NTP Time Sync

Automatically syncs time using NTP (WIB, GMT+7) for accurate attendance logs.

✔️ SPIFFS Storage

Stores student data in /data_siswa.txt.

🧩 Hardware Required
Component	Description
ESP32	Main controller with WiFi
MFRC522	RFID card reader
OLED 1.3” SH1106	Display attendance info
Buzzer	Audible feedback
RFID Tags/Cards	Student identifiers
🔌 Pin Connections
RFID (MFRC522 → ESP32)
RFID Pin	ESP32 Pin
VCC	3.3V
GND	GND
SCK	GPIO 18
MOSI	GPIO 23
MISO	GPIO 19
SS	GPIO 14
RST	GPIO 27
OLED SH1106
OLED Pin	ESP32 Pin
VCC	3.3V
GND	GND
SCL	22
SDA	21
Buzzer

GPIO 4

![WhatsApp Image 2025-11-24 at 15 06 28_8ea978cc](https://github.com/user-attachments/assets/8a321af6-3cbd-4285-ab08-b7ed32f6d8dc)


📬 Telegram Setup

Create a bot with @BotFather

Copy the bot token → replace BOTtoken

Get your Chat ID → replace CHAT_ID

ESP32 sends attendance logs directly to your Telegram chat

🧠 How the System Works

ESP32 connects to WiFi & syncs time from NTP server

RFID card is tapped → UID read

UID is checked in the SPIFFS student file

System determines:

🟢 On Time

🟡 Late

🔴 Unknown Card

OLED shows the result and the buzzer provides feedback

Telegram receives a detailed attendance message

![WhatsApp Image 2025-11-24 at 14 55 00_9afc7bc2](https://github.com/user-attachments/assets/709633da-f333-481d-b592-5b8bfe181bce)



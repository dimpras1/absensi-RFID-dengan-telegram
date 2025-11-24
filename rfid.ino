#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <SPIFFS.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>
#include "time.h"

// -------------------- PIN --------------------
#define SS_PIN 14
#define RST_PIN 27
#define BUZZER 4
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
MFRC522 rfid(SS_PIN, RST_PIN);

//| **VCC**      | 3.3V 
//| **GND**      | GND
//| **SCK**      | GPIO 18
//| **MOSI**     | GPIO 23
//| **MISO**     | GPIO 19
//| **SCL**      | 22
//| **SDA**      | 21

// -------------------- WIFI & TELEGRAM --------------------
const char* ssid = "*******";
const char* password = "********";
#define BOTtoken "8245745549:AAEiDhz8dbLnztE8wmbRv2HA_BiyqCMjwJw"
#define CHAT_ID "5904921434"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan;
const int botRequestDelay = 1000;

// -------------------- NTP --------------------
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600; // WIB
const int daylightOffset_sec = 0;

// -------------------- JAM MASUK --------------------
int jamMasuk = 7;
int menitMasuk = 0;

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  // --- OLED SETUP ---
  Wire.begin(21, 22);
  if (!display.begin(0x3C, true)) {
    Serial.println("❌ OLED SH1106 tidak terdeteksi!");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 25);
  display.println("Inisialisasi...");
  display.display();

  if (!SPIFFS.begin(true)) {
    Serial.println("❌ Gagal mount SPIFFS!");
    return;
  }

  WiFi.begin(ssid, password);
  Serial.print("🔗 Menghubungkan WiFi");
  display.clearDisplay();
  display.setCursor(0, 25);
  display.println("Menghubungkan WiFi...");
  display.display();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n✅ Terhubung ke WiFi!");
  client.setInsecure();

  display.clearDisplay();
  display.setCursor(0, 25);
  display.println("WiFi Terhubung!");
  display.display();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("🕒 Sinkronisasi waktu NTP...");

  bot.sendMessage(CHAT_ID, "🤖 Bot Absensi siap digunakan!", "");
  Serial.println("=== Sistem Absensi RFID Aktif ===");

  display.clearDisplay();
  display.setCursor(5, 10);
  display.println("Sistem Absensi Aktif");
  display.setCursor(15, 20);
  display.println("Tempelkan Kartu");
  display.display();
}

// ==================== FUNGSI WAKTU ====================
String getTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Gagal ambil waktu";
  }
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}

bool isLate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return false;
  if (timeinfo.tm_hour > jamMasuk) return true;
  if (timeinfo.tm_hour == jamMasuk && timeinfo.tm_min > menitMasuk) return true;
  return false;
}

// ==================== TAMBAH DATA SISWA ====================
void addSiswa(String data) {
  File file = SPIFFS.open("/data_siswa.txt", FILE_APPEND);
  if (!file) {
    Serial.println("❌ Gagal membuka file!");
    return;
  }
  file.println(data);
  file.close();
  Serial.println("✅ Siswa ditambahkan: " + data);
}

// ==================== LIST DATA SISWA ====================
String listSiswa() {
  File file = SPIFFS.open("/data_siswa.txt", FILE_READ);
  if (!file) return "Belum ada data siswa.";
  
  String daftar = "";
  while (file.available()) {
    daftar += file.readStringUntil('\n');
  }
  file.close();
  return daftar;
}

// ==================== CEK UID DI FILE ====================
String cekUID(String uid) {
  File file = SPIFFS.open("/data_siswa.txt", FILE_READ);
  if (!file) return "";

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.startsWith(uid)) {
      file.close();
      return line; // contoh: "A1B2C3D4 Budi"
    }
  }
  file.close();
  return "";
}

// ==================== HANDLE PESAN TELEGRAM ====================
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;

    if (text == "/start" || text == "/menu") {
      bot.sendMessage(CHAT_ID,
        "📋 *Menu Absensi*\n"
        "/add [UID] [Nama] - Tambah siswa baru\n"
        "/list - Lihat daftar siswa\n"
        "/jam [HH:MM] - Atur jam masuk\n",
        "Markdown");
    }
    else if (text.startsWith("/add")) {
      int firstSpace = text.indexOf(' ');
      if (firstSpace == -1) {
        bot.sendMessage(CHAT_ID, "Format salah.\nContoh: /add A1B2C3D4 Budi", "");
      } else {
        String data = text.substring(firstSpace + 1);
        addSiswa(data);
        bot.sendMessage(CHAT_ID, "✅ Data siswa ditambahkan:\n" + data, "");
      }
    }
    else if (text == "/list") {
      bot.sendMessage(CHAT_ID, listSiswa(), "");
    }
    else if (text.startsWith("/jam")) {
      int sep = text.indexOf(':');
      if (sep == -1) {
        bot.sendMessage(CHAT_ID, "Format salah.\nContoh: /jam 07:00", "");
      } else {
        jamMasuk = text.substring(5, sep).toInt();
        menitMasuk = text.substring(sep + 1).toInt();
        bot.sendMessage(CHAT_ID, "🕖 Jam masuk diatur ke " + String(jamMasuk) + ":" + String(menitMasuk), "");
      }
    }
  }
}

// ==================== LOOP ====================
void loop() {
  // --- Cek Telegram ---
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  // --- Cek RFID ---
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  Serial.println("=========================");
  Serial.println("UID Terdeteksi: " + uid);

  String data = cekUID(uid);
  String waktu = getTime();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("UID: " + uid);
  display.setCursor(0, 25);
  display.println("Waktu: " + waktu);

  if (data != "") {
    String status = isLate() ? "Terlambat" : "Tepat Waktu";
    display.setCursor(0, 40);
    display.println(status);
    display.display();

    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);

    bot.sendMessage(CHAT_ID, data + "\n🕒 " + waktu + "\n" + status, "");
  } else {
    display.setCursor(0, 40);
    display.println("❌ Tidak dikenal!");
    display.display();

    digitalWrite(BUZZER, HIGH);
    delay(300);
    digitalWrite(BUZZER, LOW);
    bot.sendMessage(CHAT_ID, "⚠️ Kartu tidak dikenal: " + uid, "");
  }

  rfid.PICC_HaltA();
  delay(1000);
}

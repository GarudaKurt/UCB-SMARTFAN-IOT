#include "configFirebase.h"

#ifdef ESP32
#include <WiFi.h>
#include <FirebaseESP32.h>
#endif

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// ===== WIFI & FIREBASE CONFIG =====
#define WIFI_SSID       "SmartFanIoT"
#define WIFI_PASSWORD   "SmartFanIoT"

#define API_KEY         "AIzaSyCX984wDmiwBhVF98KtTrtNNeeC0C6rQDk"
#define DATABASE_URL    "https://fan-control-4b762-default-rtdb.firebaseio.com"
#define USER_EMAIL      "testuser@gmail.com"
#define USER_PASSWORD   "test123456!"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

static unsigned long lastFirebaseRead = 0;
static unsigned long prevTime = 0;
const unsigned long FIREBASE_INTERVAL = 2000;

CONFIGFIREBASE::CONFIGFIREBASE() {}

void CONFIGFIREBASE::initTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Syncing time");
  time_t now = time(nullptr);
  while (now < 100000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nTime synchronized");
}

bool CONFIGFIREBASE::initFirebase() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  unsigned long wifiStart = millis();
  unsigned long lastDot = millis();
  bool connected = false;

  while (!connected) {
    unsigned long now = millis();

    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      Serial.println("\nWiFi Connected!");
      break;
    }

    if (now - wifiStart >= 120000) {
      Serial.println("\nWiFi connection timeout! Escaping to continue...");
      break;
    }

    if (now - lastDot >= 1000) {
      Serial.print(".");
      lastDot = now;
    }

    delay(10);
  }

  if (connected) {
    initTime(); // only sync time if WiFi connected
  } else {
    Serial.println("Proceeding without WiFi...");
    return false;
  }

  // Firebase config (can still initialize, but calls will fail until WiFi connects)
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback;

  Firebase.reconnectWiFi(true);
  fbdo.setBSSLBufferSize(4096, 1024);

  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  return true;
}


// ===== Helper to read boolean =====
bool CONFIGFIREBASE::getBoolValue(const char* path) {
  if (!Firebase.ready()) return false;

  if (Firebase.getBool(fbdo, path)) {
    return fbdo.boolData();
  } else {
    Serial.println(fbdo.errorReason());
    return false;
  }
}

bool CONFIGFIREBASE::hasPowerOn() {
  static unsigned long lastRead = 0;

  if (millis() - lastRead < FIREBASE_INTERVAL) return false;
  lastRead = millis();

  if (!Firebase.ready()) return false;

  if (Firebase.getBool(fbdo, "/systemsControl/power")) {
    return fbdo.boolData();
  }

  Serial.println(fbdo.errorReason());
  return false;
}

bool CONFIGFIREBASE::statusMode() {
  static unsigned long lastRead = 0;

  if (millis() - lastRead < FIREBASE_INTERVAL) return false;
  lastRead = millis();

  if (!Firebase.ready()) return false;

  if (Firebase.getBool(fbdo, "/systemsControl/status")) {
    return fbdo.boolData();
  }

  Serial.println(fbdo.errorReason());
  return false;
}

// ===== Main speed logic =====
int CONFIGFIREBASE::remoteSpeed() {
  if (millis() - lastFirebaseRead < FIREBASE_INTERVAL) {
    return -1; // no update yet
  }
  lastFirebaseRead = millis();

  if (getBoolValue("/systemsControl/slow")) {
    return 100;
  }

  if (getBoolValue("/systemsControl/moderate")) {
    return 180;
  }

  if (getBoolValue("/systemsControl/rapid")) {
    return 255;
  }

  return 0;
}

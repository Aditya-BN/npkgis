#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <AntaresESP32HTTP.h>
//#include <LiquidCrystal_I2C.h>
//===

// #include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "0610"
#define WIFI_PASSWORD "masuk123"
//test1
#define DATABASE_URL "https://agrisoil-dev-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define DATABASE_SECRET "zJRSwvyUtt8xYDklM4mBm3pBbPLZXp2YsVj4KMbC"

HardwareSerial GPSSerial(2);  // Use Serial2 for ESP32 (GPIO16 - RX2, GPIO17 - TX2)



// FirebaseData firebaseData;
// FirebaseJson json;
// FirebaseData fbdo;

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

Adafruit_GPS GPS(&GPSSerial);
float latitude;
float llongitude;
int satellites;



void setup() {
  Serial.begin(115200);   // Initialize the built-in Serial for debugging
  GPSSerial.begin(9600);  // Initialize the GPS serial communication

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  // pinMode(LED, OUTPUT);
  Serial.println(F("Starting"));
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  // Firebase.begin(FIREBASE_HOST, FIREBASE_Authorization_key);

/* Assign the database URL and database secret(required) */
    config.database_url = DATABASE_URL;
    config.signer.tokens.legacy_token = DATABASE_SECRET;

    Firebase.reconnectNetwork(true);

    // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
    // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    /* Initialize the library with the Firebase authen and config */
    Firebase.begin(&config, &auth);

  //  timeClient.begin();
  //  timeClient.setTimeOffset(25200);
}

void loop() {
  char c = GPS.read();
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) {
      return;
    }
  }

  if (GPS.fix) {
    latitude = GPS.latitudeDegrees;
    llongitude = GPS.longitudeDegrees;
    satellites = GPS.satellites;

    Serial.print("Lat: ");
    Serial.println(latitude, 4);
    Serial.print("Lon: ");
    Serial.println(llongitude, 4);
    Serial.print("Satellites: ");
    Serial.println(satellites);

    Firebase.pushFloat(fbdo, "/Alat_Ukur2/N", latitude);
    Firebase.pushFloat(fbdo, "/Alat_Ukur2/P", llongitude);
    //    Firebase.pushFloat(firebaseData, "/Alat_Ukur2/K", 1);
    //    Firebase.pushFloat(firebaseData, "/Alat_Ukur2/PH", 1);
    //    Firebase.pushFloat(firebaseData, "/Alat_Ukur2/Moist", 1);
  }
  //        Firebase.pushString(firebaseData, "/Alat_Ukur2/Waktu", datehour);
  //        Firebase.pushFloat(firebaseData, "/Alat_Ukur2/N", latitude);
  //        Firebase.pushFloat(firebaseData, "/Alat_Ukur2/P", llongitude);
  //        Firebase.pushFloat(firebaseData, "/Alat_Ukur2/K", 1);
  //        Firebase.pushFloat(firebaseData, "/Alat_Ukur2/PH", 1);
  //        Firebase.pushFloat(firebaseData, "/Alat_Ukur2/Moist", 1);
  //  delay(900);
}

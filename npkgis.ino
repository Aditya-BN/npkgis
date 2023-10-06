#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// #include <AntaresESP32HTTP.h>
// #include <LiquidCrystal_I2C.h>
//===

// #include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "0610"
#define WIFI_PASSWORD "masuk123"
// test1
#define DATABASE_URL "https://agrisoil-dev-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define DATABASE_SECRET "zJRSwvyUtt8xYDklM4mBm3pBbPLZXp2YsVj4KMbC"

HardwareSerial GPSSerial(2); // Use Serial2 for ESP32 (GPIO16 - RX2, GPIO17 - TX2)

void sendToFirebase();

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // will store last time LED was updated

// constants won't change:
const long interval = 5000; // interval at which to blink (milliseconds)

bool ShowNPKorPH;

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

Adafruit_SSD1306 display(128, 64, &Wire, 4);

// library oled

#define NPKa 35
int val, Nx, Px, Kx, vaql0, vaql1, vaql2, vaql3, an0, an1, an2, an3;

String dataSend = "";

// declarasi pin
int nilaiN;
int nilaiP;
int nilaiK;
int nilaiMo;
float nilaipH;
// int an0 = A0;
// int an1 = A1;
// int an2 = A1;
int pH = 35;
// int moisture = A4;
// int an5 = A5;
String Ns, Ps, Ks, pHs, Moists;

void setup()
{
  Serial.begin(115200);  // Initialize the built-in Serial for debugging
  GPSSerial.begin(9600); // Initialize the GPS serial communication

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED)
  {
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

void loop()
{
  char c = GPS.read();
  if (GPS.newNMEAreceived())
  {
    if (!GPS.parse(GPS.lastNMEA()))
    {
      return;
    }
  }

  if (GPS.fix)
  {
    latitude = GPS.latitudeDegrees;
    llongitude = GPS.longitudeDegrees;
    satellites = GPS.satellites;

    Serial.print("Lat: ");
    Serial.println(latitude, 4);
    Serial.print("Lon: ");
    Serial.println(llongitude, 4);
    Serial.print("Satellites: ");
    Serial.println(satellites);
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    Serial.println("kirim db");
    showNPK();
    sendToFirebase();
  }

  //        Firebase.pushString(firebaseData, "/Alat_Ukur2/Waktu", datehour);
  //        Firebase.pushFloat(firebaseData, "/Alat_Ukur2/N", latitude);
  //        Firebase.pushFloat(firebaseData, "/Alat_Ukur2/P", llongitude);
  //        Firebase.pushFloat(firebaseData, "/Alat_Ukur2/K", 1);
  //        Firebase.pushFloat(firebaseData, "/Alat_Ukur2/PH", 1);
  //        Firebase.pushFloat(firebaseData, "/Alat_Ukur2/Moist", 1);
  //  delay(900);
}

void sendToFirebase()
{
  Firebase.setFloat(fbdo, "/Alat1/LAT", latitude);
  Firebase.setFloat(fbdo, "/Alat1/LONG", llongitude);
  Firebase.setFloat(fbdo, "/Alat1/N", Nx);
  Firebase.setFloat(fbdo, "/Alat1/P", Px);
  Firebase.setFloat(fbdo, "/Alat1/K", Kx);
  Firebase.setFloat(fbdo, "/Alat1/PH", nilaipH);
  Firebase.setFloat(fbdo, "/Alat1/MOIST", nilaiMo);
}

void showNPK()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  Serial.print("gps---");

  int anxx = analogRead(NPKa);
  int anxxx = map(anxx, 0, 4095, 0, 1023);
  if (anxxx < 1010)
  {
    maptegangan();
    sensorpH();
    sensorMoist();
  }
  else if (anxxx > 1010)
  {
    Nx = 0;
    Px = 0;
    Kx = 0;
    nilaipH = 0;
    nilaiMo = 0;
  }

  if (ShowNPKorPH)
  {
    Serial.print("something 1");
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 0);
    display.println("N");
    display.setCursor(0, 18);
    display.println(Nx);
    display.setCursor(55, 0);
    display.println("P");
    display.setCursor(45, 18);
    display.println(Px);
    display.setCursor(100, 0);
    display.println("k");
    display.setCursor(90, 18);
    display.println(Kx);
    display.display();
  }
  else
  {
    Serial.print("something 2");
    display.clearDisplay();
    display.setCursor(10, 0);
    display.println("pH");
    display.setCursor(2, 18);
    display.println(nilaipH);
    display.setCursor(60, 0);
    display.println("Moist");
    display.setCursor(70, 18);
    display.println(nilaiMo);
    display.setCursor(99, 18);
    display.println("%");
    display.display();
  }
}
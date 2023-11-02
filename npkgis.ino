//=========================== LIBRARIES ===========================
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// #include <NTPClient.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

//http
#include <HTTPClient.h>
String URL ="https://sistempemetaan2023.com/dashboard/saveData";

//=========================== DEFINE VALUE ===========================
#define WIFI_SSID "Turuu"
#define WIFI_PASSWORD "turu12345"

#define DATABASE_URL "https://agrisoil-dev-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define DATABASE_SECRET "zJRSwvyUtt8xYDklM4mBm3pBbPLZXp2YsVj4KMbC"

#define NPKa 35

const long interval = 5000;
uint32_t previousMillis = 0;
bool ShowNPKorPH;

//=========================== Function prototype ===========================
void sendToFirebase();
void showNPK();
void sensorpH();
void sensorMoist();
void maptegangan();
void firebaseSetup();
void setupWifi();
void konekMyAdmin();

//=========================== FIREBASE ===========================
/* 3. Define the Firebase Data object */
FirebaseData fbdo;
/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;
/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

//=========================== GPS ===========================
// Use Serial2 for ESP32 (GPIO16 - RX2, GPIO17 - TX2)
HardwareSerial GPSSerial(2);
Adafruit_GPS GPS(&GPSSerial);
float latitude;
float llongitude;
int satellites;

//=========================== OLED ===========================
Adafruit_SSD1306 display(128, 32, &Wire, 4);

//=========================== SENSOR ===========================
int val, Nx, Px, Kx, vaql0, vaql1, vaql2, vaql3, an0, an1, an2, an3;

String dataSend = "";

// declarasi pin
int nilaiN;
int nilaiP;
int nilaiK;
int nilaiMo;
float nilaipH;
int pH = 35;
String Ns, Ps, Ks, pHs, Moists, postData, payload;
int httpCode;

void setup()
{
  Serial.begin(115200);  // Initialize the built-in Serial for debugging
  GPSSerial.begin(9600); // Initialize the GPS serial communication

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);

  setupWifi();
  firebaseSetup();

  //  timeClient.begin();
  //  timeClient.setTimeOffset(25200);
}

void setupWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting...");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }

  Serial.println(F("Starting"));
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void firebaseSetup()
{
  /* Assign the database URL and database secret(required) */
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);
}

void maptegangan()
{
  an1 = analogRead(NPKa);
  int anx1 = map(an1, 0, 4095, 0, 1023);
  Nx = map(anx1, 0, 1023, 350, 60);

  Serial.print("AnalogA0 = ");
  Serial.println(anx1);
  Serial.print(" ");

  if (Nx < 0)
  {
    Nx = 0;
  }
  else if (Nx > 900)
  {
    Nx = 900;
  }

  String message11 = "N : " + (String)Nx + " PPM  ";
  String message1 = "    SOIL NPK     ";
  //==========

  //  an2 = analogRead(NPKa);
  ////  vaql2 = map(an2, 0, 1023, 0, 1023);
  //  float Pxx = map(an2, 0, 1023, 10000, 100);
  //  Px= (Pxx/100);
  //  Serial.print("AnalogA0 = ");
  //  Serial.println(an2);
  //  Serial.print(" ");
  //  if (Px <1) {
  //  Px = 1;
  //  }
  //  else if( Px > 100)
  //  Px = 100;
  //  else {
  //  Px = Px;
  //  }

  an2 = analogRead(NPKa);
  //  vaql2 = map(an2, 0, 1023, 0, 1023);
  int anx2 = map(an2, 0, 4095, 0, 1023);
  Px = map(anx2, 100, 1023, 32, 5);
  //  Px= (Pxx/100);

  Serial.print("AnalogA0 = ");
  Serial.println(anx2);
  Serial.print(" ");

  if (Px < 0)
  {
    Px = 0;
  }
  else if (Px > 100)
    Px = 100;
  else
  {
    Px = Px;
  }
  String message21 = "P : " + (String)Px + " PPM ";
  String message2 = "    SOIL NPK     ";

  ////==========

  an3 = analogRead(NPKa);
  //  vaql3 = map(an3, 0, 1023, 0, 1023);
  int anx3 = map(an3, 0, 4095, 0, 1023);
  Kx = map(anx3, 30, 1023, 210, 70);

  Serial.print("AnalogA0 = ");
  Serial.println(an3);
  Serial.print(" ");

  if (Kx < 0)
  {
    Kx = 0;
  }
  else if (Kx > 700)
  {
    Kx = 700;
  }

  String message31 = "K : " + (String)Kx + " PPM ";
  String message3 = "    SOIL NPK     ";
}

void sensorMoist()
{
  //    float nilaiMo;
  int hasilPembacaan = analogRead(NPKa);
  int anxm = map(hasilPembacaan, 0, 4095, 0, 1023);
  nilaiMo = map(anxm, 0, 1023, 100, 20);
  //    nilaiMo = (100-((hasilPembacaan/1023)*100));

  Serial.print("Persentase Kelembaban Tanah = ");
  Serial.print(nilaiMo);
  Serial.println("%");
  
  // delay(2000);
}

void sensorpH()
{
  float anpH = analogRead(NPKa);
  int anxpH = map(anpH, 0, 4095, 0, 1023);
  float anpH1 = map(anxpH, 0, 950, 10, 35);

  nilaipH = (-0.0693 * anpH1) + 7.3855;

  if (nilaipH < 1)
  {
    nilaipH = 1;
  }
  else if (nilaipH > 14)
  {
    nilaipH = 14;
  }

  Serial.print(" PH = ");
  Serial.println(nilaipH);
  
  // delay(2000);
}

void sendToFirebase()
{
  Serial.println("Kirim ke FIREEEEbase");
  Firebase.setFloat(fbdo, "/Alat3_rt/LAT", latitude);
  Firebase.setFloat(fbdo, "/Alat3_rt/LONG", llongitude);
  Firebase.setFloat(fbdo, "/Alat3_rt/N", Nx);
  Firebase.setFloat(fbdo, "/Alat3_rt/P", Px);
  Firebase.setFloat(fbdo, "/Alat3_rt/K", Kx);
  Firebase.setFloat(fbdo, "/Alat3_rt/PH", nilaipH);
  Firebase.setFloat(fbdo, "/Alat3_rt/MOIST", nilaiMo);

 Serial.println("Kirim ke FIREEEEbase");
  Firebase.pushFloat(fbdo, "/Alat3/LAT", latitude);
  Firebase.pushFloat(fbdo, "/Alat3/LONG", llongitude);
  Firebase.pushFloat(fbdo, "/Alat3/N", Nx);
  Firebase.pushFloat(fbdo, "/Alat3/P", Px);
  Firebase.pushFloat(fbdo, "/Alat3/K", Kx);
  Firebase.pushFloat(fbdo, "/Alat3/PH", nilaipH);
  Firebase.pushFloat(fbdo, "/Alat3/MOIST", nilaiMo);
}

void showNPK()
{

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
    Serial.println("something 1");
    display.clearDisplay();
    display.setTextSize(2);
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
    Serial.println("something 2");
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
  ShowNPKorPH = !ShowNPKorPH;
  
}

void loop()
{
  konekMyAdmin();
  
  // GPS
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
}
  void konekMyAdmin(){
  //sent to myAdmin
  postData = "kelembaban=" + String(nilaiMo) + "&ph=" + String(nilaipH) + "&nilai_n=" + String(Nx) + "&nilai_p=" + String(Px) + "&nilai_k=" + String(Kx) + "&latitude=" + String(latitude) + "&longitude=" + String(llongitude);
  
  //KONEK HTTP
  HTTPClient http;
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
   httpCode = http.POST(postData);
  payload = "";

  if(httpCode > 0) {
    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      payload = http.getString();
      Serial.println(payload);
    } else {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  delay(800);
  
  http.end();  //Close connection
 
  Serial.print("URL : ");  Serial.println(URL);
  Serial.print("Data: "); Serial.println(postData);
  Serial.print("httpCode: "); Serial.println(httpCode);
  Serial.print("payload: "); Serial.println(payload);
}
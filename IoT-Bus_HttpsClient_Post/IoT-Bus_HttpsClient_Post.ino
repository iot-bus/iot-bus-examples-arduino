/**************************************************************
 *
 * This sketch connects to a website and downloads a page.
 * It can be used to perform HTTP/RESTful API calls.
 *
 * For this example, you need to install ArduinoHttpClient library:
 *   https://github.com/arduino-libraries/ArduinoHttpClient
 *   or from http://librarymanager/all#ArduinoHttpClient
 *
 * TinyGSM Getting Started guide:
 *   http://tiny.cc/tiny-gsm-readme
 *
 * SSL/TLS is currently supported only with: SIM8xx, uBlox
 *
 * For more HTTP API examples, see ArduinoHttpClient library
 *
 **************************************************************/

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_UBLOX

// Increase RX buffer if needed
//#define TINY_GSM_RX_BUFFER 512

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

// Uncomment this if you want to see all AT commands
#define DUMP_AT_COMMANDS

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Use Hardware Serial on Mega, Leonardo, Micro
// Change to Serial2 for ESP32
#define SerialAT Serial2

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "pwg"; // this is my APN for US Mobile
const char user[] = "";
const char pass[] = "";

// Server details
//const char server[] = "vsh.pp.ua";
//const char resource[] = "/TinyGSM/logo.txt";
const char server[] = "api.scriptrapps.io";
const char resource[] = "/HelloDevice";
const char data[] = "myName=ian&auth_token=S0I0QzgyMEM1NjpzY3JpcHRyOkQxMTQwNjBERUQwN0FEREQ5NTBGOTk0MEU2ODNCQTk3";
const int  port = 443;

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

TinyGsmClientSecure client(modem);
HttpClient http(client, server, port);

void powerOn(int _rstpin){
  pinMode(_rstpin, OUTPUT);
  digitalWrite(_rstpin, LOW);
  delay(100);
  digitalWrite(_rstpin, HIGH);
  delay(1000);
  digitalWrite(_rstpin, LOW);
}

void setup() {

  // Power on the modem
  powerOn(33); // power on io 33
  
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Initializing modem..."));
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print(F("Modem: "));
  SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  if (!modem.hasSSL()) {
    SerialMon.println(F("SSL is not supported by this modem"));
    while(true) { delay(1000); }
  }
}

void loop() {
  SerialMon.print(F("Waiting for network..."));
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print(F("Performing HTTPS POST request... "));
  http.connectionKeepAlive(); // This is needed for HTTPS GET - doesn't seem to make a difference on POST
  // POST is terminating without us being able to read the full response

  int err = http.post("/HelloDevice", "application/x-www-form-urlencoded", data);

  if (err != 0) {
    SerialMon.println(F("failed to connect"));
    delay(10000);
    return;
  }
 // if we remove this then we can see first chunk being read by CIPRXGET
//  int status = http.responseStatusCode();
//  SerialMon.println(status);
//  if (!status) {
//    delay(10000);
//    return;
//  }

  while (http.headerAvailable()) {
    String headerName = http.readHeaderName();
    String headerValue = http.readHeaderValue();
    SerialMon.println(headerName + " : " + headerValue);
  }

  int length = http.contentLength();
  if (length >= 0) {
    SerialMon.print(F("Content length is: "));
    SerialMon.println(length);
  }
  if (http.isResponseChunked()) {
    SerialMon.println(F("The response is chunked"));
  }

  String body = http.responseBody();
  SerialMon.println(F("Response:"));
  SerialMon.println(body);

  SerialMon.print(F("Body length is: "));
  SerialMon.println(body.length());

  // Shutdown

  http.stop();
  SerialMon.println(F("Server disconnected"));

  modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));

  // Do nothing forevermore
  while (true) {
    delay(1000);
  }
}


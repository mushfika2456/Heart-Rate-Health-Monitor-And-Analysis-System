#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
//#include<UnoWiFiDevEd.h>
#endif
#include <json/FirebaseJson.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */

#define WIFI_SSID "AndroidAP3DA7" //"CIK1000M_AC-3d4c"
#define WIFI_PASSWORD "udao7866"


// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyDl7VN-07GbPLLZ-_8I297kZ_9I0dMucJE"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://myhealths2-6ab3f-default-rtdb.firebaseio.com/"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

//#define WIFI_SSID "AndroidAP3DA7"//"Hassan's iPhone"//"CIK1000M_AC-3d4c"
//#define WIFI_PASSWORD "udao7866"//"@Appa2138" //"3c9066af3d4c"

#define USER_EMAIL "rahman.mushfika2456@gmail.com"
#define USER_PASSWORD "rahman12"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

FirebaseJsonArray sdata;
FirebaseData firebaseData;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;
uint32_t notConnectedCounter = 0;

#define SENSOR 2

long instance1 = 0, timer;

double hr = 0, interval = 0;

long value = 0, count1 = 0;
long x;

bool flag = 0;
//const int D2 = 35;
#define shutdown_pin 10

#define threshold 0.55  // to identify R peak

#define timer_value 10000  // 10 seconds timer to calculate hr

const int potPin = 35;

// variable for storing the potentiometer value
int potValue = 0;
int ecg_min = 3700;  // initialize to the maximum possible value
int ecg_max = 1200;
float ecg_norm;
double ecg_norm_array[250];  // declare an array of type double with size 250
int ecg_norm_index = 0;
//int dac;

void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  delay(10000);
  //---------------------------------------------------------
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";


  Firebase.begin(&config, &auth);
  delay(10000);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  //Firebase.setDoubleDigits(5);
  instance1 = micros();

  timer = millis();
}

void loop() {
  // Reading potentiometer value
  potValue = analogRead(potPin);
  //dac = (potValue*3.3)/4095;
  //Serial.println(potValue);
  //delay(30);//20

  if (potValue < ecg_min) {
    ecg_min = potValue;
  }

  if (potValue > ecg_max) {
    ecg_max = potValue;
  }

  ecg_norm = (float)(potValue - ecg_min) / (ecg_max - ecg_min);
Serial.println(ecg_norm);
  /*Serial.print("min = ");
Serial.print(ecg_min);
Serial.println ("max = ");
Serial.print(ecg_max);*/
  //-------------------------heart rate trial-------------------------------

  if ((ecg_norm > threshold) && (!flag)) {

    count1++;

    Serial.println("in");

    flag = 1;

    interval = micros() - instance1;  //RR interval

    instance1 = micros();

  }

  else if ((ecg_norm < threshold)) {

    flag = 0;
  }

  if ((millis() - timer) > 10000) {
    hr = count1 * 6;

    timer = millis();

    count1 = 0;
  }
  Serial.print("count");
  Serial.println(count1);


  Serial.print("hr");
  Serial.print(hr);
  delay(1);



  ecg_norm_array[ecg_norm_index] = ecg_norm;

  // increment the index of the array
  ecg_norm_index++;

  // check if the index has reached the end of the array
  if (ecg_norm_index == 250) {
    // reset the index to 0
    ecg_norm_index = 0;


    //-----------------sending it to firebase------------------------------------

    //sdata.clear();
    for (int i = 0; i < 250; i++) {
      sdata.set("/[" + String(i) + "]", ecg_norm_array[i]);
    }
    Firebase.setArray(firebaseData, "/ECG-arduino/Signal", sdata);
    Firebase.setDouble(firebaseData, "/ECG-arduino/hr", hr);
  }

  delay(5);
}

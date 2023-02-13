#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>


#include "addons/TokenHelper.h"      // Provide the token generation process info.
#include "addons/RTDBHelper.h"       // Provide the RTDB payload printing info and other helper functions.


// Insert Firebase project API Key
#define API_KEY "AIzaSyANkFHM-1Db7wWXjnIE9PwhYo1FfSNoPxI"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "hajebram23@gmail.com"
#define USER_PASSWORD "espdemotest"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://esp-firebase-demo-4d1c7-default-rtdb.firebaseio.com"

// Inset WiFi ssid and password
#define WIFI_SSID  "TELUS8853"
#define WIFI_PASSWORD  "FHDt9JL58xfR"


// Define Firebase objects
FirebaseData firebase_db;
FirebaseAuth firebase_auth;
FirebaseConfig firebase_config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String lightintensity_path;


// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 5000;


// Initialize WiFi
void setupWIFI() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Write float values to the database
void sendFloat(String path, float value){
  if (Firebase.RTDB.setFloat(&firebase_db, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + firebase_db.dataPath());
    Serial.println("TYPE: " + firebase_db.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebase_db.errorReason());
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(32, INPUT);
  setupWIFI();

  // Assign the api key (required)
  firebase_config.api_key = API_KEY;

  // Assign the user sign in credentials
  firebase_auth.user.email = USER_EMAIL;
  firebase_auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  firebase_config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  firebase_db.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  firebase_config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  firebase_config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&firebase_config, &firebase_auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((firebase_auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }

  // Print user UID
  uid = firebase_auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid;

  // Update database path for sensor readings
  lightintensity_path = databasePath + "/LightIntensity"; // --> UsersData/<user_uid>/lightIntensity
 

}


void loop() {
  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Send readings to database:
    sendFloat(lightintensity_path, analogRead(32));
  }
}
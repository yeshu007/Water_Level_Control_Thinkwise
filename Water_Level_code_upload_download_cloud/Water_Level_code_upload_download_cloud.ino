/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-hc-sr04-ultrasonic-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#define DATABASE_URL "https://thinkwise-fb3db-default-rtdb.firebaseio.com/"
#define API_KEY "AIzaSyCWRvMkx-cM8yzo6MXyX_y9S9z8cYYW9YU"
#define WIFI_SSID "iPhone"
#define WIFI_PASSWORD "bangalore"

// Define Firebase Data object
FirebaseData firebaseData;

FirebaseAuth auth;
FirebaseConfig config;

const int trigPin = 12;
const int echoPin = 14;

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

long duration;
int distanceCm;
float distanceInch;

void setup() {
  Serial.begin(115200); // Starts the serial communication
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
 Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);

  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = "aaa@gmail.com";//USER_EMAIL;
  auth.user.password = "123456";//USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  firebaseData.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);
  //Firebase.begin(API_KEY, DATABASE_URL);

  //Firebase.reconnectWiFi(true);
}
static int distanceCm_old;
void loop() {
  int distance_difference =0;
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  //Serial.print("duration (cm): ");
 // Serial.println(duration);
  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  //distanceCm_old =distanceCm;
  // Prints the distance on the Serial Monitor
  if(distanceCm_old > distanceCm)
  {
    distance_difference =distanceCm_old-distanceCm;
  }
  else
  {
    distance_difference =distanceCm - distanceCm_old;
  }
  if(distance_difference >20)
  {
    if(distanceCm!=distanceCm_old)
    {
    Serial.print("Distance (cm): ");
    Serial.println(distanceCm);
    }
    distanceCm_old =distanceCm;
  }
    if (Firebase.setString(firebaseData, "/level", distanceCm)) {
    
    Serial.println(" Uploaded Successfully");
  }
  else {
    Serial.println("  Not Successfully");
    Serial.println(firebaseData.errorReason());
  }
  Serial.printf("Get string value from firebase... %s\n", Firebase.getString(firebaseData, "/level") ? firebaseData.to<const char *>() : firebaseData.errorReason().c_str());
  delay(2000);
}

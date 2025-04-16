#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>


//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Jio-Yesh"
#define WIFI_PASSWORD "Hanu@123"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAFKTHx10wCs8Lm2MSISzZjmFCuJJe98aw"//sdyxRvAD0Lj7CFn4HQtPfwJucpz10x9GvZBFy3GE

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "water-level-controller-257a1-default-rtdb.firebaseio.com"

// Define connections to sensor
#define TRIGPIN 12
#define ECHOPIN 13

#define MAX_TANK_DEPTH 150//in cm
#define ADJUSTER 30//in cm
#define sample_array_size 10 // amount of samples need o be sampled
#define MIN_DETECTION_DIST 22
#define MAX_VALUE 600
// Floats to calculate distance
float duration, distance;
float prev_sample, currrent_sample;

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

unsigned long previousMillis = 0;
unsigned long interval = 10000;//10sec

unsigned long previousMillis_error = 0;
unsigned long interval_error = 10000; // 30 sec

float Sump_Water_Distance() 
{
  // Set the trigger pin LOW for 2uS
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
 
  // Set the trigger pin HIGH for 20us to send pulse
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(20);
 
  // Return the trigger pin to LOW
  digitalWrite(TRIGPIN, LOW);
 
  // Measure the width of the incoming pulse
  duration = pulseIn(ECHOPIN, HIGH);
 
  // Determine distance from duration
  // Use 343 metres per second as speed of sound
  // Divide by 1000 as we want millimeters
 
  //distance = (duration / 2) * 0.343;
  distance = (duration / 2) * 0.0343;
 
  // Print result to serial monitor
  Serial.print("distance: ");
  Serial.print(distance);
  Serial.println(" cm ");

  // Serial.print("duration: ");
  // Serial.print(duration);
  // Serial.println(" s");

  // Delay before repeating measurement
  delay(100);

  return distance;
}
void firebase_Error_check(String S)
{
  unsigned long currentMillis_error = millis();
  //String S_compare = "token is not ready (revoked or expired)";
  Serial.print("S_error =");
  Serial.println(S);
    // substring(index) looks for the substring from the index position to the end:
  // you can also look for a substring in the middle of a string:
    if (currentMillis_error - previousMillis_error >=interval_error)
    {
      Serial.print(millis());
      Serial.println("Encountered firebase error restarting....");
      //firebase_init();
      //if(firebase_init_counter > max_init_per_start)
      if ((S.substring(0, 5) == "token") && (S.substring(9, 12) == "not")&&(S.substring(13, 18) == "ready")&&(S.substring(31,38) == "expired"))
      {
        Serial.println("Restart ESP");
        ESP.restart();
      }
      previousMillis_error = currentMillis_error;
    }
}
void init_wifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void setup()
{
  Serial.begin(115200);
 
  // Set pinmodes for sensor connections
  pinMode(ECHOPIN, INPUT);
  pinMode(TRIGPIN, OUTPUT);

  init_wifi();
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
void Sampler()
{
  //Start
  int i = 0;
  String S_error="";
  //float data_array[sample_array_size] = {0} , wp = 0; 
  float water_percentage = 0;
  float result = 0;

  currrent_sample = Sump_Water_Distance();
  
 // Serial.print("currrent_sample: ");
 // Serial.println(currrent_sample);

  if(currrent_sample < MIN_DETECTION_DIST)
  {  
    currrent_sample = 0;
  }
  else if(currrent_sample > MAX_VALUE)
  {
    currrent_sample = 0;
  }
  else
  {
    currrent_sample = currrent_sample;
  }
  result = (currrent_sample - prev_sample);

  if (result > 0)
  {
    result = result;
  }
  else
  {
    result = (result * -1);
  }
//store it in a variable for uploading to cloud
  if(result < 10)
  {
    water_percentage = (currrent_sample * (0.6666666));

    if(water_percentage > 100)
    {
      water_percentage = 100;
    }
    else
    {
      water_percentage = water_percentage;
    }
    water_percentage = 100-water_percentage;
    if (Firebase.ready() && signupOK ) 
    {
      if (Firebase.RTDB.setFloat(&fbdo, "IOT_Control_4Load_mod/WP",water_percentage))
      {
        Serial.print("Water Percentage: ");
        Serial.println(water_percentage);
      }
      else 
      {
        Serial.println(S_error = fbdo.errorReason());
        firebase_Error_check(S_error);
      }
    }
  }
  else
  {
    if (Firebase.ready() && signupOK ) 
    {
      if (Firebase.RTDB.setFloat(&fbdo, "IOT_Control_4Load_mod/WP",255))
      {
        Serial.print("Water Percentage: ");
        Serial.println(255);
      }
      else 
      {
        Serial.println(S_error = fbdo.errorReason());
        firebase_Error_check(S_error);
      }
    }
  }
  //Read last 10 input samples and store in array
  // for(i=0;i<sample_array_size;i++)
  // {
  //   data_array[i] = Sump_Water_Distance();
  //   if(data_array[i] > (MAX_TANK_DEPTH + ADJUSTER))
  //   {
  //     data_array[i] = 0.0;
  //   }
  // }

  prev_sample = currrent_sample;
  //Serial.print("prev_sample: ");
  //Serial.println(prev_sample);
  //end
}
void loop()
{
  unsigned long currentMillis = millis();
  Sampler();
  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval))
  {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    init_wifi();
    previousMillis = currentMillis;
  }
  delay(500);
}
/*****************************************************************************************************************************************************/
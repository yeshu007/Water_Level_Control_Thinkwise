#include <Arduino.h>
#include <ESP8266WiFi.h>
//#include <Firebase_ESP_Client.h>
#include <FirebaseESP8266.h>


//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
//#define WIFI_SSID "Jio-Yesh"
//#define WIFI_PASSWORD "Hanu@123"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAFKTHx10wCs8Lm2MSISzZjmFCuJJe98aw"//sdyxRvAD0Lj7CFn4HQtPfwJucpz10x9GvZBFy3GE

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://water-level-controller-257a1-default-rtdb.firebaseio.com"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

unsigned long previousMillis = 0;
unsigned long interval = 10000;//10sec

unsigned long previousMillis_error = 0;
unsigned long interval_error = 10000; // 30 sec

int test_data = 0;

String WIFI_SSID = "Jio-Yesh";
String WIFI_PASSWORD = "Hanu@123";

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

void loop()
{
  unsigned long currentMillis = millis();
  String S_error = "";
  // upload to cloud
  if (Firebase.ready() && signupOK ) 
    {
      if (Firebase.setString(fbdo, "IOT_Control_4Load_mod/TD",test_data))
      {
        Serial.print("test_data: ");
        Serial.println(test_data);
      }
      else 
      {
        Serial.println(S_error = fbdo.errorReason());
        firebase_Error_check(S_error);
      }
    }
    test_data++;
  Serial.printf("Get string value from firebase... %s\n", Firebase.getString(fbdo, "IOT_Control_4Load_mod/TD") ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
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
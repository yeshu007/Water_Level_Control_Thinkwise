#include <time.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// Insert your network credentials
#define WIFI_SSID "Jio-Yesh"
#define WIFI_PASSWORD "Hanu@123"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAFKTHx10wCs8Lm2MSISzZjmFCuJJe98aw"//sdyxRvAD0Lj7CFn4HQtPfwJucpz10x9GvZBFy3GE
#define DATABASE_URL "https://water-level-controller-257a1-default-rtdb.firebaseio.com/"
//#define DATABASE_URL "https://thinkwise-fb3db-default-rtdb.firebaseio.com/"

#define TRIGPIN_SUMP 5
#define ECHOPIN_SUMP 4
#define TRIGPIN_OT 14
#define ECHOPIN_OT 12

#define MAX_SUMP_DEPTH 150//in cm
#define MAX_OT_DEPTH 125//in cm
#define ADJUSTER 30//in cm
#define sample_array_size 10 // amount of samples need o be sampled
#define MIN_DETECTION_DIST 3
#define MAX_VALUE 600

#define MAX_ALIVE_COUNTER 999

// Floats to calculate distance
float duration_ot, distance_ot, duration_sump, distance_sump;
float prev_sample_ot, currrent_sample_ot, prev_sample_sump, currrent_sample_sump;

long timezone = 0;
byte daysavetime = 1;

//Define Firebase Data object
FirebaseData firebaseData;

FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

unsigned long previousMillis = 0;
unsigned long interval = 10000;//10sec

unsigned long previousMillis_error = 0;
unsigned long interval_error = 10000; // 30 sec

int alive_counter = 0;

float OT_Water_Distance() {
 
  // Set the trigger pin LOW for 2uS
  digitalWrite(TRIGPIN_OT, LOW);
  delayMicroseconds(2);
 
  // Set the trigger pin HIGH for 20us to send pulse
  digitalWrite(TRIGPIN_OT, HIGH);
  delayMicroseconds(20);
 
  // Return the trigger pin to LOW
  digitalWrite(TRIGPIN_OT, LOW);
 
  // Measure the width of the incoming pulse
  duration_ot = pulseIn(ECHOPIN_OT, HIGH);
 
  // Determine distance from duration
  // Use 343 metres per second as speed of sound
  // Divide by 1000 as we want millimeters
 
  //distance = (duration / 2) * 0.343;
  distance_ot = (duration_ot / 2) * 0.0343;
 
  // Print result to serial monitor
  Serial.print("distance_ot: ");
  Serial.print(distance_ot);
  Serial.println(" cm ");

  // Serial.print("duration: ");
  // Serial.print(duration);
  // Serial.println(" s");

  // Delay before repeating measurement
  delay(100);

  return distance_ot;
}
float Sump_Water_Distance() {
 
  // Set the trigger pin LOW for 2uS
  digitalWrite(TRIGPIN_SUMP, LOW);
  delayMicroseconds(2);
 
  // Set the trigger pin HIGH for 20us to send pulse
  digitalWrite(TRIGPIN_SUMP, HIGH);
  delayMicroseconds(20);
 
  // Return the trigger pin to LOW
  digitalWrite(TRIGPIN_SUMP, LOW);
 
  // Measure the width of the incoming pulse
  duration_sump = pulseIn(ECHOPIN_SUMP, HIGH);
 
  // Determine distance from duration
  // Use 343 metres per second as speed of sound
  // Divide by 1000 as we want millimeters
 
  //distance = (duration / 2) * 0.343;
  distance_sump = (duration_sump / 2) * 0.0343;
 
  // Print result to serial monitor
  Serial.print("distance_sump: ");
  Serial.print(distance_sump);
  Serial.println(" cm ");

  // Serial.print("duration: ");
  // Serial.print(duration);
  // Serial.println(" s");

  // Delay before repeating measurement
  delay(100);

  return distance_sump;
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

void setup(){
  Serial.begin(115200);
 
  // Set pinmodes for sensor connections
  pinMode(ECHOPIN_SUMP, INPUT); //TRIGPIN_SUMP
  pinMode(TRIGPIN_SUMP, OUTPUT);
  pinMode(ECHOPIN_OT, INPUT);
  pinMode(TRIGPIN_OT, OUTPUT);

  init_wifi();

  // After connecting to WiFi
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  // Set your timezone
  setenv("TZ", "IST-5:30", 1);  // Change to your timezone
  tzset();

  //configTime(3600 * timezone, daysavetime * 3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = "m.yesh007@gmail.com";//USER_EMAIL;
  auth.user.password = "9071295134";//USER_PASSWORD;

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
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  firebaseData.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);
}
void Alive_Counter()
{
  String S_error="";
  String S_test="";
  alive_counter++;
  if(alive_counter == MAX_ALIVE_COUNTER)
  {
    alive_counter = 0;
  }
  //Serial.println("I am Alive:");
  if (Firebase.ready() && signupOK ) 
    {
      if (Firebase.setFloat(firebaseData, "IOT_Control_4Load_mod/AC",alive_counter))
      {
        Serial.print("Alive_Counter: ");
        Serial.println(alive_counter);
      }
      else 
      {
        Serial.println(S_error = firebaseData.errorReason());
        firebase_Error_check(S_error);
      }
    }
    // Serial.printf("Get string value from firebase... %s\n", Firebase.getString(firebaseData, "IOT_Control_4Load_mod/OP") ? (S_test = firebaseData.to<const char *>()) : firebaseData.errorReason().c_str());
    // Serial.print("S_test: ");
    // Serial.println(S_test);
}

void Sampler_OT()
{
  //Start
  int i = 0;
  String S_error="",OF_read = "";
  //float data_array[sample_array_size] = {0} , wp = 0; 
  float water_percentage = 0;
  float result = 0;

  currrent_sample_ot = OT_Water_Distance();
  //currrent_sample = HC_SR04_Yesh.ping_cm();

  currrent_sample_ot = currrent_sample_ot - 19; //dist in cm from USS to OF sensor

  if (currrent_sample_ot >= 0)
  {
    currrent_sample_ot = currrent_sample_ot;
  }
  else
  {
    currrent_sample_ot = (currrent_sample_ot * -1);
  }
  
  Serial.print("currrent_sample_ot: ");
  Serial.println(currrent_sample_ot);



  // if(currrent_sample_ot < MIN_DETECTION_DIST)
  // {  
  //   currrent_sample_ot = 0;
  // }
  // else if(currrent_sample_ot > MAX_VALUE)
  // {
  //   currrent_sample_ot = 0;
  // }
  // else
  // {
  //   currrent_sample_ot = currrent_sample_ot;
  // }
  result = (currrent_sample_ot - prev_sample_ot);

  if (result >= 0)
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
    water_percentage = (currrent_sample_ot * (0.81));//100/122

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
      if(Firebase.getString(firebaseData, "IOT_Control_4Load_mod/OF"))
      {
          OF_read = (firebaseData.to<const char *>());
          Serial.print("OF_read: ");
          Serial.println(OF_read);
      }
      else
      {
          Serial.println(S_error = firebaseData.errorReason().c_str());
          firebase_Error_check(S_error);
      }
      if(OF_read == "1")
      {
        Serial.println("OF: 1");
        //water_percentage = 100.0;
      }
      else
      {
        Serial.println("Not OF ");
      }
      if (Firebase.setFloat(firebaseData, "IOT_Control_4Load_mod/OP",water_percentage))
      {
        Serial.print("Water Percentage_OT: ");
        Serial.println(water_percentage);
      }
      else 
      {
        Serial.println(S_error = firebaseData.errorReason());
        firebase_Error_check(S_error);
      }
    }
  }
  else
  {
    if (Firebase.ready() && signupOK ) 
    {
      if (Firebase.setFloat(firebaseData, "IOT_Control_4Load_mod/OP",255))
      {
        Serial.print("Water Percentage_OT: ");
        Serial.println(255);
      }
      else 
      {
        Serial.println(S_error = firebaseData.errorReason());
        firebase_Error_check(S_error);
      }
    }
  }
  prev_sample_ot = currrent_sample_ot;
  //Serial.print("prev_sample: ");
  //Serial.println(prev_sample);
  //end
}
void Sampler_SUMP()
{
  //Start
  int i = 0;
  String S_error="",SF_read = "";
  //float data_array[sample_array_size] = {0} , wp = 0; 
  float water_percentage = 0;
  float result = 0;

  currrent_sample_sump = Sump_Water_Distance();
  //currrent_sample = HC_SR04_Yesh.ping_cm();
  // if(currrent_sample_sump > 18) 
  // {
    currrent_sample_sump = currrent_sample_sump - 15; //dist in cm from USS to SF sensor
  // }
  // else
  // {
  //   currrent_sample_sump = currrent_sample_sump;
  // }
  if (currrent_sample_sump >= 0)
  {
    currrent_sample_sump = currrent_sample_sump;
  }
  else
  {
    //currrent_sample_sump = (currrent_sample_sump * -1);
    currrent_sample_sump = 0;
  }
  Serial.print("currrent_sample_sump: ");
  Serial.println(currrent_sample_sump);

  // if(currrent_sample_sump < MIN_DETECTION_DIST)
  // {  
  //   currrent_sample_sump = 0;
  // }
  // else if(currrent_sample_sump > MAX_VALUE)
  // {
  //   currrent_sample_sump = 0;
  // }
  // else
  // {
  //   currrent_sample_sump = currrent_sample_sump;
  // }
  result = (currrent_sample_sump - prev_sample_sump);

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
    water_percentage = (currrent_sample_sump * (0.66));//100/150

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
      if(Firebase.getString(firebaseData, "IOT_Control_4Load_mod/SF"))
      {
          SF_read = (firebaseData.to<const char *>());
          Serial.print("SF_read: ");
          Serial.println(SF_read);
      }
      else
      {
          Serial.println(S_error = firebaseData.errorReason().c_str());
          firebase_Error_check(S_error);
      }
      if((SF_read == "1"))
      {
        Serial.println("SF: 1");
        //water_percentage = 100.0;
      }
      else
      {
        Serial.println("Not SF ");
      }
      if (Firebase.setFloat(firebaseData, "IOT_Control_4Load_mod/WP",water_percentage))
      {
        Serial.print("Water Percentage: ");
        Serial.println(water_percentage);
      }
      else 
      {
        Serial.println(S_error = firebaseData.errorReason());
        firebase_Error_check(S_error);
      }
    }
  }
  else
  {
    if (Firebase.ready() && signupOK ) 
    {
      if (Firebase.setFloat(firebaseData, "IOT_Control_4Load_mod/WP",255))
      {
        Serial.print("Water Percentage: ");
        Serial.println(255);
      }
      else 
      {
        Serial.println(S_error = firebaseData.errorReason());
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

  prev_sample_sump = currrent_sample_sump;
  //Serial.print("prev_sample: ");
  //Serial.println(prev_sample);
  //end
}
void Live_Time()
{
  String S_error="";
  String S_test="";
  struct tm tmstruct;
  delay(2000);
  tmstruct.tm_year = 0;
  getLocalTime(&tmstruct, 5000);
  //Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
  if(tmstruct.tm_hour > 12)
  {
    tmstruct.tm_hour = tmstruct.tm_hour - 12;
  }

  Serial.printf("\nNow is : %02d:%02d:%02d\n", tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
  if (Firebase.ready() && signupOK ) 
    {
      if (Firebase.setFloat(firebaseData, "IOT_Control_4Load_mod/TH",tmstruct.tm_hour))
      {
        Serial.print("tmstruct.tm_hour: ");
        Serial.println(tmstruct.tm_hour);
      }
      else 
      {
        Serial.println(S_error = firebaseData.errorReason());
        firebase_Error_check(S_error);
      }
      if (Firebase.setFloat(firebaseData, "IOT_Control_4Load_mod/TM",tmstruct.tm_min))
      {
        Serial.print("tmstruct.tm_min: ");
        Serial.println(tmstruct.tm_min);
      }
      else 
      {
        Serial.println(S_error = firebaseData.errorReason());
        firebase_Error_check(S_error);
      }
      if (Firebase.setFloat(firebaseData, "IOT_Control_4Load_mod/TS",tmstruct.tm_sec))
      {
        Serial.print("tmstruct.tm_sec: ");
        Serial.println(tmstruct.tm_sec);
      }
      else 
      {
        Serial.println(S_error = firebaseData.errorReason());
        firebase_Error_check(S_error);
      }
    }
    // Serial.printf("Get string value from firebase... %s\n", Firebase.getString(firebaseData, "IOT_Control_4Load_mod/OP") ? (S_test = firebaseData.to<const char *>()) : firebaseData.errorReason().c_str());
    // Serial.print("S_test: ");
    // Serial.println(S_test);
}
void loop()
{
  unsigned long currentMillis = millis();
  Sampler_OT();
  Sampler_SUMP();
  //Alive_Counter();
  Live_Time();
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

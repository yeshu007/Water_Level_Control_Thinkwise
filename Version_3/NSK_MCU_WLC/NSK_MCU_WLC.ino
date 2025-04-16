#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>


/* 1. Define the WiFi credentials */
#define WIFI_SSID "Jio-Yesh"
#define WIFI_PASSWORD "Hanu@123"

//#define WIFI_SSID "YESHS23ultra"
//#define WIFI_PASSWORD "Yesh12345"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyAFKTHx10wCs8Lm2MSISzZjmFCuJJe98aw"//sdyxRvAD0Lj7CFn4HQtPfwJucpz10x9GvZBFy3GE 

/* 3. Define the RTDB URL */
#define DATABASE_URL "water-level-controller-257a1-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "m.yesh007@gmail.com"
#define USER_PASSWORD "9071295134"

#define REQUESTED     1
#define NOT_REQUESTED 0
#define WF_MAX_VALUE 750

int M_ON_req_flag = NOT_REQUESTED;

// Define Firebase Data object
FirebaseData firebaseData;

FirebaseAuth auth;
FirebaseConfig config;

int WaterFlowValue = 0;  // variable to store the value coming from the sensor

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif
//START->MY CHANGES
String S_org="";
String S_error="";


//For generic esp8266-----
uint8_t Water_Flow_st = 12;
uint8_t NEED_RESET = 13;
uint8_t NR_Recv_Req = 14;
int Water_FLow_an = A0;   // select the input pin for the potentiometer
//--------end-------------
bool signupOK = false;

char MC_val = '\0',NR_val = '\0' ;
int string_extracter = 4;

unsigned long previousMillis = 0;
unsigned long interval = 10000;//60 sec

unsigned long previousMillis_error = 0;
unsigned long interval_error = 10000; // 30 sec

int max_init_per_start = 6;
int firebase_init_counter = 0;

//END->MY CHANGES

void  firebase_init()
{
  firebase_init_counter++;
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

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
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);
}
void init_wifi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}
void setup()
{
  Serial.begin(115200);

//START->MY CHANGES
  pinMode(NR_Recv_Req, OUTPUT);
  pinMode(NEED_RESET, OUTPUT);
  pinMode(Water_Flow_st, OUTPUT); 

  digitalWrite(NEED_RESET, LOW); 
  digitalWrite(NR_Recv_Req, HIGH); 
  digitalWrite(Water_Flow_st, HIGH);
  

//END->MY CHANGES

  init_wifi();
  firebase_init();
}

void MOTOR_ON_LOGIC(String S)
{
  Serial.print("Complete String = ");
  Serial.println(S);

  int pos_MC  = S.indexOf("MC");

  Serial.print("MC = ");
  Serial.println(pos_MC);

  int pos_NR  = S.indexOf("NR");

  Serial.print("NR = ");
  Serial.println(pos_NR);

  MC_val=S[pos_MC+string_extracter];
  NR_val=S[pos_NR+string_extracter];

  if(MC_val == '"')
  {
    MC_val=S[pos_MC+string_extracter+1];
  }
  if(NR_val == '"')
  {
    NR_val=S[pos_NR+string_extracter+1];
  }
  Serial.print("MC NR = ");
  Serial.print(MC_val);Serial.print(" ");Serial.println(NR_val);

  if((NR_val=='5')&&(MC_val == '0'))// && (M_ON_req_flag == NOT_REQUESTED))
  {
    digitalWrite(NEED_RESET, HIGH); //It will short the push button using BC547
    delay(50);
    M_ON_req_flag = REQUESTED;
    Serial.println("MOTOR_ON REQUESTED and request recieved (MC =2)");
    digitalWrite(NR_Recv_Req, LOW); //NR=5 recieved and communicated to node mcu to make Mc =2 (req_recv)
    delay(1000);
    digitalWrite(NEED_RESET, LOW);
    delay(50);
    digitalWrite(NR_Recv_Req, HIGH);//
  }
  else if(NR_val=='0')
  {
    digitalWrite(NR_Recv_Req, HIGH);//
    M_ON_req_flag = NOT_REQUESTED;
    Serial.println("MOTOR_ON NOT REQUESTED");
  }
}
void WATER_FLOW_MONITOR()
{
  Serial.print("Water flow: ");
  Serial.println(WaterFlowValue);
  if((WaterFlowValue > WF_MAX_VALUE) && (MC_val == '1'))
  {
    digitalWrite(Water_Flow_st, LOW);
    Serial.println("Water is flowing");
    //WaterFlow_flag = FLOW_START;
  }
  if(MC_val != '1')
  {
    digitalWrite(Water_Flow_st, HIGH);
    Serial.println("Water is not flowing");
  }
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
  // if ((S.substring(0, 5) == "token") && (S.substring(9, 12) == "not")&&(S.substring(13, 18) == "ready")&&(S.substring(31) == "expired")) 
  // {
  //   Serial.println("Restart ESP");
  //   ESP.restart();
  // }
}
//Restart test
/*void loop()
{
  unsigned long currentMillis = millis();
  Serial.println("came here");
  if ((currentMillis - previousMillis >=interval))
    {
      Serial.print(millis());
      Serial.println("Restarting ESP...");
      ESP.restart();
      //WiFi.disconnect();
      //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      previousMillis = currentMillis;
    }
    delay(500);
}*/
void loop()
{
    unsigned long currentMillis = millis();
    // read the value from the sensor:
    WaterFlowValue = analogRead(Water_FLow_an);
    WATER_FLOW_MONITOR();
    if(Firebase.get(firebaseData,"/IOT_Control_4Load_mod")) 
    {
      S_org=firebaseData.stringData();
      previousMillis_error = millis();
    }     
    else
    {
      Serial.println(S_error = firebaseData.errorReason());
      firebase_Error_check(S_error);
    }
    MOTOR_ON_LOGIC(S_org);
    S_org = ""; 
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

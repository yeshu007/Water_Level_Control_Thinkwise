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
#define API_KEY "AIzaSyAFKTHx10wCs8Lm2MSISzZjmFCuJJe98aw"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "water-level-controller-257a1-default-rtdb.firebaseio.com"

#define RELAY_ON  1
#define RELAY_OFF 0

#define REQ_RECV 0
#define REQ_NOT_RECV 1

#define FLOWING 0
#define NOT_FLOWING 1 


#define Max_Wait_count 50 // 'x'(around 15 sec for value 4) sec wait after motor on and report dry pump error

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

//unsigned long sendDataPrevMillis = 0;
//int count = 0;
bool signupOK = false;

int Power_Led_an = A0;   // select the input pin for the potentiometer
int powerValue = 0;  // variable to store the value coming from the sensor

uint8_t OF_Led = D0;
uint8_t OM_Led = D1;
uint8_t OL_Led = D2;

uint8_t NR_Recv_rq= D3;
uint8_t  WaterFlow_st= D4;

uint8_t MC_Relay = D8;

uint8_t SF_Led = D7;
uint8_t SL_Led = D6;
//uint8_t NSK_Trigger_st = D8;

uint8_t Water_Flow_Wait_count = 0;

int MC_Relay_Val = 0;
int OF_Led_Val = 0;
int OM_Led_Val = 0;
int OL_Led_Val = 0;
int SF_Led_Val = 0;
int SL_Led_Val = 0;

int NR_Recv_val = 0;
int WaterFlow_val = 0;
int NSK_Trigger_val = 0;

unsigned long previousMillis = 0;
unsigned long interval = 10000;//10sec

void ICACHE_RAM_ATTR isr1() 
{
    WaterFlow_val   = FLOWING;
    //Serial.println(" IN Water ISR2 ...............");
}
void ICACHE_RAM_ATTR isr2() 
{
    NR_Recv_val   = REQ_RECV;
    //Serial.println(" IN NR REQ ISR1 ...............");    
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

  pinMode(WaterFlow_st, INPUT_PULLUP);
  attachInterrupt(WaterFlow_st, isr1, FALLING);
  pinMode(NR_Recv_rq, INPUT_PULLUP);
  attachInterrupt(NR_Recv_rq, isr2, FALLING);
  pinMode(MC_Relay, INPUT);

  //pinMode(NSK_Trigger_st, INPUT); 
  pinMode(OF_Led, INPUT);
  pinMode(OM_Led, INPUT);
  pinMode(OL_Led, INPUT);
  pinMode(SF_Led, INPUT);
  pinMode(SL_Led, INPUT); 

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
void OF_VAL_UPLOAD()
{  
  if(OF_Led_Val == 0)
    {    if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/OF",0))
    {
       Serial.print("OF_Led_Val: OFF ");
       Serial.println(OF_Led_Val);
    }
    else 
    {
      Serial.println("Failed OF REASON: " + fbdo.errorReason());
    } }else if(OF_Led_Val == 1)
    {    if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/OF",1))
    {
       Serial.print("OF_Led_Val: ON ");
       Serial.println(OF_Led_Val);
    }
    else 
    {
      Serial.println("Failed OF REASON: " + fbdo.errorReason());
    } }else{}
}

void OM_VAL_UPLOAD()
{    
  if(OM_Led_Val == 0)
    {    if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/OM",0))
    {
       Serial.print("OM_Led_Val:  OFF ");
       Serial.println(OM_Led_Val);
    }
    else 
    {
      Serial.println("Failed OM REASON: " + fbdo.errorReason());
    } }else if(OM_Led_Val == 1)
    {    if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/OM",1))
    {
       Serial.print("OM_Led_Val:  ON ");
       Serial.println(OM_Led_Val);
    }
    else 
    {
      Serial.println("Failed OM REASON: " + fbdo.errorReason());
    } }else{}
}

void OL_VAL_UPLOAD()
{    
  if(OL_Led_Val == 0)
    {        if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/OL",0))
    {
       Serial.print("OL_Led_Val:  OFF ");
       Serial.println(OL_Led_Val);
    }
    else 
    {
      Serial.println("Failed OL REASON: " + fbdo.errorReason());
    } }else if(OL_Led_Val == 1)
    {        if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/OL",1))
    {
       Serial.print("OL_Led_Val:  ON ");
       Serial.println(OL_Led_Val);
    }
    else 
    {
      Serial.println("Failed OL REASON: " + fbdo.errorReason());
    } }else{}
}

void SF_VAL_UPLOAD()
{    
  if(SF_Led_Val == 0)
    {        if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/SF",0))
    {
       Serial.print("SF_Led_Val:  OFF ");
       Serial.println(SF_Led_Val);
    }
    else 
    {
      Serial.println("Failed SF REASON: " + fbdo.errorReason());
    }}else if(SF_Led_Val == 1)
    {        if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/SF",1))
    {
       Serial.print("SF_Led_Val:  ON ");
       Serial.println(SF_Led_Val);
    }
    else 
    {
      Serial.println("Failed SF REASON: " + fbdo.errorReason());
    } }else{}
}
void SL_VAL_UPLOAD()
{    
  if(SL_Led_Val == 0)
    {    
      if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/SL",0))
      {
        Serial.print("SL_Led_Val:  OFF ");
        Serial.println(SL_Led_Val);
      }
      else 
      {
        Serial.println("Failed SL REASON: " + fbdo.errorReason());
      } 
    }
    else if(SL_Led_Val == 1)
    {           
      if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/SL",1))
      {
        Serial.print("SL_Led_Val:  ON ");
        Serial.println(SL_Led_Val);
      }
      else 
      {
        Serial.println("Failed SL REASON: " + fbdo.errorReason());
      }  
    }else{}
}
void MC_NR_VAL_UPLOAD()
{
  if(MC_Relay_Val == RELAY_ON)
    {
      Water_Flow_Wait_count++;
      //if(relay_prev_state == RELAY_OFF)
      {
        //MC = 1
        if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/MC",1))
        {
          Serial.print("MC_Relay_Val:  RELAY_ON ");
          Serial.println(MC_Relay_Val);
        }
        else 
        {
          Serial.println("Failed MC REASON: " + fbdo.errorReason());
        }
        //relay_prev_state = RELAY_ON;
      }
    }
    else
    {
      //MC = 0
      //if(relay_prev_state == RELAY_ON)
      Water_Flow_Wait_count = 0;
      {
        if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/MC",0))
        {
          Serial.print("MC_Relay_Val:  RELAY_OFF ");
          Serial.println(MC_Relay_Val);
        }
        else 
        {
          Serial.println("Failed MC REASON: " + fbdo.errorReason());
        }
       // relay_prev_state = RELAY_OFF;
      }
    } 
}
void POWER_VAL_UPLOAD()
{    
  if(powerValue > 450)
  {    
    if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/PO",1))
    {
      Serial.print("powerValue: PowerON ");
      Serial.println(powerValue);
    }
    else 
    {
      Serial.println("Failed SL REASON: " + fbdo.errorReason());
    } 
  }
  else 
  {          
    if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/PO",0))
    {
      Serial.print("powerValue: PowerOFF ");
      Serial.println(powerValue);
    }
    else 
    {
      Serial.println("Failed SL REASON: " + fbdo.errorReason());
    }  
  }
}
void NR_RECV_VAL_UPLOAD()
{
  NR_Recv_val   = digitalRead(NR_Recv_rq);    
  if(NR_Recv_val == REQ_RECV)
  {  
    // if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/MC",2))
    // {
    //   Serial.println("MC = 2: REQ_RECV");
    // }
    // else 
    // {
    //   Serial.println("Failed SL REASON: " + fbdo.errorReason());
    // }   
    if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/NQ",1))
    {
      Serial.println("NR_Recv_val: REQ_RECV");
    }
    else 
    {
      Serial.println("Failed SL REASON: " + fbdo.errorReason());
    } 
  }
  else if(NR_Recv_val == REQ_NOT_RECV)
  {          
    if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/NQ",0))
    {
      Serial.println("NR_Recv_val: REQ_NOT_RECV");
    }
    else 
    {
      Serial.println("Failed SL REASON: " + fbdo.errorReason());
    }  
  }else{}
}
void WATER_FLOW_VAL_UPLOAD()
{
  //Below code for MC_relay->D5->input_pullup, but need to be careful with h/w damage if enabled 
  if(MC_Relay_Val == RELAY_ON)
  {
    Serial.print("Max_Wait_count: ");
    Serial.println(Max_Wait_count);
    
    if(Water_Flow_Wait_count < Max_Wait_count)
    {    
      if(WaterFlow_val == FLOWING)
      {
        Water_Flow_Wait_count = 0;    
        if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/WF",1))
        {
          Serial.print("WaterFlow_val: FLOWING ");
          Serial.println(WaterFlow_val);
        }
        else 
        {
          Serial.println("Failed SL REASON: " + fbdo.errorReason());
        } 
      }
      else if(WaterFlow_val == NOT_FLOWING)
      {          
        if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/WF",2))
        {
          Serial.print("WaterFlow_val: NOT_FLOWING ");
          Serial.println(WaterFlow_val);
        }
        else 
        {
          Serial.println("Failed SL REASON: " + fbdo.errorReason());
        }  
      }else{}
    }
    else if(Water_Flow_Wait_count >= Max_Wait_count)
    {
      if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/WF",3))
        {
          Serial.print("WaterFlow_val: Dry Pump ");
          Serial.println(WaterFlow_val);
        }
        else 
        {
          Serial.println("Failed SL REASON: " + fbdo.errorReason());
        }
    }
  }
  else if(MC_Relay_Val == RELAY_OFF)
  {
    if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/WF",0))
    {
      Serial.print("WaterFlow_val: MOTOR OFF ");
      Serial.println(WaterFlow_val);
    }
    else 
    {
      Serial.println("Failed SL REASON: " + fbdo.errorReason());
    }
  }
}
/*void NSK_TRIGGER_VAL_UPLOAD()
{    
  if(NSK_Trigger_val == TRIGGERRED)
  {    
    if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/NT",1))
    {
      Serial.print("NSK_Trigger_val:TRIGGERRED ");
      Serial.println(NSK_Trigger_val);
    }
    else 
    {
      Serial.println("Failed SL REASON: " + fbdo.errorReason());
    } 
  }
  else if(NSK_Trigger_val == NOT_TRIGGERRED)
  {          
    if (Firebase.RTDB.setInt(&fbdo, "IOT_Control_4Load_mod/NT",0))
    {
      Serial.print("NSK_Trigger_val:NOT_TRIGGERRED ");
      Serial.println(NSK_Trigger_val);
    }
    else 
    {
      Serial.println("Failed SL REASON: " + fbdo.errorReason());
    }  
  }else{}
}*/
void loop()
{
  unsigned long currentMillis = millis();

  MC_Relay_Val = digitalRead(MC_Relay);
  
  OF_Led_Val   = digitalRead(OF_Led);
  OM_Led_Val   = digitalRead(OM_Led);
  OL_Led_Val   = digitalRead(OL_Led);

  SF_Led_Val   = digitalRead(SF_Led);
  SL_Led_Val   = digitalRead(SL_Led);

  WaterFlow_val   = digitalRead(WaterFlow_st);
  //NSK_Trigger_val   = digitalRead(NSK_Trigger_st);
  Serial.print("WaterFlow_val : "); Serial.println(WaterFlow_val);
  // read the value from the sensor:
  powerValue = analogRead(Power_Led_an);
//#if 0
  if (Firebase.ready() && signupOK ) 
  {
      MC_NR_VAL_UPLOAD();
      NR_RECV_VAL_UPLOAD();

      WATER_FLOW_VAL_UPLOAD();
      NR_RECV_VAL_UPLOAD();

      OF_VAL_UPLOAD();
      NR_RECV_VAL_UPLOAD();

      OM_VAL_UPLOAD();
      NR_RECV_VAL_UPLOAD();

      OL_VAL_UPLOAD();
      NR_RECV_VAL_UPLOAD();

      SF_VAL_UPLOAD();
      NR_RECV_VAL_UPLOAD();

      SL_VAL_UPLOAD();
      NR_RECV_VAL_UPLOAD();

      POWER_VAL_UPLOAD();
      NR_RECV_VAL_UPLOAD();
    // }

  }
//#endif
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
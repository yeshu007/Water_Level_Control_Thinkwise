//#ifndef SGBotic_I2CPing_h
//#define SGBotic_I2CPing_h

#include "Wire.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


#if (ARDUINO >= 100)
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif


#define HC_SR04_I2C_ADDR 0x57

class HC_SR04_I2CPing {

  public:
    HC_SR04_I2CPing();
    
    float ping_cm();
    
    
  private:
    byte SR04_I2CADDR;  
    byte ds[3];
    float distance = 0;
    int i;
};


//#include "SGBotic_I2CPing.h"

// Define milliseconds delay for ESP8266 platform
#if defined(ESP8266)

  #include <pgmspace.h>
  #define _delay_ms(ms) delayMicroseconds((ms) * 1000)

// Use _delay_ms from utils for AVR-based platforms
#elif defined(__avr__)
  #include <util/delay.h>

// Use Wiring's delay for compability with another platforms
#else
  #define _delay_ms(ms) delay(ms)
#endif


HC_SR04_I2CPing::HC_SR04_I2CPing()
{
    SR04_I2CADDR = HC_SR04_I2C_ADDR;
    ds[0]=0;
    ds[1]=0;
    ds[2]=0;
    
    // Start I2C
    Wire.begin();
    
}

float HC_SR04_I2CPing::ping_cm()
{
    Wire.beginTransmission(SR04_I2CADDR);
    Wire.write(1);          //1 = cmd to start meansurement
    Wire.endTransmission();
    delay(120);             //1 cycle approx. 100mS. 
    i = 0;
    Wire.requestFrom(0x57,3);  //read distance       
    while (Wire.available())
    {
     ds[i++] = Wire.read();
    }        
    // Serial.print("ds[0] : ");
    // Serial.println(ds[0]);
    // Serial.print("ds[1] : ");
    // Serial.println(ds[1]);
    // Serial.print("ds[2] : ");
    // Serial.println(ds[2]);
    // distance = (unsigned long)(ds[0]) * 65536;
    // distance = distance + (unsigned long)(ds[1]) * 256;
    // distance = (distance + (unsigned long)(ds[2])) / 10000;

    distance = (float)(ds[0]) * 65536;
    distance = distance + (float)(ds[1]) * 256;
    distance = (distance + (float)(ds[2])) / 10000;
    
    //distance=(unsigned long)((ds[0]*65536+ds[1]*256+ds[2])/10000);  
    
    if ((1<=distance)&&(900>=distance))    //measured value between 1cm to 9meters
    {
        return distance;
    }else 
    {
        return 0;
    }
    
}    


HC_SR04_I2CPing HC_SR04_Yesh;

void setup() {

  Serial.begin(9600);
  lcd.init();                     // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(3,0); //(position,line)
  lcd.print("Hello, world!");
  lcd.setCursor(1,1);
  lcd.print("Project success");
  delay(1000);
}

void loop() {
  float dist = 0;
  dist = HC_SR04_Yesh.ping_cm();
  lcd.clear();
  lcd.setCursor(3,0); //(position,line)
  lcd.print("Distance : ");
  lcd.setCursor(1,1);
  lcd.print(dist);
  Serial.print("Distance : ");
  Serial.println(dist);
  delay(500);
}

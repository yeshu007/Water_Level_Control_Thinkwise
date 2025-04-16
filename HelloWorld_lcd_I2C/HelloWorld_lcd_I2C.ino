//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  lcd.begin();                      // initialize the lcd for Esp8266
  //lcd.init(); // for nano ant others
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(3,0); //(position,line)
  lcd.print("Hello, world!");
  lcd.setCursor(1,1);
  lcd.print("Project success");
}


void loop()
{
}

#include <Arduino.h>
#include <HCSR04.h>

byte triggerPin = 5;
byte echoPin = 6;
byte VCC = 4;
byte GND = 7;
// Floats to calculate distance
float duration, distance;

void setup () {
  Serial.begin(9600);
  pinMode(VCC, OUTPUT);
  pinMode(GND, OUTPUT);
  digitalWrite(VCC, HIGH);
  digitalWrite(GND, LOW);

  // Set pinmodes for sensor connections
  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT);
  //HCSR04.begin(triggerPin, echoPin);

}

float Sump_Water_Distance() {
 
  // Set the trigger pin LOW for 2uS
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
 
  // Set the trigger pin HIGH for 20us to send pulse
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(20); //delayMicroseconds(20);
 
  // Return the trigger pin to LOW
  digitalWrite(triggerPin, LOW);
 
  // Measure the width of the incoming pulse
  duration = pulseIn(echoPin, HIGH);
 
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

void loop () {
    //float Distance_float = Sump_Water_Distance();
  String S1 = "YESH";
  String S2 = "WANTH";
  String S3 = S1+S2;
  Serial.print("String S3 = ");
  Serial.println(S3);

  //double* distances = HCSR04.measureDistanceCm();
  // Serial.print("1: ");
  // Serial.print(distances[0]);
  // Serial.println(" cm");
  
  Serial.println("---");
  delay(500);
}
// Define connections to sensor
#define TRIGPIN 3
#define ECHOPIN 2
byte VCC = 4;
byte GND = 7;

#define MAX_TANK_DEPTH 150//in cm
#define ADJUSTER 30//in cm
#define sample_array_size 10 // amount of samples need o be sampled
#define MIN_DETECTION_DIST 22
#define MAX_VALUE 600
// Floats to calculate distance
float duration, distance;
float prev_sample, currrent_sample;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(VCC, OUTPUT);
  pinMode(GND, OUTPUT);
  digitalWrite(VCC, HIGH);
  digitalWrite(GND, LOW);
 
  // Set pinmodes for sensor connections
  pinMode(ECHOPIN, INPUT);
  pinMode(TRIGPIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
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

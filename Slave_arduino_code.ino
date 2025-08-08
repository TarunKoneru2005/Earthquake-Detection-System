#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Initialize the LCD with I2C address 0x27, 16 columns, and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define buzzer 2      // Buzzer will turn on only when the master confirms earthquake
#define led 4        // LED indicates tremor detection
#define buttonDec 5   // Button to decrease threshold
#define buttonInc 6   // Button to increase threshold  

#define x A2
#define y A1
#define z A0

struct sensorData
{
  byte sensorID;
  int signalValue;
};

sensorData sensor;

RF24 radio(9, 8); // CE, CSN Pins for NRF24L01
const byte masterAddress[6] = "MSTR1";
const byte myAddress[6] = "SLV03";


long start_tremor;
long start_eq;

//Variables
int xsample = 0;
int ysample = 0;
int zsample = 0;

/* Threshold values */
int mag_thresh = 50; //Default value, if the EEPROM is empty

#define samples 50
#define buzTime 10000  // Duration for which buzzer/LED stays on

void setup() 
{
  sensor.sensorID = 2; // 0 -> sensor1, 1 -> sensor2, 2 -> sensor3
  sensor.signalValue = 0;
  
  delay(100);
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  delay(1000);

  lcd.print("Initializing...");
  delay(2500);
  lcd.clear();

  lcd.print("EarthQuake ");
  lcd.setCursor(0, 1);
  lcd.print("Detector ");
  delay(3000);
  lcd.clear();
  lcd.print("System ");
  lcd.setCursor(0, 1);
  lcd.print("Initialized...");
  delay(2500);
  lcd.clear();
  lcd.print("Calibrating.....");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");
  delay(2500);

  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(buttonInc, INPUT_PULLUP);
  pinMode(buttonDec, INPUT_PULLUP);

  digitalWrite(led, LOW);
  noTone(buzzer);

  // Retrieve the last saved threshold value from EEPROM
  mag_thresh = EEPROM.read(0);
  if (mag_thresh < 10 || mag_thresh > 60) {
    mag_thresh = 50; // Reset to default if invalid value
  }

  for (int i = 0; i < samples; i++) 
  {
    xsample += analogRead(x);
    ysample += analogRead(y);
    zsample += analogRead(z);
  }

  xsample /= samples;
  ysample /= samples;
  zsample /= samples;

  lcd.clear();
  lcd.print("Calibration");
  lcd.setCursor(0, 1);
  lcd.print("Complete!");

  lcd.clear();
  lcd.print("Device Ready");
  delay(2500);
  lcd.clear();



  // NRF24L01 Setup
  radio.begin();
  radio.openWritingPipe(masterAddress);
  radio.openReadingPipe(1, myAddress);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(0x76);
  radio.startListening();
}

void loop() {
  int value1 = analogRead(x);
  int value2 = analogRead(y);
  int value3 = analogRead(z);
  
  int xValue = xsample - value1;
  int yValue = ysample - value2;
  int zValue = zsample - value3;
  
  int magnitude = sqrt(sq(xValue) + sq(yValue) + sq(zValue));

  // Adjust threshold using buttons
  if (digitalRead(buttonInc) == HIGH) 
  {
    mag_thresh += 5;
    EEPROM.write(0, mag_thresh); //Save updated value
    delay(300);
  }
  
  if (digitalRead(buttonDec) == HIGH) 
  {
    mag_thresh -= 5;
    EEPROM.write(0, mag_thresh); //save Updated value
    delay(300);
  }

  // Update LCD Display: Threshold values + X, Y, Z values
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Thres:"); lcd.print(mag_thresh);
  lcd.setCursor(9, 0);
  lcd.print(" Mag:"); lcd.print(magnitude);

  lcd.setCursor(0, 1);
  lcd.print("X:"); lcd.print(padValue(xValue));
  lcd.setCursor(6, 1);
  lcd.print("Y:"); lcd.print(padValue(yValue));
  lcd.setCursor(11, 1);
  lcd.print("Z:"); lcd.print(padValue(zValue));

  static bool tremorAlert = false;
  static bool earthquakeconfirmed = false;

  // Earthquake detection logic
  // !earthquakeconfirmed is added by me

  if (magnitude >= mag_thresh && !tremorAlert) 
  {
    start_tremor = millis();
    tremorAlert = true;

    Serial.print("Tremor Detected: X:"); Serial.print(xValue);Serial.print(",");
    Serial.print(" | Y:"); Serial.print(yValue);Serial.print(",");
    Serial.print(" | Z:"); Serial.print(zValue);Serial.print(",");
    Serial.print(" | Magnitude:"); Serial.println(magnitude);
    delay(800);

    lcd.clear();
    lcd.print("TREMOR");
    lcd.setCursor(0, 1);
    lcd.print("DETECTED!!");
    digitalWrite(led, HIGH); // LED turns ON for local tremor detection
    delay(10000);

    // Send magnitude data to master controller
    radio.stopListening();
    sensor.signalValue = 1;
    radio.write(&sensor, sizeof(sensor));
    Serial.println("Data Sent!!");
  } 


  // Check if master has confirmed earthquake
  radio.startListening();
  if (radio.available()) 
  {
    int confirmation_signal = 0;
    radio.read(&confirmation_signal, sizeof(confirmation_signal));
    Serial.print("Data_received. confirm_signal = ");
    Serial.println(confirmation_signal);
    if (confirmation_signal != 0)
    {
      lcd.clear();
      lcd.print("signal Recieved!!");
      delay(1500);
      lcd.clear();
      earthquakeconfirmed = true;
    }
  }

  // Activate buzzer only if master confirms earthquake
  if (earthquakeconfirmed) 
  {
    start_eq = millis();
    // Send X, Y, Z values to Serial Monitor
    Serial.print("Earthquake: X: "); Serial.print(xValue); Serial.print(",");
    Serial.print(" | Y: "); Serial.print(yValue); Serial.print(",");
    Serial.print(" | Z: "); Serial.print(zValue); Serial.print(",");
    Serial.print(" | Magnitude: "); Serial.println(magnitude);
    delay(800);

    lcd.clear();
    lcd.print("EARTHQUAKE !!!!");
    lcd.setCursor(0, 1);
    lcd.print("ALERT ACTIVATED");
    digitalWrite(led,HIGH);
    tone(buzzer, 200);
    delay(15000);
    lcd.clear();
  }

  if(tremorAlert && (millis() - start_tremor) >= buzTime)
  {
    tremorAlert = false;
    digitalWrite(led,LOW);
    lcd.clear();
    lcd.print("Alert OFF");
    delay(2700);
    lcd.clear();
    sensor.signalValue = 0;
  }

  if (earthquakeconfirmed && (millis() - start_eq) >= buzTime) 
  {
    earthquakeconfirmed = false;
    noTone(buzzer);
    digitalWrite(led,LOW);
    lcd.clear();
    lcd.print("Alarm OFF");
    delay(2700);
    lcd.clear();
    sensor.signalValue = 0;
  }

  Serial.print("Normal: | X:"); Serial.print(xValue);Serial.print(",");
  Serial.print(" | Y:"); Serial.print(yValue);Serial.print(",");
  Serial.print(" | Z:"); Serial.print(zValue);Serial.print(",");
  Serial.print(" | Magnitude:"); Serial.println(magnitude);

  delay(800);
  radio.stopListening();

}

// Function to format sensor values for LCD display 
String padValue(int value) 
{
  String strValue = String(value);
  while (strValue.length() < 5) 
  {
    strValue += " ";
  }
  return strValue;
} 
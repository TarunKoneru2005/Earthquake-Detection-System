#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 8); // CE, CSN pins

const byte addressMSTR[6] = "MSTR1"; 
const byte slaveAddresses[3][6] = {"SLV01" , "SLV02" , "SLV03"};

#define MAX_SENSORS 3
#define TIME_WINDOW 25000
#define LED 4
 
struct sensorData {
  byte sensorID;
  int signalValue;
};

sensorData received_Data;
unsigned long tremorTimes[MAX_SENSORS];
bool sensorTriggered[MAX_SENSORS];
unsigned long lastConfirmationTime = 0;
int confirmation_signal = 0;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(0x76);

  radio.openReadingPipe(1, addressMSTR);
  radio.startListening();

  Serial.println("Master Controller Ready");
  pinMode(LED, OUTPUT);

  for (int i = 0; i < MAX_SENSORS; i++) {
    sensorTriggered[i] = false;
    tremorTimes[i] = 0;
  }

  received_Data.sensorID = 0;
  received_Data.signalValue = 0;
}

void loop() {
  if (radio.available()) {
    radio.read(&received_Data, sizeof(received_Data));
    Serial.print("Received from Sensor-"); Serial.print(received_Data.sensorID + 1);
    Serial.print(": signal_value = "); Serial.println(received_Data.signalValue);

    if (received_Data.signalValue == 1) {
      byte ID = received_Data.sensorID;
      if (ID < MAX_SENSORS && (millis() - tremorTimes[ID] > TIME_WINDOW)) {
        sensorTriggered[ID] = true;
        tremorTimes[ID] = millis();
      }
    }
  }

  int count = 0;
  for (int i = 0; i < MAX_SENSORS; i++) {
    if (sensorTriggered[i] && (millis() - tremorTimes[i] <= TIME_WINDOW)) {
      count++;
    }
  }

  Serial.print("Tremor Count = "); Serial.println(count);
  delay(2000);

  // If 2 or more tremors confirmed, alert all slaves
  if (count >= 2 && millis() - lastConfirmationTime > TIME_WINDOW) {
    Serial.println("Earthquake Confirmed! Sending alert to sensors...");
    confirmation_signal = 1;

    radio.stopListening(); // Switch to write mode

    for (int i = 0; i < MAX_SENSORS; i++) {
      radio.openWritingPipe(slaveAddresses[i]); // Send to each slave
      bool success = radio.write(&confirmation_signal, sizeof(confirmation_signal));
      Serial.print("Sent to Slave "); Serial.print(i + 1); 
      Serial.println(success ? " ✅" : " ❌");
      delay(100);
    }

    digitalWrite(LED, HIGH);
    delay(20000);
    digitalWrite(LED, LOW);

    for (int i = 0; i < MAX_SENSORS; i++) {
      sensorTriggered[i] = false;
      tremorTimes[i] = 0;
    }

    lastConfirmationTime = millis();  
    confirmation_signal = 0;          // Reset after sending
    radio.startListening();           // Resume listening
  }
}
# Earthquake-Detection-System
## Introduction
The system uses three remote nodes equipped with ADXL-335 accelerometer sensors and 
NRF24L01 modules to wirelessly transmit data to a central master controller (Arduino Nano). 
An earthquake is confirmed only if two or more nodes detect tremors simultaneously, 
minimizing false alerts. Alerts are issued through a buzzer and LED, while an LCD displays 
real-time data. The setup also includes buttons for adjusting sensitivity and logs data using 
Microsoft Excel’s Data Streamer. This system is designed for easy deployment in areas 
lacking conventional seismic monitoring infrastructure.
## System Architechture
The Earthquake Detection and Alert System is built around a distributed network of sensor 
nodes and a central master controller.  
Each node is responsible for sensing vibrations, and the master controller makes the final 
decision on triggering alerts. The communication between these nodes is done wirelessly via 
the NRF24L01 modules. 
### 1. Hardware Requirements
**Accelerometer Sensors:** Detect ground vibrations at each slave node.

**NRF24L01 Modules:** Provide wireless communication between each slave node and the 
master Arduino Nano.

**Arduino Nano (Master Controller):** Receives vibration data from slave nodes and 
determines whether an earthquake has occurred based on predefined logic. 

**Buzzer and LED:** Provide immediate audio-visual alerts in case of confirmed seismic activity. 

**I2C LCD Display:** Displays current status and sensor data in real-time. 

**Push Buttons:** Allow manual adjustment of vibration sensitivity thresholds.

### 2. Software Requirements
**Arduino IDE:** Used to write, compile and upload the code for the Arduino nano 
microcontroller. 

**Visual Studio Code:** Used for typing and running the python script for data visualisation. 

**Matplotlib:** Python library used for plotting the serial data received from the microcontroller. 

**Pyserial:** Python library used for reading the serial data from the controller and using it in the 
python script.   

**Microsoft Excel (Data Streamer):** Used to store the earthquake data received from the 
microcontroller in an Excel sheet for further analysis and other use cases. 

## Working Principle
The Earthquake Detection and Alert System operates on the principle of multi-node vibration 
sensing and consensus-based alert triggering, ensuring accurate and reliable detection of 
seismic events. 
### 1. Vibration Detection: 
Each of the three slave nodes is equipped with an accelerometer sensor that continuously 
measures ground vibrations.  
The sensor outputs analog values representing motion intensity, which are processed by the 
microcontroller at each node. The info is displayed on the I2C LCD Display present at each 
sensor node. 
### Wireless Communication: 
The slave nodes use NRF24L01 modules to transmit the vibration data wirelessly to a central 
master controller (Arduino Nano). These modules operate on the 2.4 GHz band and support 
reliable, low-latency communication. 
### 3. Event Confirmation Logic: 
The master controller receives vibration readings from all three nodes. An earthquake event is 
confirmed only when two or more nodes detect vibrations that exceed the predefined 
threshold within a short time window. This approach reduces false positives caused by local 
disturbances. 
### 4. Alert Mechanism 
Once an earthquake is confirmed: 
 A buzzer is activated to provide an audible warning and an LED flashes to give a visual 
alert. 
 An LCD display shows the status and vibration levels from each node. 
### 5. Dynamic Threshold Adjustment: 
Two push buttons are integrated into the system, allowing users to manually increase or 
decrease the vibration threshold. This provides flexibility to adapt the system to varying 
environmental conditions. 
### 6. Data Logging and Visualization: 
The Arduino Nano sends vibration data to a connected computer via serial communication. 
Using Microsoft Excel’s Data Streamer add-in, this data is logged in real-time and can be 
used for further analysis and monitoring. 
The visualization of the seismic data is done using a python script. This python script uses the 
dynamic plotting features available in the Matplotlib library and the serial data from the 
sensor node is received by the script using the Pyserial library. 

/* Landslide monitoring with LoRaWAN.
 * 
 * Board: Arduino Leonardo
 * Lora module: MTDOT 915
 * Sensor: Extension meter
 * 
 * Author: J. Barthélemy
 * Version: 1.0
 */

#include "LowPower.h"

// Parameters
int   WIPER_PIN  = 0;       # Wiper pin for the sensor
int   GROUND_PIN = 2;       # Ground pin for the sensor
int   N_READINGS = 1000;    # Number of readings
float VREF       = 5.120;   # To be calibrated for each board!
float calib      = 1004.88; # To be calibrated for each extension meter

// Setup: setting the serial interfaces and joining the TTN network
void setup() {

  Serial.begin(9600);
  Serial1.begin(115200); 
  join_lora();

}

// Joining the lorawan network
void join_lora() {

  send_at_command("AT","OK",500);
  send_at_command("AT","OK",500);
  send_at_command("AT+PN=1","OK",500);
  send_at_command("AT+FSB=2","OK",500);
  send_at_command("AT+NJM=1","OK",500);
  send_at_command("AT+NI=0,=== APP EUI ===","ID",1000);
  send_at_command("AT+NK=0,=== APP KEY ===","Key",1000);
  send_at_command("AT+TXP=20","OK",500);
  send_at_command("AT+JOIN","OK",10000);

}

// Sending AT commands to the LoraWan module
void send_at_command(const char* cmd, const char* expected_ans, int waiting_period) {

  // cleaning buffer
  while (Serial1.available() > 0) Serial1.read();

  char response[200];
  memset(response, '\0', 200);

  bool ack = false;
  while (ack == false) {
 
    // sending the command and waiting
    Serial1.println(cmd);
    delay(waiting_period);
    
    // receiving the answer 
    uint8_t i = 0;
    while (Serial1.available() > 0) {
      response[i] = Serial1.read();
      i++;      
    }
    Serial.write(response);

    // check received answer with expectation
    if (strstr(response, expected_ans) != NULL) {
      ack = true;
    }

  }

}

// Sending the data in argument using Cayenne LPP format for analog input
void send_data_bin(float val_1) {

  // convert float to int
  unsigned int val = (int) val_1 * 10;
  // convert int to a char of hex
  char val_hex[4];
  memset(val_hex,'\0', 4);
  sprintf(val_hex,"%04x", val);

  // prepare the payload
  String string_channel = String("01");
  String string_type    = String("02"); 
  String string_val(val_hex);
  String string_payload = String("AT+SENDB=" + string_channel + string_type + string_val);

  // copy it into a char array
  char at_payload[string_payload.length() + 1];
  memset(at_payload,'\0', string_payload.length() + 1);
  string_payload.toCharArray(at_payload, string_payload.length() + 1);

  // send it via lorawan
  send_at_command(at_payload,"OK",10000);

}

// Main loop: reading data, sending it and sleep
void loop() {

  float dist = 0;
  float ground_val = 0;

  // reading the measures and average them
  for( int i = 0; i < N_READINGS; i++) {
    float wiper_val = analogRead(WIPER_PIN) * VREF;  
    dist = dist + (calib * ((wiper_val - ground_val) / (VREF - ground_val)));
    dist = dist / 2;
  }
  // scaling the measure
  dist = round(dist / 1000.0);

  //send_data(dist);
  send_data_bin(dist);
  
  // sleeping for 5 mintures
  for( int j = 0; j < 35; j++) {
    LowPower.powerDown(SLEEP_8S,ADC_OFF, BOD_OFF);
  }
  
}

#include "LowPower.h"


int WIPER_PIN      = 0;
int GROUND_PIN     = 2;
int N_READINGS     = 1000;

float VREF = 5.120;
float calib = 1004.88;

void setup() {

  Serial.begin(9600);
  Serial1.begin(115200);
 
  join_lora();

}


// joining the lorawan network
void join_lora() {

  send_at_command("AT","OK",500);
  send_at_command("AT","OK",500);
  send_at_command("AT+PN=1","OK",500);
  send_at_command("AT+FSB=2","OK",500);
  send_at_command("AT+NJM=1","OK",500);
  send_at_command("AT+NI=0,70B3D57EF0006A30","ID",1000);
  send_at_command("AT+NK=0,11735D89A02090D70EA8FE587DAA6E2D","Key",1000);
  send_at_command("AT+TXP=20","OK",500);
  send_at_command("AT+JOIN","OK",10000);

}


// sending AT commands to the LoraWan module
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

void send_data(float val_1) {

  // prepare the payload
  char at_payload[20];
  memset(at_payload,'\0', 20);
  const char sep[] = ";";
  strcat(at_payload,"AT+SEND=");

  // ... adding val_1
  char val_1_str[8];
  dtostrf(val_1, -6, 1, val_1_str);
  strcat(at_payload, val_1_str);

  // send it via lorawan
  send_at_command(at_payload,"OK",10000);

}

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

  //Serial.println("Payload:");
  //Serial.println(string_payload);

  // send it via lorawan
  send_at_command(at_payload,"OK",10000);

}

void loop() {

  float dist = 0;
  float ground_val = 0;

  //float dist = calib * ((wiper_val - ground_val) / (VREF - ground_val));
  
  for( int i = 0; i < N_READINGS; i++) {
    float wiper_val = analogRead(WIPER_PIN) * VREF;  
    dist = dist + (calib * ((wiper_val - ground_val) / (VREF - ground_val)));
    dist = dist / 2;
  }
  dist = round(dist / 1000.0);

  //Serial.print("Distance=");
  //Serial.println(dist);

  //send_data(dist);
  send_data_bin(dist);
  
  //delay(2875);
  //delay(1000 * 60 * 5) 

  for( int j = 0; j < 35; j++) {
    LowPower.powerDown(SLEEP_8S,ADC_OFF, BOD_OFF);
  }
  
}

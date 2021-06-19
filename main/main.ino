#include <Wire.h>
#include <CAN.h>

//RELAYS CLOSED WHEN HIGH SIGNAL
//Precharge 

//#define NLG5_ST 0x610
#define NLG5_CTL 0x618
#define NLG5_ACT_I 0x611

#define FEATHER_LED 13
#define PIN_RELAY_PRECHARGE 5
#define PIN_RELAY_HV 6

#define PRECHARGE_SUCCESS_COUNT 10

const int NLG5_VOLTAGE_MASK = 0x00FF0000;

float pack_voltage, prev_pack_voltage;
uint8_t charge_enable = 0;
uint8_t precharge_counter = 0;
uint8_t precharge_enable = 0;

void setup() {

  //Serial.begin(10000);
  //Serial.println("INFO: Feather initializing...\r\n");
  
  pinMode(FEATHER_LED, OUTPUT);
  digitalWrite(FEATHER_LED, HIGH);

  //Turn relay on when high
  pinMode(PIN_RELAY_PRECHARGE, OUTPUT);
  digitalWrite(PIN_RELAY_PRECHARGE, HIGH);
  
  pinMode(PIN_RELAY_HV, OUTPUT);
  
  if (!CAN.begin(500000)) {
    //Serial.println("ERROR: CAN.begin failed!\r\n");
    while(1) {
      digitalWrite(FEATHER_LED, LOW);
      delay(500);
      digitalWrite(FEATHER_LED, HIGH);
      delay(500);
    }
  }

}

void loop() {

  int packet_size = CAN.parsePacket();
  int received_id = CAN.packetId();

  if(packet_size) {
    //Serial.print("\r\n----------\r\nReceived CAN packet of size: ");
    //Serial.print(packet_size);
    //Serial.print("\r\nWith an ID of: ");
    //Serial.print(received_id);

    switch(received_id) {
      case NLG5_ACT_I: {
        //Serial.print("\r\nReceived ID = NLG5_ACT_I: ");
        uint8_t received_data[packet_size];
        for (uint8_t i=0; i<packet_size; i++) {
          received_data[i] = CAN.read();
          //Serial.print("0x");
          //Serial.print(received_data[i], HEX);
          //Serial.print(", ");
        }
        prev_pack_voltage = pack_voltage;
        pack_voltage = (received_data[4]| received_data[5] << 8)/100.0;
        //Serial.print("\r\nPack voltage: ");
        //Serial.print(pack_voltage);
  
        //If stable voltage success, increment success counter
        if (pack_voltage == prev_pack_voltage) precharge_counter++;
        //Reset counter, voltage fluctuation
        else precharge_counter = 0;
        if (precharge_counter >= PRECHARGE_SUCCESS_COUNT-1) charge_enable = 1;
        break;
      } 
      
    }
    
  }

  if (charge_enable) {
     delay(500);
     //Serial.print("\r\n\r\nINFO: Charge enabled!\r\n");
     digitalWrite(PIN_RELAY_HV, HIGH);
     //Do charge sequence
     while(1);
  }
  /*
  CAN.beginPacket(NLG5_CTL);
  CAN.write(0xC); // Bit 2 and 3 of 0x618, request voltage
  CAN.endPacket();
  */
  
}

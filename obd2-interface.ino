#include <global.h>
#include <Canbus.h>
#include <mcp2515_defs.h>
#include <mcp2515.h>
#include <defaults.h>

String UserInput;
char canBuffer[456];

void sendMessage(unsigned char mode, unsigned char pid, char *buffer) {
  // build can message
  tCAN message;
  message.id = PID_REQUEST;
  message.header.rtr = 0;
  message.header.length = 8;
  message.data[0] = 0x02;
  message.data[1] = mode;
  message.data[2] = pid;
  message.data[3] = 0x00;
  message.data[4] = 0x00;
  message.data[5] = 0x00;
  message.data[6] = 0x00;
  message.data[7] = 0x00;

  mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);

  if (mcp2515_send_message(&message)) {
    if (mcp2515_check_message()) {
      if (mcp2515_get_message(&message)) {
        sprintf(buffer, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", message.id, message.data[0], message.data[1], message.data[2], message.data[3], message.data[4], message.data[5], message.data[6], message.data[7]);
      }
    }
  }
}

// returns true on correct input
bool checkInput(String input) {
  String val1 = input.substring(0, 4);
  String val2 = input.substring(5);
  if (val1.substring(0,2) == "0x" && val2.substring(0,2) == "0x") {
    if (!isHexadecimalDigit(val1.charAt(2)) || !isHexadecimalDigit(val1.charAt(3))) {
      return false;
    }
    if (!isHexadecimalDigit(val2.charAt(2)) || !isHexadecimalDigit(val2.charAt(3))) {
      return false;
    }
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(9600);
  Serial.println("obd2-interface");
  Serial.println("--------------");
  if (Canbus.init(CANSPEED_500)) {
    Serial.println("can init successful");
  } else {
    Serial.println("can init failed");
    return;
  }

  Serial.println("Enter Mode and PID for your request in hex like this: 0x01 0x0D");
}

void loop() {
 while(Serial.available()) {
   UserInput = Serial.readString();
   Serial.println("> " + UserInput);
   
   if (!checkInput(UserInput)) {
     Serial.println("Error: wrong format");
     break;
   }
   
   char input[16];
   char* pEnd;
   UserInput.toCharArray(input, 16);
   int mode = strtol(input, &pEnd, 16);
   int pid = strtol(pEnd, &pEnd, 16);
   
   /* check entry for dev
   char outstring[32];
   sprintf(outstring, "Mode: %d PID: %d\n", mode, pid);
   Serial.print(outstring);
   */
   char buffer[64];
   sendMessage(mode, pid, buffer);
   if (buffer[0] == 0) {
    Serial.println("Error: could not get message");
   } else {
    Serial.println(buffer);
   }
 }
}

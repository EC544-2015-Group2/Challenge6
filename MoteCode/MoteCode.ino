#include <XBee.h>
#include <SoftwareSerial.h>

#define PIN_BLUE_LED    11   // leader
#define PIN_RED_LED     12    // infected follower
#define PIN_GREEN_LED   13  // follower

#define MSG_ELECTION    0xB0
#define MSG_INFECTION   0xB1
#define MSG_CLEAR       0xB2
#define MSG_DISCOVERY   0xB3

XBee xbee = XBee();
SoftwareSerial xbeeSerial(2, 3);
ZBRxResponse rxResponse = ZBRxResponse();

uint16_t myAddress16;
uint16_t listAddress16[10];
int state = 0;
bool infected = false;

void setup() {
  Serial.begin(57600);
  xbeeSerial.begin(57600);
  xbee.begin(xbeeSerial);
  delay(2000);

  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(BLUE_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  do{
    do  xbee.send(AtCommandRequest((uint8_t*){'M','Y'}));
    while (!xbee.readPacket(5000) || xbee.getResponse().getApiId != AT_COMMAND_RESPONSE);

    AtCommandResponse atResponse = AtCommandResponse();
    xbee.getResponse().getAtCommandResponse(atResponse);
  } while (!atResponse.isOk());

  myAddress16 = atResponse.getValue(0) << 8 + atResponse.getValue(1);
}

void loop() {
  xbee.readPacket();
  if (xbee.getResponse().isAvailable() && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
    xbee.getResponse().getZBRxResponse(rxResponse);
    if (state == 2 && > rxResponse.getRemoteAddress16() > myDevice16) {
      txRequest.setAddress64(0x00000000, 0x0000FFFF);
      txRequest.setPayload(ELECTION);
      txRequest.setPayloadLength(1);
      xbee.send(txRequest);
      state = 1;
    } else if (state != 2 && rxResponse.getData(0) == INFECTION) {
      digitalWrite(BLUE_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      infection = true;
    } else {
      state = 0;
    }
  }

// Anything below this line has not been reviewed. 

  xbee.readPacket();
  if (xbee.getResponse().isAvailable() && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
    xbee.getResponse().getZBRxResponse(rxResponse);

  }

 

  switch (state) {
    case 0:
      // following
      xbee.readPacket();
      if (xbee.getResponse().isAvailable() && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
        xbee.getResponse().getZBRxResponse(rxResponse);
        int address16 = rxResponse.getRemoteAddress16();
        if (sizeof(moteIDs == 0)) {
          moteIDs[0] = address64;
        } else {
          for (int i = 0; i < sizeof(moteIDs); i++) {
            if (moteIDs[i] == address64) {
              break;
            } else {
              if (i == sizeof(moteIDs) - 1) {
                moteIDs[sizeof(moteIDs)] = address64;
              }
            }
          }
        }
        if (rxResponse.getData(0) == "INFECT") {
          digitalWrite(RED_LED, HIGH);
          digitalWrite(GREEN_LED, LOW);
        }

      }
  }
  break;

case 1:
  // waiting for reply -  can probably replace with timeout
  break;
case 2:
  // leading
  break;
case 3:
  // waiting for victory msg - can probably replace with timeout
  break;

}
}


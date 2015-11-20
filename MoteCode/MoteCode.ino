#include <XBee.h>
#include <SoftwareSerial.h>

XBee xbee = XBee();
SoftwareSerial xbeeSerial(2, 3);
ZBRxResponse rxResponse = ZBRxResponse();
ZBTxRequest txRequest = ZBTxRequest();

int myDevice16;
int moteIDs[1];
int state = 0;

const uint8_t ELECTION = 0xB0,
              INFECTION = 0xB1,
              CLEAR = 0xB2;

boolean infected = false;

const int blueLED = 11,   // leader
          redLED = 12,    // infected follower
          greenLED = 13;  // follower

void setup() {
  Serial.begin(57600);
  xbeeSerial.begin(57600);
  xbee.begin(xbeeSerial);
  delay(2000);

  pinMode(blueLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  digitalWrite(blueLED, LOW);
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
}

void loop() {
  xbee.readPacket();
  if (xbee.getResponse().isAvailable() && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
    xbee.getResponse().getZBRxResponse(rxResponse);
    int address16 = rxResponse.getRemoteAddress16();
    if (state == 2 && address16 > myDevice16) {
      txRequest.setAddress64(0x00000000, 0x0000FFFF);
      txRequest.setPayload(ELECTION);
      txRequest.setPayloadLength(1);
      xbee.send(txRequest);
      state = 1;
    } else if (state != 2 && rxResponse.getData(0) == INFECTION) {
      digitalWrite(blueLED, LOW);
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, LOW);
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
          digitalWrite(redLED, HIGH);
          digitalWrite(greenLED, LOW);
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


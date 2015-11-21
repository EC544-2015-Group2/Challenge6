#include <XBee.h>
#include <SoftwareSerial.h>

#define PIN_BLUE_LED              11    // leader
#define PIN_RED_LED               12    // infected
#define PIN_GREEN_LED             13    // clear

#define ELECTION_REPLY_TIMEOUT    1000
#define ELECTION_VICTORY_TIMEOUT  2000

const uint8_t MSG_ELECTION = 0xB0,
              MSG_ACK = 0xB1,
              MSG_VICTORY = 0xB2,
              MSG_INFECTION = 0xB3,
              MSG_CLEAR = 0xB4,
              MSG_DISCOVERY = 0xB5;

const uint8_t myCommand[] = {'M', 'Y'};

XBee xbee = XBee();
SoftwareSerial xbeeSerial(2, 3);
ZBRxResponse rxResponse = ZBRxResponse();
ZBTxRequest txRequest;
AtCommandRequest atRequest = AtCommandRequest((uint8_t*)myCommand);
AtCommandResponse atResponse;

uint32_t myAddress64;   // 32-bit integer as the first 32 bits are same for all Xbee devices (0013a200)
uint32_t listAddress64[10];
uint32_t leaderAddress64;
uint8_t numDevices = 0;
bool isLeader = false;
bool isInfected = false;

void setup() {
  Serial.begin(57600);
  xbeeSerial.begin(57600);
  xbee.begin(xbeeSerial);
  delay(2000);

  initLedPins();
  myAddress64 = getMyAddress64();
  broadcastMyAddress64();
}

void loop() {
  if (xbee.readPacket(1) && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
    xbee.getResponse().getZBRxResponse(rxResponse);
    if (rxResponse.getRemoteAddress64().getLsb() > myAddress64) {
      isLeader = false;
      leaderAddress64 = rxResponse.getRemoteAddress64().getLsb();
    }
    switch (rxResponse.getData(0)) {
      case MSG_DISCOVERY:
        uint32_t remoteAddress64;
        for (int i = 1; i < 5; i++) remoteAddress64 |= rxResponse.getData(i) << (5 - i);
        for (int i = 0; i < numDevices; i++)
          if (listAddress64[i] == remoteAddress64)  break;
          else listAddress64[numDevices++] = remoteAddress64;
        break;

      case MSG_ELECTION:
        txRequest = ZBTxRequest(XBeeAddress64(0x0013a200, rxResponse.getRemoteAddress64().getLsb()), (uint8_t*) &MSG_ACK, 1);
        xbee.send(txRequest);
        beginElection();
        break;

      case MSG_VICTORY:
        uint32_t newLeaderAddress64 = 0;
        for (int i = 1; i < 5; i++)  newLeaderAddress64 |= rxResponse.getData(i) << (5 - i);
        if (newLeaderAddress64 < myAddress64)  beginElection();
        else leaderAddress64 = newLeaderAddress64;
        break;
    }
  }
}

void initLedPins(void) {
  pinMode(PIN_BLUE_LED, OUTPUT);
  pinMode(PIN_RED_LED, OUTPUT);
  pinMode(PIN_GREEN_LED, OUTPUT);

  digitalWrite(PIN_BLUE_LED, LOW);
  digitalWrite(PIN_RED_LED, LOW);
  digitalWrite(PIN_GREEN_LED, HIGH);
}

uint32_t getMyAddress64(void) {
  do {
    do    xbee.send(atRequest);
    while (!xbee.readPacket(5000) || xbee.getResponse().getApiId() != AT_COMMAND_RESPONSE);

    xbee.getResponse().getAtCommandResponse(atResponse);
  } while (!atResponse.isOk());

  return atResponse.getValue()[0] << 8 + atResponse.getValue()[1];
}

void broadcastMyAddress64(void) {
  uint8_t discoverMessagePayload[5];
  discoverMessagePayload[0] = MSG_DISCOVERY;
  for (int i = 1; i < 5; i++)  discoverMessagePayload[i] = myAddress64 >> 8 * (i - 1) & 0xFF;
  txRequest = ZBTxRequest(XBeeAddress64(0x00000000, 0x0000FFFF), discoverMessagePayload, 5);
  xbee.send(txRequest);
}

void beginElection(void) {
  uint8_t countDevices = 0;
  bool ack = false;
  for (int i = 0; i < numDevices; i++)
    if (listAddress64[i] > myAddress64) {
      txRequest = ZBTxRequest(XBeeAddress64(0x0013a200, listAddress64[i]), (uint8_t*)MSG_ELECTION, 1);
      xbee.send(txRequest);
      countDevices++;
    }
  if (countDevices > 0) {
    uint32_t timeout = millis() + ELECTION_REPLY_TIMEOUT;
    while (millis() < timeout) {
      if (xbee.readPacket(1) && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
        xbee.getResponse().getZBRxResponse(rxResponse);
        switch (rxResponse.getData(0)) {
          case MSG_ELECTION:
            txRequest = ZBTxRequest(XBeeAddress64(0x0013a200, rxResponse.getRemoteAddress64().getLsb()), (uint8_t*) MSG_ACK, 1);
            xbee.send(txRequest);
            break;
          case MSG_ACK:
            timeout = millis() + ELECTION_VICTORY_TIMEOUT;
            ack = true;
            break;
          case MSG_VICTORY:
            isLeader = false;
            leaderAddress64 = 0;
            for (int i = 1; i < 5; i++) leaderAddress64 |= rxResponse.getData(i) << (5 - i);
            return;
        }
      }
    }
  }
  if (ack) beginElection();
  else {
    uint8_t victoryMessagePayload[5];
    victoryMessagePayload[0] = MSG_VICTORY;
    for (int i = 1; i < 5; i++)  victoryMessagePayload[i] = myAddress64 >> 8 * (i - 1) & 0xFF;
    txRequest = ZBTxRequest(XBeeAddress64(0x00000000, 0x0000FFFF), victoryMessagePayload, 5);
    xbee.send(txRequest);
    isLeader = true;
    leaderAddress64 = myAddress64;
  }
}

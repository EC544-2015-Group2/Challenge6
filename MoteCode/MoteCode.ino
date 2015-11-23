#include <XBee.h>
#include <SoftwareSerial.h>

#define PIN_BLUE_LED              11    // leader
#define PIN_RED_LED               12    // infected
#define PIN_GREEN_LED             13    // clear

#define ELECTION_REPLY_WAIT_PERIOD      1000
#define ELECTION_VICTORY_WAIT_PERIOD    2000
#define LEADER_HEARTBEAT_PERIOD         3000

const uint8_t MSG_ELECTION = 0xB0,
              MSG_ACK = 0xB1,
              MSG_VICTORY = 0xB2,
              MSG_INFECTION = 0xB3,
              MSG_CLEAR = 0xB4,
              MSG_DISCOVERY = 0xB5,
              MSG_HEARTBEAT = 0xB6;

const uint8_t slCommand[] = {'S', 'L'};

XBee xbee = XBee();
SoftwareSerial xbeeSerial(2, 3);
ZBRxResponse rxResponse = ZBRxResponse();
ZBTxRequest txRequest;
AtCommandRequest atRequest = AtCommandRequest((uint8_t*)slCommand);
AtCommandResponse atResponse;

uint32_t myAddress64, leaderAddress64, remoteAddress64;   // 32-bit integer as the first 32 bits are same for all Xbee devices (0013a200)
uint32_t listAddress64[10];
uint8_t numDevices = 0;
bool isInfected = false;

bool isElecting = false, isAcknowledged = false;
uint32_t electionTimeout, leaderHeartbeatTimeout;
uint8_t heartbeatsLost = 0;

void setup() {
  Serial.begin(57600);
  xbeeSerial.begin(57600);
  xbee.begin(xbeeSerial);
  delay(2000);

  initLedPins();
  getMyAddress64();
  leaderAddress64 = myAddress64;
  sendCommand(0x0000FFFF, (uint8_t*)&MSG_DISCOVERY, 1);
  leaderHeartbeatTimeout = millis() + LEADER_HEARTBEAT_PERIOD;
}

void loop() {
  readAndHandlePackets();
  if (isElecting && millis() > electionTimeout) {
    isElecting = false;
    if (isAcknowledged) beginElection();
    else {
      sendCommand(0x0000FFFF, (uint8_t*)&MSG_VICTORY, 1);
      leaderAddress64 = myAddress64;
      digitalWrite(PIN_BLUE_LED, HIGH);
    }
  }
  if (millis() > leaderHeartbeatTimeout) {
    if (leaderAddress64 == myAddress64) {
      sendCommand(0x0000FFFF, (uint8_t*) &MSG_HEARTBEAT, 1);
      leaderHeartbeatTimeout = millis() + LEADER_HEARTBEAT_PERIOD / 2;
    } else beginElection();
  }
}

void initLedPins(void) {
  pinMode(PIN_BLUE_LED, OUTPUT);
  pinMode(PIN_RED_LED, OUTPUT);
  pinMode(PIN_GREEN_LED, OUTPUT);

  digitalWrite(PIN_BLUE_LED, HIGH);
  digitalWrite(PIN_RED_LED, LOW);
  digitalWrite(PIN_GREEN_LED, HIGH);
}

void getMyAddress64(void) {
  do {
    do    xbee.send(atRequest);
    while (!xbee.readPacket(5000) || xbee.getResponse().getApiId() != AT_COMMAND_RESPONSE);

    xbee.getResponse().getAtCommandResponse(atResponse);
  } while (!atResponse.isOk());
  memcpy(&myAddress64, atResponse.getValue(), 4);
}

void sendCommand(uint32_t destinationAddress64, uint8_t* payload, uint8_t length) {
  Serial.print("MSG_OUT:");
  Serial.print(destinationAddress64);
  Serial.print(":");
  switch (payload[0]) {
    case MSG_ELECTION: Serial.println("ELECTION"); break;
    case MSG_ACK: Serial.println("ACK"); break;
    case MSG_VICTORY: Serial.println("VICTORY"); break;
    case MSG_INFECTION: Serial.println("INFECTION");  break;
    case MSG_CLEAR: Serial.println("CLEAR");  break;
    case MSG_DISCOVERY: Serial.println("DISCOVERY"); break;
    case MSG_HEARTBEAT: Serial.println("HEARTBEAT"); break;
  }
  txRequest = ZBTxRequest(XBeeAddress64(0x00000000, destinationAddress64), payload, length);
  xbee.send(txRequest);
}

void beginElection(void) {
  Serial.println("Began election");
  if (isElecting)  return;
  else isElecting = true;
  isAcknowledged = false;
  uint8_t countDevices = 0;
  for (int i = 0; i < numDevices; i++)
    if (listAddress64[i] > myAddress64) {
      sendCommand(listAddress64[i], (uint8_t*) &MSG_ELECTION, 1);
      countDevices++;
    }
  if (countDevices > 0) electionTimeout = millis() + ELECTION_REPLY_WAIT_PERIOD;
  else electionTimeout = millis();
}

void readAndHandlePackets(void) {
  if (xbee.readPacket(1) && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
    xbee.getResponse().getZBRxResponse(rxResponse);
    remoteAddress64 = rxResponse.getRemoteAddress64().getLsb();
    if (remoteAddress64 > leaderAddress64) beginElection();     // VERIFY WHETHER YOU ACTUALLY NEED THIS
    Serial.print("MSG_IN:");
    Serial.print(remoteAddress64);
    Serial.print(":");
    switch (rxResponse.getData(0)) {
      case MSG_DISCOVERY:
        Serial.println("DISCOVERY");
        if (rxResponse.getDataLength() > 1) {
          memcpy(&leaderAddress64, rxResponse.getData() + 1, sizeof(leaderAddress64));
        } else {
          uint8_t msgPayload[5];
          msgPayload[0] = MSG_DISCOVERY;
          memcpy(msgPayload + 4, &leaderAddress64, sizeof(leaderAddress64));
          sendCommand(remoteAddress64, msgPayload, 5);
        }

        for (int i = 0; i < numDevices; i++)
          if (listAddress64[i] == remoteAddress64)  break;
        listAddress64[numDevices++] = remoteAddress64;
        break;

      case MSG_ELECTION:
        Serial.println("ELECTION");
        sendCommand(remoteAddress64, (uint8_t*)&MSG_ACK, 1);
        beginElection();
        break;

      case MSG_ACK:
        if (isElecting) {
          Serial.print("ACK");
          Serial.println(remoteAddress64);
          electionTimeout = millis() + ELECTION_VICTORY_WAIT_PERIOD;
          isAcknowledged = true;
        }
        break;

      case MSG_VICTORY:
        if (remoteAddress64 > myAddress64) {
          Serial.println("VICTORY");
          leaderAddress64 = remoteAddress64;
          isElecting = false;
          digitalWrite(PIN_BLUE_LED, LOW);
        }
        else beginElection();
        break;

      case MSG_HEARTBEAT:
        Serial.println("HEARTBEAT");
        leaderHeartbeatTimeout = millis() + LEADER_HEARTBEAT_PERIOD;
    }
  }
}

#include <XBee.h>
#include <SoftwareSerial.h>

#define PIN_BLUE_LED              11    // leader
#define PIN_RED_LED               12    // infected
#define PIN_GREEN_LED             13    // clear
#define PIN_BUTTON                5     // infection toggle

#define ELECTION_REPLY_WAIT_PERIOD      2000
#define ELECTION_VICTORY_WAIT_PERIOD    3000
#define LEADER_HEARTBEAT_PERIOD         6000
#define IMMUNITY_PERIOD                 3000
#define RDM_DELAY                       500 
#define MESSAGE_QUEUE                   100

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
int button_state = LOW, last_button_state = HIGH;
int debounce_timestamp = 0;
int debounce_delay = 50;
int lastClearedTimestamp = 0;
int messageQueueTimestamp = 0;

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
  if (leaderAddress64 != myAddress64) {
    int reading = digitalRead(PIN_BUTTON);
    if (reading != last_button_state) debounce_timestamp = millis();
    if (millis() - debounce_timestamp > debounce_delay) {
      if (reading != button_state) {
        button_state = reading;
        if (button_state == LOW) {
          isInfected = true;
          sendCommand(0x0000FFFF, (uint8_t*)&MSG_INFECTION, 1);
        }
      }
    }
    last_button_state = reading;

    //LED CHANGES
    if (isInfected == false) {
      digitalWrite(PIN_GREEN_LED, HIGH);
      digitalWrite(PIN_RED_LED, LOW);
      digitalWrite(PIN_BLUE_LED, LOW);
    } else if (isInfected == true) {
      digitalWrite(PIN_GREEN_LED, LOW);
      digitalWrite(PIN_RED_LED, HIGH);
      digitalWrite(PIN_BLUE_LED, LOW);
    }
  } else if (leaderAddress64 == myAddress64) {
    int reading = digitalRead(PIN_BUTTON);
    if (reading != last_button_state) debounce_timestamp = millis();
    if (millis() - debounce_timestamp > debounce_delay) {
      if (reading != button_state) {
        button_state = reading;
        if (button_state == LOW) {
          sendCommand(0x0000FFFF, (uint8_t*)&MSG_CLEAR, 1);
        }
      }
    }
    last_button_state = reading;
  }

  if (myAddress64 == leaderAddress64) {
    digitalWrite(PIN_BLUE_LED, HIGH);
    digitalWrite(PIN_GREEN_LED, LOW);
    digitalWrite(PIN_RED_LED, LOW);
  }
  else digitalWrite(PIN_BLUE_LED, LOW);
  readAndHandlePackets();
  if (isElecting && millis() > electionTimeout) { 
    isElecting = false;
    leaderHeartbeatTimeout = millis() + LEADER_HEARTBEAT_PERIOD / 2;
    if (isAcknowledged) beginElection();                              // I think this is a problem source - queueing of messages from every member with each ACK  
    else {
      sendCommand(0x0000FFFF, (uint8_t*)&MSG_VICTORY, 1);
      leaderAddress64 = myAddress64;
    }
  }
  if (!isElecting && millis() > leaderHeartbeatTimeout) {
    if (leaderAddress64 == myAddress64) {
      sendCommand(0x0000FFFF, (uint8_t*) &MSG_HEARTBEAT, 1);
      leaderHeartbeatTimeout = millis() + LEADER_HEARTBEAT_PERIOD / 2;
    } else {
      Serial.println("Leader dead. Relecting");
      beginElection();
    }
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
  Serial.print("AT response: ");
  for (int i = 0; i < atResponse.getValueLength(); i++) {
    Serial.print(atResponse.getValue()[i], HEX);
    Serial.print(" ");
  }
  for (int i = 0; i < 4; i++) {
    uint32_t tempVal = atResponse.getValue()[i];
    myAddress64 |= tempVal << 8 * (3 - i);
  }
  Serial.print("myAddress64: ");
  Serial.println(myAddress64, HEX);
}

void serialLog(bool in, uint32_t address64, uint8_t payload) {
  if (in)  Serial.print("MSG_IN");
  else Serial.print("                                       MSG_OUT");
  Serial.print(":");
  Serial.print(address64, HEX);
  Serial.print(":");
  switch (payload) {
    case MSG_ELECTION: Serial.println("ELECTION"); break;
    case MSG_ACK: Serial.println("ACK"); break;
    case MSG_VICTORY: Serial.println("VICTORY"); break;
    case MSG_INFECTION: Serial.println("INFECTION");  break;
    case MSG_CLEAR: Serial.println("CLEAR");  break;
    case MSG_DISCOVERY: Serial.println("DISCOVERY"); break;
    case MSG_HEARTBEAT: Serial.println("HEARTBEAT"); break;
  }
}

void sendCommand(uint32_t destinationAddress64, uint8_t* payload, uint8_t length) {
  serialLog(false, destinationAddress64, payload[0]);
  txRequest = ZBTxRequest(XBeeAddress64(0x00000000, destinationAddress64), payload, length);
  xbee.send(txRequest);
}

void beginElection(void) {
  if (isElecting)  return;
  else isElecting = true;
  Serial.println("Began election");
  isAcknowledged = false;
  uint8_t countDevices = 0;
  Serial.println("Candidates:");
  for (int i = 0; i < numDevices; i++) {
    Serial.println(listAddress64[i], HEX);
    if (listAddress64[i] > myAddress64) {
      sendCommand(listAddress64[i], (uint8_t*) &MSG_ELECTION, 1);
      countDevices++;
    }
  }
  if (countDevices > 0) electionTimeout = millis() + ELECTION_REPLY_WAIT_PERIOD;
  else electionTimeout = millis() + RDM_DELAY;  // I think it is looping between this and the read packet function - this is the only timeout that is millis() 
                                                // and it immediately satisfies the if (millis() > electionTimeout) logic. Give it at least 0.5sec.
}

void readAndHandlePackets(void) {
  if (xbee.readPacket(1) && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
    xbee.getResponse().getZBRxResponse(rxResponse);
    remoteAddress64 = rxResponse.getRemoteAddress64().getLsb();
    //    if (remoteAddress64 > leaderAddress64) beginElection();     // VERIFY WHETHER YOU ACTUALLY NEED THIS
    serialLog(true, remoteAddress64, rxResponse.getData(0));

    bool inList = false;
    for (int i = 0; i < numDevices; i++)
      if (listAddress64[i] == remoteAddress64)
        inList = true;
    if (!inList) listAddress64[numDevices++] = remoteAddress64;
    switch (rxResponse.getData(0)) {
      
                                               // Prevent queueing: if messages arrive with a frequency greater than 250ms throw them out. May cause problems with CLEAR message,
                                               // need to test.
     if (millis() - messageQueueTimestamp < MESSAGE_QUEUE) break; 
      messageQueueTimestamp = millis();
      
      case MSG_DISCOVERY:
        if (rxResponse.getDataLength() > 1) {
          memcpy(&leaderAddress64, rxResponse.getData() + 1, sizeof(leaderAddress64));
          if (leaderAddress64 < myAddress64) beginElection();
        } else {
          uint8_t msgPayload[5];
          msgPayload[0] = MSG_DISCOVERY;
          memcpy(msgPayload + 4, &leaderAddress64, sizeof(leaderAddress64));
          sendCommand(remoteAddress64, msgPayload, 5);
        }
        break;

      case MSG_ELECTION:
        sendCommand(remoteAddress64, (uint8_t*)&MSG_ACK, 1);
        beginElection();
        break;

      case MSG_ACK:
        electionTimeout = millis() + ELECTION_VICTORY_WAIT_PERIOD;
        isAcknowledged = true;
        break;

      case MSG_VICTORY:
        if (remoteAddress64 > myAddress64) {
          leaderAddress64 = remoteAddress64;
          isElecting = false;
          leaderHeartbeatTimeout = millis() + LEADER_HEARTBEAT_PERIOD / 2;
        }
        else beginElection();
        break;

      case MSG_HEARTBEAT:
        if (remoteAddress64 > leaderAddress64) leaderAddress64 = remoteAddress64;
        if (remoteAddress64 < myAddress64) beginElection();
        leaderHeartbeatTimeout = millis() + LEADER_HEARTBEAT_PERIOD;
        break;

      case MSG_INFECTION:
        if (millis() - lastClearedTimestamp > IMMUNITY_PERIOD) {
          if (leaderAddress64 != remoteAddress64) {
            isInfected = true;
            digitalWrite(PIN_RED_LED, HIGH);
            digitalWrite(PIN_GREEN_LED, LOW);
            sendCommand(0x0000FFFF, (uint8_t*)&MSG_INFECTION, 1);
          }
        }
        break;

      case MSG_CLEAR:
        if (isInfected = true) {
          isInfected = false;;
          digitalWrite(PIN_RED_LED, LOW);
          digitalWrite(PIN_GREEN_LED, HIGH);
          sendCommand(0x0000FFFF, (uint8_t*)&MSG_CLEAR, 1);
          lastClearedTimestamp = millis();
        }
        break;
    }
  }
}

bool isInfected = false;
int lastClearedTimestamp = 0;
#define IMMUNITY_PERIOD = 3000;

int infectionTimeout = 0;
#define INFECTION_BROADCAST_PERIOD = 3000;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  
  if (isInfected) {
    if (millis() - infectionTimeout > INFECTION_BROADCAST_PERIOD) {
      txRequest = ZBTxRequest(XBeeAddress64(0x00000000, 0x0000FFFF, (uint8_t*) MSG_INFECT, 1));
      xbee.send(txRequest);
      infectionTimeout = millis();


    case MSG_INFECTION:
      if (millis() - lastClearedTimestamp  < IMMUNITY_PERIOD || isLeader) break;
      isInfected = true;
      digitalWrite(PIN_BLUE_LED, LOW);
      digitalWrite(PIN_RED_LED, HIGH);
      digitalWrite(PIN_GREEN_LED, LOW);
      txRequest = ZBTxRequest(XBeeAddress64(0x00000000, 0x0000FFFF, (uint8_t*) MSG_INFECT, 1));
      xbee.send(txRequest);
      break;
    case MSG_CLEAR:
      if (isInfected = true) {
        isInfected = false;
        digitalWrite(PIN_BLUE_LED, LOW);
        digitalWrite(PIN_RED_LED, LOW);
        digitalWrite(PIN_GREEN_LED, HIGH);
        ZBTxRequest(XBeeAddress64(0x00000000, 0x0000FFFF, (uint8_t*) MSG_CLEAR, 1));
        xbee.send(txRequest);
      }
      lastClearedTimestamp = millis();
      break;

    }



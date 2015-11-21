bool isInfected = false;


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

case MSG_INFECTION:
  if (isLeader == false) {
    isInfected = true;
    digitalWrite(PIN_BLUE_LED, LOW);
    digitalWrite(PIN_RED_LED, HIGH);
    digitalWrite(PIN_GREEN_LED, LOW);
    txRequest = ZBTxRequest(XBeeAddress64(0x00000000, 0x0000FFFF, (uint8_t*) MSG_INFECT, 1);
    xbee.send(txRequest);    
  }
  break;
case MSG_CLEAR:
  if (isInfected = true) {
    isInfected = false;
    digitalWrite(PIN_BLUE_LED, LOW);
    digitalWrite(PIN_RED_LED, LOW);
    digitalWrite(PIN_GREEN_LED, HIGH);
    ZBTxRequest(XBeeAddress64(0x00000000, 0x0000FFFF, (uint8_t*) MSG_CLEAR, 1);
    xbee.send(txRequest);    
  }
  break;

}



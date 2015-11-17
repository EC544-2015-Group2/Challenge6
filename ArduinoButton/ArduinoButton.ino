#include <XBee.h>

const int button_pin = 5;
const int red_led = 12;
const int green_led = 13;

unsigned long debounce_timestamp = 0, debounce_delay = 50;
int button_state = LOW, last_button_state = LOW;

XBee xbee = XBee();

void setup() {
	Serial.begin(9600);
	xbee.begin(Serial);
	pinMode(button_pin, INPUT);
	digitalWrite(button_pin, HIGH);
	pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);
	digitalWrite(red_led, HIGH);
  digitalWrite(green_led, LOW);
}

void loop() {
	int reading = digitalRead(button_pin);
//  Serial.print(digitalRead(button_pin));
	if(reading != last_button_state)	debounce_timestamp = millis();
	if(millis() - debounce_timestamp > debounce_delay){
		if(reading != button_state){
			button_state = reading;
			if(button_state == LOW){
        
				digitalWrite(red_led, !digitalRead(red_led));
        digitalWrite(green_led, !digitalRead(green_led));
			}
		}
	}
	last_button_state = reading;
  
}

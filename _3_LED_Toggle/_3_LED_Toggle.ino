const int buttonPin = 8;    // the number of the pushbutton pin
const int bluePin = 13;     
const int redPin = 12;     
const int greenPin = 11;     

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

// Variables will change:
int blueState = HIGH;         // the current state of the output pin
int redState = LOW;         // the current state of the output pin
int greenState = LOW;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

int count = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin, INPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  // set initial LED state
  digitalWrite(bluePin, blueState);
  digitalWrite(redPin, redState);
  digitalWrite(greenPin, greenState);
}

void loop() {
  // put your main code here, to run repeatedly:
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
    if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        count++;
        if(count==0) {
          blueState=HIGH;
          greenState=LOW;
          redState =LOW;
        }
        if(count==1) {
          blueState = LOW;
          redState = HIGH;
          greenState = LOW;
        }
        if(count==2) {
          redState = LOW;
          greenState = HIGH;
          blueState = LOW;
        }
        if(count==3) {
          count=0;
           blueState=HIGH;
          greenState=LOW;
          redState =LOW;
        }
      }
    }
  }

  // set the LED:

    digitalWrite(bluePin, blueState);
    digitalWrite(redPin, redState);
    digitalWrite(greenPin, greenState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;

}

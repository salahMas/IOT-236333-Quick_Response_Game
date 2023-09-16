//------------------------- 5 buttons test--------
const int buttonPin = 4;
const int ledPin = 2;

const int buttonPin1 = 19;
const int ledPin1 = 18;

const int buttonPin2 = 23;
const int ledPin2 = 22;

const int buttonPin3 =27;
const int ledPin3 = 26;

const int buttonPin4 = 33;
const int ledPin4 = 32;

int buttonState = 0;      // Variable to store the button state
int buttonState1=0;
int buttonState2=0;
int buttonState3=0;
int buttonState4=0;
unsigned long ledOnTime = 0;
unsigned long reactionTime = 0;
bool gameInProgress = false;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); // Configure the button pin as input with pull-up resistor
  pinMode(ledPin, OUTPUT);           // Configure the LED pin as output
  pinMode(buttonPin1, INPUT_PULLUP); // Configure the button pin as input with pull-up resistor
  pinMode(ledPin1, OUTPUT);           // Configure the LED pin as output
  pinMode(buttonPin2, INPUT_PULLUP); // Configure the button pin as input with pull-up resistor
  pinMode(ledPin2, OUTPUT);           // Configure the LED pin as output
  pinMode(buttonPin3, INPUT_PULLUP); // Configure the button pin as input with pull-up resistor
  pinMode(ledPin3, OUTPUT);           // Configure the LED pin as output
  pinMode(buttonPin4, INPUT_PULLUP); // Configure the button pin as input with pull-up resistor
  pinMode(ledPin4, OUTPUT);           // Configure the LED pin as output
  
  Serial.begin(9600); // Initialize Serial communication for debugging
}

void loop() {
  buttonState = digitalRead(buttonPin); // Read the state of the button
  buttonState1 = digitalRead(buttonPin1); // Read the state of the button
  buttonState2 = digitalRead(buttonPin2); // Read the state of the button
  buttonState3 = digitalRead(buttonPin3); // Read the state of the button
  buttonState4 = digitalRead(buttonPin4); // Read the state of the button



  if (!gameInProgress) {
    digitalWrite(ledPin, LOW); // Turn off the LED
    digitalWrite(ledPin1, LOW); // Turn off the LED
    digitalWrite(ledPin2, LOW); // Turn off the LED
    digitalWrite(ledPin3, LOW); // Turn off the LED
    digitalWrite(ledPin4, LOW); // Turn off the LED

    if (buttonState == LOW ||buttonState1==LOW ||buttonState2 == LOW||buttonState3 == LOW||buttonState4 == LOW) {
      Serial.print("salah");
      gameInProgress = true;
      ledOnTime = millis(); // Record the time when the button is pressed
    }
  } else {
    digitalWrite(ledPin, HIGH); // Turn on the LED
    digitalWrite(ledPin1, HIGH); // Turn on the LED
    digitalWrite(ledPin2, HIGH); // Turn on the LED
    digitalWrite(ledPin3, HIGH); // Turn on the LED
    digitalWrite(ledPin4, HIGH); // Turn on the LED

    if (buttonState == LOW ||buttonState1==LOW ||buttonState2 == LOW||buttonState3 == LOW||buttonState4 == LOW) {
      reactionTime = millis() - ledOnTime; // Calculate the reaction time
      Serial.print("Reaction Time: ");
      Serial.print(reactionTime);
      Serial.println(" ms");
      gameInProgress = false;
    }
  }

  delay(10); // Add a small delay for stability
}
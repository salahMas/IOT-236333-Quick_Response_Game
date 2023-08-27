const int buttonPin = 4;
const int ledPin = 2;

int buttonState = 0;      // Variable to store the button state
unsigned long ledOnTime = 0;
unsigned long reactionTime = 0;
bool gameInProgress = false;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); // Configure the button pin as input with pull-up resistor
  pinMode(ledPin, OUTPUT);           // Configure the LED pin as output
  
  Serial.begin(9600); // Initialize Serial communication for debugging
}

void loop() {
  buttonState = digitalRead(buttonPin); // Read the state of the button

  if (!gameInProgress) {
    digitalWrite(ledPin, LOW); // Turn off the LED
    if (buttonState == LOW) {
      gameInProgress = true;
      ledOnTime = millis(); // Record the time when the button is pressed
    }
  } else {
    digitalWrite(ledPin, HIGH); // Turn on the LED
    if (buttonState == LOW) {
      reactionTime = millis() - ledOnTime; // Calculate the reaction time
      Serial.print("Reaction Time: ");
      Serial.print(reactionTime);
      Serial.println(" ms");
      gameInProgress = false;
    }
  }

  delay(10); // Add a small delay for stability
}
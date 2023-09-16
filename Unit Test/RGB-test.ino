// ---------- start button ---------------------------
#include <Arduino.h>

// Define the pins for the button, RGB LED, and their initial states
const int buttonPin = 15; // Replace with your actual button pin
const int redPin = 14;   // Replace with your actual red LED pin
const int greenPin = 13; // Replace with your actual green LED pin


bool isButtonPressed = false;

void setup() {
  // Initialize button pin as an input
  pinMode(buttonPin, INPUT_PULLUP);

  // Initialize RGB LED pins as outputs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  //pinMode(bluePin, OUTPUT);

  // Initialize serial communication for debugging
  Serial.begin(9600);
}

void loop() {
  // Read the state of the button
  isButtonPressed = digitalRead(buttonPin) == LOW;

  // Update the RGB LED based on the button state
  if (isButtonPressed) {
    // Button is pressed, turn on the LED in a specific color (e.g., blue)
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    //digitalWrite(bluePin, HIGH);
    Serial.println("Button is pressed.");
  } else {
    // Button is not pressed, turn off the LED
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    //digitalWrite(bluePin, LOW);
    Serial.println("Button is not pressed.");
  }

  delay(100); // Add a small delay to debounce the button
}
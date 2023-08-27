const int buttonPin = 4;
const int ledPin = 2;

int buttonState = 0;      // Variable to store the button state
unsigned long ledOnTime = 0;
unsigned long reactionTime = 0;
bool gameInProgress = false;

#include <HardwareSerial.h>
HardwareSerial MP3(2); // Use UART2 for MP3 player communication

static int8_t set_volume[] = {0x7e, 0x03, 0x31, 0x50, 0xef};
static int8_t select_SD_card[] = {0x7e, 0x03, 0X35, 0x01, 0xef};
static int8_t play_first_song[] = {0x7e, 0x04, 0x41, 0x00, 0x01, 0xef};
static int8_t pauseCmd[] = {0x7e, 0x02, 0x02, 0xef};

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); // Configure the button pin as input with pull-up resistor
  pinMode(ledPin, OUTPUT);           // Configure the LED pin as output

  Serial.begin(9600); // Initialize Serial communication for debugging
  MP3.begin(9600, SERIAL_8N1, 17, 16); // Initiate the Serial MP3 Player Module
  send_command_to_MP3_player(select_SD_card, 5);
  send_command_to_MP3_player(set_volume, 5);
}

void loop() {
  buttonState = digitalRead(buttonPin); // Read the state of the button

  if (!gameInProgress) {
    digitalWrite(ledPin, LOW); // Turn off the LED
    if (buttonState == LOW) {
      gameInProgress = true;
      ledOnTime = millis(); // Record the time when the button is pressed
      send_command_to_MP3_player(play_first_song, 6); // Play the first song
    }
  } else {
    digitalWrite(ledPin, HIGH); // Turn on the LED
    if (buttonState == LOW) {
      reactionTime = millis() - ledOnTime; // Calculate the reaction time
      Serial.print("Reaction Time: ");
      Serial.print(reactionTime);
      Serial.println(" ms");
      send_command_to_MP3_player(pauseCmd, 4); // Pause the song
      gameInProgress = false;
    }
  }

  delay(10); // Add a small delay for stability
}

void send_command_to_MP3_player(int8_t command[], int len) {
  Serial.print("\nMP3 Command => ");
  for (int i = 0; i < len; i++) {
    MP3.write(command[i]);
    Serial.print(command[i], HEX);
  }
  delay(1000);
}
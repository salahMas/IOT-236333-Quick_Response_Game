// Connections
// ESP32:           
//                                Serial MP3 Player Module (OPEN-SMART)
// GPIO17 ------------------------ TX
// GPIO16 ------------------------ RX

// Include required libraries:
#include <HardwareSerial.h>

// Define the Serial MP3 Player Module.
HardwareSerial MP3(2); // Use UART2 for MP3 player communication

// Define the required MP3 Player Commands:

static int8_t set_volume[] = {0x7e, 0x03, 0x31, 0x50, 0xef}; // 7E 03 06 00 EFstatic int8_t set_volume[] = {0x7e, 0x03, 0x31, volume, 0xef}; 

// Select storage device to TF card
static int8_t select_SD_card[] = {0x7e, 0x03, 0X35, 0x01, 0xef}; // 7E 03 35 01 EF
// Play with index: /01/001xxx.mp3
static int8_t play_first_song[] = {0x7e, 0x04, 0x41, 0x00, 0x01, 0xef}; // 7E 04 41 00 01 EF
// Play with index: /01/002xxx.mp3
static int8_t play_second_song[] = {0x7e, 0x04, 0x41, 0x00, 0x02, 0xef}; // 7E 04 41 00 02 EF
// Play the song.
static int8_t play[] = {0x7e, 0x02, 0x01, 0xef}; // 7E 02 01 EF
// Pause the song.
static int8_t pauseCmd[] = {0x7e, 0x02, 0x02, 0xef}; // 7E 02 02 EF

void setup() {
  // Initiate the serial monitor.
  Serial.begin(9600);
  // Initiate the Serial MP3 Player Module.
  MP3.begin(9600, SERIAL_8N1, 17, 16);
  // Select the SD Card.
  send_command_to_MP3_player(select_SD_card, 5);
  send_command_to_MP3_player(set_volume, 5);
}

void loop() {
  // Play the second song.
  send_command_to_MP3_player(play_first_song, 6);
}


void send_command_to_MP3_player(int8_t command[], int len){
  Serial.print("\nMP3 Command => ");
  for(int i=0;i<len;i++){ MP3.write(command[i]); Serial.print(command[i], HEX); }
  delay(1000);
}
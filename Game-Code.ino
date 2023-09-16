#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>
#include <WiFi.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <HardwareSerial.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <iostream>
#include <sstream>
#include <string>
#include <ArduinoJson.h>
#include <stdio.h>
#include <stdlib.h>
#include <SPIFFS.h>
#include <Adafruit_NeoPixel.h>
#include <map>
#include <vector>
#include <HTTPClient.h>

//5v -> ring+mp3
// GND - left -> mp3,ring,start
// GND - Right -> 5 buttons

#define BUTTON1_PIN 4 
#define BUTTON2_PIN 33 
#define BUTTON3_PIN 23 
#define BUTTON4_PIN 27
#define BUTTON5_PIN 19
#define START_BUTTON 15
#define START_R 14 // red 
#define START_G 13 // green 
#define LED1_PIN 2
#define LED2_PIN 18
#define LED3_PIN 22
#define LED4_PIN 26
#define LED5_PIN 21
// Which pin on the Arduino is connected to the NeoPixels?
#define RING_PIN 12  // On Trinket or Gemma, suggest changing this to 1
#define PIN            12
#define SIZE_ARR(Arr) (sizeof(Arr) / sizeof(*(Arr)))

// How many NeoPixels are attached to the Arduino?
#define DELAY_MS       50     // Delay between lighting up each NeoPixel (in milliseconds)
#define NUMPIXELS 16           // Define the number of NeoPixels in your ring

// Declare our NeoPixel strip object:
// Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELSRING, PIN, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


//---------------------------------WIFI --------------------------------------//

WiFiServer server(80);
bool connection = false;
bool res;
WiFiManager wifiManager;
std::vector<const char*> menu = { "wifi", "info" };

const char* ssid = "HOTWiFi_5";
const char* password = "0123456789";


//-----------------------------------GLOBAL VARIABLES----------------------------------------//
enum State {
  STATE_START,
  STATE_GAME,
  STATE_WIN,
  STATE_LOSE,
  STATE_IDLE,
  STATE_WIFI,
  STATE_WIFI_IDLE
};

enum State_Message {
  STATE_START_GAME,
  STATE_LEVEL,
  STATE_WAITING_NEXT_GAME,
  ID_STATE,
  SETTINGS_STATE
};

State_Message state_message = STATE_START_GAME;


unsigned long levels_time[] = {2000, 1800, 1600, 800, 400 };
unsigned long led_pins[] = { LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN, LED5_PIN };
unsigned long buttons_pins[] = { BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN, BUTTON4_PIN, BUTTON5_PIN };

char result[50];
char result2[50];
char resultTime[50];
char resultAvg[50];
char resultHighest[50];
bool level_switch = true;
State current_state = STATE_IDLE;
State prev_state = STATE_IDLE;

int buttons_state[] = { HIGH, HIGH, HIGH, HIGH, HIGH };
int last_buttons_state[] = { HIGH, HIGH, HIGH, HIGH, HIGH };
int readings[] = { HIGH, HIGH, HIGH, HIGH, HIGH };  //////// we need to check it
int num_of_buttons = 5;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

uint32_t colors[16];

unsigned long ledOnTime = 1600;
unsigned long ledStartTime = 0;
bool ledOn = false;

int counter = -1;

int currentLEDNumber = 0;
int toneNumber = 0;

unsigned int level = 0;
String chat_id;
String keyboardJson;
String message;
int reading1;
int reading2;
int current = 1;
double highest_score = 0;
double avg_response_time = 0;
unsigned long firstLedStartTime = 0;
unsigned long lastLedPressedTime = 0;
unsigned int num_of_games = 0;
int user_id;

double total_time = 0;
double min_total_time = 0;
double avg_response = 0;
double highest_avg_response;
int num_of_win_games = 0;

String table;
int extra = 0;

int brightness = 150;


//-------------------------------TELEGRAM BOT--------------------------------//
#define BOT_TOKEN "6687385499:AAH63dnYXMG5D-n4agrtflzgNA8gn37VLnM"
const unsigned long BOT_MTBS = 100;  // mean time between scan messages
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;



//--------------------------------FLASH CODE ------------------------------------------//

struct UserData{
  int numPresses;
  double time;
};

std::map<int, UserData> userMap;
std::vector<std::pair<int, UserData>> vec;

bool compareUserData(std::pair<int,UserData> & a , std::pair<int,UserData> & b){
  if ((a.second.time / (a.second.numPresses)) < (b.second.time / b.second.numPresses)){
    return true;
  }else
    return false;
}



void sortMapByTime() {
  std::vector<std::pair<int, UserData>> vec(userMap.begin(), userMap.end());

  std::sort(vec.begin(), vec.end(),compareUserData);

  userMap.clear();
  for (std::vector<std::pair<int, UserData>>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
    Serial.println(it->first);
    userMap[it->first] = it->second;
  }
}

void insertUser(int userId, int presses,double time) {
  UserData user = {presses, time};
  userMap[userId] = user;
}

//------------------------mp3--------------------------------------------------------
#include <HardwareSerial.h>
HardwareSerial MP3(2);  // Use UART2 for MP3 player communication
static int8_t set_volume[] = { 0x7e, 0x03, 0x31, 0x50, 0xef };
static int8_t select_SD_card[] = { 0x7e, 0x03, 0X35, 0x01, 0xef };         // 7E 03 35 01 EF
static int8_t play_start_game[] = { 0x7e, 0x04, 0x41, 0x00, 0x01, 0xef };  // 7E 04 41 00 01 EF start the game 0001
static int8_t play_do[] = { 0x7e, 0x04, 0x41, 0x00, 0x06, 0xef };          // 7E 04 41 00 02 EF 0002
static int8_t play_re[] = { 0x7e, 0x04, 0x41, 0x00, 0x07, 0xef };
static int8_t play_mi[] = { 0x7e, 0x04, 0x41, 0x00, 0x08, 0xef };
static int8_t play_fa[] = { 0x7e, 0x04, 0x41, 0x00, 0x09, 0xef };
static int8_t play_sol[] = { 0x7e, 0x04, 0x41, 0x00, 0x0a, 0xef };
static int8_t play_la[] = { 0x7e, 0x04, 0x41, 0x00, 0x0b, 0xef };
static int8_t play_si[] = { 0x7e, 0x04, 0x41, 0x00, 0x0c, 0xef };
static int8_t play_fail[] = { 0x7e, 0x04, 0x41, 0x00, 0x02, 0xef };
static int8_t play_timeout[] = { 0x7e, 0x04, 0x41, 0x00, 0x03, 0xef };
static int8_t play_win[] = { 0x7e, 0x04, 0x41, 0x00, 0x05, 0xef };
static int8_t play_level_up[] = { 0x7e, 0x04, 0x41, 0x00, 0x04, 0xef };
static int8_t volume_up[] =  {0x7e, 0x02, 0x05, 0xef};//7E 02 05 EF
static int8_t volume_down[] = {0x7e, 0x02, 0x06, 0xef};//7E 02 06 EF


static int8_t* led_tones[] = { play_do, play_re, play_mi, play_fa, play_sol, play_la, play_si };


void send_command_to_MP3_player(int8_t command[], int len) {
  for (int i = 0; i < len; i++) {
    MP3.write(command[i]);
  }

  //delay(90);
}
//--------------------------------------------------------------------------------------------


//----------------------------------function for the telegram--------------------------------//



bool checkId(String to_check) {
  int idValue = to_check.toInt();
  if (idValue >= 100000000 && idValue <= 999999999) {
    user_id = idValue;
    return true;
  }
  return false;
}

String gifLevelsUrl[9] ={"https://media.giphy.com/media/13VLdHIQRb8zQc/giphy.gif" ,
                          "https://media.giphy.com/media/pKt7w9ILVOdWw/giphy.gif" ,
                          "https://media.giphy.com/media/d7id4BY2NQnJe/giphy.gif" ,
                          "https://media.giphy.com/media/U7oYLyQqXM9sA/giphy.gif" ,
                          "https://media.giphy.com/media/xRCASav6bgz9m/giphy.gif" ,
  /*sendGIF(5) win*/      "https://media.giphy.com/media/xULW8CPwOHXPua8NTa/giphy.gif" ,
 /*sendGIF(6)game over*/  "https://media.giphy.com/media/fMAUtRvUTaMTfAagVW/giphy.gif" ,
/*sendGIF(7) volume up*/  "https://media.giphy.com/media/PqQIVXwPWkcMg/giphy.gif" ,
/*sendGIF(8)volume down*/   "https://tenor.com/71XB.gif"};

/////////////////////////////////////
void sendGIF(int index) {
    String url = "https://api.telegram.org/bot" + String(BOT_TOKEN) + "/sendDocument?chat_id=" + String(chat_id);
    String gifUrl = gifLevelsUrl[index];

    // Send the POST request
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "document=" + gifUrl;
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(response);
    } else {
        Serial.print("Error on sending POST request. HTTP error code: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}
////////

char idResultRanking[50];
char pressesResultRanking[50];
char timeResultRanking[50];
char avgResultRanking[50];
char rankResult[50];
double avg;

void handleNewMessages(int numNewMessages) {
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  String name;
  for (int i = 0; i < numNewMessages; i++) {
    chat_id = bot.messages[i].chat_id;  //singular user - not used
    String text = bot.messages[i].text;
    bot.name = bot.messages[i].from_name;
    if (name == "") {
      name = "Guest";
    }
    switch (state_message) {
      case STATE_START_GAME:
        if (text.indexOf("start") != -1) {
          String welcome = "Welcome to QUICK RESPONSE GAME " + name + "☺.\n";
          welcome += "Insert your Id! ";
          bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", "", true);
          state_message = ID_STATE;
        }
        break;
      case ID_STATE:
        if (checkId(text)) {
          String choose_level = "Choose the game level! ";
          keyboardJson = "[[\"level1️⃣\",\"level2️⃣\"], [\"level3️⃣\",\"level4️⃣\"],[\"level5️⃣🔥\"]]";
          bot.sendMessageWithReplyKeyboard(chat_id, "choose_level", "", keyboardJson, true);
          state_message = STATE_LEVEL;
        } else {
          String choose_another_id = "Invalid id! Please insert your id again!";
          bot.sendMessageWithReplyKeyboard(chat_id, "choose_another_id", "", "", true);
        }
        break;
      case STATE_LEVEL:
        if (text.indexOf("level1️⃣") != -1) {
          bot.sendMessage(chat_id, "Got it! Press the green button to start the game! ", "");
          level = 1;
          ledOnTime = levels_time[level - 1];
          digitalWrite(START_G, HIGH);
          current_state = STATE_START;
          state_message = STATE_WAITING_NEXT_GAME;
          extra = 0;
        }
        if (text.indexOf("level2️⃣") != -1) {
          bot.sendMessage(chat_id, "Got it! Press the green button to start the game! ", "");
          level = 2;
          ledOnTime = levels_time[level - 1];
          digitalWrite(START_G, HIGH);
          current_state = STATE_START;
          state_message = STATE_WAITING_NEXT_GAME;
          extra = 16;
        }
        if (text.indexOf("level3️⃣") != -1) {
          bot.sendMessage(chat_id, "Got it! Press the green button to start the game! ", "");
          level = 3;
          ledOnTime = levels_time[level - 1];
          digitalWrite(START_G, HIGH);
          current_state = STATE_START;
          state_message = STATE_WAITING_NEXT_GAME;
          extra = 32;
        }
        if (text.indexOf("level4️⃣") != -1) {
          bot.sendMessage(chat_id, "Got it! Press the green button to start the game! ", "");
          level = 4;
          ledOnTime = levels_time[level - 1];
          digitalWrite(START_G, HIGH);
          current_state = STATE_START;
          state_message = STATE_WAITING_NEXT_GAME;
          extra = 48;
        }
        if (text.indexOf("level5️⃣🔥") != -1) {
          bot.sendMessage(chat_id, "Got it! Press the green button to start the game! ", "");
          level = 5;
          ledOnTime = levels_time[level - 1];
          digitalWrite(START_G, HIGH);
          current_state = STATE_START;
          state_message = STATE_WAITING_NEXT_GAME;
          extra = 64;
        }
        break;
      case STATE_WAITING_NEXT_GAME:
        if (text.indexOf("Statistics") != -1){
          String word = "Your highest score per second is:\n";
          avg = userMap[user_id].numPresses / userMap[user_id].time;
          sprintf(avgResultRanking, "%.3f", avg);
          word += avgResultRanking;
          word += " presses per second";
          bot.sendMessage(chat_id, word.c_str(), "");
        }
        if (text.indexOf("Rank") != -1) {
          std::vector<std::pair<int, UserData>> vec(userMap.begin(), userMap.end());
          std::sort(vec.begin(), vec.end(),compareUserData);
          int counterRank = 1;
          table =  "| Rank |        -Id-         |   Average Response   |\n";
          for (std::vector<std::pair<int, UserData>>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
            sprintf(idResultRanking, "%d", it->first);
            sprintf(timeResultRanking, "%f", it->second.time);
            sprintf(pressesResultRanking, "%d", it->second.numPresses);
            avg = it->second.time / it->second.numPresses ;
            sprintf(avgResultRanking, "%.3f", avg);
            sprintf(rankResult, "%d", counterRank);
            if(counterRank >= 10){
              table += "|  -";
              table += rankResult;
              table += "-  | ";
            }else{
              table += "|   -";
              table += rankResult;
              table += "-   | ";
            }
            

            table += idResultRanking;
            if((it->second.time / it->second.numPresses) > 9.999 ){
              table += " | ";
            }else{
              table += " |   ";
            }
            table += avgResultRanking;
            table += " [sec/presses] |\n";
            counterRank++;
          }
          bot.sendMessage(chat_id, table.c_str(), "");
        }
        if (text.indexOf("new game") != -1) {
          keyboardJson = "[[\"level1️⃣\",\"level2️⃣\"], [\"level3️⃣\",\"level4️⃣\"],[\"level5️⃣🔥\"]]";
          bot.sendMessageWithReplyKeyboard(chat_id, "Choose a level for the game ", "", keyboardJson, true);
          current_state = STATE_IDLE;
          ledOn = false;
          counter = -1;
          state_message = STATE_LEVEL;
        }
        if(text.indexOf("Settings") != -1){
          keyboardJson = "[[\"Volume ⬆️🔊\", \"Volume ⬇️🔊\"], [\"Ring's Brightness ⬆️🔆\", \"Ring's Brightness ⬇️🔅\"]]";
          bot.sendMessageWithReplyKeyboard(chat_id, "Choose one of the settings ", "", keyboardJson, true);
          state_message = SETTINGS_STATE;
        }
        if (text.indexOf("Quit the game") != -1) {
          keyboardJson = "[[\"start\"]]";
          bot.sendMessageWithReplyKeyboard(chat_id, "Good luck! ", "", keyboardJson, true);
          current_state = STATE_IDLE;
          state_message = STATE_START_GAME;
          counter = -1;
          num_of_games = 0;
          num_of_win_games = 0;
        }
        break;
      case SETTINGS_STATE:
        keyboardJson = "[[\"new game\",\"Quit the game\" ], [\"Statistics\" ,\"Rank\"] ,[\"Settings\"]]";
        if(text.indexOf("Volume ⬆️🔊") != -1){
          send_command_to_MP3_player(volume_up, 4);
          sendGIF(7);
          bot.sendMessageWithReplyKeyboard(chat_id, "", "", keyboardJson, true);
          state_message = STATE_WAITING_NEXT_GAME;
        }
        if(text.indexOf("Volume ⬇️🔊") != -1){
          send_command_to_MP3_player(volume_down, 4);
          sendGIF(8);
          bot.sendMessageWithReplyKeyboard(chat_id, "", "", keyboardJson, true);
          state_message = STATE_WAITING_NEXT_GAME;
        }
        if(text.indexOf("Ring's Brightness ⬆️🔆") != -1){
          if(brightness <= 245){
            brightness+=10;
          }
          
          strip.setBrightness(brightness);
          bot.sendMessageWithReplyKeyboard(chat_id, "🔆 ", "", keyboardJson, true);
          state_message = STATE_WAITING_NEXT_GAME;
        }
        if(text.indexOf("Ring's Brightness ⬇️🔅") != -1){
          if(brightness >= 10){
            brightness-= 10;
          }
          strip.setBrightness(brightness);
          bot.sendMessageWithReplyKeyboard(chat_id, "🔅 ", "", keyboardJson, true);
          state_message = STATE_WAITING_NEXT_GAME;
        }
        break;
    }
  }
}

//--------------------------------help functions--------------------------------------------//

void wrongButton(int LedNumber) {
  total_time += (millis() - ledStartTime);
  Serial.println("Button 1 is Pressed and Led 2 Turn on .... Game Over GG");
  strip.clear();
  strip.show();
  send_command_to_MP3_player(play_fail, 6);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);
  digitalWrite(LED5_PIN, LOW);
  theaterChase(strip.Color(255,   0,   0), 200); // Red, half brightness
  ledOn = false;
  current_state = STATE_LOSE;
}

//////////////////////////////////////////////////////////////////////////
void winHandler(int LedNumber) {
  send_command_to_MP3_player(play_win, 6);
  digitalWrite(led_pins[LedNumber - 1], LOW);
  strip.clear();
  strip.show();
  theaterChase(strip.Color(  0, 255, 0),  200);
  ledOn = false;
  current_state = STATE_WIN;
}

////////////////////////////////////////////////////////////////////////////
void rightButton(int LedNumber) {
  total_time += (millis() - ledStartTime);
  if ((counter + 1) % 16 == 0) {
    if (level == 5) {
      avg_response = total_time / 80;
      if (num_of_win_games == 0) {
        min_total_time = total_time;
        highest_avg_response = avg_response;
      }
      if (avg_response < highest_avg_response) {
        highest_avg_response = avg_response;
      }
      if(min_total_time > total_time){
        min_total_time = total_time;
      }
      num_of_win_games++;
      counter++;
      winHandler(LedNumber);
    } else {
      updatePixels();
      level = level + 1;
      digitalWrite(led_pins[LedNumber - 1], LOW);
      send_command_to_MP3_player(play_level_up, 6);
      turnOffRing();
      sendGIF(level -1);
      ledOnTime = levels_time[level - 1];
      level_switch = true;
    }
  } else {
    updatePixels();
    toneNumber = random(0, 7);
    send_command_to_MP3_player(led_tones[toneNumber], 6);
    digitalWrite(led_pins[LedNumber - 1], LOW);
  }
  ledOn = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void timeOutHandler() {
  send_command_to_MP3_player(play_timeout, 6);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);
  digitalWrite(LED5_PIN, LOW);
  theaterChase(strip.Color(255,   0,   0), 200); // Red, half brightness
  Serial.println("TIME OUT............. LOSE THE GAME");
  ledOn = false;
  current_state = STATE_LOSE;
}

int prevLEDNumber = 0;
////////////////////////////////////////////////////////////////////////////
void turnOnLed() {
  prevLEDNumber = currentLEDNumber;
  currentLEDNumber = random(1, 6);
  Serial.println("led number on ");
  Serial.print(currentLEDNumber);
  if (prevLEDNumber == currentLEDNumber) {
    int flickerDelay = random(90, 100); // Random delay between flickers (adjust as needed)
    
    delay(flickerDelay);
    digitalWrite(led_pins[currentLEDNumber - 1], HIGH); // Turn on the LED
    delay(flickerDelay);        // Delay for a short period
    
    digitalWrite(led_pins[currentLEDNumber - 1], LOW);  // Turn off the LED
    delay(flickerDelay);        // Delay for a short period
  }
  Serial.print("----");
  Serial.print(counter);
  Serial.print("----");
  counter++;
  digitalWrite(led_pins[currentLEDNumber - 1], HIGH);
  ledStartTime = millis();
  ledOn = true;
}

///////////////////////////////////////////////////////////////////////////////
void theaterChase(uint32_t color, int wait) {
  for (int a = 0; a < 5; a++) {
    chaseEffect(DELAY_MS , color);
  }
}
////////////////////////////////////////////////////////////////////////////////
// void turnOffRing(){
//   for (int j = 0; j < NUMPIXELS; j++) {
//     strip.setPixelColor(j, strip.Color(0, 0, 0));
//   }
// }

/////////////////////////////////////////////////////////////////////////////////
void updatePixels() {
  // Clear all pixels
  strip.clear();
  // Turn on specific pixel for other levels
  int current_counter = (counter) % 16;
  colors[current_counter] = getRandomColor();
  for (int i = 0; i < current_counter+1; i++) {
    strip.setPixelColor(i, colors[i]);
  }
  strip.show();  // Show the updated state
}

//////////////////////////////////////////////////////////////////////////////////
uint32_t getRandomColor() {
  // Generate a random color
  uint8_t r = random(256);
  uint8_t g = random(256);
  uint8_t b = random(256);

  return strip.Color(r, g, b);
}

//////////////////////////////////////////////////////////////////////////
void telegramHandleMessages() {
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    if (numNewMessages) {
      Serial.println(numNewMessages);
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}


//-------------------------------------------------------------------------------------------//





void setup() {

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(LED5_PIN, OUTPUT);
  pinMode(START_R, OUTPUT);
  pinMode(START_G, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);
  pinMode(BUTTON5_PIN, INPUT_PULLUP);
  pinMode(START_BUTTON, INPUT_PULLUP);
  

  // Initialize ring pins
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(brightness); // Set BRIGHTNESS to about 1/5 (max = 255)
  ////////////////////////
  

  

  Serial.begin(9600);
  MP3.begin(9600, SERIAL_8N1, 17, 16);
  send_command_to_MP3_player(select_SD_card, 5);
  send_command_to_MP3_player(set_volume, 5);
  Serial.println("Start a New Game !!! Good Luck");
  randomSeed(analogRead(A0));

///////////////////////////////////////////////////

  wifiManager.resetSettings();//remove any previous wifi settings
  wifiManager.setMenu(menu);
  while (!wifiManager.autoConnect("QuickResponseGameAP")) {
    chaseEffect(DELAY_MS , strip.Color(255, 255, 255));
    Serial.println("Failed to connect");
  }
  connection = true;
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  ////////////////////////telegram init
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
  turnOffRing();
  digitalWrite(START_R, LOW);
  digitalWrite(START_G, LOW);
}

double curr_time;
char resultId[50];
double tot_time = 0;
char resultLevel[50];


void loop() {
  switch (current_state) {
    case STATE_START: 
      if(WiFi.status() != WL_CONNECTED){
        current_state = STATE_WIFI;
        Serial.print("WiFi disconnected");
        digitalWrite(START_R, HIGH);
        current_state = STATE_WIFI;
        break;
      }
      if (digitalRead(START_BUTTON) == LOW) {
        digitalWrite(START_R, LOW);
        digitalWrite(START_G, LOW);
        send_command_to_MP3_player(play_start_game, 6);
        sendGIF(level -1);
        delay(1000);
        current_state = STATE_GAME;
      }
      break;
    case STATE_GAME:
      if(WiFi.status() != WL_CONNECTED){
        current_state = STATE_WIFI;
        Serial.print("WiFi disconnected");
        digitalWrite(START_R, HIGH);
        current_state = STATE_WIFI;
        break;
      }
      if (level_switch) {
        level_switch = false;
      }
      if (!ledOn) {
        turnOnLed();
      }
      if (ledOn && (millis() - ledStartTime >= ledOnTime)) {
        timeOutHandler();
        break;
      }
      for (int a = 0; a < num_of_buttons; a++) {
        readings[a] = digitalRead(buttons_pins[a]);
      }
      if (readings[0] != last_buttons_state[0] || readings[1] != last_buttons_state[1] || readings[2] != last_buttons_state[2] || readings[3] != last_buttons_state[3] || readings[4] != last_buttons_state[4]) {
        lastDebounceTime = millis();
      }
      if (millis() - lastDebounceTime > debounceDelay) {
        lastLedPressedTime = millis();
        for (int i = 0; i < num_of_buttons; i++) {
          if (readings[i] != buttons_state[i]) {
            buttons_state[i] = readings[i];
            if (buttons_state[i] == LOW && currentLEDNumber == i + 1) {
              rightButton(i + 1);
            } else if (buttons_state[i] == LOW && currentLEDNumber != i + 1) {
              wrongButton(i + 1);
            }
          }
        }
      }
      for (int b = 0; b < num_of_buttons; b++) {
        last_buttons_state[b] = readings[b];
      }
      break;
    case STATE_WIN:
      turnOffRing();
      digitalWrite(START_R, LOW);
      digitalWrite(START_G, LOW);
      total_time = total_time /1000;
      avg_response = total_time / 80;
      sprintf(resultAvg, "%.3f", avg_response);
      sprintf(resultLevel , "%d" ,level);
      sprintf(resultId, "%d", user_id);
      table = "|        -Id-         | Level |   Average Response   |\n";
      //table += "|-------------------------------------------|\n";
      table += "| ";
      table += resultId;
      table += " |   -";
      table += resultLevel;
      if(avg_response > 9.999 ){
        table += "-   | ";
      }else{
        table += "-   |   ";
      }
      table += resultAvg;
      table += " [sec/presses] |\n";
      sendGIF(5);
      bot.sendMessage(chat_id, table.c_str(), "");
      tot_time = 0;
      if(userMap.find(user_id) != userMap.end()){
       // if((counter+extra) >= userMap[user_id].numPresses){
        if((total_time / (counter+extra)) < (userMap[user_id].time / userMap[user_id].numPresses)){
          counter = 80;
        }
        userMap.erase(user_id);
      }
      // tot_time += total_time;
      insertUser(user_id ,counter, total_time);
      keyboardJson = "[[\"new game\",\"Quit the game\" ], [\"Statistics\" ,\"Rank\"] ,[\"Settings\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Would you like to start a new game? ", "", keyboardJson, true);
      current_state = STATE_IDLE;
      break;
    case STATE_LOSE:
      turnOffRing();
      digitalWrite(START_R, LOW);
      digitalWrite(START_G, LOW);
      total_time = total_time /1000;
      Serial.println("your total_time:");
      Serial.println(total_time);
      Serial.println("your counter :");
      Serial.println(counter);
      //sprintf(resultTime, "%.3f", total_time);
      avg_response = total_time / counter ;
      sprintf(resultAvg, "%.3f", avg_response);
      sprintf(resultId, "%d", user_id);
      //sprintf(result, "%d", counter);
      sprintf(resultLevel , "%d" ,level);
      table = "|        -Id-         | Level |   Average Response   |\n";
      table += "| ";
      table += resultId;
      table += " |   -";
      table += resultLevel;
      if(avg_response > 9.999 ){
        table += "-   | ";
      }else{
        table += "-   |   ";
      }
      table += resultAvg;
      table += " [sec/presses] |\n";
      sendGIF(6);
      bot.sendMessage(chat_id, table.c_str(), "");
      keyboardJson = "[[\"new game\",\"Quit the game\" ], [\"Statistics\" ,\"Rank\"] ,[\"Settings\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Would you like to start a new game? ", "", keyboardJson, true);
      current_state = STATE_IDLE;
      
      if(userMap.find(user_id) != userMap.end()){
        if((total_time / (counter)) < (userMap[user_id].time / userMap[user_id].numPresses)){
          counter = counter;
        }
        userMap.erase(user_id);
      }
      insertUser(user_id, counter , total_time );
      break;
    case STATE_IDLE:
      if(WiFi.status() != WL_CONNECTED){
        //current_state = STATE_WIFI;
        Serial.print("WiFi disconnected");
        digitalWrite(START_R, HIGH);
        //prev_state = current_state;
        current_state = STATE_WIFI_IDLE;
        break;
      }
      // digitalWrite(START_R, HIGH);
      telegramHandleMessages();
      break;
    case STATE_WIFI:
      wifiManager.resetSettings();//remove any previous wifi settings
      wifiManager.setMenu(menu);
      while (!wifiManager.autoConnect("QuickResponseGameAP")) {
        chaseEffect(DELAY_MS , strip.Color(255, 255, 255));
        Serial.println("Failed to connect");
      }
      connection = true;
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      bot.sendMessage(chat_id, "Now you are coneected again. Press the red button to continue the game! ", "");
      current_state = STATE_START;
      break;
    case STATE_WIFI_IDLE:
      wifiManager.resetSettings();//remove any previous wifi settings
      wifiManager.setMenu(menu);
      
      res = wifiManager.autoConnect("QuickResponseGameAP");
      while (!res) {
        chaseEffect(DELAY_MS , strip.Color(255, 255, 255));
        Serial.println("Failed to connect");
      }
      connection = true;
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      current_state = STATE_IDLE;
      break;
  }
}


// Function to create a chasing effect
void chaseEffect(int delayTime , uint32_t color) {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(delayTime);
    strip.setPixelColor(i, strip.Color(0, 0, 0)); // Turn off the current pixel
  }
}

void turnOffRing(){
  for (int j = 0; j < NUMPIXELS; j++) {
    strip.setPixelColor(j, strip.Color(0, 0, 0));
  }
  strip.show();
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 01 Test Connection of ESP32 CAM with Telegram
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/telegram-esp32-cam-photo-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

/*
 * Uteh Str
 * 
 * The source of this project is from : https://RandomNerdTutorials.com/telegram-esp32-cam-photo-arduino/
 * I made some changes and modifications.
 */

/* ======================================== Including the libraries. */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
/* ======================================== */

/* ======================================== Variables for network. */
// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "David OnePlus 9 Pro 5G";      //--> Enter your SSID / your WiFi network name.
const char* password = "David777";  //--> Enter your WiFi password.
/* ======================================== */

/* ======================================== Variables for telegram bot tokens. */
String BOTtoken = "6874017316:AAGt3NbP4V8janK_UZeLg2Bbiceo0c-lIko";  //--> your Bot Token (Get from Botfather).
/* ======================================== */

/* ======================================== @myidbot ID */
// Use @myidbot to find out the chat ID of an individual or a group.
// Also note that you need to click "start" on a bot before it can message you.
String CHAT_ID = "2140096680";
/* ======================================== */

/* ======================================== Initialize WiFiClientSecure. */
WiFiClientSecure clientTCP;
/* ======================================== */

/* ======================================== Initialize UniversalTelegramBot. */
UniversalTelegramBot bot(BOTtoken, clientTCP);
/* ======================================== */

/* ======================================== Variables for millis. */
// Checks for new messages every 1 second (1000 ms).
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
/* ======================================== */

/* ======================================== Defines HIGH and LOW with ON and OFF (for LED FLash). */
#define ON HIGH
#define OFF LOW
/* ======================================== */

#define FLASH_LED_PIN 4             //--> LED Flash PIN (GPIO 4)

/* ________________________________________________________________________________ Subroutine to turn on or off the LED Flash. */
void LEDFlash_State (bool ledState) {
  digitalWrite(FLASH_LED_PIN, ledState);
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ Subroutines to handle what to do after a new message arrives. */
void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  /* ---------------------------------------- "For Loop" to check the contents of the newly received message. */
  for (int i = 0; i < numNewMessages; i++) {
    /* ::::::::::::::::: Check ID (ID obtained from IDBot/@myidbot). */
    /*
     * If the chat_id is different from your chat ID (CHAT_ID), it means that someone (that is not you) has sent a message to your bot.
     * If that’s the case, ignore the message and wait for the next message.
     */
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      Serial.println("Unauthorized user");
      Serial.println("------------");
      continue;
    }
    /* ::::::::::::::::: */

    /* ::::::::::::::::: Print the received message. */
    String text = bot.messages[i].text;
    Serial.print("Incoming command/message : ");
    Serial.println(text);
    /* ::::::::::::::::: */

    /* ::::::::::::::::: Check conditions based on commands sent from your telegram BOT. */
    // If it receives the "/start" message, we’ll send the valid commands to control the ESP. This is useful if you happen to forget what are the commands to control your board.
    String send_feedback_message = "";
    String from_name = bot.messages[i].from_name;

    // The condition if the command received is "/start".
    if (text == "/start") {
      Serial.println("Reply to the start command.");
      send_feedback_message += "From the ESP32-CAM :\n\n";
      send_feedback_message += "Welcome , " + from_name + "\n";
      send_feedback_message += "Use the following commands to interact with the ESP32-CAM \n";
      send_feedback_message += "/ping_esp32cam : Send ping to test connection\n";
      send_feedback_message += "/led_flash_on : Turn on the LED Flash\n";
      send_feedback_message += "/led_flash_off : Turn off the LED Flash";
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
      Serial.println("------------");
    }
    
    // The condition if the command received is "/ping_esp32cam".
    if (text == "/ping_esp32cam") {
      Serial.println("Reply to the ping command.");
      send_feedback_message += "From the ESP32-CAM :\n\n";
      send_feedback_message += "The ESP32 CAM replies to the ping command.\n";
      send_feedback_message += "Connection OK.\n\n";
      send_feedback_message += "/start : to see all commands.";
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
      Serial.println("------------");
    }
    
    // The condition if the command received is "/led_flash_on".
    if (text == "/led_flash_on") {
      LEDFlash_State(ON);
      Serial.println("LED Flash ON.");
      Serial.println("Reply to the led_flash_on command.");
      send_feedback_message += "From the ESP32-CAM :\n\n";
      send_feedback_message += "The ESP32 CAM replies to the led_flash_on command.\n";
      send_feedback_message += "LED Flash ON.\n\n";
      send_feedback_message += "/start : to see all commands.";
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
      Serial.println("------------");
    }
    
    // The condition if the command received is "/led_flash_off".
    if (text == "/led_flash_off") {
      LEDFlash_State(OFF);
      Serial.println("LED Flash OFF.");
      Serial.println("Reply to the led_flash_off command.");
      send_feedback_message += "From the ESP32-CAM :\n\n";
      send_feedback_message += "The ESP32 CAM replies to the led_flash_off command.\n";
      send_feedback_message += "LED Flash OFF.\n\n";
      send_feedback_message += "/start : to see all commands.";
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
      Serial.println("------------");
    }
    /* ::::::::::::::::: */
  }
  /* ---------------------------------------- */
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ VOID SETTUP() */
void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //--> Disable brownout detector.

   /* ---------------------------------------- Init serial communication speed (baud rate). */
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n");
  /* ---------------------------------------- */

  /* ---------------------------------------- Set LED Flash as output and make the initial state of the LED Flash is off. */
  pinMode(FLASH_LED_PIN, OUTPUT);
  LEDFlash_State(OFF);
  /* ---------------------------------------- */

  /* ---------------------------------------- Connect to Wi-Fi. */
  WiFi.mode(WIFI_STA);
  Serial.println("------------");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); //--> Add root certificate for api.telegram.org

  /* ::::::::::::::::: The process of connecting ESP32 CAM with WiFi Hotspot / WiFi Router. */
  /*
   * The process timeout of connecting ESP32 CAM with WiFi Hotspot / WiFi Router is 20 seconds.
   * If within 20 seconds the ESP32 CAM has not been successfully connected to WiFi, the ESP32 CAM will restart.
   * I made this condition because on my ESP32-CAM, there are times when it seems like it can't connect to WiFi, so it needs to be restarted to be able to connect to WiFi.
   */
  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    LEDFlash_State(ON);
    delay(250);
    LEDFlash_State(OFF);
    delay(250);
    if(connecting_process_timed_out > 0) connecting_process_timed_out--;
    if(connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }
  /* ::::::::::::::::: */
  
  LEDFlash_State(OFF);
  Serial.println();
  Serial.print("Successfully connected to ");
  Serial.println(ssid);
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP()); 
  Serial.println("------------");
  Serial.println();
  /* ---------------------------------------- */
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ VOID LOOP() */
void loop() {
  /* ---------------------------------------- Condition to check if any new messages come in. */
  // Checks for new messages every 1 second (see "botRequestDelay" value).
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println();
      Serial.println("------------");
      Serial.println("Got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
/* ________________________________________________________________________________ */
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
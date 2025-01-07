//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 02 ESP32 CAM Take and Send Photo to Telegram
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
#include "esp_camera.h"
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

/* ======================================== Defining the camera's GPIO on the ESP32 Cam. */
// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
/* ======================================== */

/* ======================================== Defines HIGH and LOW with ON and OFF (for LED FLash). */
#define ON HIGH
#define OFF LOW
/* ======================================== */

#define FLASH_LED_PIN 4             //--> LED Flash PIN (GPIO 4)

bool capturePhotoWithFlash = false; //--> Variable for trigger to turn on LED Flash when taking photos.

bool sendPhoto = false;             //--> Variables for photo sending triggers.

/* ________________________________________________________________________________ String function to parse String */
// Source : https://www.electroniclinic.com/reyax-lora-based-multiple-sensors-monitoring-using-arduino/
// This function is used to get the status of sending photos in json.
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ Subroutine for sending feedback messages when photos are successful or failed to be sent to telegram. */
void FB_MSG_is_photo_send_successfully (bool state) {
  String send_feedback_message = "";
  if(state == false) {
    send_feedback_message += "From the ESP32-CAM :\n\n";
    send_feedback_message += "ESP32-CAM failed to send photo.\n";
    send_feedback_message += "Suggestion :\n";
    send_feedback_message += "- Reset ESP32-CAM\n";
    send_feedback_message += "- Change FRAMESIZE (see Drop down frame size in void configInitCamera)\n";
    Serial.print(send_feedback_message);
    send_feedback_message += "\n\n";
    send_feedback_message += "/start : to see all commands.";
    bot.sendMessage(CHAT_ID, send_feedback_message, "");
  } else {
    Serial.println("Successfully sent photo.");
    send_feedback_message += "From the ESP32-CAM :\n\n";
    send_feedback_message += "Photo sent successfully.\n\n";
    send_feedback_message += "/start : to see all commands.";
    bot.sendMessage(CHAT_ID, send_feedback_message, ""); 
  }
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ Subroutine to turn on or off the LED Flash. */
void LEDFlash_State (bool ledState) {
  digitalWrite(FLASH_LED_PIN, ledState);
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ Subroutine for camera configuration. */
void configInitCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  /* ---------------------------------------- init with high specs to pre-allocate larger buffers. */
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; //--> FRAMESIZE_ + UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
    config.jpeg_quality = 10;  
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  
    config.fb_count = 1;
  }
  /* ---------------------------------------- */

  /* ---------------------------------------- camera init. */
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    Serial.println();
    Serial.println("Restart ESP32 Cam");
    delay(1000);
    ESP.restart();
  }
  /* ---------------------------------------- */

  /* ---------------------------------------- Drop down frame size for higher initial frame rate (Set the frame size) */
  /*
   * If the photo sent by the ESP32-CAM is corrupt or the ESP32-CAM fails to send the photo, to resolve it, follow the steps below :
   * - FRAMESIZE settings :
   *   > Change "s->set_framesize(s, FRAMESIZE_SXGA);" to a lower frame size, such as FRAMESIZE_VGA, FRAMESIZE_CIF and so on.
   * 
   * From the tests I did, the highest photo framesize that can be sent properly is "FRAMESIZE_SXGA".
   * If I use "FRAMESIZE_UXGA", the photo can still be sent, but the resolution of the received photo does not match the "FRAMESIZE_UXGA" resolution.
   * I'm still looking for a solution about that. If you know the cause and solution, please comment in the comments column on this video project on Youtube.
   */

  /*
   * UXGA   = 1600 x 1200 pixels
   * SXGA   = 1280 x 1024 pixels
   * XGA    = 1024 x 768  pixels
   * SVGA   = 800 x 600   pixels
   * VGA    = 640 x 480   pixels
   * CIF    = 352 x 288   pixels
   * QVGA   = 320 x 240   pixels
   * HQVGA  = 240 x 160   pixels
   * QQVGA  = 160 x 120   pixels
   */
   
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_SXGA);  //--> FRAMESIZE_ + UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  /* ---------------------------------------- */
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
    Serial.println(text);
    /* ::::::::::::::::: */

    /* ::::::::::::::::: Check conditions based on commands sent from your telegram BOT. */
    // If it receives the "/start" message, we’ll send the valid commands to control the ESP. This is useful if you happen to forget what are the commands to control your board.
    String send_feedback_message = "";
    String from_name = bot.messages[i].from_name;

    // The condition if the command received is "/start".
    if (text == "/start") {
      send_feedback_message += "From the ESP32-CAM :\n\n";
      send_feedback_message += "Welcome , " + from_name + "\n";
      send_feedback_message += "Use the following commands to interact with the ESP32-CAM \n";
      send_feedback_message += "/capture_photo : takes a new photo\n";
      send_feedback_message += "/capture_photo_with_LEDFlash : takes a new photo with LED FLash\n";
      send_feedback_message += "/flash_on : Turn on the LED Flash \n";
      send_feedback_message += "/flash_off : Turn off LED Flash \n";
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
      Serial.println("------------");
    }
    
    // The condition if the command received is "/flash_on".
    if (text == "/flash_on") {
      LEDFlash_State(ON);
      Serial.println("LED Flash On");
      Serial.println("------------");
      send_feedback_message += "From the ESP32-CAM :\n\n";
      send_feedback_message += "LED Flash On\n\n";
      send_feedback_message += "/start : to see all commands.";
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
    }
    
    // The condition if the command received is "/flash_off".
    if (text == "/flash_off") {
      LEDFlash_State(OFF);
      Serial.println("LED Flash Off");
      Serial.println("------------");
      send_feedback_message += "From the ESP32-CAM :\n\n";
      send_feedback_message += "LED Flash Off\n\n";
      send_feedback_message += "/start : to see all commands.";
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
    }
    
    // The condition if the command received is "/capture_photo".
    if (text == "/capture_photo") {
      sendPhoto = true;
      Serial.println("New photo request");
    }
    
    // The condition if the command received is "/capture_photo_with_LEDFlash".
    if (text == "/capture_photo_with_LEDFlash") {
      capturePhotoWithFlash = true;
      sendPhoto = true;
      Serial.println("New photo request");
    }
    /* ::::::::::::::::: */
  }
  /* ---------------------------------------- */
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ Subroutine for the process of taking and sending photos. */
String sendPhotoTelegram() {
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  /* ---------------------------------------- The process of taking photos. */
  Serial.println("Taking a photo...");

  /* ::::::::::::::::: Turns on the LED Flash if the command received is "/capture_photo_with_LEDFlash". */
  if(capturePhotoWithFlash == true) {
    LEDFlash_State(ON);
  }
  delay(1000);
  /* ::::::::::::::::: */
  
  /* ::::::::::::::::: Taking a photo. */ 
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    Serial.println("Restart ESP32 Cam");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  /* ::::::::::::::::: */

  /* ::::::::::::::::: Turn off the LED Flash after successfully taking photos. */
  if(capturePhotoWithFlash == true) {
    LEDFlash_State(OFF);
    capturePhotoWithFlash = false;
  }
  /* ::::::::::::::::: */
  Serial.println("Successful photo taking.");
  /* ---------------------------------------- */
  

  /* ---------------------------------------- The process of sending photos. */
  Serial.println("Connect to " + String(myDomain));

  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    Serial.print("Send photos");
    
    String head = "--Esp32Cam\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n";
    head += CHAT_ID; 
    head += "\r\n--Esp32Cam\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--Esp32Cam--\r\n";

    /* ::::::::::::::::: If you only use low framesize, such as CIF, QVGA, HQVGA and QQVGA, then use the variables below to save more memory. */
    //uint16_t imageLen = fb->len;
    //uint16_t extraLen = head.length() + tail.length();
    //uint16_t totalLen = imageLen + extraLen;
    /* ::::::::::::::::: */

    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;
  
    clientTCP.println("POST /bot"+BOTtoken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=Esp32Cam");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   //--> timeout 10 seconds (To send photos.)
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
      Serial.print(".");
      delay(100);      
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    clientTCP.stop();
    Serial.println(getBody);

    /* ::::::::::::::::: The condition to check if the photo was sent successfully or failed. */
    // If the photo is successful or failed to send, a feedback message will be sent to Telegram.
    if(getBody.length() > 0) {
      String send_status = "";
      send_status = getValue(getBody, ',', 0);
      send_status = send_status.substring(6);
      
      if(send_status == "true") {
        FB_MSG_is_photo_send_successfully(true);  //--> The photo was successfully sent and sent an information message that the photo was successfully sent to telegram.
      }
      if(send_status == "false") {
        FB_MSG_is_photo_send_successfully(false); //--> The photo failed to send and sends an information message that the photo failed to send to telegram.
      }
    }
    if(getBody.length() == 0) FB_MSG_is_photo_send_successfully(false); //--> The photo failed to send and sends an information message that the photo failed to send to telegram.
    /* ::::::::::::::::: */
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  
  Serial.println("------------");
  return getBody;
  /* ---------------------------------------- */
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ VOID SETTUP() */
void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //--> Disable brownout detector.

   /* ---------------------------------------- Init serial communication speed (baud rate). */
  Serial.begin(115200);
  delay(1000);
  /* ---------------------------------------- */

  /* ---------------------------------------- Set LED Flash as output and make the initial state of the LED Flash is off. */
  pinMode(FLASH_LED_PIN, OUTPUT);
  LEDFlash_State(OFF);
  /* ---------------------------------------- */

  /* ---------------------------------------- Config and init the camera. */
  Serial.println();
  Serial.println();
  Serial.println("Start configuring and initializing the camera...");
  configInitCamera();
  Serial.println("Successfully configure and initialize the camera.");
  Serial.println();
  /* ---------------------------------------- */

  /* ---------------------------------------- Connect to Wi-Fi. */
  WiFi.mode(WIFI_STA);
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
  Serial.println();
  /* ---------------------------------------- */
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ VOID LOOP() */
void loop() {
  /* ---------------------------------------- Conditions for taking and sending photos. */
  if (sendPhoto) {
    Serial.println("Preparing photo...");
    sendPhotoTelegram(); 
    sendPhoto = false; 
  }
  /* ---------------------------------------- */

  /* ---------------------------------------- Condition to check if any new messages come in. */
  // Checks for new messages every 1 second (see "botRequestDelay" value).
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println();
      Serial.println("------------");
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
/* ________________________________________________________________________________ */
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
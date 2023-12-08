
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define BOTtoken "6736977239:AAGBpBdbA-ucR8xencH8aSdPw_81sA5jAdQ"  
char ssid[] = "Gagan";         // Wifi SSID 
char password[] = "12345678";     // Wifi Password
String buttons[] = {"Light 1", "Light 2 "};  
int pin[] = {5, 4};                      

bool protection = 0;                      
int chatID_acces[] = {}; 
String on_symbol="✅ ";  
String off_symbol="☑ "; 


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
int quantity;
int Bot_mtbs = 3000;
long Bot_lasttime;   
bool Start = false;
const int ledPin = 2;
int ledStatus = 0;
String keyboardJson = "";

int messageID;

// Define global variables to track relay state and timing
bool relayOpen = false;
bool automaticClosePending = false;
unsigned long lastCommandTime = 0;
const unsigned long autoCloseDelay = 2000;  // 1 second delay

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String m_id = String(bot.messages[i].message_id);
    if (bot.messages[i].type == "callback_query") {
      // Existing code for handling callback queries...
    } else {
      String text = bot.messages[i].text;
      Serial.println(m_id);
      String from_name = bot.messages[i].from_name;
      if (from_name == "") from_name = "Guest";
      int i2 = 0;
      do {
        if (!protection || String(chatID_acces[i2]) == chat_id) {
          if (text == "/switch") {
            // Existing code for sending the status message...
          }

          if (text == "/open") {
            if (!relayOpen) {
              // If the relays are not already open, open them
              for (int i = 0; i < quantity; i++) {
                digitalWrite(pin[i], LOW);  // Turn off the relay
              }
              relayOpen = true;

              // Set the time for automatic relay closure
              automaticClosePending = true;
              lastCommandTime = millis();

              bot.sendMessage(chat_id, "Both relays are now OPEN. They will reset to CLOSED in 2 second.");
            } else if (automaticClosePending) {
              // The relays are already open, so cancel the automatic close
              automaticClosePending = false;
              bot.sendMessage(chat_id, "Relays are already OPEN. The reset to CLOSED is canceled.");
            }
          }

          if (text == "/close") {
            // Close the relays
            for (int i = 0; i < quantity; i++) {
              digitalWrite(pin[i], HIGH);  // Turn on the relay
            }
            relayOpen = false;
            automaticClosePending = false;
            bot.sendMessage(chat_id, "Both relays are now CLOSED.");
          }

          // Automatic relay closure after 1 second if pending
          if (automaticClosePending && millis() - lastCommandTime > autoCloseDelay) {
            for (int i = 0; i < quantity; i++) {
              digitalWrite(pin[i], HIGH);  // Turn on the relay
            }
            relayOpen = false;
            automaticClosePending = false;
            bot.sendMessage(chat_id, "Both relays are now CLOSED automatically.");
          }

          break;
        } else {
          if (i2 == ((sizeof(chatID_acces) / sizeof(int)) - 1) && text == "/open" || (sizeof(chatID_acces) / sizeof(int)) == 0 && text == "/open") {
            bot.sendMessage(chat_id, "No access, Chat ID: " + chat_id, "");
          }
        }

        i2++;
      } while (i2 < (sizeof(chatID_acces) / sizeof(int)));
    }
  }
}





void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
  Serial.print(".");
  delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  quantity=sizeof(pin)/sizeof(int);
  for (int i=0; i<quantity; i++) {
  pinMode(pin[i], OUTPUT);
  digitalWrite(pin[i], HIGH);
      }

  for (int i=0; i<quantity; i++) {
  if(i==0) keyboardJson += "[";
  keyboardJson += "[{ \"text\" : \"";
  keyboardJson += buttons[i];
  keyboardJson += "\", \"callback_data\" : \"";
  keyboardJson += buttons[i];
  keyboardJson += "\" }]";
  if(i==quantity-1){
    keyboardJson += "]";
    }else{
  keyboardJson += ",";  
  }
  }
  delay(10);
  client.setInsecure();
}

void loop() {
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    Bot_lasttime = millis();
  }
}

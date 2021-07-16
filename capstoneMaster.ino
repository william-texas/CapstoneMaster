SYSTEM_THREAD(ENABLED);
#include "MQTT.h"
#include "SparkFun_VCNL4040_Arduino_Library.h"
void callback(char *topic, byte *payload, unsigned int length);
MQTT client("lab.thewcl.com", 1883, callback);
VCNL4040 proximitySensor;
volatile bool interruptOccured = false;                                                                                                                                                                                       //must be declared volatile if used inside an isr.
uint16_t count1 = 0;                                                                                                                                                                                                          // num of 1s
uint16_t count2 = 0;                                                                                                                                                                                                          // num of 2s
uint16_t count3 = 0;                                                                                                                                                                                                          // num of 3s
String types[5] = {"Adjective", "Adjective", "Noun", "Noun", "Plural noun"};                                                                                                                                                  //, "A game", "Plural noun", "Verb ending in 'ing'", "Verb ending in 'ing'", "Plural noun", "Verb ending in 'ing'"};
String words[5] = {"A vacation is when you take a trip to some ", " place with your ", " family. Usually you go to some place that is near a/an ", " or up on a/an ", " . A good vacation place is one where you can ride "}; //, "or play", "or go hunting for", ". I like to spend my time", "or", ". When parents go on a vacation, they spend their time eating three", " a day, and fathers play golf, and mothers sit around"};
String answers[5];
volatile int countPlace = 0; // counts which place you are on in answers[] array
String ac1 = "";             // answer choice 1
String ac2 = "";             // answer choice 2
String ac3 = "";             // answer choice 3
int maxVotes = 5;
bool paused = false;
bool allVotesIn = false;
bool hasSentType = false; //for publish adjective etc
bool hasSentVoteMessage = false;
String voteMessage = "Start voting.";
void isr()
{
  // your interrupt handler code here
  interruptOccured = true;
}
void callback(char *topic, byte *payload, unsigned int length)
{
  if (paused == false) {
    String pre = "";
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    pre = p;
    if (length > 1 && p[0] == '1')
    {
      ac1 = pre.substring(2, length + 1);
    }
    if (length > 1 && p[0] == '2')
    {
      ac2 = pre.substring(2, length + 1);
    }
    if (length > 1 && p[0] == '3')
    {
      ac3 = pre.substring(2, length + 1);
    }
    if (length == 1 && p[0] == '1')
    {
      count1++;
    }
    if (length == 1 && p[0] == '2')
    {
      count2++;
    }
    if (length == 1 && p[0] == '3')
    {
      count3++;
    }
    //delay(10000);
    // make own function call it after 10 sec
    // voteFunction()
  }
}
void setup()
{
  pinMode(D6, INPUT_PULLUP);         // defaults to HIGH
  attachInterrupt(D6, isr, FALLING); // D6 will now generate an interrupt on the falling edge and will run the code in the isr
  Serial.begin(9600);
  Wire.begin();
  proximitySensor.begin();
  client.connect(System.deviceID());
  client.subscribe("madlibs");
}
 // if interrupt button is pressed
//plugging this argon into power is meant to be the trigger that starts the game. to restart it, press the reset button.
//each prompt lets you have 10 seconds to decide your answer.
void loop()
{
  unsigned int proxValue = proximitySensor.getProximity();
  bool value = false;      // button true or false
  value = digitalRead(D6); // reads the button either true or false
  if (interruptOccured)
  {
    interruptOccured = false;
  }
  if (value == 0)
  {
    if (paused == true)
    {
      interrupts(); // enables interrupt handling
      paused = false;
      client.publish("madlibs", "UNPAUSED");
      delay(500);
    }
    else
    {
      noInterrupts();
      paused = true;
      Serial.println();
      Serial.println("in the false");
      delay(500);
      client.publish("madlibs", "PAUSED");
    }
  }
  if (client.isConnected())
  {
      client.loop();
      //Serial.print("hasSentType:");
      //Serial.println(hasSentType);
      if (hasSentType == false)
      {
        client.publish("madlibs", types[countPlace]);
        hasSentType = true;
      }
      while (ac1 != "" && ac2 != "" && ac3 != "" && hasSentVoteMessage == false)
      {
        client.publish("madlibs", voteMessage);
        hasSentVoteMessage = true;
      }
      while (count1 + count2 + count3 == maxVotes)
      {
        //Serial.print("totalVotes");
        //Serial.println(count1 + count2 + count3);
        //Serial.println(maxVotes);
        if (count1 > count2 && count1 > count3)
        { //need to reset all variables that run once
          client.publish("madlibs", ac1);
          Serial.println("Answer 1 has won: ");
          Serial.println(ac1);
          answers[countPlace] = ac1;
          count1 = 0;
          count2 = 0;
          count3 = 0;
          ac1 = "";
          ac2 = "";
          ac3 = "";
          hasSentType = false;
          hasSentVoteMessage = false;
        }
        else if (count2 > count1 && count2 > count3)
        {
          client.publish("madlibs", ac2);
          Serial.println("Answer 2 has won: ");
          Serial.println(ac2);
          answers[countPlace] = ac2;
          count1 = 0;
          count2 = 0;
          count3 = 0;
          ac1 = "";
          ac2 = "";
          ac3 = "";
          hasSentType = false;
          hasSentVoteMessage = false;
        }
        else if (count3 > count2 && count3 > count1)
        {
          client.publish("madlibs", ac3);
          Serial.println("Answer 3 has won: ");
          Serial.println(ac3);
          answers[countPlace] = ac3;
          count1 = 0;
          count2 = 0;
          count3 = 0;
          ac1 = "";
          ac2 = "";
          ac3 = "";
          hasSentType = false;
          hasSentVoteMessage = false;
        }
        // if there is a tie between two counts

        else if(count1==count2){
          String tieString = "";
          tieString = "It was a tie between choice 1 and 2, choice 1 wins by chance";
          client.publish("madlibs", tieString);
          client.publish("madlibs", ac1);
          Serial.println("Answer 1 has won: ");
          Serial.println(ac1);
          answers[countPlace] = ac1;
          count1 = 0;
          count2 = 0;
          count3 = 0;
          ac1 = "";
          ac2 = "";
          ac3 = "";
          hasSentType = false;
          hasSentVoteMessage = false;
        }
        else if(count2==count3){
          String tieString = "";
          tieString = "It was a tie between choice 2 and 3, choice 2 wins by chance";
          client.publish("madlibs", tieString);
          client.publish("madlibs", ac2);
          Serial.println("Answer 2 has won: ");
          Serial.println(ac2);
          answers[countPlace] = ac2;
          count1 = 0;
          count2 = 0;
          count3 = 0;
          ac1 = "";
          ac2 = "";
          ac3 = "";
          hasSentType = false;
          hasSentVoteMessage = false;
        }
        else if(count1 == count3){
          String tieString = "";
          tieString = "It was a tie between choice 1 and 3, choice 3 wins by chance";
          client.publish("madlibs", tieString);
          client.publish("madlibs", ac3);
          Serial.println("Answer 3 has won: ");
          Serial.println(ac3);
          answers[countPlace] = ac3;
          count1 = 0;
          count2 = 0;
          count3 = 0;
          ac1 = "";
          ac2 = "";
          ac3 = "";
          hasSentType = false;
          hasSentVoteMessage = false;
        }
        Serial.print("This was the answer selected: ");
        Serial.println(answers[countPlace]);
        countPlace++;
        Serial.print(countPlace);
        //delay(5000);
      }
      if (countPlace == 5)
      { // 5 for debug
        String output = "";
        for (int i = 0; i < 5; i++)
        {
          output += words[i];
          output += answers[i];
        }
        output += ".";
        client.publish("madlibs", output);
        client.publish("madlibs", "The game has ended, next game will begin!");
        countPlace = 0;
      }
      if (proxValue > 2000) {
        String output = "";
        for (int i = 0; i < countPlace; i++)
        {
          output += words[i];
          output += answers[i];
        }
        output += ".";
        client.publish("madlibs", output);
        delay(1000);
      }
  }
  else
  {
    client.connect(System.deviceID());
    client.subscribe("madlibs");
  }
}
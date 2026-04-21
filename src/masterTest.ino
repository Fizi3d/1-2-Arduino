//D5
#define CSN_PIN 5
//D3
#define CE_PIN 3
#define AMPLIFICATION 1
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <LiquidCrystal.h>

//RS, E, D4, D5, D6, D7
LiquidCrystal lcd(2,4,6,7,8,9); 

RF24 radio(CE_PIN, CSN_PIN);

const String drawWords[6] = {"DRAW!", "FLAW!", "DRAG!", "DREW!", "DROW!", "DRAT!"};

const String status[20] = {"The can feels", "stable :)","The can is firm","to the touch","LOOK OUT","BEHIND YOU","STOP! STOP NOW!","wait, nevermind","speed up a bit","ur probably fine","You hear a faint","scream inside","The can is now"," very warm", "it's about time", "slowpoke","something smells","funny","DRAW!","oops, wrong game"};


const byte txAddr[6] = "00001"; // console → controller
const byte rxAddr[6] = "00002"; // controller → console

//menu
int menuIndex = 0;
String menuItems[3] = {"Quick Draw", "Soda Shake", "Samurai Slash"};

//button pin
#define BTN_UP A0

#define BTN_SELECT A1




void display(String line1, String line2=""){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(line1);
  lcd.setCursor(0,1);
  lcd.print(line2);
}

// ================= MENU =================
void menu(){
  lcd.clear();

  for(int i = 0; i < 2; i++){
    int itemIndex = (menuIndex + i) % 3;

    lcd.setCursor(0, i);

    if(i == 0){
      lcd.print("> ");
    } 
    
    else {
      lcd.print("  ");
    }

    lcd.print(menuItems[itemIndex]);
  }
}



void setup() {
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  Serial.begin(9600);

  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(76);

  radio.openWritingPipe(txAddr);
  radio.openReadingPipe(1, rxAddr);

  radio.stopListening();

  //Serial.println("Console ready");

  lcd.begin(16,2);  //16 columns and 2 rows

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  delay(1000);

  display("Console Ready");
  delay(1500);


  menu();
}


/*READ THIS IF YOU'RE GOING TO IMPLEMENT ANY GAMES

You don't need to know exactly how the radio transmission works, except that the controllers communicate to the console by directly sending info.
Game states are to be stored on the console, and controller actions are stored on controllers.
Only send the console your controller data if it's a relevant action.

Depending on what you're sending the console, your data will go inside a custom Struct of maximum 32 bytes.
The struct can contain literally anything you want to send the console as long as it's within this limit.
The point of this is to send all necessary data in one instruction.

an example of your data struct might be:
struct quickDraw{
  bool button;
  int y_position;
  int x_position;
}

I'll handle the specifics about sending and receiving info.
For now, if you want to write code to send the console/controllers something, write it like this and I'll change it later:
SEND *CONSOLE/CONTROLLER* StructData

*/


void loop() {
//This is gonna be the loop for the master when it's not in a game. Games will loop inside their own function.
  if(digitalRead(BTN_UP) == LOW){
    menuIndex++;
    if(menuIndex > 2){
      menuIndex = 0;
    }
    menu();
    delay(200);
  }

  if(digitalRead(BTN_SELECT) == LOW){
    
    if(menuIndex==0){
      quickDraw();
    }
    else if (menuIndex == 1){
      sodaShake();
    }
    else if (menuIndex == 2){
      samuraiSlash();
    }
    display("Returning...", "Menu");
    delay(1500);
    menu();
    
  }
}


void quickDraw() {
  //Serial.println("started");
  int gameId = 1;
  radio.write(&gameId,sizeof(gameId));
  Serial.println("initialized");


  //DISPLAY START TEXT
  display("Wait for \"DRAW\"", "Then SHOOT!");
  delay(3000);
  display("False word=", "Dont press!");
  delay(3000);

  display("START!");
  delay(1000);


  bool inGame = true;
  bool draw = false;
  radio.startListening();
  radio.flush_rx();

  while (inGame) {

    int randomTimer = random(2000,5000);
    int randomWord = random(7);
    int identifier;

    // Wait random time
    for (int i = 0; i < randomTimer; i += 5) {
      delay(5);

      if (radio.available()) {
        radio.read(&identifier, sizeof(identifier));
        if (!draw) {
          if (identifier==1) {
            display("Too Early!", "Green Loses!");
          }
          else if(identifier==2){
            display("Too Early!", "Orange Loses!");
          }
        }
        else{
          if (identifier == 1) {
            display("Green Wins!");
            if (radio.available()) {
              display("It's A Tie!");
            }
          }
          else if (identifier == 2) {
            display("Orange Wins!");
            if (radio.available()) {
              display("It's A Tie!");
            }
          }
        }
          inGame = false;
          break;
      }
    }
    if (draw && inGame) {
      inGame = false;
      display("Too Slow!","No Winners!");
      delay(3000);
    }
    randomWord > 0 ? randomWord--: randomWord = randomWord;
    String generatedWord = drawWords[randomWord];
    if (generatedWord == "DRAW!") draw = true;
    // Show word
    if (inGame) {
      display(generatedWord);
    }
  }
  radio.flush_rx();
  radio.stopListening();
  int end = 5;
  radio.write(&end, sizeof(end));

}

/*instructions: one controller is going to be a can of soda. Players take turns shaking and passing it around.
Shaking the controller increases the amount of pressure in the can, until it reaches a threshold and explodes.
When a player has shaken it as much as they want, they press the button and pass it to the next person. */

void sodaShake() {
  Serial.println("started");
  bool inGame = true;
  int gameId = 2;
  radio.write(&gameId, sizeof(gameId));
  Serial.println("initialized");
  delay(400);
  int thresh = random(300,600);
  radio.write(&thresh,sizeof(thresh));
  float shakeValue;
  
  display("Shake the soda!","but be careful..");
  delay(2000);
  display("Too much and","it might burst!");
  delay(2000);

  while (inGame) {
    delay(1000);
    display("Shake!");
    radio.startListening();
    while (!radio.available());
    radio.read(&shakeValue, sizeof(shakeValue));

    if (shakeValue == -1) {
      display("BOOM!","You Lost!");
      radio.flush_rx();
      radio.stopListening();
      delay(5000);
      return;
    }

    float thing = shakeValue;
    float otherThing = thresh;
    float huh = shakeValue / thresh * 5;
    int what = int(trunc(huh));
    if (what < 2) {
     display(status[what*2], status[what*2+1]);
    }
    else {
      int funny = random(2,9);
      display(status[funny*2], status[funny*2+1]);
    }

    delay(3000);
    display("Ready?");
  }
}


  void samuraiSlash() {
    bool inGame = true;

    display("Samurai Duel","Commence!");
    delay(2000);
    display("Green, hold up", "your sword!");
    delay(3000);
    display("Orange, get", "ready to catch!");
    int gameId = 3;
    radio.write(&gameId,sizeof(gameId));
    Serial.println("initialized");
    radio.startListening();
    delay(3000);


    bool greenSlicing;
    int greenScore = 0;
    int orangeScore = 0;
    while (inGame) {
      display("START!");
      radio.startListening();
      radio.flush_rx();
      bool waiting = true;
      int packet;
      while (waiting) {
        delay(5);
        if (radio.available()) {
          radio.read(&packet,sizeof(packet));
          if (packet == 7) {
            if (greenSlicing) {
              display("Orange got", "sliced!");
              greenScore++;
            }
            else {
              display("Green got", "sliced!");
              greenScore++;
            }
            waiting = false;
            delay(2000);
          }
          else if (packet == 6) {
            delay(200);
            if (radio.available()) {
              radio.read(&packet,sizeof(packet));
              if (packet == 7) {
                if (greenSlicing) {
                  display("Orange caught", "the sword!");
                  orangeScore++;
                }
                else {
                  display("Green caught", "the sword!");
                  greenScore++;
                }
                waiting = false;
                delay(2000);
              }
            }
          }
        }
      }
      radio.stopListening();
      delay(1);
      String orangeString = "Orange: " + orangeScore;
      String greenString = "Green: " + greenScore;
      display("test","orangeString");
      if (greenScore - orangeScore > 2) {
        display("Green Wins!");
        delay(2000);
        inGame = false;
        int end = 9;
        radio.write(&end, sizeof(end));
      }
      else if (orangeScore - greenScore > 2) {
        display("Orange Wins!");
        delay(2000);
        inGame = false;
        int end = 9;
        radio.write(&end, sizeof(end));
      }
      else {
        int swap = 8;
        radio.write(&swap,sizeof(swap));

        greenSlicing = !greenSlicing;
        if (greenSlicing) {
          display("Green, hold up", "your sword!");
          delay(2000);
          display("Orange, get", "ready to catch!");
          delay(2000);
        }
        else {
          display("Orange, hold up", "your sword!");
          delay(2000);
          display("Green, get", "ready to catch!");
          delay(2000);
        }
      }
    }
  }

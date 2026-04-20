//D5
#define CSN_PIN 5;
//D3
#define CE_PIN 3;
#define AMPLIFICATION 1;
#define WRITE_ADDRESS "00001";
#define CONTROLLER_1_ADDRESS "00002";
#define CONTROLLER_2_ADDRESS "00003";
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <LiquidCrystal.h>

//RS, E, D4, D5, D6, D7
LiquidCrystal lcd(2,4,6,7,8,9); 

RF24 radio(CE_PIN, CSN_PIN);

const String drawWords[6] = {"DRAW!", "FLAW!", "DRAG!", "DREW!", "DROW!", "DRAT!"}

const byte addresses[][6] = {WRITE_ADDRESS,CONTROLLER_1_ADDRESS,CONTROLLER_2_ADDRESS};

//menu
int menuIndex = 0;
String menuItems[2] = {"Quick Draw", "Soda Shake"};

//button pins
#define BTN_UP 9
#define BTN_DOWN 10
#define BTN_SELECT A0

void display(String line1, String line2=""){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(line1);
  lcd.setCursor(0,1);
  lcd.print(line2);
}

void menu(){
  lcd.clear();
  lcd.setCursor(0,0);

  if(menuIndex == 0){
    lcd.print(">");
  }
  else{
    lcd.print(" ");
  }

  lcd.print(menItems[0]);

  lcd.setCursor(0,1);

  if(menuIndex==1){
    lcd.print(">");
  }
  else{
    lcd.print(" ");
  }
  lcd.print(menuItems[1]);
}
void setup() {
  //opens the radio channel at the address in writing mode.
  radio.begin();
  radio.openReadingPipe(1,addresses[1]);
  radio.openReadingPipe(2,addresses[2]);
  radio.openWritingPipe(addresses[0]);
  radio.setPALevel(AMPLIFICATION);
  radio.stopListening();
  lcd.begin(16,2);  //16 columns and 2 rows

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);

  display("Quick Draw", "Loading...");
  delay(2000);
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
  if(digitalRead(BTN_UP)==LOW){
    menuIndex--;
    if(menuIndex<0){
      menuIndex=1;
    }
    menu();
    delay(200);
  }

  if(digitalRead(BTN_DOWN)==LOW){
    menuIndex++;
    if(menuIndex>1){
      menuIndex=0;
    }
    menu();
    delay(200);
  }

  if(digitalRead(BTN_SELECT)==LOW){
    
    if(menuIndex==0){
      quickDraw();
    }
    else{
      sodaShake();
    }

    menu();
    delay(300);
  }
}


void quickDraw() {


  //DISPLAY START TEXT
  display("Wait for word", "Then SHOOT!");
  delay(3000);
  display("False word=", "Dont press!");
  delay(3000);

  display("START!");
  delay(1000);
  


  bool inGame = true;
  bool draw = false;

    while (inGame) {

    int randomTimer = random(500,7000);
    int randomWord = random(6);
    String generatedWord = drawWords[randomWord];
    int identifier;

    // Wait random time
    for (int i = 0; i < randomTimer; i += 5) {
      delay(5);

      if (radio.available()) {
        radio.read(&identifier, sizeof(identifier));
        display("Too Early!", "You Lose!");
        delay(3000);
        return;
      }
    }

    // Show word
    display(generatedWord);

    bool isDraw = (generatedWord == "DRAW!");

    unsigned long startTime = millis();

    while (millis() - startTime < 3000) {

      if (radio.available()) {
        radio.read(&identifier, sizeof(identifier));

        delay(50);
        if (radio.available()) {
          radio.read(&identifier, sizeof(identifier));
          display("Tie!", "Both shot!");
        }
        else {
          if (!isDraw) {
            display("Wrong Word!", "You Lose!");
          }
          else {
            if (identifier == 1) {
              display("Blue Wins!");
            } 
            else {
              display("Red Wins!");
            }
          }
        }

        delay(4000);
        return;
      }
    }

    display("Too Slow!");
    delay(3000);
    return;
  }
}

/*instructions: one controller is going to be a can of soda. Players take turns shaking and passing it around.
Shaking the controller increases the amount of pressure in the can, until it reaches a threshold and explodes.
When a player has shaken it as much as they want, they press the button and pass it to the next person. 
void sodaShake() {
  bool inGame = true;
  while (inGame) {

  }
}

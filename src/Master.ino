#define CSN_PIN 8;
#define CE_PIN 7;
#define AMPLIFICATION 1;
#define WRITE_ADDRESS "00001";
#define CONTROLLER_1_ADDRESS "00002";
#define CONTROLLER_2_ADDRESS "00003";
#define LED_PIN 5;
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(CE_PIN, CSN_PIN);

const String drawWords[6] = {"DRAW!", "FLAW!", "DRAG!", "DREW!", "DROW!", "DRAT!"}

const byte addresses[][6] = {WRITE_ADDRESS,CONTROLLER_1_ADDRESS,CONTROLLER_2_ADDRESS};

void setup() {
  //opens the radio channel at the address in writing mode.
  radio.begin();
  radio.openReadingPipe(1,addresses[1]);
  radio.openReadingPipe(2,addresses[2]);
  radio.openWritingPipe(addresses[0]);
  radio.setPALevel(AMPLIFICATION);
  radio.stopListening()
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

}


void quickDraw() {
  bool inGame = true;
  bool draw = false;

  //DISPLAY START TEXT

  //DISPLAY the word "START!" when instructions end

  while (inGame) {

    int randomTimer = random(500,7000);
    int randomWord = random(5);
    String generatedWord = drawWords(randomWord);
    int identifier;


    for (int i = 0; i < randomTimer; random += 5) {
      delay(5);


      if (radio.available()) {
        radio.read(&identifier, sizeOf(identifier));
        if (radio.available()) {
          radio.read(&identifier, sizeOf(identifier));
          //DISPLAY "It was a tie!"
        }
        else {
          if (identifier == 1) {
            //DISPLAY "Blue Wins!"
          }
          else {
            //DISPLAY "Red Wins!"
          }
        }
        inGame = false;
        delay(5000);
      }

    }

    if (randomWord == 0) draw = true;

    //DISPLAY generatedWord

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

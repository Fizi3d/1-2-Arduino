#include <SPI.h>
#include <RF24.h>

RF24 radio(3, 5); // CE, CSN

const byte rxAddr[6] = "00001"; // console → controller
const byte txAddr[6] = "00002"; // controller → console

void setup() {
  Serial.begin(9600);

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(76);

  radio.openReadingPipe(1, rxAddr);
  radio.openWritingPipe(txAddr);

  radio.startListening();

  Serial.println("Controller ready");
}

void loop() {
  if (radio.available()) {
    int msg;
    radio.read(&msg, sizeof(msg));

    Serial.print("Got: ");
    Serial.println(msg);

    radio.stopListening();
    delayMicroseconds(200);

    int reply = 2;
    radio.write(&reply, sizeof(reply));

    Serial.println("Replied");

    radio.startListening();
  }
}

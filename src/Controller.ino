#include <SPI.h>
#include <RF24.h>

RF24 radio(5, 6); // CE, CSN

const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(76);

  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("TX Ready");
}

void loop() {
  int data = 1;

  bool ok = radio.write(&data, sizeof(data));

  Serial.print("Sent OK: ");
  Serial.println(ok);

  delay(500);
}

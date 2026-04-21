#define CSN_PIN 6
#define CE_PIN 5
#define AMPLIFICATION 1
#define READ_ADDRESS "00001"

//2 for red
#define IDENTIFIER 1

#define WRITE_ADDRESS "00002"

const byte rxAddr[6] = "00001"; // console → controller
const byte txAddr[6] = "00002"; // controller → console

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(CE_PIN, CSN_PIN);

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>


float Ax;
float Ay;
float Az;

// Values unique to each MPU6050.
float xMax = 1.02;
float xMin = -0.98;
float yMax = 1.01;
float yMin = -0.98;
float zMax = 1.13;
float zMin = -0.94;

float xOffset = (xMax+xMin)/2;
float yOffset = (yMax+yMin)/2;
float zOffset = (zMax+zMin)/2;

float xScale = 2/(xMax-xMin);
float yScale = 2/(yMax-yMin);
float zScale = 2/(zMax-zMin);

float rollRAW;
float pitchRAW;
float rollLP=0;
float pitchLP=0;

float Gx;
float Gy;
float Gz;

float gyroXOffset = 0;
float gyroYOffset = 0;
float gyroZOffset = 0;

float rollG = 0;
float pitchG = 0;
float yawG = 0;

float rollComp = 0;
float pitchComp = 0;

float deltaRoll = 0;
float deltaPitch = 0;

unsigned long tStart;

float accMag = 0;
float prevAccMag = 0;
float deltaAcc = 0;
float shakeValue = 0;
float totalShakeValue = 0;
float maxShakeValue = 1000;

unsigned long currentGame;
const int button = 4;


Adafruit_MPU6050 mpu;

void calibrateGyro() {
  // Calibrates Gyroscope to reduce drift
  Serial.println("Calibrating Gyroscope. Keep controller stationary.");
  delay(500);
  float sumX = 0;
  float sumY = 0;
  float sumZ = 0;
  int numpoints = 100;
  
  for (int i = 0; i < numpoints; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sumX = sumX + g.gyro.x;
    sumY = sumY + g.gyro.y;
    sumZ = sumZ + g.gyro.z;
    delay(10);
  }
  gyroXOffset = sumX/numpoints;
  gyroYOffset = sumY/numpoints;
  gyroZOffset = sumZ/numpoints;
  Serial.println("Calibration complete!");
}

void calculateAngle() {
  // Collects accel, gyro, and temp data from MPU
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  // Assigns accel and gyro values accordingly and accounts for offsets
  Ax = a.acceleration.x/9.81;
  Ay = a.acceleration.y/9.81;
  Az = a.acceleration.z/9.81;

  Ax = xScale * (Ax-xOffset);
  Ay = yScale * (Ay-yOffset);
  Az = zScale * (Az-zOffset);

  Gx = g.gyro.x - gyroXOffset;
  Gy = g.gyro.y - gyroYOffset;
  Gz = g.gyro.z - gyroZOffset;

  float dt = (millis() - tStart)/1000.0;
  tStart = millis();

  // Calculates roll, pitch, and yaw via gyroscope
  deltaRoll = dt * Gy * 180.0/3.14;
  deltaPitch = dt * Gx * 180.0/3.14;

  rollG = rollG + deltaRoll;
  pitchG = pitchG + deltaPitch;
  yawG = yawG + dt * Gz * 180.0/3.14;
  
  // Calculates pitch and roll via accelerometer and applies a Low Pass Filter
  pitchRAW = atan2(Ay, sqrt(Az*Az + Ax*Ax))*180.0/3.14;
  rollRAW = atan2(-Ax, sqrt(Az*Az + Ay*Ay))*180.0/3.14;

  pitchLP = 0.75*pitchLP + 0.25*pitchRAW;
  rollLP = 0.75*rollLP + 0.25*rollRAW;

  // Combines both pitch and roll methods for a Complementary Filter
  // These are the most accurate values. Use these for any games.
  rollComp = 0.25*rollLP + 0.75*(rollComp + deltaRoll);
  pitchComp = 0.25*pitchLP + 0.75*(pitchComp + deltaPitch);
}

void calculateMovement() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  Ax = a.acceleration.x/9.81;
  Ay = a.acceleration.y/9.81;
  Az = a.acceleration.z/9.81;

  Ax = xScale * (Ax-xOffset);
  Ay = yScale * (Ay-yOffset);
  Az = zScale * (Az-zOffset);

  accMag = sqrt(Ax*Ax + Ay*Ay + Az*Az);
  deltaAcc = abs(accMag - prevAccMag);
  shakeValue = shakeValue + deltaAcc;
  prevAccMag = accMag;
}

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
  
  pinMode(button, INPUT);
  // Initializes the MPU6050
  mpu.begin();
  Serial.println("MPU6050 Started!");
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ);

  calibrateGyro();

  tStart = millis();

  delay(5);
  radio.startListening();
  radio.flush_rx();

}


void loop() {
  //this is the state of the controller when it's not in a game. It just listens for a signal to tell it what game to play.
  radio.startListening();
  int game = 0;
  if (radio.available()) {
    Serial.println("Initialized");
  }
  radio.read(&game, sizeof(game));
  calculateAngle();
  switch (game) {
    case 1:
    playQuickDraw();
    break;

    case 2:
    playSodaShake();
    break;

    case 3:
    playSamuraiSlicer();
  }
}
  
void playQuickDraw() {
  Serial.println("GAME STARTED");
  delay(7000);
  radio.startListening();
  bool inGame = true;
  while (inGame) {
    if (radio.available()) {
      int signal;
      radio.read(&signal,sizeof(signal));
      if (signal == 1) {
        inGame = false;
        break;
      }
    }
    calculateAngle();
    if ((pitchComp > -15) && (digitalRead(button) == HIGH)) {
      int ident = IDENTIFIER;
      radio.stopListening();
      radio.write(&ident, sizeof(ident));
      radio.startListening();
      inGame = false;
    }
    delay(1);
  }
}

void playSodaShake() {
  shakeValue = 0;
  delay(5000);
  // Recieve totalShakeValue
  while (digitalRead(button) == LOW) {
    calculateMovement();
    totalShakeValue += shakeValue;
    if (totalShakeValue > maxShakeValue) { // Could change this into an interrupt
      // Send soda explodes
    }
  }
  // Send totalShakeValue
}

void playSamuraiSlicer() {
  float samStartTime = millis();
  calculateAngle();
  if (pitchComp < 30) {
    float samStopTime = millis();
    float cutTime = samStopTime - samStartTime;
    // Send cutTime
  }
}
/*
void playSamuraiCatcher() {
  float samStartTime = millis();
  if (digitalRead(button) == HIGH) { // Could change this into an interrupt
    float samStopTime = millis();
    float catchTime = samStopTime - samStartTime;
    // Send catchTime
  }
}

  delay(5);
}
*/

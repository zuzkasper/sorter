#include <Servo.h>
#include <LiquidCrystal.h>

//Color numbers
#define RED 1
#define GREEN 2;
#define BLUE 3;
#define YELLOW 4;

int colors[4][6];

int redLow=-1;
int redHigh=-1;
int greenLow=-1;
int greenHigh=-1;
int blueLow=-1;
int blueHigh=-1;

// TCS3200 pins 
#define S0 2
#define S1 3
#define S2 4
#define S3 5
#define sensorOut 6

// Solenoid pins
#define sol1 9
#define sol2 10
#define sol3 11
#define sol4 12

// LCD connections
const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// define servos
Servo servo1; //feeder
Servo servo2; //conveyer belt

// Servo pins
#define servopin1 7
#define servopin2 8

/* there are 5 stations (numbered 0-4)
 *  station[0]=under color sensor
 *  station[1-4]=solenoids 1-4
 */
int stations[5]={0,0,0, 0,0}; //initialize as nothing in any station
int stationPins[5]={0,sol1,sol2,sol3,sol4}; //list of corresponding pins for each station (station 0 does not have a pin)


// Stores frequency read by the photodiodes
int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

// initialize position variable for servos
int pos=0;

void moveFeeder(){
  for (pos = 0; pos <= 140; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo1.write(pos);              // tell servo to go to position in variable 'pos'                   
    delay(5);// waits 15ms for the servo to reach the position
  }
  delay(800);
  for (pos = 140; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    servo1.write(pos);              // tell servo to go to position in variable 'pos'                 
    delay(5);// waits 15ms for the servo to reach the position
  }
  delay(250);
}

void moveConveyer(){
  int i=0;
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo2.write(pos);              // tell servo to go to position in variable 'pos'                   
    delay(5);// waits 15ms for the servo to reach the position
  }
  delay(800);
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    servo2.write(pos);              // tell servo to go to position in variable 'pos'                 
    delay(5);// waits 15ms for the servo to reach the position
  }

  //move color info 1 station to match conveyer
  for (i=4; i>0; i--){
    stations[i]=stations[i-1];
  }
}

/*
 * Initial set up of sorter
 */

void setup() {
  // set up LCD columns and rows
  lcd.begin(16,2);
  //Below LCD HELLO WORLD Test
  lcd.print("HAO LIN <3");
  
  //Servo setup
  servo1.attach(servopin1);
  servo2.attach(servopin2);
  
  // Setting the sensor outputs
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  // Setting the solenoids outputs
  pinMode(sol1, OUTPUT);
  pinMode(sol2, OUTPUT);
  pinMode(sol3, OUTPUT);
  pinMode(sol4, OUTPUT);

  // Initialize solenoids to HIGH (off)
  digitalWrite(sol1,HIGH);
  digitalWrite(sol2,HIGH);
  digitalWrite(sol3,HIGH);
  digitalWrite(sol4,HIGH);
  
  // SettingsorOut as an input
  pinMode(sensorOut, INPUT);
  
  // Setting frequency scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);
  
   // Begins serial communication 
  Serial.begin(9600);
  Serial.print("Hello World!");//DELETE

  //calibrate colors
  calibration();
}

/*
 * There are 4 main components of the automation of the sorter.
 * Move feeder, get color of new M&M, shoot M&Ms into correct containers, and move conveyer belt
 */

void loop() {

  /*
   * Calibrate the color frequencies
   */
  //if(calibrated==0){
  //  calibration();
  //  calibrated=1;
  //}
  int i=0;
  /*
   * 1. move feeder full motion
   */
   //delay(100);
   moveFeeder();
   //delay(100);
  
  /* 
   * 2. Get color of item
   * set station[0] to that color
   * i.e. this color is currently under the sensor
   */
  stations[0]=getColor();

  /*
   * 3. check each station for correct color and fire solenoids 
   * (only have to check stations 1-4 since station 0 doesnt have a solenoid; i.e. i=1 is starting point)
   */
  //i.e. if color at a station matches the color that gets blasted there
  for (i=1; i<5; ++i){
    if (stations[i]==i){   //if item is in right spot, push it off
      digitalWrite(stationPins[i],LOW);   //turn on solenoid
      delay(250);     //keep solonoid on for 250ms
      digitalWrite(stationPins[i], HIGH); //turn off solenoid
     stations[i]=0;
     }
  }
  
  /*
   * 4.move conveyer 1 chain length and move color info to match
   */
   //delay(100);
   moveConveyer();
   //delay(100);
  
}

/*
 * code to get color of item
 * will return integer 1-4 (corresponding to colors of each station)
 * void for now
 */


 int getColor(){
  getFrequencies();
  int i=1;
  while(i<5){
    //Serial.print(i);
    if(redFrequency>colors[i-1][0]&&redFrequency<colors[i-1][1] && greenFrequency>colors[i-1][2]&&greenFrequency<colors[i-1][3] && blueFrequency>colors[i-1][4]&&blueFrequency<colors[i-1][5]){
      Serial.print("Color: ");
      Serial.print(i);
      return i;
    }
    i++;
  }
 }

 //calibrates RED, GREEN, BLUE, YELLOW Frequencies
void calibration(){
  Serial.print("Calibration\n");
  int i=0;
  servo1.write(0);
  servo2.write(0);
  //delay(100);
  //moveFeeder();
  Serial.print("MoveConveyer\n");
  moveConveyer();
  //delay(100);
  while(i<4){
    //delay(100);
    Serial.print("MoveFeeder\n");
    moveFeeder();
    //delay(100);
    Serial.print("MoveConveyer\n");
    moveConveyer();
    //delay(100);
    int count=0;
    Serial.print("Color: ");
    Serial.print(i);
    Serial.print("\n");
    while(count<25){
      getFrequencies();
      if (redLow==-1 || redLow>redFrequency){
        redLow=redFrequency-5;
      }
      if (redHigh==-1 || redHigh<redFrequency){
        redHigh=redFrequency+5;
      }
      if (greenLow==-1 || greenLow>greenFrequency){
        greenLow=greenFrequency-5;
      }
      if (greenHigh==-1 || greenHigh<greenFrequency){
        greenHigh=greenFrequency+5;
      }
      if (blueLow==-1 || blueLow>blueFrequency){
        blueLow=blueFrequency-5;
      }
      if (blueHigh==-1 || blueHigh<blueFrequency){
        blueHigh=blueFrequency+5;
      }
      count++;
    }
    if(i==1){
      digitalWrite(sol1,LOW);   //turn on solenoid
      delay(250);     //keep solonoid on for 250ms
      digitalWrite(sol1, HIGH); //turn off solenoid
    }
    if(i==3){
      digitalWrite(sol2,LOW);   //turn on solenoid
      delay(250);     //keep solonoid on for 250ms
      digitalWrite(sol2, HIGH); //turn off solenoid
    }
    colors[i][0]=redLow;
    colors[i][1]=redHigh;
    colors[i][2]=greenLow;
    colors[i][3]=greenHigh;
    colors[i][4]=blueLow;
    colors[i][5]=blueHigh;
    //delay(10000);
    redLow=-1;
    redHigh=-1;
    greenLow=-1;
    greenHigh=-1;
    blueLow=-1;
    blueHigh=-1;
    i++;
  }
  //delay(100);
  moveConveyer();
  //delay(100);
  moveConveyer();
  digitalWrite(sol3,LOW);   //turn on solenoid
  delay(250);     //keep solonoid on for 250ms
  digitalWrite(sol3, HIGH); //turn off solenoid
  moveConveyer();
  //delay(100);
  moveConveyer();
  //delay(100);
  digitalWrite(sol4,LOW);   //turn on solenoid
  delay(250);     //keep solonoid on for 250ms
  digitalWrite(sol4, HIGH); //turn off solenoid
  i=0;
  int j=0;
  for (i=0; i<4; i++){
    for (j=0; j<6;j++){
      Serial.print(colors[i][j]);
      Serial.print(" ");
    }
    Serial.print("\n");
  }
  Serial.print("Calibration Done\n");
}


void getFrequencies(){
  // Setting RED (R) filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  
  // Reading the output frequency
  redFrequency = pulseIn(sensorOut, LOW);
  
   // Printing the RED (R) value
  Serial.print("R = ");
  Serial.print(redFrequency);
  delay(100);
  
  // Setting GREEN (G) filtered photodiodes to be read
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  
  // Reading the output frequency
  greenFrequency = pulseIn(sensorOut, LOW);
  
  // Printing the GREEN (G) value  
  Serial.print(" G = ");
  Serial.print(greenFrequency);
  delay(100);
 
  // Setting BLUE (B) filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  
  // Reading the output frequency
  blueFrequency = pulseIn(sensorOut, LOW);
  
  // Printing the BLUE (B) value 
  Serial.print(" B = ");
  Serial.println(blueFrequency);
  delay(100);
}

#include <Servo.h>

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

int calibrated=0;

/*
 * Initial set up of sorter
 */

void setup() {
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
  
  // Setting the sensorOut as an input
  pinMode(sensorOut, INPUT);
  
  // Setting frequency scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);
  
   // Begins serial communication 
  Serial.begin(9600);

  calibration();
}

/*
 * There are 4 main components of the automation of the sorter.
 * Move feeder, get color of new M&M, shoot M&Ms into correct containers, and move conveyer belt
 */

void loop() {

 /* if(calibrated==0){
    calibrated=1;
    calibration();
  }*/
  int i=0;
  /*
   * 1. move feeder full motion
   */
  for (pos = 0; pos <= 160; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo1.write(pos);              // tell servo to go to position in variable 'pos'                   
    delay(5);// waits 15ms for the servo to reach the position
  }
  delay(800);
  for (pos = 160; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    servo1.write(pos);              // tell servo to go to position in variable 'pos'                 
    delay(5);// waits 15ms for the servo to reach the position
  }
  delay(250);
  
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
    Serial.print("\n");
    Serial.print(stations[i]);
    Serial.print("\n");
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
  for (pos = 0; pos <= 160; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo2.write(pos);              // tell servo to go to position in variable 'pos'                   
    delay(5);// waits 15ms for the servo to reach the position
  }
  delay(800);
  for (pos = 160; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    servo2.write(pos);              // tell servo to go to position in variable 'pos'                 
    delay(5);// waits 15ms for the servo to reach the position
  }

  //move color info 1 station to match conveyer
  for (i=4; i>0; i--){
    stations[i]=stations[i-1];
  }
}

/*
 * code to get color of item
 * will return integer 1-4 (corresponding to colors of each station)
 * void for now
 */

 int getColor(){
  Serial.print("Getting Color ");
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
  int i=1;
  while(i<5){
    //Serial.print(i);
    if(redFrequency>colors[i-1][0]&&redFrequency<colors[i-1][1] && greenFrequency>colors[i-1][2]&&greenFrequency<colors[i-1][3] && blueFrequency>colors[i-1][4]&&blueFrequency<colors[i-1][5]){
      Serial.print("\nColor shot: ");
      Serial.print(i);
      Serial.print("\n");
      return i;
    }
    i++;
  }
  return 0;
 }

 //calibrates RED, GREEN, BLUE, YELLOW Frequencies
void calibration(){
  int count=0;
  delay(10000);
  Serial.print("Red: ");
  while(count<50){
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
  colors[0][0]=redLow;
  colors[0][1]=redHigh;
  colors[0][2]=greenLow;
  colors[0][3]=greenHigh;
  colors[0][4]=blueLow;
  colors[0][5]=blueHigh;
  delay(10000);
  redLow=-1;
  redHigh=-1;
  greenLow=-1;
  greenHigh=-1;
  blueLow=-1;
  blueHigh=-1;
  Serial.print("Green: ");
  count=0;
  while(count<50){
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
  colors[1][0]=redLow;
  colors[1][1]=redHigh;
  colors[1][2]=greenLow;
  colors[1][3]=greenHigh;
  colors[1][4]=blueLow;
  colors[1][5]=blueHigh;
  delay(10000);
  redLow=-1;
  redHigh=-1;
  greenLow=-1;
  greenHigh=-1;
  blueLow=-1;
  blueHigh=-1;
  count=0;
  Serial.print("Blue: ");
  while(count<50){
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
  colors[2][0]=redLow;
  colors[2][1]=redHigh;
  colors[2][2]=greenLow;
  colors[2][3]=greenHigh;
  colors[2][4]=blueLow;
  colors[2][5]=blueHigh;
  delay(10000);
  redLow=-1;
  redHigh=-1;
  greenLow=-1;
  greenHigh=-1;
  blueLow=-1;
  blueHigh=-1;
  count=0;
  Serial.print("Yellow: ");
  while(count<50){
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
  colors[3][0]=redLow;
  colors[3][1]=redHigh;
  colors[3][2]=greenLow;
  colors[3][3]=greenHigh;
  colors[3][4]=blueLow;
  colors[3][5]=blueHigh;
  redLow=-1;
  redHigh=-1;
  greenLow=-1;
  greenHigh=-1;
  blueLow=-1;
  blueHigh=-1;
  int i=0;
  int j=0;
  for (i=0; i<4; i++){
    for (j=0; j<6;j++){
      Serial.print(colors[i][j]);
      Serial.print(" ");
    }
    Serial.print("\n");
  }
  
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


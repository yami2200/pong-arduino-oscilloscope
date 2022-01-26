#include <Wire.h>
#include "rgb_lcd.h"
rgb_lcd lcd;
// #####################  VARIABLE CONFIGURATION  #######################
// arduino settings
int X_pin = 6;
int Y_pin = 5;
int JoyJ1 = A1;
int JoyJ2 = A2;

// game settings
int timeGame = 100;

// players settings
int thicknessPaddle = 3;
int heightPaddle = 20;
float speedPaddle = 1.3;

// ball settings
float initDirHorizontal = 0.7;
float initDirVertical = 1.1;
float sizeBall = 4;
const float maxAngle = 1.3;

// #####################################################################
// #####################  NOT CONFIGURABLE VARIABLE  ###################
bool inMenu = false;

int delay_point = 1600;
const byte letter_3[18] = {0,0,
30,20,
70,20,
70,50,
30,50,
70,50,
70,80,
30,80,
0,100};
const byte letter_2[16] = {0,0,
70,20,
30,20,
30,50,
70,50,
70,80,
30,80,
0,100};
const byte letter_1[10] = {0,0,
50,20,
50,50,
50,80,
0,100};
float positionP1 = 50.0;
float positionP2 = 50.0;
int scoreP1 = 0;
int scoreP2 = 0;

float ballX = 50.0;
float ballY = 50.0;
float ballSpeedCounter = 1.0;
float dirHorizontal = initDirHorizontal;
float dirVertical = initDirVertical;
const float initSpeedBall = dirHorizontal + dirVertical;

long timeNow = 0;

// #####################################################################
// ##############################  SETUP  ##############################

void setup() {
  pinMode(X_pin, OUTPUT);
  pinMode(Y_pin, OUTPUT);
  pinMode(JoyJ1, INPUT);
  pinMode(JoyJ2, INPUT);
  Serial.begin(9600);

  delay_point = 1600;

  TCCR0A = (TCCR0A & B10100011 + TCCR0A | B10100011);
  TCCR0B = (TCCR0B & B00000001 + TCCR0B | B00000001);
  TIMSK0 = (TIMSK0 & B11111000);

  lcd.begin(16, 2);
  countdown();
  displayLCD();
}

// #####################################################################
// ###############################  LOOP  ##############################

void loop() {
  int P1_axis = analogRead(JoyJ1);
  int P2_axis = analogRead(JoyJ2);
  if(inMenu){
    if(P1_axis > 600 && P2_axis > 600){
      restartGame();
    }
    return;
  }
  timeNow = timeNow + 1;
  ballSpeedCounter = ballSpeedCounter + 0.005;
  displayLCD();

  if(getTime() <= 0){
    endGame();
    return;
  }

  // Movement Player
  if(P1_axis > 600){
    positionP1 = positionP1 - speedPaddle;
  } else if(P1_axis < 400){
    positionP1 = positionP1 + speedPaddle;
  }

  if(P2_axis > 600){
    positionP2 = positionP2 - speedPaddle;
  } else if(P2_axis < 400){
    positionP2 = positionP2 + speedPaddle;
  }

  // Colision Players
  if(positionP2 + (heightPaddle / 2 ) > 90) { positionP2 = 80; }
  if(positionP2 - (heightPaddle / 2 ) < 10) { positionP2 = 20; }
  if(positionP1 + (heightPaddle / 2 ) > 90) { positionP1 = 80; }
  if(positionP1 - (heightPaddle / 2 ) < 10) { positionP1 = 20; }

  // Movement Ball
  ballX = ballX + dirHorizontal * ballSpeedCounter;
  ballY = ballY + dirVertical * ballSpeedCounter;

  // Collision Ball & Borders Screen
  if(ballY + (sizeBall / 2) > 90) { dirVertical = -abs(dirVertical);}
  if(ballY - (sizeBall / 2) < 10) { dirVertical = abs(dirVertical);}

  // Ball Collision Paddle P1
  if(ballX - (sizeBall / 2) < (10 + thicknessPaddle + relativeCollisionHori()) && dirHorizontal < 0){
    if( (ballY + (sizeBall / 2) < positionP1 + (heightPaddle/2) + relativeCollisionVerti() && ballY + (sizeBall / 2) > positionP1 - (heightPaddle/2) - relativeCollisionVerti()) || (ballY - (sizeBall / 2) < positionP1 + (heightPaddle/2) + relativeCollisionVerti() && ballY - (sizeBall / 2) > positionP1 - (heightPaddle/2) - relativeCollisionVerti()) ){
      if(ballY > positionP1 + heightPaddle / 2) dirVertical = maxAngle;
      else if(ballY < positionP1 - heightPaddle / 2) dirVertical = -maxAngle;
      else { dirVertical = ((ballY - positionP1)/(heightPaddle/2)) * maxAngle;}
      dirHorizontal = (initSpeedBall - abs(dirVertical));
    }
  }
  // Ball Collision Paddle P2
  if(ballX + (sizeBall / 2) > (90 - thicknessPaddle - relativeCollisionHori()) && dirHorizontal > 0){
    if( (ballY + (sizeBall / 2) < positionP2 + (heightPaddle/2) + relativeCollisionVerti() && ballY + (sizeBall / 2) > positionP2 - (heightPaddle/2) - relativeCollisionVerti()) || (ballY - (sizeBall / 2) < positionP2 + (heightPaddle/2) + relativeCollisionVerti() && ballY - (sizeBall / 2) > positionP2 - (heightPaddle/2) - relativeCollisionVerti()) ){
      if(ballY > positionP2 + heightPaddle / 2) dirVertical = maxAngle;
      else if(ballY < positionP2 - heightPaddle / 2) dirVertical = -maxAngle;
      else { dirVertical = ((ballY - positionP2)/(heightPaddle/2)) * maxAngle;}
      dirHorizontal = -(initSpeedBall - abs(dirVertical));
    }
  }

  // Ball scores in P1 side
  if(ballX - (sizeBall / 2) < 10) { 
    scoreP2 = scoreP2 + 1;
    goal(2);
  }
  // Ball scores in P2 side
  if(ballX + (sizeBall / 2) > 90) { 
    scoreP1 = scoreP1 + 1;
    goal(1);
  }

  // Display Screen
  displayScreen();
}

// get Relatvie collsion horizontally
float relativeCollisionHori(){
  float speedHori = abs(dirHorizontal) * ballSpeedCounter;
  if(speedHori > 5) speedHori = 5;
  return speedHori;
}

// get Relatvie collsion vertically
float relativeCollisionVerti(){
  float speedHori = abs(dirVertical) * ballSpeedCounter;
  if(speedHori > 5) speedHori = 5;
  return speedHori;
}

// display the screen on oscilloscope
void displayScreen(){
  plot(0,0);
  plot(0, positionP1 - (heightPaddle / 2));

  plot(10 + thicknessPaddle, positionP1 - (heightPaddle / 2));
  plot(10 + thicknessPaddle, positionP1);
  plot(10 + thicknessPaddle, positionP1 + (heightPaddle / 2));

  plot(0, positionP1 + (heightPaddle / 2));
  plot(0, 100);
  plot(ballX, 1000);
    
  plot(ballX, ballY + (sizeBall / 2));
  plot(ballX - (sizeBall / 2), ballY + (sizeBall / 2));
  plot(ballX - (sizeBall / 2), ballY - (sizeBall / 2));
  plot(ballX + (sizeBall / 2), ballY - (sizeBall / 2));
  plot(ballX + (sizeBall / 2), ballY + (sizeBall / 2));
  plot(ballX, ballY + (sizeBall / 2));

  plot(ballX, 1000);
  plot(100, 100);
  plot(100, positionP2 + (heightPaddle / 2));

  plot(90 - thicknessPaddle, positionP2 + (heightPaddle / 2));
  plot(90 - thicknessPaddle, positionP2);
  plot(90 - thicknessPaddle, positionP2 - (heightPaddle / 2));

  plot(100, positionP2 - (heightPaddle / 2));
  plot(100,0);
}

// Restart game by resetting all variables
void restartGame(){
  positionP1 = 50;
  positionP2 = 50;
  ballX = 50.0;
  ballY = 50.0;
  dirHorizontal = initDirHorizontal;
  dirVertical = initDirVertical;
  ballSpeedCounter = 1.0;
  scoreP1 = 0;
  scoreP2 = 0;
  timeNow = 0;
  inMenu = false;
  lcd.clear();
  countdown();
}

// display Score & Time on LCD
void displayLCD(){
  lcd.setCursor(0, 0);
  lcd.setColor(BLUE);
  lcd.print("SCORE : ");
  if(scoreP1 <10) lcd.print("0");
  lcd.print(scoreP1);
  lcd.print(" - ");
   if(scoreP2 <10) lcd.print("0");
  lcd.print(scoreP2);
  lcd.setCursor(0,1);
  lcd.print("TIME  :  ");
  int timing = getTime();
  if(timing < 100) lcd.print(" ");
  if(timing < 10) lcd.print(" ");
  lcd.print(timing);
  lcd.print("'");
}

// end the game
void endGame(){
  lcd.clear();
  if(scoreP1 == scoreP2){
    lcd.setCursor(5, 0);
    lcd.print("DRAW !!!");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("PLAYER");
    lcd.print((scoreP1 > scoreP2 ? "1" : "2"));
    lcd.print(" WINS !!!");
  }
  lcd.setCursor(0, 1);
  lcd.print("SCORE : ");
  if(scoreP1 <10) lcd.print("0");
  lcd.print(scoreP1);
  lcd.print(" - ");
   if(scoreP2 <10) lcd.print("0");
  lcd.print(scoreP2);
  delay(1000000);
  showMenu();
}

// Show menu to restart the game
void showMenu(){
  inMenu = true;
  lcd.clear();
  lcd.print("v RESTART GAME v");
}

// CountDown 3,2,1
void countdown(){
  displayLCD();
  const int maxT = 80;
  for(int t = 0; t < maxT; t++){
    for(int i = 0; i<18; i+=2){
      plot(letter_3[i],letter_3[i+1]);
    }
  }
  for(int t = 0; t < maxT; t++){
    for(int i = 0; i<16; i+=2){
      plot(letter_2[i],letter_2[i+1]);
    }
  }
  for(int t = 0; t < maxT; t++){
    for(int i = 0; i<10; i+=2){
      plot(letter_1[i],letter_1[i+1]);
    }
  }
}

// Get speed of the ball in m/h
int getSpeedKmh(){
  float speedB = abs(dirHorizontal) * ballSpeedCounter + abs(dirVertical) * ballSpeedCounter;
  return (speedB / 80.0) * 8.0 * 25.0 * 3600.0 / 100.0; 
}

// Score a goal
void goal(int player){
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("GOOAAAL !!!");
    lcd.setCursor(5,1);
    lcd.print("PLAYER");
    lcd.print(player);
    delay(50000);
    for(int i = 0; i < 5; i++){
      lcd.setColor(RED);
      delay(30000);
      lcd.setColor(BLUE);
      delay(30000);
    }
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("BALL SPEED : ");
    lcd.setCursor(5,1);
    lcd.print(getSpeedKmh());
    lcd.print("m/h");
    delay(500000);
    lcd.clear();

    countdown();

    positionP1 = 50;
    positionP2 = 50;
    ballX = 50.0;
    ballY = 50.0;
    if(dirHorizontal>0) dirHorizontal = initDirHorizontal * (-1);
    else dirHorizontal = initDirHorizontal;
    dirVertical = initDirVertical;
    ballSpeedCounter = 1.0;
}

// print a point
void plot(float x, float y){
  analogWrite(X_pin, x);
  analogWrite(Y_pin, y);
  delayMicroseconds(delay_point);
}

int getTime(){
  return timeGame - (timeNow / 25);
}

// #####################################################################

// Réglage : 2V / 2V, Mode XY, M 5ms , Channel 1 et 2 position = -10V, Valeur potentiomètre 1998. 
// 5 Janvier : 200mV / 200 mV, channel 1 et 2 = -1V
// 2 condensateurs 100 nF
// 2 résistances 4.7k.
// Potentiomètre 10k
// Ecran de jeu 80x80 => [10, 90]

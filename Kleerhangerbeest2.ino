/*=============================================================================================================
                                 K L E E R H A N G E R B E E S T
                                M I N H A S   E N G E N H O C A S
                             Versão fácil para Arduino e 2 servos do
                                  = Coat Hanger Walking Robot =
                              - Robô Andarilho de Cabide de Arame - 
                    originalmente desenvolvido por Jerome Demers e Gareth Branwyn

O nome kleerhangerbeest é holandês e significa bich-cabide, tendo sido inspirado nos robôs eólicos
chamados strandbeest (bicho-de-praia) de Theo Jansen - http://www.strandbeest.com/
                                      
Referências originais do Coat Hanger Walking Robot:
- http://www.instructables.com/id/How-to-build-the-one-motor-walker/
- http://www.technogumbo.com/projects/single-servo-walking-robot/
- https://www.youtube.com/watch?v=bxxmN2sQvjk
- https://solarbotics.com/product/abgchw/
- https://www.youtube.com/watch?v=L984o5mT9HI (com motor de rotacao continua)

O código abaixo foi comentado em inglês para facilidade de compreensão por todos.

Aldo von Wangenheim <awangenh at inf.ufsc.br> June, 2014
https://minhasengenhocas.wordpress.com/
=============================================================================================================*/
#include <Servo.h> 
 
Servo frontServo;         // create servo object to control the front legs 
Servo hindServo;          // create servo object to control the hind legs 
 
int pos = 90;             // variable to store the servo position 
                          // initialized with both legs parallel in a standing position
int steppingDelay = 500;  // pause between robot step cycles in ms.
int servoDelay = 20;      // waiting pause between servo moves to allow the servo to reach the position  
int moveAngle = 4;        // angle between servo moves - the bigger the angle, the faster the robot walks
int startAngle = 70;
int stopAngle;
int maxAngle = 180;
int debug = false;        // debugging mode (change this to true if you want to see an output on the console)
 
void setup() 
{ 
  frontServo.attach(9);  // attaches the front leg servo on pin 9 to the servo object 
  hindServo.attach(10);  // attaches the hind leg servo on pin 10 to the servo object 
  stopAngle = maxAngle - startAngle;
  
  frontServo.write(pos);             // tell front legs to go to position in variable 'pos' 
  hindServo.write(pos);              // tell hind legs to go to position in variable 'pos' 
                                     // in order to put both legs parallel in standing position
                                     
  if (debug)                         // if debugging mode is set, open serial monitor
     Serial.begin(38400);
} 
 
 
void loop() 
{ 
  delay(steppingDelay);              // stop a bit before start sweeping back
  for(pos = startAngle; pos < stopAngle; pos += moveAngle)   
  {                                  // goes in steps of moveAngle degrees
    if (debug) {                     // IF debugging mode is set, then show angles on serial monitor
      Serial.print(pos);
      Serial.print(" / ");
      Serial.println(maxAngle - pos);
    }
    frontServo.write(pos);           // tell front leg servo to go to position in variable 'pos' 
    delay(servoDelay);               // waits 15ms or more for the servo to reach the position 
                                     // changed to 20ms
  } 
  for(pos = startAngle; pos < stopAngle; pos += moveAngle)   
  {                                  // goes in steps of moveAngle degrees
    if (debug) {                     // IF debugging mode is set, then show angles on serial monitor
      Serial.print(pos);
      Serial.print(" / ");
      Serial.println(maxAngle - pos);
    }
    hindServo.write(maxAngle - pos); // tell hind leg servo to go to position in variable 'maxAngle - pos' 
    delay(servoDelay);               // waits 15ms or more for the servo to reach the position 
                                     // changed to 20ms
  } 

  delay(steppingDelay);              // stop a bit before start sweeping back
  for(pos = stopAngle; pos > (startAngle - 10); pos -= moveAngle)   
  {                                  // goes in steps of moveAngle degrees 
    if (debug) {                     // IF debugging mode is set, then show angles on serial monitor
      Serial.print(pos);
      Serial.print(" / ");
      Serial.println(maxAngle - pos);
    }
    frontServo.write(pos);           // tell front leg servo to go to position in variable 'pos' 
    delay(servoDelay);               // waits 15ms or more for the servo to reach the position 
                                     // changed to 20ms
  } 
  for(pos = stopAngle; pos > (startAngle - 10); pos -= moveAngle)   
  {                                  // goes in steps of moveAngle degrees 
    if (debug) {                     // IF debugging mode is set, then show angles on serial monitor
      Serial.print(pos);
      Serial.print(" / ");
      Serial.println(maxAngle - pos);
    }
    hindServo.write(maxAngle - pos); // tell hind leg servo to go to position in variable 'maxAngle - pos' 
    delay(servoDelay);               // waits 15ms or more for the servo to reach the position 
                                     // changed to 20ms
  } 
} 

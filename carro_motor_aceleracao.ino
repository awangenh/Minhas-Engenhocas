
#include <Servo.h>

 
Servo us_servo;  // create servo object to control the ultrasound servo
Servo steering_servo;  // create servo object to control the steering wheel servo

 
int pos = 0;    // variable to store the servo position

int inputPin=4;  // connect digital I/O 4 to the ECHO/Rx Pin
int outputPin=5; // connect digital I/O 5 to the TRIG/TX Pin
int greenLed=3;  
int redLed=7; 

// H-bridge pins
int speedPin = 11;
int directionPin = 12;

//int relayPin=11;
int us_echo, us_echo_dir, us_echo_esq;
int min_distance = 45;   // Distancia de echo considerado obstáculo
int max_distance = 200;  // Eco a partir do qual distancia é considerada infinita
int tempo_curva = 2000;  // Tempo durante o qual direção fica virada em curva

/* ------ Variaveis e constantes do Conta-Giros ------- */
int   dtPin = 2;                  // Se mudar pino, TEM que mudar a interrupção abaixo
int   interrupcaoContaGiros = 0;  // TEM que ser 0 se dtPin = 2....

unsigned long  ultimaMudanca = 0; // Tempo absoluto na ultima leitura do contagiros
long  intervalo = 0;

const int altaVelocidade = 100;   // Intervalo max. entre leituras para a velocidade ser considerada alta
const int baixaVelocidade = 600;  // Intervalo max. entre leituras para a velocidade ser considerada baixa
int  velocidade;
/* -------------------------------------------------------*/


/*==============================================
         Analog motor control functions
           (employs H-bridge L298N)
  ==============================================*/
void praFrente() 
{
 Serial.print("Full speed forward\n");
 digitalWrite(directionPin, HIGH); // Set direction as forward
 analogWrite(speedPin, 0); // Set the speed as full
}

void devagarPraFrente() 
{
 //Serial.print("Half speed forward\n");
 digitalWrite(directionPin, HIGH); // Set direction as forward
 analogWrite(speedPin, 100); // Set the speed as half
}

void praTras()
{
 Serial.print("Full speed backwards\n");
 digitalWrite(directionPin, LOW ); // Set direction as reverse
 analogWrite(speedPin, 255); // Set the speed as full speed
}

void devagarPraTras()
{
 Serial.print("Half speed backwards\n");
 digitalWrite(directionPin, LOW ); // Set direction as reverse
 analogWrite(speedPin, 160); // Set the speed as half speed
}

void parar()
{
 Serial.print("Stop\n");
 analogWrite(speedPin, 0); // Set the speed to stop
}

/*==============================================
         Ultrassound pinging functions
  ==============================================*/
int ping()
{
  int firstEcho;
  int secondEcho;
  int thirdEcho;
  int distance;
  int distance_cm;

  // first echo
  digitalWrite(outputPin, LOW);  // send low pulse for 2μs
  delayMicroseconds(2);
  digitalWrite(outputPin, HIGH); // send high pulse for 10μs
  delayMicroseconds(15);
  digitalWrite(outputPin, LOW);  // back to low pulse
  distance = pulseIn(inputPin, HIGH);  // read echo value
  firstEcho= distance/29/2;  // in cm
  // Serial.print(firstEcho); 
  delay(50);
  // second echo
  digitalWrite(outputPin, LOW);  // send low pulse for 2μs
  delayMicroseconds(2);
  digitalWrite(outputPin, HIGH); // send high pulse for 10μs
  delayMicroseconds(15);
  digitalWrite(outputPin, LOW);  // back to low pulse
  distance = pulseIn(inputPin, HIGH);  // read echo value
  secondEcho= distance/29/2;  // in cm
  /* Serial.print("  "); 
  Serial.print(secondEcho); */
  delay(50);
  // third echo
  digitalWrite(outputPin, LOW);  // send low pulse for 2μs
  delayMicroseconds(2);
  digitalWrite(outputPin, HIGH); // send high pulse for 10μs
  delayMicroseconds(15);
  digitalWrite(outputPin, LOW);  // back to low pulse
  distance = pulseIn(inputPin, HIGH);  // read echo value
  thirdEcho= distance/29/2;  // in cm
  /* Serial.print("  "); 
  Serial.println(thirdEcho); */
  delay(50);
  if ((firstEcho < min_distance) && (secondEcho < min_distance) && (thirdEcho < min_distance))
    {
      return((firstEcho + secondEcho + thirdEcho) / 3); 
    }
  else
    {
      return(max_distance);
    }
}

void us_direita() {
    us_servo.write(45);              
    delay (500);
}  

void us_esquerda() {
    us_servo.write(135);              
    delay (500);
}  


// ATTENTION: Steering servo is mounted inverted in relation to the US servo

void steering_direita() {
    steering_servo.write(135);              
    delay (500);
}  

void steering_esquerda() {
    steering_servo.write(45);              
    delay (500);
}  

/* ================================================ */
void reageObstaculo()
/* 
  Dá marcha a ré (por via das dúvidas)
  Troca para luz vermelha.
  Avalia distancias direita e esquerda.
  Escolhe caminho mais livre e faz curva.
  ================================================ */
 {
     // Obstaculo!
     // Muda cores dos LEDs de indicação de modo
     digitalWrite(redLed, HIGH);
     digitalWrite(greenLed, LOW);
     // Para motor
     // digitalWrite(relayPin, LOW);
     parar();
     // Dar a re um pouco...
     devagarPraTras();
     delay(1000);
     parar();
     // Executa varredura sonar...
     // Move servo para direita
     us_direita();
     // Pinga eco direito
     us_echo_dir = ping();
     // Move servo para esquerda
     us_esquerda();
     // Pinga eco esquerdo
     us_echo_esq = ping();
     // Reseta posicao servo US
     us_servo.write(90);
     delay (1000);
     if (us_echo_esq > us_echo_dir)
       {
         steering_esquerda();
       }
     else
       {
         steering_direita();
       }
     // Faz a curva!!!
     // Liga motor (se já estiver ligado, continua ligado...)
     // digitalWrite(relayPin, HIGH);
     devagarPraFrente();
     delay(tempo_curva);
     steering_servo.write(90);  // steer straight ahead
     delay (1000);     
 }     



/* ==============================================
       Rotinas de propriocepção do veículo
       
Desenvolvidas para monitorar se o veículo está
andando ou se parou/encalhou...

Desenvolvidas para um goniômetro Keyes KY-40, que
possui 5 saídas:
GND, +, SW (chaveia ao apertar eixo), DT e CLK. 

Em nosso código conectamos 3: -/+ e DT a dtPin.
Basta para avaliar velocidade quando não se deseja 
saber sentido.

Necessita do artificio de programação feio de
usar uma variável conta-giros global...
   ==============================================*/
   
/* ============================================== */
int avaliaVelocidade()
/* Retorna 0, 1 ou 2, dependendo se:
   0 -> Parou
   1 -> Está lento (devemos acelerar - achou grama, tapete peludo ou inclinação?)
   2 -> Velocidade nominal atingida....
   ==============================================*/
{
  intervalo = millis() - ultimaMudanca;
  Serial.print(intervalo);
  if (intervalo > altaVelocidade)
    {
      if (intervalo > baixaVelocidade)
      {
        Serial.println(" -> parou...");
        return(0);
      }
      else
      {
        Serial.println(" -> lento...");        
        return(1);
      }
    }
  else
    {
      Serial.println(" -> rapido...");        
      return(2);
    }
}

/* ============================================== */
void reageRotacao()
/*   
     Função chamada por interrupção sempre que o
     goniômetro apresenta uma leitura de rotação.
   ============================================== */
{
   ultimaMudanca = millis();
}

/* ============================================== */
void inicializaContaGiros()
/* ============================================== */
{
  ultimaMudanca = millis();
  pinMode(dtPin, INPUT);
  attachInterrupt(interrupcaoContaGiros, reageRotacao, CHANGE); // encoder pin na interrupção 0 (dtPin = pin 2)
}



/*==============================================
                  Funções Auxiliares
  ==============================================*/
void blinkRed()
{
     digitalWrite(redLed, HIGH);
     delay(160);
     digitalWrite(redLed, LOW);
     delay(160);
     digitalWrite(redLed, HIGH);
     delay(160);
     digitalWrite(redLed, LOW);
     delay(160);
     digitalWrite(redLed, HIGH);
     delay(160);
     digitalWrite(redLed, LOW);
}

void blinkGreen()
{
     digitalWrite(greenLed, HIGH);
     delay(160);
     digitalWrite(greenLed, LOW);
     delay(160);
     digitalWrite(greenLed, HIGH);
     delay(160);
     digitalWrite(greenLed, LOW);
     delay(160);
     digitalWrite(greenLed, HIGH);
     delay(160);
     digitalWrite(greenLed, LOW);
}


/*==============================================
                  Main Setup
  ==============================================*/
void setup()
{
  Serial.begin(9600);
  pinMode(inputPin, INPUT);
  pinMode(outputPin, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  
  // H-bridge pins
  pinMode(directionPin, OUTPUT); // Set Pin directionPin as output, this is the motor direction control
  //pinMode(relayPin, OUTPUT);

  // Servos initialization
  us_servo.attach(10);  // attaches the servo on pin 10 to the servo object that does US scans
  steering_servo.attach(9);  // attaches the servo on pin 9 to the steering wheel servo object
  
  // Inicialização da propriocepção
  inicializaContaGiros();
  blinkGreen();
  Serial.println("Partindo...");
}


/*==============================================
                  Main Loop
  ==============================================*/
void loop()
{
  us_echo = ping();
  if (us_echo == max_distance)
   {
     // Freie Fahrt!
     digitalWrite(redLed, LOW);
     digitalWrite(greenLed, HIGH);
     // Liga motor (se já estiver ligado, continua ligado...)
     // digitalWrite(relayPin, HIGH);
     devagarPraFrente();
   }
  else
   {
     // Obstaculo!
     reageObstaculo();
   }     
  velocidade = avaliaVelocidade();
  if (velocidade == 1)
    {
      //acelera
      praFrente();
      blinkGreen();
      //blinkGreen();
      devagarPraFrente();
    }
  if (velocidade == 0)
    {
      blinkRed();
      //marcha  a ré
      reageObstaculo();
    }
}

#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

SoftwareSerial rs485(10, 11);

//Declaramos o pino onde se vai ligar o cabo DATA do sensor
#define Pin 3

//Declaramos o pino como bus para a comunicação OneWire
OneWire ourWire(Pin);

//Iniciamos 1 instancia da livraria DallasTemperature
DallasTemperature sensors(&ourWire);

uint8_t ok_desliga_2[6] = {0, 2, 5, 13, 15, 35};
uint8_t ok_liga_2[6] = {0, 2, 5, 13, 17, 37};
uint8_t ok_sensor2[6] = {0, 2, 5, 11, 77, 95};
uint8_t c[6] = {0, 0, 0, 0, 0, 0};
int k = 0;
int j = 0;
int tipo = 0;
int controle = 0;
int LEDPin = 5;
int sensor = 0;
uint8_t z;

void checksum()
{
  byte y = (c[0] + c[1] + c[2] + c[3] + c[4] - c[5]);
  if (c[1] == 2 && y == 0)
  {
    if (c[3] == 13 && c[4] == 15)
    {
      controle = 1;
    }
    else if (c[3] == 13 && c[4] == 17)
    {
      controle = 2;
    }
    else if (c[3] == 11 && c[4] == 0)
    {
      controle = 3;
    }
  }
  else
  {
    controle = 0;
    Serial.println("Checksum Inválido!");
  }
}

void transmite() //rotina para responder mensagem
{
  digitalWrite(6, HIGH); //habilita transmissao
  delay(5);
  if (controle == 1) //resposta para comando de desligar led
  {
    for (j = 0; j <= 5; j++)
    {
      rs485.write(ok_desliga_2[j]);
      Serial.println(ok_desliga_2[j]);
    }
    Serial.println("enviado desliga");
  }
  else if (controle == 2) //resposta para comando de ligar led
  {
    for (j = 0; j <= 5; j++)
    {
      rs485.write(ok_liga_2[j]);
      Serial.println(ok_liga_2[j]);
    }
    Serial.println("enviado liga");
  }
  else if (controle == 3) //resposta para comando de ler sensor
  {
    for (j = 0; j <= 5; j++)
    {
      rs485.write(ok_sensor2[j]);
      Serial.println(ok_sensor2[j]);
    }
    Serial.println("enviado sensor");
  }
  else
  {
    controle = 0;
  }
  digitalWrite(6, LOW); //desabilita transmissão e habilita recepção
  //  delay(10);
}

void setup()
{

  pinMode(6, OUTPUT);       //pino de controle
  pinMode(LEDPin, OUTPUT);  //led de saida
  digitalWrite(6, LOW);     //inicia em modo recepção
  analogWrite(LEDPin, 255); //inicia led ligado
  Serial.begin(9600);       //inicia serial
  rs485.begin(9600);        //inicia comunicacao rs485
  sensors.begin();          //prepara para leitura do sensor
}

void loop()
{

  //Ve se tem sinal na Serial, e manda para Serial1
  while (rs485.available())
  {
    if (k <= 5)
    {
      byte x = rs485.read();
      c[k] = x;
      Serial.println(c[k]);
      k = k + 1;
    }
    controle = k;
  }
  delay(5);
  if (controle == 6)
  {
    checksum();        //rotina checksum irá verificar validade e setar tipo de controle(1 para desligar led, 2 para ligar led ou 3 para ler sensor)
    if (controle == 1) //se for pra ligar ou desligar led...
    {
      transmite();
      analogWrite(LEDPin, 0);
      delay(5);
    }

    else if (controle == 2) //se for pra ligar led...
    {
      transmite();
      analogWrite(LEDPin, 255);
      delay(5);
    }

    else if (controle == 3) //se for pra ler sensor...
    {
      z = 0;
      sensors.requestTemperatures();
      sensor = sensors.getTempCByIndex(0);
      ok_sensor2[4] = sensor;
      Serial.println(sensor);
      Serial.println(" ");
      for (j = 0; j <= 4; j++)
      {
        z = z + ok_sensor2[j];
      }
      ok_sensor2[5] = z;
      transmite();
    }

    k = 0;
    controle = 0;
    for (j = 0; j <= 4; j++)
    {
      c[k] = 0;
      //                 Serial.println(c[k]);
    }
  }
}

#include <SoftwareSerial.h>
#include <stdlib.h>
#include <cstdlib>
#include <ESP8266WiFi.h> //Biblioteca que gerencia o WiFi.
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiServer.h>                   //Biblioteca que gerencia o uso do TCP.
WiFiServer servidor(80);                  //Cria um objeto "servidor" na porta 80 (http).
WiFiClient cliente;                       //Cria um objeto "cliente".
SoftwareSerial rs485(14, 12, false, 256); //Biblioteca para utilizar pinagem definida para comunicacao serial

const char *ssid = "Projeto";      //Nome da rede utilizada
const char *password = "12345678"; //Senha de acesso da rede
String html;                       //String que armazena o corpo do site.
const uint8_t BUFFER_SIZE = 20;    //Variavel relacionada a utilizacao do buffer da serial
uint8_t size = 0;
char buffer[BUFFER_SIZE];
//Variáveis com os frames pre formatados de solicitaçao dos serviços nos escravos
uint8_t le_entrada1[6] = {0, 1, 8, 11, 0, 20}; 
uint8_t liga_1[6] = {0, 1, 8, 13, 17, 39};
uint8_t desliga_1[6] = {0, 1, 8, 13, 15, 37};
uint8_t le_entrada2[6] = {0, 2, 8, 11, 0, 21};
uint8_t liga_2[6] = {0, 2, 8, 13, 17, 40};
uint8_t desliga_2[6] = {0, 2, 8, 13, 15, 38};

int i = 0;                      //Variavel de contagem
int j = 0;                      //Variavel de contagem
byte x = 0;                     //Variavel de esvaziamento do buffer rs485
byte z[6] = {1, 2, 3, 4, 5, 6}; //Variavel que recebera os bytes de resposta recebidos pelo mestre
byte sum;                       //Variável de soma para verificar consistencia dos dados recebidos
int ctrl = 0;                   //Variavel que seleciona tipo de serviço

int checksum()
{
  sum = ((z[0] + z[1] + z[2] + z[3] + z[4] - z[5])); //Calculo do checksum. Soma-se os 5 primeiros bytes e diminui do sexto. Seo resultado for zero, os dados estão consistentes
  if (sum == 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void entrada_1()
{ //Se for solicitação de leitura analogica do escravo 1, roda esta funcao...
  digitalWrite(D7, HIGH);

  for (int i = 0; i <= 5; i++)
  {
    rs485.write(le_entrada1[i]);
  }

  ctrl = 1;
  digitalWrite(D7, LOW);
  x = rs485.read(); //leitura para limpar o buffer
  delay(30);
}

void liga_led1()
{ //Se for solicitação de ligar led do escravo 1, roda esta funcao...
  digitalWrite(D7, HIGH);
  delay(10);

  for (int i = 0; i <= 5; i++)
  {
    rs485.write(liga_1[i]);
  }

  ctrl = 2;
  digitalWrite(D7, LOW);
  x = rs485.read(); //leitura para limpar o buffer
  delay(30);
}

void desliga_led1()
{                         //Se for solicitação de deslgar led 1, roda esta funcao...
  digitalWrite(D7, HIGH); //Habilita conversor TTL para transmissão
  delay(10);

  for (int i = 0; i <= 5; i++)
  {
    rs485.write(desliga_1[i]); //escreve frame no barramento
  }

  ctrl = 3;
  digitalWrite(D7, LOW); //Uma vez enviado, habilita conversor TTL para recepção
  x = rs485.read();      //leitura para limpar o buffer
  delay(30);
}

void entrada_2()
{ //Se for solicitação de leitura analogica do escravo 2, roda esta funcao...
  digitalWrite(D7, HIGH);

  for (int i = 0; i <= 5; i++)
  {
    rs485.write(le_entrada2[i]);
  }

  ctrl = 4;
  digitalWrite(D7, LOW);
  delay(1000);
}

void liga_led2()
{ //Se for solicitação de ligar led do escravo 2, roda esta funcao
  digitalWrite(D7, HIGH);
  delay(10);

  for (int i = 0; i <= 5; i++)
  {
    rs485.write(liga_2[i]);
  }

  ctrl = 5;
  digitalWrite(D7, LOW);
  x = rs485.read(); //leitura para limpar o buffer
  delay(30);
}

void desliga_led2()
{ //Se for solicitação de desligar led do escravo 2, roda esta funcao
  digitalWrite(D7, HIGH);
  delay(10);

  for (int i = 0; i <= 5; i++)
  {
    rs485.write(desliga_2[i]);
  }

  ctrl = 6;
  digitalWrite(D7, LOW);
  x = rs485.read(); //leitura para limpar o buffer
  delay(30);
}

void setup()
{

  pinMode(D7, OUTPUT);        // Define pino que habilita transmissão/recepção
  Serial.begin(9600);         // Inicializa a comunicacao serial com uma taxa de 9600 bauds.
  rs485.begin(9600);          // Inicializa comunicação RS-485
  WiFi.mode(WIFI_STA);        //Habilita node como station
  WiFi.begin(ssid, password); //Conecta na rede
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Conexao falhou! Reiniciando...");
    delay(5000); //Se conexão falhou, espera cinco segundos
    ESP.restart();
  }

  ArduinoOTA.onStart([]() {
   Serial.println("Inicio...");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("nFim!");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progresso: %u%%r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro [%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Autenticacao Falhou");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Falha no Inicio");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Falha na Conexao");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Falha na Recepcao");
    else if (error == OTA_END_ERROR)
      Serial.println("Falha no Fim");
  });

  ArduinoOTA.begin();
  Serial.println("Pronto");
  Serial.print("Endereco IP: ");
  Serial.println(WiFi.localIP());
  servidor.begin(); //Inicia o Servidor.
}

void loop()
{

  cliente = servidor.available(); //Diz ao cliente que há um servidor disponivel.

  if (cliente == true) //Se houver clientes conectados, ira enviar o HTML.
  {
    String req = cliente.readStringUntil('\r'); //Faz a leitura do Cliente.
    Serial.println(req);                        //Printa o pedido no Serial monitor.

    if (req.indexOf("/ligaled1") != -1) //solicitação para acionar o led do escravo 1
    {
      liga_led1();
    }
    else if (req.indexOf("/desligaled1") != -1) //solicitação para desligar o led o led
    {
      desliga_led1();
    }
    else if (req.indexOf("/leentrada1") != -1) //solicitação para acionar o led do escravo 1
    {
      entrada_1();
    }
    else if (req.indexOf("/ligaled2") != -1) //solicitação para acionar o led do escravo 1
    {
      liga_led2();
    }
    else if (req.indexOf("/desligaled2") != -1) //solicitação para desligar o led o led
    {
      desliga_led2();
    }
    else if (req.indexOf("/leentrada2") != -1) //solicitação para acionar o led do escravo 1
    {
      entrada_2();
    }
    else
    {
      cliente.print("Comando invalido!");
      cliente.print("\nEscreva proximo comando:");
    }

    if (rs485.available() > 0) //busca o frame de confirmacao enviado pelo escravo
    {
      j = 0;
      while (j <= 5)
      {
        z[j] = (rs485.read());
        j = j + 1;
      }
      checksum();                       //verifica se os dados são consistentes
      String Json = "";                 //variável para configurar o dados de resposta a ser enviado
      if (checksum() == 1 && ctrl == 1) //Se os dados são consistentes...
      {
        Json = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + String("Potenciometro da planta 1: ") + String(z[4]) + String(" %") + "\r\n\r\n"; //Cabeçalho de configuração do tipo de mensagem a ser enviada, e envio das devidas informações
        cliente.print(Json);
        delay(1);
      }
      else if (checksum() == 1 && ctrl == 2)
      {
        Json = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + String("Led do escravo 1 ligado!") + "\r\n\r\n";
        cliente.print(Json);
        delay(1);
      }
      else if (checksum() == 1 && ctrl == 3)
      {
        Json = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + String("Led do escravo 1 desligado!") + "\r\n\r\n";
        cliente.print(Json);
        delay(1);
      }
      else if (checksum() == 1 && ctrl == 4)
      {
        Json = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + String("Temperatura da planta 2: ") + String(z[4]) + String(" ºC") + "\r\n\r\n";
        cliente.print(Json);
        delay(1);
      }
      else if (checksum() == 1 && ctrl == 5)
      {
        Json = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + String("Led do escravo 2 ligado!") + "\r\n\r\n";
        cliente.print(Json);
        delay(1);
      }
      else if (checksum() == 1 && ctrl == 6)
      {
        Json = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + String("Led do escravo 2 desligado!") + "\r\n\r\n";
        cliente.print(Json);
        delay(1);
      }
      else
      {
        Json = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + String("Erro de recebimento. Digite o comando novamente!") + "\r\n\r\n";
        cliente.print(Json);
        delay(1);
      }
    }
    digitalWrite(D7, LOW);
    delay(100);
  }
}

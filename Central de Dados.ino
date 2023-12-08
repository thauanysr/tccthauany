// - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
// Sistema de Supervisão e Controle Aplicado a Sistema de  //
// Irrigação para Pequenos Agricultores do Distrito Federal//
//           Trabalho de Conclusão de Curso                //
//                                                         //
//           Tecnologia em Automação Industrial            //
//                                                         //
//            Thauany Soares - 171056610031                //
//                                                         //
//                    Brasília, 2023                       //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - //


#define BLYNK_TEMPLATE_ID "TMPL2l8guw1tS"                         //Define o identificador do template no Blynk.
#define BLYNK_TEMPLATE_NAME "Automação de Sistema de Irrigação"   //Define o nome do template no Blynk.
#define BLYNK_AUTH_TOKEN "50MPea4rS_-NllcAhRwUaOhJGt6kiNm6"       //Define o código de autenticação. 

#define BLYNK_PRINT Serial                                       //Printa informações no port serial para fins de teste.
#define ESP8266_BAUD 9600                                        //Define a velocidade do comunicação.

#include <ESP8266_Lib.h>                                         //Inclusão da biblioteca do ESP8266.
#include <BlynkSimpleShieldEsp8266.h>                            //Inclusão da biblioteca do Blynk.
#include <SoftwareSerial.h>                                      //Inclusão da biblioteca que emula pinos TX e RX em outros pinos digitais.
#include <RF24.h>                                                //Inclusão da biblioteca do módulo transceptor de radiofrequência.

RF24 radio(9,10);                                                //Inicialização da biblioteca. 
SoftwareSerial EspSerial(2, 3);                                  //Inicialização da biblioteca (RX, TX).                            

char ssid[] = "iPhone de Thauany";                               //Define o nome da rede WiFi a ser conectada.
char pass[] = "12345678";                                        //Define a senha da rede para conexão.

BlynkTimer timer;                                                //Inicializa uma variável "timer" para sincronizar a conexão da rede. 

struct DadosSup{                                                 //Pacote de dados que será recebido.
  int valorUmi=0;
  float valor_temp=0;
  int valor_nivel=0;
}dados_rec;

float tempVal;                                                  //Definição da variável relativa ao sensor de temperatura.
int umiVal;                                                     //Definição da variável relativa ao sensor de umidade do solo.
int nivelVal;                                                   //Definição da variável relativa ao sensor de nível do reservatório.
int releVal;                                                    //Definição da variável relativa ao estado do relé.
int relePress;                                                  //Define a variável de estado anterior do relé.
int Val;

const uint64_t pipeOut = 0xE8E8F0F0E1LL;                        //Endereçamento do canal de comunicação.
ESP8266 wifi(&EspSerial);                                       //Inicializa a conexão WiFi. 


void setup() {
  Serial.begin(9600);
  radio.begin();                                               //Inicializa a comunicação sem fio.
  radio.setPALevel( RF24_PA_LOW );                             //Define o nível do amplificador de potência.
  radio.setDataRate( RF24_250KBPS );                           //Define a velocidade de dados a serem enviados (RF24_250KBPS / RF24_1MBPS  / RF24_2MBPS).
  radio.openWritingPipe(pipeOut);                              //Define o endereço para envio de dados. 
  radio.openReadingPipe(1, pipeOut);                           //Define o canal de recepção de dados.
  radio.startListening();                                      //O módulo fica disponível para receber mensagem.
  EspSerial.begin(ESP8266_BAUD);                               //Inicializa a comunicação com o módulo WiFi.
  delay(10);

  Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass);             //Envia os dados de conexão para o Blynk.
  timer.setInterval(1000L, sendUptime);                        //Define o tempo de envio de dados para o Blynk.

  
}

void loop() {
  for(int i=0; i<=49; i++){
    if (radio.available()) {                                  //Verifica se há mensagem no canal. 
      radio.read( &dados_rec, sizeof(DadosSup) );             //Se houver mensagem, esta é armazenada na variável "dados_rec".
    }
    delay(10); 
  }
  if(relePress != releVal){                                   //Verifica o estado atual do relé.
    radio.stopListening();                                    //Interrompe a recepção de dados.
    radio.write( &releVal , sizeof(releVal));                 //Envia o valor da variável "releVal" para a estação remota.
    relePress = !relePress;                                   //Inverte o valor da variável de controle "relePress".
    radio.startListening();                                   //O módulo reinicia a recepção de mensagem.
  }

  tempVal=dados_rec.valor_temp;                              //Lê o dado do pacote recebido e armazena da respectiva variável.       
  umiVal=dados_rec.valorUmi;
  nivelVal=dados_rec.valor_nivel;

  Blynk.run();                                              //Inicializa o Blynk.
  timer.run();                                              //Inicializa o timer do Blynk.

  
  Serial.print("Valor Temperatura:");                      //Printa valores para fins de teste.
  Serial.println(dados_rec.valor_temp);
  
  Serial.print("Valor Umidade:");
  Serial.print(dados_rec.valorUmi);
  Serial.println("%");

  Serial.print("Valor Nivel:");
  Serial.println(dados_rec.valor_nivel);

  Serial.print("Valor Rele:");
  Serial.println(releVal);
}

void sendUptime(){                                        //Função que obtém a temperatura, umidade e nível e envia o valor para os datastreams.
  Blynk.virtualWrite(V3, tempVal);                        //Envia ao datastream o valor de temperatura.
  Blynk.virtualWrite(V2, nivelVal);                       //Envia ao datastream o valor do nível.
  Blynk.virtualWrite(V0, umiVal);                         //Envia ao datastream o valor de umidade do solo.
  
}

BLYNK_WRITE(V1){                                          //Obtém o valor do datastream.
  int Val = param.asInt();                                //Armazena o valor do datastream na variável "Val". 
  if( Val==1){                                            //Define a variável "releVal" de acordo com o valor obtido no datastream.
    releVal=1;
  }else{
    releVal=0;
  }
  
  Serial.print("Botao Pressionado");
}

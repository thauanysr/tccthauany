// - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
// Sistema de Supervisão e Controle Aplicado a Sistema de  //
// Irrigação para Pequenos Agricultores do Distrito Federal//
//           Trabalho de Conclusão de Curso                //
//             Firmware da Estação Remota                  //
//                                                         //
//                                                         //
//           Tecnologia em Automação Industrial            //
//                                                         //
//            Thauany Soares - 171056610031                //
//                                                         //
//                    Brasília, 2023                       //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

#include <OneWire.h>             //Inclusão de biblioteca do protocolo One-Wire do sensor de temperatura.
#include <DallasTemperature.h>   //Inclusão de biblioteca do sensor de temperatura.
#include <RF24.h>                //Incusão de biblioteca do transceptor de radiofrequência.



#define leitura_temp 2          //Pino responsável pelo sensor de temperatura.
#define leitura_umi A0          //Pino responsável pelo sensor de umidade do solo.
#define leitura_nivel 3         //Pino responsável pelo sensor de nível do reservatório. 
#define control_rele 4          //Pino responsável pelo controle do rele.

int valorUmi;                   // Valor medido pelo sensor de umidade do solo (%). 
float valor_temp;               // Valor medido pelo sensor de temperatura.
int valor_nivel;                // Valor medido pelo sensor de nível do reservatório.
int valor_analog_umi = 0;       // Valor medido pelo sensor de umidade do solo no pino analógico.
int analogSoloSeco = 1022;      // Valor de referência para umidade mínima.
int analogSoloMolhado = 500;    // Valor de referência para umidade máxima.
int releVal;                    // Nível lógico do relé.
int releValOld;                 // Nível lógico anterior do relé.

struct DadosSup{                // Pacote de dados a serem enviados para a Central de Dados.
  int valorUmi=0;
  float valor_temp=0;
  int valor_niv=0;
}dados_env;

const uint64_t pipeOut = 0xE8E8F0F0E1LL;         //Endereçamento do canal de comunicação. 

OneWire oneWire(leitura_temp);                   //Inicialização da biblioteca OneWire.
DallasTemperature temp_sensor(&oneWire);         //Inicialização da biblioteca DallasTemperature.
RF24 radio(9, 10);                               //Inicialização da biblioteca RF24.     
 
byte endereco[][6] = {"1node","2node"};       

void setup() {                                   
  Serial.begin(9600);

  pinMode(leitura_nivel,INPUT);                 //Define o pino 3 como entrada.
  pinMode(control_rele,OUTPUT);                 //Define o pino 4 como saída.
   
  temp_sensor.begin();                          //Inicializa o sensor de temperatura.
  radio.begin();                                //Inicializa a comunicação sem fio.
  radio.openWritingPipe(pipeOut);               //Define o endereço para envio de dados. 
  radio.openReadingPipe(1,pipeOut);             //Define o canal de recepção de dados. 
  radio.setPALevel(RF24_PA_LOW);                //Define o nível do amplificador de potência. 
  radio.setDataRate( RF24_250KBPS );            //Define a velocidade de dados a serem enviados (RF24_250KBPS / RF24_1MBPS  / RF24_2MBPS). 
  radio.startListening();                       //O módulo fica disponível para receber mensagem.
  
  digitalWrite(control_rele,LOW);               //Define o estado do relé.
  
}

void loop() {                                   
  for(int i=0; i<=30; i++){
    if (radio.available()) {                   //Verifica se há mensagem no canal.                
      radio.read( &releVal , sizeof(releVal)); //Se houver mensagem, esta é armazenada na variável "releVal".
      
      if( releVal != releValOld ){             //Verifica o estado anterior dda variável "releVal".
        if( releVal == 1) {
          digitalWrite(control_rele,HIGH);     //Ativa o relé.
        }else{
          digitalWrite(control_rele,LOW);
        }

        releValOld = releVal;                  //Armazena o valor atual na variável de controle "releValOld".
      } else {
        if(valorUmi >= 30 && valorUmi < 70){   //Verifica o valor de Umidade do Solo.
          digitalWrite(control_rele,HIGH);     //Ativa o relé
        } else {
          digitalWrite(control_rele,LOW);
        }
      }
    }
    delay(10); 
  }
  for( int i=1; i<=5; i++){
    switch(i){
      
      case 1:                                  //Caso criado para fins de teste. 
        Serial.println();                      //Sinaliza o ínicio da mensagem. 
        Serial.print("Inicio");
        Serial.println();
        break;
        
      case 2:                                 //Responsável pela aquisição dos dados do sensor de temperatura.
        Serial.println(); 
        Serial.print("caso2");
        Serial.println(); 
        temp_sensor.requestTemperatures();    //Faz a requisão da leitura do sensor.
        Serial.print("A temperatura é: ");    
        dados_env.valor_temp = 26.7; // temp_sensor.getTempCByIndex(0);  //Armazena o valor de temperatura na variável do pacote de dados a ser enviado.
        Serial.print(temp_sensor.getTempCByIndex(0));          
        Serial.println();
        break;
        
      case 3:                                //Responsável pela aquisição dos dados do sensor de umidade do solo.
        Serial.println(); 
        Serial.print("caso3");
        Serial.println(); 

        valor_analog_umi=analogRead(leitura_umi);                                 //Lê o pino analógico.
        valorUmi = constrain(valor_analog_umi,analogSoloMolhado,analogSoloSeco);  //Restringe o valor lido no pino analógico ao intervalo composto pelos valores de referência.
        valorUmi = map(valorUmi,analogSoloMolhado,analogSoloSeco,100,0);          //Realiza uma proporção dentro do intervalo de 0 a 100 com o valor obtido no intervalo de refência. 
        Serial.print("Umidade do solo: "); 
        Serial.print(valorUmi);                                                   
        Serial.println("%"); 
        dados_env.valorUmi=valorUmi;                   //Armazena o valor de umidade na variável do pacote de dados a ser enviado.  
        
        break;
        
      case 4:                                          //Responsável pela aquisição dos dados do sensor de nível do reservatório.
        Serial.println(); 
        Serial.print("caso4");
        Serial.println(); 
  
        Serial.print("Valor Nível de Agua:");
        valor_nivel = digitalRead(leitura_nivel);      //Realiza a leitura do pino 3.
        Serial.println(valor_nivel);      
        dados_env.valor_niv=valor_nivel;               //Armazena o valor do nível na variável do pacote de dados a ser enviado.  
        Serial.println(); 
        
        
        break;


      case 5:                                         //Responsável pela aquisição dos dados do relé.
        Serial.println(); 
        Serial.print("caso5");
        Serial.println(); 
        Serial.print("Valor Rele:");
        Serial.println(releVal);                      //Informa o valor da variável "releVal" recebido.
        if(releVal == 1){
          digitalWrite(control_rele,HIGH);
        }else{
          digitalWrite(control_rele,LOW);
        }
        
        break;
      
      default:
        Serial.print("default");
        Serial.println(); 
      
        
    }       
  }
  radio.stopListening();                              //Interrompe a recepção de mensagem.
  radio.write( &dados_env , sizeof(DadosSup));        //Envia o pacote de dados para a Central de Dados.
  radio.startListening();                             //Módulo fica disponível para receber mensagem novamente.
  
}

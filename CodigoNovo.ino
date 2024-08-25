#include "dht.h" //INCLUSÃO DE BIBLIOTECA
//#include "dht.h" //INCLUSÃO DE BIBLIOTECA

 
const int pinoDHT11 = 15; //PINO ANALÓGICO UTILIZADO PELO DHT11
#define pinSensorTrepidacao T8
#define pinSensorChuva 4



dht DHT; //VARIÁVEL DO TIPO DHT
#include <arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define pinoDigitalUmidadeSolo 5
//#define pinoAnalogicoUmidadeSolo 4

#define PIN_SENU T9

#define LED_BUILTIN 15
#define PIN_LED 15
#define PIN_LED1 2
#define TOPICO_SUBSCRIBE_LED         "topico_liga_desliga_led"
#define TOPICO_PUBLISH_TEMPERATURA   "topico_sensor_temperatura"
#define TOPICO_PUBLISH_UMIDADE       "topico_sensor_umidade"
#define TOPICO_PUBLISH_UMIDADE_SOLO  "topico_sensor_umidade_solo"
#define TOPICO_PUBLISH_TREPIDACAO    "topico_trepidacao" 
#define TOPICO_PUBLISH_CHUVA         "topico_chuva"
#define TOPICO_PUBLISH_OLA           "Ola_mundo"
//#define ID_MQTT  "IoT_PUC_SG_mqtt"     
#define ID_MQTT "ProconcepSistemas"

int porcUmidade;
int porcTrepidacao;
int porcChuva;
//const char* SSID     = "EPD AP2"; // SSID / nome da rede WI-FI que deseja se conectar
//const char* PASSWORD = "2018vozmail2018"; // Senha da rede WI-FI que deseja se conectar

//const char* SSID     = "NETVIRTUA_202 2.4GHz"; // SSID / nome da rede WI-FI que deseja se conectar
//const char* PASSWORD = "Tuza1976"; // Senha da rede WI-FI que deseja se conectar


const char* SSID     = "AndroidAP"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "txgu8338";
const char* BROKER_MQTT = "test.mosquitto.org";

//const char* BROKER_MQTT = "192.168.0.6";

//const char* BROKER_MQTT = "192.168.3.116";

int BROKER_PORT = 1883; // Porta do Broker MQTT


WiFiClient espClient; 
PubSubClient MQTT(espClient); 
long numAleatorio;


void initWiFi(void);
void initMQTT(void);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT(void);
void reconnectWiFi(void);
void VerificaConexoesWiFIEMQTT(void);


void SensorDeUmidadeSolo ()
{
  const int pinoSensor = PIN_SENU;                    //PINO UTILIZADO PELO SENSOR
  int valorLido = analogRead(pinoSensor);             // fazendo a leitura do Sensor de umidade do solo
  float UmdPerc;
  UmdPerc = 100 * ((4095-(float)valorLido) / 4095);
  porcUmidade = (int)UmdPerc;
     /*
  if(porcUmidade > 60 && porcUmidade < 100)
  {
    Serial.println(" Status: Solo umido");

  }
  else
  if(porcUmidade > 30 && porcUmidade < 60)
  {
    Serial.println(" Status: Umidade moderada");
  
  }
  else
  {
    Serial.println(" Status: Solo seco");
      
  }
  */
}

void trepidacao()
{
  const int pinoTrepidacao = pinSensorTrepidacao;
  int valor = analogRead(pinoTrepidacao);
  float percentualTrepidacao;
  percentualTrepidacao = 100 * ((4095-(float)valor) / 4095);
  porcTrepidacao = (int)percentualTrepidacao;
     
  if(porcTrepidacao > 60 && porcTrepidacao < 100)
  {
    Serial.println(" Status: Trepidação alta");

  }
  else
  if(porcTrepidacao > 30 && porcTrepidacao < 60)
  {
    Serial.println(" Status: Trepidação moderada");
  
  }
  else
  {
    Serial.println(" Status: Baixa trepidação");
      
  }
}

void chuva()
{
    const int pinoChuva = pinSensorChuva;
    int valor1 = analogRead(pinoChuva);
    float percentualChuva;
    percentualChuva = 100 * ((4095-(float)valor1) / 4095);
    porcChuva = (int)percentualChuva;


}

void initWiFi(void)
{
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");
  reconnectWiFi();
}

void initMQTT(void)
{
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada 

}

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  String msg;
  /* obtem a string do payload recebido */
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;
  }
  Serial.print("Chegou a seguinte string via MQTT: ");
  Serial.println(msg);
  /* toma ação dependendo da string recebida */
  if (msg.equals("L"))
  {
    digitalWrite(PIN_LED, HIGH);
    Serial.println("LED aceso mediante comando MQTT");
  }
  if (msg.equals("D"))
  {
    digitalWrite(PIN_LED, LOW);
    Serial.println("LED apagado mediante comando MQTT");
  }

  if (msg.equals("A"))
  {
    digitalWrite(PIN_LED1, HIGH);
    Serial.println("LED apagado mediante comando MQTT");
  }
  if (msg.equals("B"))
  {
    digitalWrite(PIN_LED1, LOW);
    Serial.println("LED apagado mediante comando MQTT");
  }
  if (msg.equals("C"))
  {
//    digitalWrite(PIN_LED2, HIGH);
    Serial.println("LED apagado mediante comando MQTT");
  }
  if (msg.equals("E"))
  {
    //digitalWrite(PIN_LED2, LOW);
    Serial.println("LED apagado mediante comando MQTT");
  }
}
/*  Função:  reconecta-se  ao  broker  MQTT  (caso  ainda  não  esteja  conectado  ou  em  caso  de  a 
conexão cair)
           em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
   Parâmetros: nenhum
   Retorno: nenhum
*/
void reconnectMQTT(void)
{
  while (!MQTT.connected())
  {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT))
    {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPICO_SUBSCRIBE_LED);
    }
    else
    {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao em 2s");
      delay(2000);
    }
  }  
}

void VerificaConexoesWiFIEMQTT(void)
{
  if (!MQTT.connected())
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
  reconnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}


void reconnectWiFi(void)
{
  
  if (WiFi.status() == WL_CONNECTED)
    return;
  WiFi.begin(SSID, PASSWORD); 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("\nIP obtido: ");
  Serial.println(WiFi.localIP());
}


void setup() {
  Serial.begin(9600); 
  delay(1000);
  Serial.println("ProconceptSistemas");
  delay(1000);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW); 
  pinMode(PIN_LED1,OUTPUT);
  digitalWrite(PIN_LED1,LOW);
  pinMode(pinoDigitalUmidadeSolo, INPUT);
  pinMode(pinSensorTrepidacao,INPUT);
  
  
  initWiFi();
  
  initMQTT();
}

     
void loop(){


  char temperatura_str[10] = {0};
  char umidade_str[10] = {0};
  char umidadeSolo_str[10] = {0};
  char umidadeSoloAnalogico_str[10] = {0};
  char trepidacao_str[10] = {0};
  char chuva_str[10] = {0};
  char ola_str[10] = {0};
  char texto[25] = "olá mundo";


  
  DHT.read11(pinoDHT11); //LÊ AS INFORMAÇÕES DO SENSOR
  Serial.println("Umidade: "); //IMPRIME O TEXTO NA SERIAL
  Serial.println(DHT.humidity); //IMPRIME NA SERIAL O VALOR DE UMIDADE MEDIDO
  Serial.println("-----------------------------------------------------------");
  //Serial.print("%"); //ESCREVE O TEXTO EM SEGUIDA
  Serial.println("Temperatura: "); //IMPRIME O TEXTO NA SERIAL
  Serial.println(DHT.temperature); //IMPRIME NA SERIAL O VALOR DE UMIDADE MEDIDO E REMOVE A PARTE DECIMAL
  Serial.println("-----------------------------------------------------------");
  //Serial.print("*C"); //IMPRIME O TEXTO NA SERIAL
  //Serial.println("Umidade do solo: ");
  //Serial.println(porcUmidade);
  //Serial.println(umidadeSoloAnalogico_str);

  delay(2000); //INTERVALO DE 2 SEGUNDOS * NÃO DIMINUIR ESSE VALOR

  /*
  char temperatura_str[10] = {0};
  char umidade_str[10] = {0};
  char umidadeSolo_str[10] = {0};
  char umidadeSoloAnalogico_str[10] = {0};
 */
/* garante funcionamento das conexões WiFi e ao broker MQTT */
  VerificaConexoesWiFIEMQTT();


  SensorDeUmidadeSolo ();
  trepidacao();
  chuva();

  sprintf(ola_str,"%s",texto);
  sprintf(temperatura_str,"%dC", (long)DHT.temperature);
  sprintf(umidade_str, "%dC", (long)DHT.humidity);
  //sprintf(umidadeSoloAnalogico_str, "%d%%", porcUmidade);
  //sprintf(trepidacao_str,"%d",porcTrepidacao);
  //sprintf(chuva_str,"%d",porcChuva);
  
  
  MQTT.publish(TOPICO_PUBLISH_TEMPERATURA,temperatura_str);
  MQTT.publish(TOPICO_PUBLISH_UMIDADE ,umidade_str);
  MQTT.publish(TOPICO_PUBLISH_UMIDADE_SOLO,umidadeSoloAnalogico_str); 
  MQTT.publish(TOPICO_PUBLISH_TREPIDACAO,trepidacao_str);
  MQTT.publish(TOPICO_PUBLISH_CHUVA,chuva_str);
  MQTT.publish(TOPICO_PUBLISH_OLA,ola_str);
  MQTT.loop();
  
  delay(2000);
}
   
 

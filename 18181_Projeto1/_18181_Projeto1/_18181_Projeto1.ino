
//inclusão das bibliotecas
#include "WiFiEsp.h"
#include "SoftwareSerial.h"
//definição do pino de rele
#define pinRele 7 
//definindo uma porta serial virtual
SoftwareSerial Serial1(8,9);
//definindo o nome, senha da rede e crinado
//uma variavel para o estado da rede
char ssid[] = "TSE_JoaoAugusto";
char pass[] = "18181";
int status = WL_IDLE_STATUS;
//cria o servidor e o leitor de resposta
WiFiEspServer server(80);
RingBuffer buf(8);

void setup() {
  // inicializa os pinos e portas, desliga o rele.
  pinMode(pinRele,OUTPUT);
  digitalWrite(pinRele,LOW);  
  Serial.begin(115200);  // porta de debug
  Serial1.begin(9600);
  // inicializa o mudolo wifi
  WiFi.init(&Serial1);
  //checa se o modulo esta conectado
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("Sem modulo");
    while(true);  //trava a execução 
  }
  //espera conectar a rede
  while (status != WL_CONNECTED){
    Serial.println("tentando conectar...");
    status = WiFi.begin(ssid,pass);
  }
  // avisa que houve a conexão, inicia o servidor e
  // printa o ip na porta de debug
  Serial.println("Conectado...");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  server.begin();
}

void loop() {
  //cria um cliente
  WiFiEspClient client = server.available();
  //checa se o cliente esta no servidor e
  // inicializa o leitor de resposta
  if (client){
    Serial.println("Novo cliente");
    buf.init();
    // enquanto o cliente esta conectado, 
    //cada resposta que o cliente passar sera lida
    while (client.connected()){
      if (client.available()){
        char c = client.read();
        buf.push(c);
        Serial.write(c);
        //reação para cada resposta do servidor
        //desconecta o cliente
        if (buf.endsWith("\r\n\r\n")){
          Serial.println("Fechando"); 
          break;
        }
        // liga o rele
        if (buf.endsWith("ON")){
          digitalWrite(pinRele, HIGH);
          buf.reset();
          Serial.println(" Ligado");
        }
        // desliga o rele
        if (buf.endsWith("OFF")){
          digitalWrite(pinRele, LOW);
          buf.reset();
          Serial.println("deligado");
        }
        
        
      }
    } // while
    client.stop();
    Serial.println("Desconectado");
  }
}

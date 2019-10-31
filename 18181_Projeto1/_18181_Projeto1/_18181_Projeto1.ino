//inclusão das bibliotecas
#include "WiFiEsp.h"
#include "SoftwareSerial.h"
//definição do pino de rele
#define pinRele 7 
//definindo uma porta serial virtual
SoftwareSerial Serial1(9,10);
//definindo o nome, senha da rede e crinado
//uma variavel para o estado da rede
char ssid[] = "TSE_JoaoAugusto";
char pass[] = "00018181";
int status = WL_IDLE_STATUS;
//cria o servidor e o leitor de resposta
WiFiEspServer server(80);
RingBuffer buf(8);

bool ligado = false; // variavel para guardar o status do led

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
    status = WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);
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
          sendHttpResponse(client);
          break;
        }
        // liga o rele
        if (buf.endsWith("GET /ON")){
          digitalWrite(pinRele, HIGH);
          buf.reset();
          Serial.println(" Ligado");
          ligado = true;
        }
        // desliga o rele
        if (buf.endsWith("GET /OFF")){
          digitalWrite(pinRele, LOW);
          buf.reset();
          Serial.println("deligado");
          ligado = false;
        }   
      }
    } // while
    client.stop();
    Serial.println("Desconectado");
  }
}

void sendHttpResponse(WiFiEspClient client){
 client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"  // a conexão será fechada depois de concluir
    "Refresh: 20\r\n"        // a pagina ira atualizar automaticamente a cada 20 sec
    "\r\n");
  client.println("\r \n");
  client.println("\r \n");
  client.println("<!DOCTYPE HTML><html>\r \n");
  client.println("<head><title>Ligar rele");
  client.println("</title></head>\r \n");
  client.println("<body> <br><br><center>\r \n");
  client.println("<H1> LIGANDO RELE</H1>\r \n");
  if (ligado) // muda a pagina de acordo com o estado da lampada
  {
    client.println("<p style='line-height:0'><font color='green'>LIGADO</font></p>");
    client.println("<form action=\"ON\" method=\"get\"><button type='submit'>Desligar</button></form>\r \n"); //faz o botão enviar a resposta que apaga o led
  }else{
    client.println("<p style='line-height:0'><font color='red'>DESLIGADO</font></p>");
    client.println("<form action=\"OFF\" method=\"get\"><button type='submit'>Ligar</button></form>"); //faz o botão enviar a resposta que acender o led
    }
  client.println("</center></body></html>\r \n");
  client.println();
  
  
  
  delay(1);
}

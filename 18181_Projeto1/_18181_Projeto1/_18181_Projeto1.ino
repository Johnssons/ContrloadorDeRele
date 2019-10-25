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
  client.println("HTTP/1.1 200 OK"); //escreve a versão do http
  client.println("Content-Type: text/html"); //escreve o tipo do conteudo(texto/html)
  client.println("");
  client.println("<!DOCTYPE HTML>"); //informa que é um html

  //estrutura da pag
  client.println("<html>"); 
  client.println("<head>"); 
  client.println("<title>Controlador de lampada</title>");
  client.println("</head>"); 
  client.println("<body>");
  client.println("<font>ESTADO ATUAL DO LED</font>"); //ESCREVE O TEXTO NA PÁGINA
  
  if (ligado) // muda a pagina de acordo com o estado da lampada
  {
    client.println("<p style='line-height:0'><font color='green'>LIGADO</font></p>");
    client.println("<a href=\"/OFF\">APAGAR</a>"); //faz o botão enviar a resposta que apaga o led
  }else{
    client.println("<p style='line-height:0'><font color='red'>DESLIGADO</font></p>");
    client.println("<a href=\"/ON\">ACENDER</a>"); //faz o botão enviar a resposta que acender o led
    }
  
  client.println("<hr />"); 
  client.println("<hr />"); 
  client.println("</body>"); 
  client.println("</html>");
  delay(1);
}

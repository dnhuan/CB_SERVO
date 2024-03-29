#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
#include <Servo.h>
// fb.com/dnhuan - bac cu lien lac de hoi nhe
Servo servo;

const char* ssid     = "tenWIWI";
const char* password = "passWIFI";

WiFiServer server(80);

String header;
int pos = D3;
int neg = D5;
int ledPos = D11;
int ledNeg = D12;
int degON = 0;
int degOFF = 180;
bool CB_ON = false;
bool wait = false;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

void CB(){
  if(wait == false){
    wait = true;
    if(CB_ON){
      servo.write(degON);
      digitalWrite(ledPos, HIGH);
      Serial.print("CB: ");
      Serial.println(CB_ON);
    }
    else{
      servo.write(degOFF);
      digitalWrite(ledPos, LOW);
      Serial.print("CB: ");
      Serial.println(CB_ON);
    }
    delay(500);
    CB_ON = !CB_ON;
    wait = false;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(ledPos, OUTPUT);
  pinMode(ledNeg, OUTPUT);
  pinMode(pos, INPUT_PULLUP);
  pinMode(neg, OUTPUT);
  digitalWrite(neg, LOW);
  digitalWrite(ledNeg, LOW);
  attachInterrupt(digitalPinToInterrupt(pos), CB, RISING);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.softAPdisconnect (true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println();
  }
  Serial.println();
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  //mDNS
  Serial.println(WiFi.localIP());
    if (!MDNS.begin("maybom")) {    // dia chi web se la maybom.local
    while (1) { // bao loi
      digitalWrite(ledPos, HIGH);
      delay(1000);
      digitalWrite(ledPos, LOW);
      delay(1000);
    }
  }
  server.begin();
  MDNS.addService("http", "tcp", 80);
  // Servo
  servo.attach(D8);
  servo.write(degOFF);
  delay(500);
}

void loop(){
  WiFiClient client = server.available();
  if (client) {                             
    Serial.println("New client connected");
    String currentLine = "";                
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();         
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {                    // if newline
          if (currentLine.length() == 0) {  // now respond
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Router
            if (header.indexOf("GET / HTTP") >= 0) {
              Serial.println("Main menu");
              Serial.print("Status: ");
              Serial.println(CB_ON);
            } else if (header.indexOf("GET /toggle") >= 0) {
              Serial.print("Toggling state to ");
              Serial.println(!CB_ON);
              CB();
            }
            
            // HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta charset=\"UTF-8\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            // CSS
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // HTML body
            client.println("<body><h1>Máy bơm nước</h1>");
            if (CB_ON) {
              client.println("<h2>Trạng thái: ĐANG BẬT</h2>");
              client.println("<p><a href=\"/toggle\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<h2>Trạng thái: ĐANG TẮT</h2>");
              client.println("<p><a href=\"/toggle\"><button class=\"button button2\">OFF</button></a></p>");
            } 

            client.println("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

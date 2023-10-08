#include "defines.h"
#include <WebSockets2_Generic.h>

int status = WL_IDLE_STATUS;

using namespace websockets2_generic;

WebsocketsClient client;

int speedLeft = 0;
int speedRight = 0;

enum DrivingDirection {
  Stop,
  Forward,
  Backward
};

enum TurningDirection {
  StraightAhead,
  Left,
  Right
};

void onEventsCallback(WebsocketsEvent event, String data) {
  (void)data;

  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("Connnection Opened");
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Connnection Closed");
  } else if (event == WebsocketsEvent::GotPing) {
    Serial.println("Got a Ping!");
  } else if (event == WebsocketsEvent::GotPong) {
    Serial.println("Got a Pong!");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("Local IP Address: ");
  Serial.println(ip);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(16);
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial && millis() < 5000)
    ;

  Serial.println("\nStarting RP2040W-RepeatingClient on " + String(BOARD_NAME));
  Serial.println(WEBSOCKETS2_GENERIC_VERSION);

  ///////////////////////////////////

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");

    // don't continue
    while (true)
      ;
  }

  Serial.print(F("Connecting to SSID: "));
  Serial.println(ssid);

  status = WiFi.begin(ssid, pass);

  delay(1000);

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    delay(500);

    // Connect to WPA/WPA2 network
    status = WiFi.status();
  }

  printWifiStatus();

  ///////////////////////////////////

  Serial.print("Connecting to WebSockets Server @");
  Serial.println(websockets_server_host);

  // run callback when messages are received
  client.onMessage([&](WebsocketsMessage message) {
    //Serial.print("Got Message: ");
    //Serial.println(message.data());
  });

  // run callback when events are occuring
  client.onEvent(onEventsCallback);

  digitalWrite(LED_BUILTIN, HIGH);
}

String lastMessage = "";

void sendMessage(String message) {  
  if (lastMessage == message) return;

  lastMessage = message;
  // try to connect to Websockets server
  bool connected = client.connect(websockets_server_host, websockets_server_port, "/");

  if (connected) {
    Serial.println(message);
    client.send(message);
  } else {
    Serial.println("Not Connected!");
  }
}

int valX = 0;  // Vor Zurück 669 - 0 Mitte ca 342
int valY = 0;  // Links Rechs 668 - 0   Mitte ca 340

void readAnalogInput() {
  // put your main code here, to run repeatedly:
  valX = analogRead(XAchse);  // read the input pin
  valY = analogRead(YAchse);  // read the input pin

  int speed = 0;
  DrivingDirection drivingDirection = Stop;
  TurningDirection turningDirection = StraightAhead;

  if (valX > 40000) {  // Forward
    speed = map(valX, 40000, 65520, 56000, 65520);
    drivingDirection = Forward;
  } else if (valX <= 25000) {  // Backward
    speed = map(valX, 25000, 280, 56000, 65520);
    drivingDirection = Backward;
  }

  int speedLeft = 0;
  int speedRight = 0;

  if (valY > 39000) {  // Left
    speedRight = map(valY, 39000, 65520, 56000, 65520);
    turningDirection = Left;
  } else if (valY <= 32000) {  //Right
    speedLeft = map(valY, 32000, 250, 56000, 65520);
    turningDirection = Right;
  }

  String message = String((int)drivingDirection) + ";" + String((int)turningDirection) + ";" + speed + ";" + speedLeft + ";" + speedRight;
  sendMessage(message);
  delay(100);
}


void loop() {

  readAnalogInput();

  // let the websockets client check for incoming messages
  if (client.available()) {
    client.poll();
  }
}

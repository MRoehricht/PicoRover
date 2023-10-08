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
  pinMode(EnA, OUTPUT);
  pinMode(EnB, OUTPUT);
  pinMode(In1, OUTPUT);
  pinMode(In2, OUTPUT);
  pinMode(In3, OUTPUT);
  pinMode(In4, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);


  analogWriteFreq(5000);
  analogWriteRange(65535);
  analogWriteResolution(16);

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
  Serial.print("Connecting to WebSockets Server @");
  Serial.println(websockets_server_host);

  // run callback when messages are received
  client.onMessage([&](WebsocketsMessage message) {
    Serial.println(message.data());
    driveCar(message.data());
  });

  // run callback when events are occuring
  client.onEvent(onEventsCallback);
  sendMessage("Rover ON");
  digitalWrite(LED_BUILTIN, HIGH);
}

void sendMessage(String message) {
  // try to connect to Websockets server
  bool connected = client.connect(websockets_server_host, websockets_server_port, "/");

  if (connected) {
    Serial.println(message);
    client.send(message);
  } else {
    Serial.println("Not Connected!");
  }
}

bool isDriveMessage(String message) {
  int semicolonCount = 0;
  for (char c : message) {
    if (c == ';') {
      semicolonCount++;
    }
  }

  return semicolonCount == 4;
}

void driveCar(String message) {
  if (!isDriveMessage(message)) return;

  DrivingDirection drivingDirection = static_cast<DrivingDirection>(split(message, ';', 0).toInt());
  TurningDirection turningDirection = static_cast<TurningDirection>(split(message, ';', 1).toInt());

  setDrivingDirection(drivingDirection, turningDirection);
  if (turningDirection == StraightAhead) {
    int speed = split(message, ';', 2).toInt();
    turnWheels(speed);
  } else {
    int speedLeft = split(message, ';', 3).toInt();
    int speedRight = split(message, ';', 4).toInt();
    turnWheels(speedLeft, speedRight);
  }
}

String split(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setDrivingDirection(DrivingDirection drivingDirection, TurningDirection turningDirection) {
  switch (drivingDirection) {
    case Stop:
      Serial.print("-- \t");
      if (turningDirection == StraightAhead) {
        digitalWrite(In1, LOW);
        digitalWrite(In2, LOW);
        digitalWrite(In3, LOW);
        digitalWrite(In4, LOW);
      } else if (turningDirection == Left) {
        Serial.print("<");
        digitalWrite(In1, LOW);
        digitalWrite(In2, LOW);
        digitalWrite(In3, HIGH);
        digitalWrite(In4, LOW);
      } else if (turningDirection == Right) {
        Serial.print(">");
        digitalWrite(In1, HIGH);
        digitalWrite(In2, LOW);
        digitalWrite(In3, LOW);
        digitalWrite(In4, LOW);
      }
      break;
    case Forward:
      Serial.print("/\\ \t");
      if (turningDirection == StraightAhead) {
        digitalWrite(In1, HIGH);
        digitalWrite(In2, LOW);
        digitalWrite(In3, HIGH);
        digitalWrite(In4, LOW);
      } else if (turningDirection == Left) {
        Serial.print("<");
        digitalWrite(In1, LOW);
        digitalWrite(In2, LOW);
        digitalWrite(In3, HIGH);
        digitalWrite(In4, LOW);
      } else if (turningDirection == Right) {
        Serial.print(">");
        digitalWrite(In1, HIGH);
        digitalWrite(In2, LOW);
        digitalWrite(In3, LOW);
        digitalWrite(In4, LOW);
      }
      break;
    case Backward:
      Serial.print("\\/ \t");
      if (turningDirection == StraightAhead) {
        digitalWrite(In1, LOW);
        digitalWrite(In2, HIGH);
        digitalWrite(In3, LOW);
        digitalWrite(In4, HIGH);
      } else if (turningDirection == Left) {
        Serial.print("<");
        digitalWrite(In1, LOW);
        digitalWrite(In2, LOW);
        digitalWrite(In3, LOW);
        digitalWrite(In4, HIGH);
      } else if (turningDirection == Right) {
        Serial.print(">");
        digitalWrite(In1, LOW);
        digitalWrite(In2, HIGH);
        digitalWrite(In3, LOW);
        digitalWrite(In4, LOW);
      }
      break;
  }
}

void turnWheels(int speed) {
  turnWheels(speed, speed);
}

void turnWheels(int speedLeft, int speedRight) {
  Serial.print(speedLeft);
  Serial.print("\t");
  Serial.print(speedRight);
  analogWrite(EnA, speedLeft);
  analogWrite(EnB, speedRight);
}


void loop() {


  // let the websockets client check for incoming messages
  if (client.available()) {
    client.poll();
  }
}

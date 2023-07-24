#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

///////// CHANGEABLE VALUES /////////

const char serverAddress[] = "home-monitoring.scaleys.co.uk";
const int serverPort = 80;
const int httpRequestDelay = 5000;

const double minutesBetweenCalls = 1.0;

///////// CHANGEABLE VALUES ABOVE /////////

EthernetClient ethernetClient;
byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0xA1, 0xCF};
const char serviceEndpoint[] = "/hotwater";
const double millisecondsInAMinute = 60000.0;

//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

const int LCD_WIDTH = 16;

const String DECREASING = "DECREASING";
const String INCREASING = "INCREASING";
const String LEVEL = "LEVEL";

byte upArrow[8] = {
  B00100,
  B01110,
  B11111,
  B10101,
  B00100,
  B00100,
  B00000,
};

byte downArrow[8] = {
  B00000,
  B00100,
  B00100,
  B10101,
  B11111,
  B01110,
  B00100,
};

void setup() {
  lcd.createChar(1, upArrow);
  lcd.createChar(0, downArrow);
  lcd.begin(LCD_WIDTH, 2);
  lcd.setCursor(0,0);

  Serial.begin(9600);

  delay(1000);

  connectToEthernet();
}

void connectToEthernet() {
  // start the Ethernet connection:
  bool connectedToNetwork = false;
  while(!connectedToNetwork) {
    Serial.println("Attempting to connect to network...");

    if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to connect, trying again...");
    } else {
        Serial.println("Connected successfully");
        connectedToNetwork = true;
    }
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
  //lcd.print("connecting...");

  Serial.print("Connected to the network IP: ");
  Serial.println(Ethernet.localIP());
  //lcd.print("Network up...");
}

void loop() {
  displayData();

  Serial.println("fetched data");
  delay(millisecondsInAMinute * minutesBetweenCalls);
}

void displayData() {
  String data = receiveDataFromServer();

  if(data.length() > 0) {
    
    Serial.println("display data " + data);
    
    lcd.clear();
    lcd.home();

    String tempStatus = "";
    int statusIndex = 0;
    
    if(data.indexOf(DECREASING) > 0) {
      tempStatus = DECREASING;
      statusIndex = data.indexOf(DECREASING);
      data.replace(DECREASING, "");
    } else if (data.indexOf(INCREASING) > 0){
      tempStatus = INCREASING;
      statusIndex = data.indexOf(INCREASING);
      data.replace(INCREASING, "");
    } else if (data.indexOf(LEVEL) > 0) {
      tempStatus = LEVEL;
      statusIndex = data.indexOf(LEVEL);
      data.replace(LEVEL, "");
    }
    
    if(statusIndex == 0){
      statusIndex = LCD_WIDTH;
    }
    
    String line1 = data.substring(0, statusIndex);
    lcd.print(line1);
    
    if(tempStatus == INCREASING) {
      lcd.print((char) 1);
    } else if (tempStatus == DECREASING) {
      lcd.print((char) 0);
    } else if (tempStatus == LEVEL) {
      lcd.print("-");
    }
    
    if(data.length() > statusIndex) {
      lcd.setCursor(0, 1);
      statusIndex++;
      lcd.print(data.substring(statusIndex, (statusIndex + LCD_WIDTH)));
    }

  }
}

String receiveDataFromServer() {
  Serial.println("Request data from server");

  String response = "";

  if (ethernetClient.connect(serverAddress, serverPort)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    ethernetClient.print("GET ");
    ethernetClient.print(serviceEndpoint);
    ethernetClient.println(" HTTP/1.1");
    ethernetClient.println("Host: " + String(serverAddress) + ":" + serverPort);
    ethernetClient.println("Accept: text/html, text/plain");
    ethernetClient.println("Pragma: no-cache");
    ethernetClient.println("Cache-Control: no-cache");
    ethernetClient.println("Connection: close");
    ethernetClient.println();

    Serial.println("Called server");
    delay(httpRequestDelay);

    String dataRead = "";
    boolean reachedData = false;
    
    while (ethernetClient.connected() || ethernetClient.available()) {
      char c = ethernetClient.read();
      //Serial.print(c);

      if(reachedData) {
        response += c;
        //Serial.println(response);
      }

      dataRead += c;

      if(dataRead.endsWith("\r\n\r\n")) {
        Serial.println("Reached the data");
        reachedData = true;
      }
    }

    Serial.println("Finished reading data");

    ethernetClient.stop();
    ethernetClient.flush();
    Serial.println("Closed connection");
  }

  Serial.println("response data to return is " + response);
  return response;
}

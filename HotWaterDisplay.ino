#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

///////// CHANGEABLE VALUES /////////

const char pompeii[] = "192.168.0.16";
const int pompeiiPort = 28080;

const double minutesBetweenCalls = 1.0;

///////// CHANGEABLE VALUES ABOVE /////////

EthernetClient pompeiiClient;
byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0xA1, 0xCF};
const char pompeiiService[] = "/aggregator/services/hot-water-display";
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
  // attempt to connect to Wifi network:
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP waiting 1 minute");
    delay(millisecondsInAMinute);

    if (Ethernet.begin(mac) == 0)
    {
      Serial.println("Failed to configure Ethernet using DHCP waiting 1 more minute");
      delay(millisecondsInAMinute);

      if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP stopping - will need reset");
        while(true);
      }
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
  String data = receiveDataFromPompeii();

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

String receiveDataFromPompeii() {
  Serial.println("Request data from Pompeii");

  String response = "";

  if (pompeiiClient.connect(pompeii, pompeiiPort)) {
    Serial.println("connected to pompeii");
    // Make a HTTP request:
    pompeiiClient.print("GET ");
    pompeiiClient.print(pompeiiService);
    pompeiiClient.println(" HTTP/1.1");
    pompeiiClient.print("Host: ");
    pompeiiClient.print(pompeii);
    pompeiiClient.print(":");
    pompeiiClient.println(pompeiiPort);
    pompeiiClient.println("Accept: text/html, text/plain");
    pompeiiClient.println("Pragma: no-cache");
    pompeiiClient.println("Cache-Control: no-cache");
    pompeiiClient.println("Connection: close");
    pompeiiClient.println();

    Serial.println("Called pompeii");
    delay(5000);

    String dataRead = "";
    boolean reachedData = false;
    
    while (pompeiiClient.connected() || pompeiiClient.available()) {
      char c = pompeiiClient.read();
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

    pompeiiClient.stop();
    pompeiiClient.flush();
    Serial.println("Closed connection");
  }

  Serial.println("response data to return is " + response);
  return response;
}

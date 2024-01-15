#include <SoftwareSerial.h>

const int rxPin = 2;
const int txPin = 3;

SoftwareSerial sim800(rxPin, txPin);

#define RELAY_1 4
const int soilSensorPin = A0;
const int moistureThreshold = 720;
unsigned long lastSoilCheckTime = 0;
const unsigned long soilCheckInterval = 30000;

String phoneNumber = "+918876255598"; // Initial phone number

String smsStatus, senderNumber, receivedDate, msg;
boolean isReply = false;

void setup() {
  pinMode(RELAY_1, OUTPUT);
  digitalWrite(RELAY_1, HIGH);

  delay(7000);

  Serial.begin(115200);
  Serial.println("Arduino serial initialize");

  sim800.begin(115200);
  Serial.println("SIM7672s software serial initialize");

  smsStatus = "";
  senderNumber = "";
  receivedDate = "";
  msg = "";

  sim800.print("AT+CMGF=1\r");
  delay(1000);

  sim800.println("AT+CMGD=1,4");
  delay(1000);
  sim800.println("AT+CMGDA= \"DEL ALL\"");
  delay(1000);

  Reply("Arduino serial initialized");
  Reply("SIM7672s software serial initialized");
  Reply("The system is now operational");
}


void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastSoilCheckTime >= soilCheckInterval) {
    lastSoilCheckTime = currentMillis;
    checkSoilMoisture();
  }

  while (sim800.available()) {
    parseData(sim800.readString());
  }

  while (Serial.available()) {
    sim800.println(Serial.readString());
  }
}

void checkSoilMoisture() {
  int soilMoisture = analogRead(soilSensorPin);

  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);

  if (soilMoisture > moistureThreshold) {
    Reply("Alert! Plants need water. Water your plants by sending a message on");
  }
}

void parseData(String buff) {
  Serial.println(buff);

  unsigned int len, index;

  index = buff.indexOf("\r");
  buff.remove(0, index + 2);
  buff.trim();

  if (buff != "OK") {
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();

    buff.remove(0, index + 2);

    if (cmd == "+CMTI") {
      index = buff.indexOf(",");
      String temp = buff.substring(index + 1, buff.length());
      temp = "AT+CMGR=" + temp + "\r";
      sim800.println(temp);
    } else if (cmd == "+CMGR") {
      extractSms(buff);

      if (senderNumber == phoneNumber) {
        doAction();
        sim800.println("AT+CMGD=1,4");
        delay(1000);
        sim800.println("AT+CMGDA= \"DEL ALL\"");
        delay(1000);
      }
    }

  } else {
  }
}

void extractSms(String buff) {
  unsigned int index;

  index = buff.indexOf(",");
  smsStatus = buff.substring(1, index - 1);
  buff.remove(0, index + 2);

  senderNumber = buff.substring(0, 13);
  buff.remove(0, 19);

  receivedDate = buff.substring(0, 20);
  buff.remove(0, buff.indexOf("\r"));
  buff.trim();

  index = buff.indexOf("\n\r");
  buff = buff.substring(0, index);
  buff.trim();
  msg = buff;
  buff = "";
  msg.toLowerCase();
}

void doAction() {
  if (msg == "off") {
    digitalWrite(RELAY_1, HIGH);
    Reply("Motor is turned off");
  } else if (msg == "on") {
    digitalWrite(RELAY_1, LOW);
    Reply("Thanks for watering the plants. The motor will stop in twenty seconds.");
    delay(20000);
    digitalWrite(RELAY_1, HIGH);
    Reply("The twenty seconds are over. Correct amount of water has been delivered.");
  }

  smsStatus = "";
  senderNumber = "";
  receivedDate = "";
  msg = "";
}

void Reply(String text) {
  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\"" + phoneNumber + "\"\r");
  delay(1000);
  sim800.print(text);
  delay(100);
  sim800.write(0x1A);
  delay(1000);
  Serial.println("SMS Sent Successfully.");
}

#include <Ethernet.h>
#include <SPI.h>
#include "plotly_streaming_ethernet.h"

// Sign up to plotly here: https://plot.ly
// View your API key and streamtokens here: https://plot.ly/settings
#define nTraces 2
// View your tokens here: https://plot.ly/settings
// Supply as many tokens as data traces
// e.g. if you want to ploty A0 and A1 vs time, supply two tokens
char *tokens[nTraces] = {"25tm9197rz", "unbi52ww8a"};
// arguments: username, api key, streaming token, filename
plotly graph = plotly("workshop", "v6w5xlbx9j", tokens, "your_filename", nTraces);


// Setup Analog Light Sensor Pin
int sensorPin = A0;    // select the input pin for the potentiometer
float rawRange = 1024; // 3.3v
float logRange = 5.0; // 3.3v = 10^5 lux

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte my_ip[] = { 199, 168, 222, 18 }; // google will tell you: "public ip address"

void startEthernet(){
    Serial.println("... Initializing ethernet");
    if(Ethernet.begin(mac) == 0){
        Serial.println("... Failed to configure Ethernet using DHCP");
        // no point in carrying on, so do nothing forevermore:
        // try to congifure using IP address instead of DHCP:
        Ethernet.begin(mac, my_ip);
    }
    Serial.println("... Done initializing ethernet");
    delay(1000);
}


void setup() {
  analogReference(EXTERNAL);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  startEthernet();

  graph.fileopt="overwrite"; // See the "Usage" section in https://github.com/plotly/arduino-api for details
  bool success;
  success = graph.init();
  if(!success){while(true){}}
  graph.openStream();
}

void loop() {
  int rawValue = analogRead(sensorPin);

  Serial.print("Raw = ");
  Serial.print(rawValue);
  graph.plot(millis(), rawValue, tokens[0]);

  Serial.print(" - Lux = ");
  Serial.println(RawToLux(rawValue));
  graph.plot(millis(), RawToLux(rawValue), tokens[1]);

  delay(50);

}

float RawToLux(int raw)
{
  float logLux = raw * logRange / rawRange;
  return pow(10, logLux);
}

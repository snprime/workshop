#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <plotly_streaming_cc3000.h>

#define flow_sensor_pin 2

#define WLAN_SSID       "wifi_network_name"
#define WLAN_PASS       "wifi_network_password"
#define WLAN_SECURITY   WLAN_SEC_WPA2

// Sign up to plotly here: https://plot.ly
// View your API key and streamtokens here: https://plot.ly/settings
#define nTraces 1
// View your tokens here: https://plot.ly/settings
// Supply as many tokens as data traces
// e.g. if you want to ploty A0 and A1 vs time, supply two tokens
char *tokens[nTraces] = {"25tm9197rz"};
// arguments: username, api key, streaming token, filename
plotly graph("workshop", "v6w5xlbx9j", tokens, "filename", nTraces);

// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);

  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }

  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

void wifi_connect(){
  /* Initialise the module */
  Serial.println(F("\n... Initializing..."));
  if (!graph.cc3000.begin())
  {
    Serial.println(F("... Couldn't begin()! Check your wiring?"));
    while(1);
  }

  // Optional SSID scan
  // listSSIDResults();

  if (!graph.cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }

  Serial.println(F("... Connected!"));

  /* Wait for DHCP to complete */
  Serial.println(F("... Request DHCP"));
  while (!graph.cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }
}


void setup() {
  graph.maxpoints = 100;
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  wifi_connect();


  bool success;
  success = graph.init();
  pinMode(flow_sensor_pin, INPUT);
  digitalWrite(flow_sensor_pin, HIGH);
  lastflowpinstate = digitalRead(flow_sensor_pin);
  useInterrupt(true);
  if(!success){while(true){}}
  graph.openStream();
}

void loop() {
  Serial.print("Freq: "); Serial.println(flowrate);
  Serial.print("Pulses: "); Serial.println(pulses, DEC);
  float liters = pulses;
  liters /= 7.5;
  liters /= 60.0;
/*
  // if a brass sensor use the following calculation
  float liters = pulses;
  liters /= 8.1;
  liters -= 6;
  liters /= 60.0;
*/
  Serial.print(liters); Serial.println(" Liters");
  graph.plot(millis(), liters, tokens[0]);
  delay(100);
}

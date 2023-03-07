#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define BUTTON_PIN 22
#define LEDS_PIN   19

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS  10

// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 50 // Set BRIGHTNESS to about 1/5 (max = 255)
#define MSG_BUFFER_SIZE	50

Adafruit_NeoPixel pixels(NUMPIXELS, LEDS_PIN, NEO_GRB + NEO_KHZ800);

// debounce 
unsigned long debounceTimeStamp;
int previousButtonState;
int buttonCount = 0;

// mqtt
const char* ssid = "FREEBOX_JAMILA_BE";
const char* password = "lamalice1960";
const char* mqtt_server = "test.mosquitto.org";
short unsigned int mqtt_port = 1883;
char msgDistance[MSG_BUFFER_SIZE];
WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("distance-topic", "hello world");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {

  Serial.begin(115200);
  // put your setup code here, to run once:
 previousButtonState = digitalRead( BUTTON_PIN );
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
  pinMode(22,INPUT_PULLUP);
  pixels.setBrightness(BRIGHTNESS);
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // put your main code here, to run repeatedly:
  if ( millis() - debounceTimeStamp >= 500 ) {
    int buttonState = digitalRead(BUTTON_PIN);
    if(buttonState != previousButtonState) {
      if(buttonState == HIGH) {
        debounceTimeStamp = millis();
        changeState();
      }
    }
    previousButtonState = buttonState;
    pixels.show();
  }
}


void changeState() {
  if(buttonCount==0) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    client.publish("colorled-topic", "off");
  } else if (buttonCount == 1) {
    pixels.setPixelColor(0, pixels.Color(150, 0, 0));
    client.publish("colorled-topic", "red");
  } else if (buttonCount == 2) {
    pixels.setPixelColor(0, pixels.Color(0,150,0));
    client.publish("colorled-topic", "green");
  } else if (buttonCount == 3) {
    pixels.setPixelColor(0, pixels.Color(0,0,150));
    client.publish("colorled-topic", "blue");
  } else {
    buttonCount = 0;
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    client.publish("colorled-topic", "off");
  }
  buttonCount=buttonCount+1;
}

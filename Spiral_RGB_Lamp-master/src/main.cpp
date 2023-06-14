// Include the necessary libraries
#include <Arduino.h>
#include <FastLED.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include "firebase.h"
#include "DHTesp.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Ticker.h>


// WiFi credentials
char ssid[] = "sogumi";
char password[] = "vincun123";

// Blynk auth token and virtual pins
char auth[] = "H6WfUYHlpgZAvaTYzQZyjSy4Dc7ewrzG";
int virtualPinButton = V0;
int VirtualRed = V1;
int VirtualGreen = V2;
int VirtualBlue = V3;
int VirtualBrightness = V4;

// FastLED definitions
#define NUM_LEDS    72
#define LED_PIN     4
#define PIN_DHT 5 
#define LCD_SDA 21
#define LCD_SCL 22
DHTesp dht;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16 chars and 2 line display

int g_Brightness = 5;
int g_PowerLimit = 3000;
bool g_LEDstate = false;

CRGB g_LEDs[NUM_LEDS] = {0};

unsigned long LastDataSentTime = 0;
const unsigned long FIVE_MINUTES = 5 * 60 * 1000;  // 5 minutes in milliseconds
unsigned long startTime = millis();  // Start time of the 5-minute period
unsigned long LastStatisticsUpdateTime = 0;  // Initialize the variable outside the loop()


void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) { }
  Serial.println("ESP32 Startup");

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(g_LEDs, NUM_LEDS);
  FastLED.setBrightness(g_Brightness);
  set_max_power_indicator_LED(LED_BUILTIN);
  FastLED.setMaxPowerInMilliWatts(g_PowerLimit);
  dht.setup(PIN_DHT, DHTesp::DHT11);
  LastDataSentTime = millis();

    lcd.init();                      // Initialize the LCD
  lcd.backlight();
  Wire.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  Blynk.begin(auth, ssid, password);

  // Initialize Firebase
  String streamPath = "/led_control";
  Firebase_Init(streamPath);

  // Assign a function to handle changes in the virtual pin state
  Blynk.virtualWrite(virtualPinButton, 0);
  Blynk.virtualWrite(VirtualRed, 0);
  Blynk.virtualWrite(VirtualGreen, 0);
  Blynk.virtualWrite(VirtualBlue, 0);
  Blynk.virtualWrite(VirtualBrightness, g_Brightness);
}

void turnOnRandomLED();

void turnOnOffLED(int value)
{
  if (value == 1) {
    // Turn on the LED strip
    FastLED.setBrightness(g_Brightness);
    FastLED.show();
    // Update the Firebase value
    Firebase.RTDB.setInt(&fbdo, "/led_control/state", value);
       // Call the function to turn on a random LED
    turnOnRandomLED();
  } if (value == 0) {
    // Turn off the LED strip
    FastLED.setBrightness(0);
    FastLED.clear();
    FastLED.show();
    // Update the Firebase value
    Firebase.RTDB.setInt(&fbdo, "/led_control/state", value);

    Serial.println("sending on off values to firebase...");
  }
}

void changeColor(int red, int green, int blue)
{
  CRGB color = CRGB(red, green, blue);
  fill_solid(g_LEDs, NUM_LEDS, color);
  FastLED.show();
  // Update the Firebase values
  Firebase.RTDB.setInt(&fbdo, "/led_control/red", red);
  Firebase.RTDB.setInt(&fbdo, "/led_control/green", green);
  Firebase.RTDB.setInt(&fbdo, "/led_control/blue", blue);

  Serial.println("Sending color data to Firebase...");
}

void turnOnRandomLED()
{
  g_LEDstate = true;
  digitalWrite(LED_PIN, HIGH);

  // Generate random RGB values for the LED color
  int red = random(256);
  int green = random(256);
  int blue = random(256);

   Serial.print("Random Color: R=");
  Serial.print(red);
  Serial.print(", G=");
  Serial.print(green);
  Serial.print(", B=");
  Serial.println(blue);

  changeColor(red, green, blue);

  FastLED.setBrightness(g_Brightness);
  FastLED.show();

  // Update the Firebase value
  Firebase.RTDB.setInt(&fbdo, "/led_control/state", 1);
}

void adjustBrightness(int brightness)
{
  g_Brightness = brightness;
  FastLED.setBrightness(g_Brightness);
  FastLED.show();
  // Update the Firebase value
  Firebase.RTDB.setInt(&fbdo, "/led_control/brightness", brightness);
}

void printValueReceived(int value, int virtualPin)
{
  Serial.print("Value received for virtual pin V");
  Serial.print(virtualPin);
  Serial.print(": ");
  Serial.println(value);
}

float minTemperature = 0.0;
float maxTemperature = 0.0;
float avgTemperature = 0.0;
float minHumidity = 0.0;
float maxHumidity = 0.0;
float avgHumidity = 0.0;
int numReadings = 0;
float temperatureSum = 0.0;
float humiditySum = 0.0;

void onSendSensor()
{
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

    lcd.setCursor(0, 0);
  lcd.print("Humidity: " + String(humidity, 1) + "%");

  lcd.setCursor(0, 1);
  lcd.print("Temp: " + String(temperature, 1) + "C");

  if (dht.getStatus()==DHTesp::ERROR_NONE)
  {
    Serial.printf("Temperature: %.2f C, Humidity: %.2f %%\n", 
      temperature, humidity);
         // Update minimum and maximum values
  if (numReadings == 0 || temperature < minTemperature) {
    minTemperature = temperature;
  }
  if (numReadings == 0 || temperature > maxTemperature) {
    maxTemperature = temperature;
  }
  if (numReadings == 0 || humidity < minHumidity) {
    minHumidity = humidity;
  }
  if (numReadings == 0 || humidity > maxHumidity) {
    maxHumidity = humidity;
  }
  
    Firebase.RTDB.setFloat(&fbdo, "/data/temperature", temperature);
    Firebase.RTDB.setFloat(&fbdo, "/data/humidity", humidity);
  }
  else
  {
    Serial.printf("DHT11 error: %d\n", dht.getStatus());
  };
  
 // Update sum for calculating average
  temperatureSum += temperature;
  humiditySum += humidity;
  numReadings++;
  Serial.println("sending sensor data to firebase...");
}

void CalculateStats() {
  // Check if there are no readings
  if (numReadings == 0) {
    // Set default or meaningful values for average
    avgTemperature = 0.0;
    avgHumidity = 0.0;
  } else {
    // Calculate average values only if there are valid readings
    if (temperatureSum != 0.0) {
      avgTemperature = temperatureSum / numReadings;
    } else {
      // Handle the case where there are no valid temperature readings
      avgTemperature = 0.0; // Set a default value or handle it according to your requirements
    }
    if (humiditySum != 0.0) {
      avgHumidity = humiditySum / numReadings;
    } else {
      // Handle the case where there are no valid humidity readings
      avgHumidity = 0.0; // Set a default value or handle it according to your requirements
    }
  }

  // Reset variables for next calculation
  numReadings = 0;
  temperatureSum = 0.0;
  humiditySum = 0.0;

  // Print and send the calculated statistics
  Serial.println("-----Statistics-----");
  Serial.printf("Min Temperature: %.2f C\n", minTemperature);
  Serial.printf("Max Temperature: %.2f C\n", maxTemperature);
  Serial.printf("Average Temperature: %.2f C\n", avgTemperature);
  Serial.printf("Min Humidity: %.2f %%\n", minHumidity);
  Serial.printf("Max Humidity: %.2f %%\n", maxHumidity);
  Serial.printf("Average Humidity: %.2f %%\n", avgHumidity);
  Serial.println("--------------------");

  // Update the Firebase values
  Firebase.RTDB.setFloat(&fbdo, "/data/min_temperature", minTemperature);
  Firebase.RTDB.setFloat(&fbdo, "/data/max_temperature", maxTemperature);
  Firebase.RTDB.setFloat(&fbdo, "/data/avg_temperature", avgTemperature);
  Firebase.RTDB.setFloat(&fbdo, "/data/min_humidity", minHumidity);
  Firebase.RTDB.setFloat(&fbdo, "/data/max_humidity", maxHumidity);
  Firebase.RTDB.setFloat(&fbdo, "/data/avg_humidity", avgHumidity);
}


void onFirebaseStream(FirebaseStream data)
{
  if (data.dataType() == "int") {
    int value = data.intData();
    // Process the received value from Firebase
    // For example, you can call functions to control the LED strip based on the received value
    if (data.streamPath() == "/led_control/state") {
      turnOnOffLED(value);
      printValueReceived(value, 0);
    } else if (data.streamPath() == "/led_control/red" && value >= 0 && value <= 255) {
      int red = value;
      // Set the red component of the LED color
      // For example, you can call a function like changeColor(red, green, blue)
      // with the appropriate values for green and blue components
      printValueReceived(red, 1);
    } else if (data.streamPath() == "/led_control/green" && value >= 0 && value <= 255) {
      int green = value;
      // Set the green component of the LED color
      // For example, you can call a function like changeColor(red, green, blue)
      // with the appropriate values for red and blue components
      printValueReceived(green, 2);
    } else if (data.streamPath() == "/led_control/blue" && value >= 0 && value <= 255) {
      int blue = value;
      // Set the blue component of the LED color
      // For example, you can call a function like changeColor(red, green, blue)
      // with the appropriate values for red and green components
      printValueReceived(blue, 3);
    } else if (data.streamPath() == "/led_control/brightness" && value >= 0 && value <= 255) {
      int brightness = value;
      // Adjust the brightness of the LED strip
      // For example, you can call a function like adjustBrightness(brightness)
      printValueReceived(brightness, 4);
    }
  } if (data.dataType() == "int") {
    int value = data.intData();
    String streamPath = data.streamPath();
    
    if (streamPath.startsWith("/led_control/")) {
      // Ignore LED control data
      return;
    }

    // Process the received value from Firebase
    // For example, you can call functions to handle the received data

    if (streamPath == "/data/temperature") {
      // Handle temperature data
      Serial.print("Received temperature: ");
      Serial.println(value);
    } else if (streamPath == "/data/humidity") {
      // Handle humidity data
      Serial.print("Received humidity: ");
      Serial.println(value);
    }
  }
}


void onSendSensor();

void loop()
{
	Blynk.run();  

   // Check if 5 minutes have passed since the last statistics update
  if (millis() - LastStatisticsUpdateTime >= FIVE_MINUTES) {
    CalculateStats();
    LastStatisticsUpdateTime = millis();
  }

  // Check if 10 seconds have passed since the last data sent
  if (millis() - LastDataSentTime >= 10000) {
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    onSendSensor();
    LastDataSentTime = millis();
  }

  // Do something else if needed
}

int targetState = 0;  // Variable to store the desired LED state
int targetBrightness = 0;  // Variable to store the desired brightness
CRGB targetColor = CRGB(0, 0, 0);  // Variable to store the desired color

// Function to handle changes in the button virtual pin state
BLYNK_WRITE(V0)
{
  int value = param.asInt();  // Read the value of the button virtual pin
  targetState = value;  // Update the desired LED state locally
  turnOnOffLED(targetState);  // Call the turnOnOffLED function with the value
  printValueReceived(value, 0);
}

// Function to handle changes in the red color virtual pin state
BLYNK_WRITE(V1)
{
  int value = param.asInt();  // Read the value of the red color virtual pin
  targetColor.r = value;  // Update the desired red component locally

  if (targetState == 1) {
    // Update the LED color only if the LED is turned on
    changeColor(targetColor.r, targetColor.g, targetColor.b);  // Update the LED color
  }

  printValueReceived(value, 1);
}

// Function to handle changes in the green color virtual pin state
BLYNK_WRITE(V2)
{
  int value = param.asInt();  // Read the value of the green color virtual pin
  targetColor.g = value;  // Update the desired green component locally

  if (targetState == 1) {
    // Update the LED color only if the LED is turned on
    changeColor(targetColor.r, targetColor.g, targetColor.b);  // Update the LED color
  }

  printValueReceived(value, 2);
}

// Function to handle changes in the blue color virtual pin state
BLYNK_WRITE(V3)
{
  int value = param.asInt();  // Read the value of the blue color virtual pin
  targetColor.b = value;  // Update the desired blue component locally

  if (targetState == 1) {
    // Update the LED color only if the LED is turned on
    changeColor(targetColor.r, targetColor.g, targetColor.b);  // Update the LED color
  }

  printValueReceived(value, 3);
}

// Function to handle changes in the brightness virtual pin state
BLYNK_WRITE(V4)
{
  int brightness = param.asInt();  // Read the value of the brightness virtual pin
  targetBrightness = brightness;  // Update the desired brightness locally

  if (targetState == 1) {
    // Adjust the LED brightness only if the LED is turned on
    adjustBrightness(targetBrightness);  // Adjust the LED brightness
  }

  printValueReceived(brightness, VirtualBrightness);
}

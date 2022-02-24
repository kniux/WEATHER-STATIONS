/*
 * Name: Estación meteorologica
 * Author: Edgar Cano
 * Date:    03/05/2021
 * Ver:   0.1
 *      - Uso del componente generico 280 (no Adafruit)
 *      - Adaptación de librerías genericas #include "i2c.h" e 
 *      - #include "i2c_BMP280.h"
 *      - No hay lectura de humedad.
 */

#include <Wire.h>                        // Include Wire library (required for I2C devices)
#include <Adafruit_GFX.h>                // Include Adafruit graphics library
#include <Adafruit_SSD1306.h>            // Include Adafruit SSD1306 OLED driver
#include <RTClib.h>
#include <DHT.h>
#include "i2c.h"
#include "i2c_BMP280.h"

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET 1

#define oledLoading     0
#define oledDateTime    1
#define oledTemHum      2
#define oledPressAlt    3
uint8_t state= oledLoading;

#define DHTPin  9
#define DHTType DHT11

Adafruit_SSD1306 display(128, 64);  // declara la resolucion del display
BMP280 bmp280;
RTC_DS1307 rtc; 
DHT dht11(DHTPin, DHTType);

float temperature;
float pascal;
int humedad;
static float meters, metersold;

long intervalOledRefresh = 500;
unsigned long previousMillisOledRefresh = 0;

long intervalBme = 1000;
unsigned long previousMillisBme = 0;

long intervalOledScreen = 5000;
unsigned long previousMillisOledScreen = 0;

/*FUNCIONES PRINCIPALES*/
void setup(void) {
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  delay(1000); 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // inicia la comunicacion I2C con el display que tiene la direccion 0x3C  
  display.setRotation(0);  
  display.clearDisplay();
  display.display(); 
  display.setTextColor(WHITE, BLACK);  

    Serial.begin(115200);
    Serial.print("Probe BMP280: ");
    if (bmp280.initialize()) Serial.println("Sensor found");
    else{
        Serial.println("Sensor missing");
        while (1) {}
    }
    // onetime-measure:
    bmp280.setEnabled(0);
    bmp280.triggerMeasurement();
}
void loop(){
/*   Máquina de Estados Finitos   */
  unsigned long currentMillis = millis();

  if((currentMillis - previousMillisOledRefresh ) >= (intervalOledRefresh)){
      previousMillisOledRefresh = currentMillis;
      oledInfo();
  }
  if((currentMillis - previousMillisBme ) >= (intervalBme)){
      previousMillisBme = currentMillis;
      readDataBme280();
  }
  if((currentMillis - previousMillisOledScreen ) >= (intervalOledScreen)){
      previousMillisOledScreen = currentMillis;
      OledChangeScreen();
  }  
}

void oledInfo(){
  display.clearDisplay();
  display.setTextColor(WHITE, BLACK);  

  switch( state){
    case oledLoading:
        displayLoading();
        break;
    case oledDateTime:
        displayDateTime();
        break;
    case oledTemHum:
        displayTempHum();
        break;    
    case oledPressAlt:
        displayPressAlt();
        break;  
    default:
    break;                
  } 
}

void readDataBme280(){
    bmp280.awaitMeasurement();
    bmp280.getTemperature(temperature);
    bmp280.getPressure(pascal);    
    bmp280.getAltitude(meters);
    metersold = (metersold * 10 + meters)/11;
    bmp280.triggerMeasurement();
/*
    Serial.print(" HeightPT1: ");
    Serial.print(metersold);
    Serial.print(" m; Height: ");
    Serial.print(meters);
    Serial.print(" Pressure: ");
    Serial.print(pascal);
    Serial.print(" Pa; T: ");
    Serial.print(temperature);
    Serial.println(" C");
*/    
}

void OledChangeScreen(){
  switch(state){
    case oledLoading:
      state= oledDateTime;
    break;
    case oledDateTime:
      state= oledTemHum;
    break; 
    case oledTemHum:
      state= oledPressAlt;
    break; 
    case oledPressAlt:
      state= oledDateTime;
    break;   
    default:
    break;  
  }
}

void displayLoading(){
 // display.drawBitmap(0,0,pato,128,64,1);
}

void displayDateTime(){
  oledPrintChar(1,2,3, "WEATHER STATION OLED");
  
  DateTime now = rtc.now();
  char bufferDate[15] = "00/00/0000";
  formatoFecha(bufferDate, now.day(), now.month(), now.year());
  oledPrintChar(2,5,17, bufferDate);
  
  char bufferTime[11] = "00:00:00";
  formatoHora(bufferTime, now.hour(), now.minute(), now.second());
  oledPrintChar(2,17,32, bufferTime);

  display.drawRect(0,0,128,64, WHITE);
  oledPrintChar(1,28,54, "HOLA :)");
  display.display();
}

void displayTempHum(){
  oledPrintChar(1,30, 3, "TEMPERATURA");
  oledPrintFloat(2,25, 15, temperature);
  display.print(" C");
  oledPrintChar(1,42, 35, "HUMEDAD");
  oledPrintFloat(2,25, 46, 0);
  display.print(" NO%");
  display.fillCircle(92,17,2, WHITE);
  display.drawRoundRect(0,0,128,64,15,WHITE);
  display.drawLine(0,32,128,32,WHITE);
  display.display();
}

void displayPressAlt(){
  oledPrintChar(1,42, 3, "PRESION");
  oledPrintFloat(2,4, 15, pascal);
  display.setCursor(74, 15);
  display.print(" hPa");
  oledPrintChar(1,42, 35, "ALTITUD");
  oledPrintFloat(2,25, 46, metersold);
  display.print(" m");
  display.drawRoundRect(0,0,128,64,15,WHITE);
  display.drawLine(0,32,128,32,WHITE);
  display.display();
}

void oledPrintChar(int text_size, int x_pos, int y_pos, char text[]){
  display.setTextSize(text_size);
  display.setCursor(x_pos, y_pos);
  display.print(text);
  display.display();
}
void oledPrintFloat(int text_size, int x_pos, int y_pos, float data){
  display.setTextSize(text_size);
  display.setCursor(x_pos, y_pos);
  display.print(data);
  display.display();
}

void formatoFecha(char bufferFecha[12], int numDia, int numMes, int numA){
  sprintf(bufferFecha, "%02d/%02d/%04d",numDia,numMes,numA);
}
void formatoHora(char bufferHora[12], int hora, int minutos, int segundos){
  sprintf(bufferHora, "%02d:%02d:%02d",hora,minutos,segundos);
}

// End of Code

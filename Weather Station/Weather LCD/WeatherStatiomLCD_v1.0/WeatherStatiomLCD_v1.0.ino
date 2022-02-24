/*
 * Name: Weather Station 
 * Date: 13/05/2021
 * Author: Edgar Cano
 * Ver: 1.0
 * Desc: - Estación meteorologica, proporciona 
 *        humedad, temperatura, hora y fecha...
 * HW:   - Sensor DS3231/DHT11/Pantalla LCD.
 */
#include <DHT.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define DHTPin  2
#define DHTType DHT11

/* Definiendo FSM del sistema */
#define S_HOME            0
#define S_REFRESH_SCREEN  1
#define S_READ_SENSORS    2
#define S_SWITCH_SCREEN   3 
uint8_t systemState = 0;    

/* Definiendo FSM para pantalla activa*/
#define SCREEN_WELCOME    0
#define SCREEN_DATEHOUR   1
#define SCREEN_HUMTEMP    2
uint8_t screenState = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2); 
DHT dht11(DHTPin, DHTType);
RTC_DS3231 rtc;
 
unsigned long previousMillisState1 = 0;
long Timeinterval_1 = 500;     // Refresh sccreen

unsigned long previousMillisState2 = 0;
long Timeinterval_2 = 2000;     // Read Sensor

unsigned long previousMillisState3 = 0;
long Timeinterval_3 = 5000;     // Switch Screen

int tempDHT = 0;
int Hum = 0;
float tempDS = 0;

// Subsistema de Gestión de Transiciones para la FSM del sistema **************************************/
void FSM_System_Transitions(){
  unsigned long currentMillis = millis();
  
  if (((currentMillis - previousMillisState1) >= Timeinterval_1 )  // Revisando que se cumpla la condición
        && (systemState == S_HOME)){   //Forzando a realizar la comparación con el estado IDLE
    previousMillisState1 = currentMillis;
    systemState = S_REFRESH_SCREEN;
  }
  if (((currentMillis - previousMillisState2) >= Timeinterval_2 )
        && (systemState == S_HOME)){
    previousMillisState2 = currentMillis;
    systemState = S_READ_SENSORS;
  }
  if (((currentMillis - previousMillisState3) >= Timeinterval_3 )
        && (systemState == S_HOME)){
    previousMillisState3 = currentMillis;
    systemState = S_SWITCH_SCREEN;
  }
}
void FSM_System(){
  switch(systemState){
    case S_HOME:              /*STATE 0: NO SE EJECUTA TAREA*/
    break;
    case S_REFRESH_SCREEN:    /*STATE 1: EJECUTA TAREA REFREZCO DE PANTALLA*/
        RefreshScreen();
    break;
    case S_READ_SENSORS:      /*STATE 2: EJECUTA TAREA LECTURA DE SENSORES*/
        ReadSensors();
    break;
    case S_SWITCH_SCREEN:     /*STATE 3: EJECUTA TAREA INTERCAMBIO DE PANTALLA ACTIVA*/
        FSM_Screens();
    break;    
    default:
    break;  
  }
  // Regresando el estado de la FSM_System al idle (S_HOME)
  if(systemState != S_HOME)
      systemState = S_HOME;
}
void RefreshScreen(){
  switch(screenState){
    case SCREEN_WELCOME:    // Refrezco bienvenida (no cambia)
        displayWelcome();
    break;
    case SCREEN_DATEHOUR:   // Refrezco fecha-hora (se nota el cambio en los segundos)
        displayDate();
        displayHour();
    break;
    case SCREEN_HUMTEMP:
        displayHumtemp();  // Refrezco humedad-temp (se nota el cambio, si lo hay, cada 2seg)
    break;
    default:
    break;
  }
}
void ReadSensors(){
  tempDHT = dht11.readTemperature();
  Hum = dht11.readHumidity();
  tempDS = rtc.getTemperature();
}
void FSM_Screens(){
  switch(screenState){
     case SCREEN_WELCOME:
        cleanScreen();        
        screenState = SCREEN_DATEHOUR;
    break;
    case SCREEN_DATEHOUR:
       cleanScreen();
        screenState = SCREEN_HUMTEMP;
    break;
    case SCREEN_HUMTEMP:
       cleanScreen();
       screenState = SCREEN_DATEHOUR;
    break;
    default:
    break;
  }
}
// Subsistema de Gestión de Pantallas  *************************************/
void displayWelcome(){
  lcd.setCursor(0,0);
  lcd.print("  Weather Station") ;
  lcd.setCursor(5,1);
  lcd.print("Ver. 1.0") ;
}  
void displayDate(){
  DateTime now = rtc.now();
  char _bufferFecha[12];
  lcd.setCursor(0, 0);  
  formatoFecha(_bufferFecha, now.day(), now.month(), now.year() );
  lcd.print(_bufferFecha);  
}
void displayHour(){
  DateTime now = rtc.now();
  char _bufferHora[10];
  lcd.setCursor(0, 1);  
  formatoHora( _bufferHora, now.hour(), now.minute(), now.second() );
  lcd.print(_bufferHora);
}
void formatoFecha(char bufferFecha[12], int numDia, int numMes, int numA){
  sprintf(bufferFecha, "%02d/%02d/%04d", numDia, numMes, numA);
}
void formatoHora(char bufferHora[10], int hora, int minu, int seg){
  sprintf(bufferHora, "%02d:%02d:%02d", hora, minu, seg);
}
void displayHumtemp(){
    char symbol = 223;
    lcd.setCursor(0, 0);   
    lcd.print("DH:");    
    lcd.print(tempDHT);
    lcd.print(symbol);  
    lcd.print(" DS:");    
    lcd.print(tempDS);
    lcd.print(symbol);  
    lcd.setCursor(0, 1);  
    lcd.print("HUMEDAD");
    lcd.setCursor(9, 1);  
    lcd.print(Hum);
    lcd.print("%");    
}
void cleanScreen(){
  lcd.clear();  
}

// Subsistema de Control Principal *************************************
void setup() {
  //Serial.begin(9600);
  dht11.begin();
  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  lcd.init();
  lcd.backlight();                  
  delay(500);  
}
void loop(){
  FSM_System_Transitions();  
  FSM_System();  
}

// End of code

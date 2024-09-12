#include <WiFi.h>
#include <WiFiClient.h>
#include <TridentTD_LineNotify.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <DFRobot_MLX90614.h>

#include "extern_variable.h"

//接腳與Token定義
#define irSensorPin 16
#define tempSensorPin 4

//RTC時間設定
const char* ntpServer = "time.google.com";
const long  gmtOffset_sec = 28800;
const int   daylightOffset_sec = 0;
char timeResult [ 20 ];

//IR Sensor 驅動
DFRobot_MLX90614_I2C sensor;

//溫度setup
OneWire oneWire ( tempSensorPin );
DallasTemperature sensors ( &oneWire );

//LCD Setup
LiquidCrystal_I2C lcd ( 0x27, 16, 2 );

//公用變數
float objectTemp;

void setup ()
{
  //距離setup
  Serial.begin ( 115200 );

  //接角及感測器設定
  pinMode ( irSensorPin, INPUT_PULLUP );
  sensors.begin ();

  //呼叫設定副程式
  wifiSetup ();
  lineSetup ();
  irSensorSetup ();
  lcdSetup ();
  timeSetup ();
}

void loop ()
{  
  //IR讀取數值變數
  int L = digitalRead ( irSensorPin );

  //偵測到物件
  if ( L == 0 )
  {
    //溫度感測器數值請求
    sensors.requestTemperatures ();

    //溫度讀取數值輸出
    LINE.notify ( "Obstacle detected" );
    LINE.notify ( sensors.getTempCByIndex ( 0 ) ); //轉換攝氏度並輸出
    lcdDetectedPrint ( sensors.getTempCByIndex ( 0 ) );

    //NTP輸出
    delay ( 500 );
    localTime ();
    lcdTimePrint ();
    LINE.notify ( timeResult );
    delay ( 500 );
  }
  
  else
  {
    //無偵測數值輸出
    LINE.notify ( "=== All clear" );
    lcdUndetectedPrint ();
  }

  delay ( 1000 );
}

//網路設定
void wifiSetup ()
{
  WiFi.mode ( WIFI_STA );
  WiFi.begin ( ssid, password );

  while ( WiFi.status () != WL_CONNECTED )
  {
    delay ( 500 );
  }
}

//Line Notify設定 
void lineSetup ()
{
  LINE.setToken ( LINE_TOKEN );
  LINE.notify ( "Line Notify Link Confirm." );
}

//LCD I2C設定
void lcdSetup ()
{
  lcd.init ();
  lcd.backlight ();

  lcd.print ( "LCD Ready" );
  delay ( 3000 );
  lcd.clear ();
}

//NTP 設定
void timeSetup ()
{
  configTime ( gmtOffset_sec, daylightOffset_sec, ntpServer );
}

//IR Sensor設定
void irSensorSetup ()
{
  while ( NO_ERR != sensor.begin () )
  {
    Serial.println ( "Communication with device failed, please check connection" );
    delay ( 3000 );
  }
  
  Serial.println ( "Begin ok!" );

  sensor.enterSleepMode ();
  delay ( 50 );

  sensor.enterSleepMode ( false );
  delay ( 200 );
}

//LCD 輸出(偵測到物件:是)
void lcdDetectedPrint ( float temp )
{
  lcd.clear ();
  lcd.setCursor ( 0, 0 );
  lcd.print ( "Detected" );

  lcd.setCursor ( 0, 1 );
  lcd.print ( temp );
}

//LCD 輸出(偵測到物件:否)
void lcdUndetectedPrint ()
{
    lcd.clear ();
    lcd.setCursor ( 0, 0 );
    lcd.print ( "=== All clear" );
}

//讀取及時時間
void localTime ()
{
  time_t rawTime;
  struct tm *info;
 
  time( &rawTime );
 
  info = localtime ( &rawTime );
 
  strftime ( timeResult, sizeof ( timeResult ), "%Y-%m-%d %H:%M", info );
}

//LCD NTP 輸出
void lcdTimePrint ()
{
  lcd.clear ();
  lcd.setCursor ( 0, 0 );
    
  lcd.print ( timeResult );
  lcd.setCursor ( 0, 1 );
}

//IR Sensor 輸出
void irSensorPrint ()
{
  objectTemp = sensor.getObjectTempCelsius ();

  Serial.print ( "Object celsius : " );
  Serial.print ( objectTemp );
  Serial.println ( " °C" );

  delay(500);
}

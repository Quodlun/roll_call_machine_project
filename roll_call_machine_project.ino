#include <WiFi.h>
#include <WiFiClient.h>
#include <TridentTD_LineNotify.h>  //IMPORTANT!!!: Please Use v3.0.5, it can NOT success the verify while using v3.0.6.
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <Adafruit_Fingerprint.h>
#include "extern_variable.h"

/*  DEBUG CODE
// 在 .cpp 文件或主程式中進行初始化
const int irSensorPin = 16;
const int tempSensorPin = 4;

const char* LINE_TOKEN = "MbcvZH9nf1mvsSkXBzEpoQDQNOaNkWpHJ2nmoSBSBpz";
const char* ssid = "When Can My Internet Get Better";
const char* password = "O00O00O0";

const char* ntpServer = "time.google.com";
const long gmtOffset_sec = 28800;  // GMT+8 的偏移量，秒數
const int daylightOffset_sec = 0;  // 沒有日光節約時間
*/

//溫度setup
OneWire oneWire ( tempSensorPin );
DallasTemperature sensors ( &oneWire );

//LCD Setup
LiquidCrystal_I2C lcd ( 0x27, 16, 2 );

// 指紋模組 Setup
HardwareSerial mySerial ( 2 ); 
Adafruit_Fingerprint finger ( &mySerial );

//共用變數
char timeResult [ 20 ];

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
  lcdSetup ();
  timeSetup ();
  fingerprintSetup();
  bumperSetup ();
}

void loop ()
{
  // 檢查指紋
  int fingerprintID = getFingerprintID();

  if ( fingerprintID >= 0 )
  {
    Serial.print ( "識別到指紋，ID: " );
    Serial.println ( fingerprintID );
    LINE.notify ( "Fingerprint ID: " + String ( fingerprintID ) );

    lcd.setCursor ( 0, 0 );  // 設定光標在第一行
    lcd.print ( "Fingerprint found" );
    lcd.setCursor ( 0, 1 );  // 設定光標在第二行顯示ID
    lcd.print ( "ID: " + String ( fingerprintID ) );

    delay ( 3000 );
    lcd.clear ();

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

      bumperWork ();
    }
  
    else
    {
    //無偵測數值輸出
      LINE.notify ( "=== All clear" );
      lcdUndetectedPrint ();
    }
  }
  
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("No Match Found");
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

//NTP設定
void timeSetup ()
{
  configTime ( gmtOffset_sec, daylightOffset_sec, ntpServer );
}

//指紋模組設定
void fingerprintSetup ()
{
  mySerial.begin ( 57600, SERIAL_8N1, 16, 17 ); // 初始化指紋傳感器
  finger.begin ( 57600 );
  delay ( 5 );

  if ( finger.verifyPassword () )
  {
    Serial.println ( "找到指紋傳感器！" );
    lcd.setCursor ( 0, 1 );
    lcd.print ( "Fingerprint OK" );
  }
  
  else
  {
    Serial.println ( "未找到指紋傳感器 :(" );
    lcd.setCursor ( 0, 1 );
    lcd.print ( "No Fingerprint" );

    while ( 1 )
    {
      delay ( 1 ); // 無限等待，因為未找到傳感器
    }
  }
}

//抽水馬達設定
void bumperSetup ()
{
  pinMode ( bumperPin, OUTPUT );
  digitalWrite ( bumperPin, LOW );
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

// 檢查是否有檢測到指紋
int getFingerprintID ()
{
  uint8_t p = finger.getImage ();
  
  // 檢查是否檢測到手指
  if ( p == FINGERPRINT_NOFINGER )
  {
    Serial.println ( "未檢測到手指" );
    return -1;
  }
  
  else if ( p == FINGERPRINT_OK )
  {
    Serial.println ( "指紋檢測成功" );
  }
  
  else
  {
    Serial.println ( "指紋檢測失敗" );

    return -1;
  }

  // 搜尋指紋匹配
  p = finger.fingerSearch ();

  if ( p == FINGERPRINT_OK )
  {
    Serial.print ( "找到匹配的指紋，ID #" ); 
    Serial.println ( finger.fingerID );

    return finger.fingerID;
  }

  else
  {
    Serial.println ( "未找到匹配的指紋" );
    LINE.notify ( "警告!有未知人士正在使用點名器" );

    return -1;
  }
}

//抽水馬達運作
void bumperWork ()
{
  digitalWrite ( bumperPin, HIGH );
  delay ( bumperDelay );
  digitalWrite ( bumperPin, LOW );
}
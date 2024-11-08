#include <WiFi.h>
#include <WiFiClient.h>
#include <TridentTD_LineNotify.h>
#include <OneWire.h> 
#include <DallasTemperature.h> 
#include <CapacitiveFingerprint.h>  // 假設電容式指紋傳感器庫

// Pin Definitions
#define LINE_TOKEN "MbcvZH9nf1mvsSkXBzEpoQDQNOaNkWpHJ2nmoSBSBpz"

// WiFi Credentials
const char* ssid     = "When Can My Internet Get Better";
const char* password = "O00O00O0";



// 假設電容式指紋傳感器的設置
HardwareSerial mySerial(2); 
CapacitiveFingerprint finger(&mySerial);  // 使用電容式指紋模組的庫

void setup() {
  Serial.begin(115200);

  // Initialize Sensors
  sensors.begin();
  wifi_setup();
  line_setup();
  fingerprint_setup();  // 初始化指紋傳感器

  Serial.println("System Ready");
}

void loop() {
  // 檢查指紋
  int fingerprintID = getFingerprintID();
  if (fingerprintID >= 0) {
    Serial.print("識別到指紋，ID: ");
    Serial.println(fingerprintID);
    LINE.notify("Fingerprint ID: " + String(fingerprintID));
  } else {
    Serial.println("No Match Found");
    LINE.notify("警告!有未知人士正在使用點名器");
  }

  delay(1000); // 每秒檢查一次
}

// WiFi 設定
void wifi_setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nConnected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Line Notify 設定
void line_setup() {
  LINE.setToken(LINE_TOKEN);
  LINE.notify("Line Notify Link Confirm.");
}

// 指紋傳感器初始化
void fingerprint_setup() {
  mySerial.begin(57600, SERIAL_8N1, 16, 17); // 初始化串口，用於指紋傳感器
  finger.begin(57600);
  delay(5);

  if (finger.verifyPassword()) {
    Serial.println("找到電容式指紋傳感器！");
  } else {
    Serial.println("未找到指紋傳感器 :(");
    while (1) delay(1); // 無限等待，因為未找到傳感器
  }
}

// 檢查是否有檢測到指紋
int getFingerprintID() {
  uint8_t p = finger.getImage();
  
  // 檢查是否檢測到手指
  if (p == CAPFINGERPRINT_NOFINGER) {  // 修改為電容式指紋的狀態常量
    Serial.println("未檢測到手指");
    return -1;
  } else if (p == CAPFINGERPRINT_OK) {  // 電容式指紋的成功狀態
    Serial.println("指紋檢測成功");
  } else {
    Serial.println("指紋檢測失敗");
    return -1;
  }

  // 搜尋指紋匹配
  p = finger.fingerSearch();
  if (p == CAPFINGERPRINT_OK) {  // 電容式匹配成功的狀態
    Serial.print("找到匹配的指紋，ID #"); 
    Serial.println(finger.fingerID);
    return finger.fingerID;
  } else {
    Serial.println("未找到匹配的指紋");
    return -1;
  }
}

}
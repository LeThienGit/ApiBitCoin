#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* ssid = "xxxx";        // Thay bằng tên Wi-Fi của bạn
const char* password = "xxxx"; // Thay bằng mật khẩu Wi-Fi của bạn

// URL của API TimeAPI
const char* timeServer = "https://timeapi.io/api/time/current/zone?timeZone=Asia%2FHo_Chi_Minh";  // API lấy giờ từ Hồ Chí Minh
const char* btcApi = "https://api.binance.com/api/v3/ticker/price?symbol=BTCUSDT";  // API giá Bitcoin

void setup() { 
  Wire.begin(21, 22);  
  lcd.begin(20, 4);     
  lcd.backlight();    

  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");

  // Kết nối Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("Connected to WiFi");
  delay(1000); 
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Gửi yêu cầu GET đến API thời gian
    http.begin(timeServer); 
    int httpCodeTime = http.GET(); // Gửi yêu cầu GET

    if (httpCodeTime > 0) {
      String payloadTime = http.getString(); // Lấy dữ liệu JSON trả về từ API thời gian
      Serial.println(payloadTime);  

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payloadTime);

      if (doc.containsKey("hour") && doc.containsKey("minute") && doc.containsKey("seconds") && 
          doc.containsKey("day") && doc.containsKey("month") && doc.containsKey("year") && 
          doc.containsKey("dayOfWeek")) {
        
        int hour = doc["hour"];
        int minute = doc["minute"];
        int seconds = doc["seconds"];
        int day = doc["day"];
        int month = doc["month"];
        int year = doc["year"];
        String dayOfWeek = doc["dayOfWeek"];
        
        // Hiển thị thời gian lên màn hình LCD
        lcd.setCursor(0, 0);  // Vị trí bắt đầu ở cột 0, dòng 0
        lcd.print(dayOfWeek);  
        lcd.print(" ");        
        lcd.print(day);       
        lcd.print("/");
        lcd.print(month);      
        lcd.print("/");
        lcd.print(year);

        lcd.setCursor(6, 1); 
        if (hour < 10) lcd.print("0");
        lcd.print(hour);       
        lcd.print(":");
        if (minute < 10) lcd.print("0");
        lcd.print(minute);
        lcd.print(":");
        if (seconds < 10) lcd.print("0");
        lcd.print(seconds);

        // Gửi yêu cầu GET đến API Binance để lấy giá Bitcoin
        http.begin(btcApi);
        int httpCodeBTC = http.GET();

        if (httpCodeBTC > 0) {
          String payloadBTC = http.getString();  // Lấy dữ liệu JSON trả về từ API Binance
          Serial.println(payloadBTC);  

          // Phân tích JSON để lấy giá Bitcoin
          DynamicJsonDocument docBTC(1024);
          deserializeJson(docBTC, payloadBTC);
          
          if (docBTC.containsKey("price")) {
            float btcPrice = docBTC["price"].as<float>(); 
            btcPrice = round(btcPrice * 100.0) / 100.0;     
          
            lcd.setCursor(0, 3);  
            lcd.print("BTC: ");
            lcd.print(btcPrice, 2); 
            lcd.print(" USDT");
          }

        } else {
          Serial.println("Failed to get BTC price");
        }
        http.end();
      } else {
        Serial.println("Error: Missing time data in JSON response");
      }
    } else {
      Serial.println("HTTP request failed, error code: " + String(httpCodeTime));
    }

    http.end(); 
  } else {
    Serial.println("Disconnected from WiFi");
  }

  delay(1000);
}

  #include <EEPROM.h>
  #include "GravityTDS.h"
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  #include <SoftwareSerial.h>
  //Cấu hình led đơn
  //4 đèn 5 quạt sục 6
  int ledPin1 = 4;
  int ledPin2 = 5;
  int ledPin3 = 6;

  // Cấu hình cảm biến TDS
  #define TdsSensorPin A1
  #define TDS_THRESHOLD 300
  GravityTDS gravityTds;

  // Cấu hình cảm biến pH
  #define pHSense A0
  #define PH_SAMPLES 10

  // Cấu hình cảm biến mực nước
  #define sensorPower 7
  #define sensorPin A2 // Thay đổi chân cảm biến mực nước thành A2

  // Bluetooth (HC-06)
  SoftwareSerial Bluetooth(2, 3); // HC-06 TX, RX

  // Biến
  float temperature = 25, tdsValue = 0;
  float adc_resolution = 1024.0;
  int waterLevel = 0; // Biến để lưu giá trị mực nước

  void setup() {
      Serial.begin(9600);         // Để gỡ lỗi
      Bluetooth.begin(9600);      // Tốc độ baud mặc định của HC-06
      pinMode(ledPin1, OUTPUT); // thiết lập chân 4 là đầu ra
      pinMode(ledPin2, OUTPUT);
      pinMode(ledPin3, OUTPUT);

      // Khởi tạo cảm biến TDS   
      gravityTds.setPin(TdsSensorPin);
      gravityTds.setAref(5.0);
      gravityTds.setAdcRange(1024);
      gravityTds.begin();

      // Cấu hình chân nguồn cho cảm biến mực nước
      pinMode(sensorPower, OUTPUT);
      digitalWrite(sensorPower, LOW); // Tắt cảm biến ban đầu

      //Cấu hình led đơn

  }

  float ph(float voltage) {
      return 7 + ((2.5 - voltage) / 0.18);
  }

  // Hàm đọc giá trị từ cảm biến mực nước
  int readSensor() {
      digitalWrite(sensorPower, HIGH);  // Bật cảm biến
      delay(10);                        // Chờ ổn định
      int val = analogRead(sensorPin);  // Đọc giá trị
      digitalWrite(sensorPower, LOW);   // Tắt cảm biến
      return val;                       // Trả về giá trị
  }
  float convertToWaterLevelPercentage(int sensorValue) {
      // Giả sử giá trị từ 0-1023 tương ứng với 0%-100%
      return (sensorValue / 5.9);  // Chuyển đổi giá trị thành phần trăm
  }

  void loop() {
      // Đo TDS
      gravityTds.setTemperature(temperature);
      gravityTds.update();
      tdsValue = gravityTds.getTdsValue();

      // Đo pH
      int pH_measurings = 0;
      for (int i = 0; i < PH_SAMPLES; i++) {
          pH_measurings += analogRead(pHSense);
          delay(10);
      }
      float pH_voltage = 5 / adc_resolution * pH_measurings / PH_SAMPLES;
      float pH_value = ph(pH_voltage);

      // Đo mực nước
      waterLevel = readSensor();
      float waterLevelPercentage = convertToWaterLevelPercentage(waterLevel); // Tính phần trăm

      // Gửi giá trị TDS, pH và mực nước qua Bluetooth
      Bluetooth.print(tdsValue, 0); 
      Bluetooth.print("|");
      Bluetooth.print(pH_value - 13, 1);
      Bluetooth.print("|");
      Bluetooth.print(waterLevelPercentage, 0); // Gửi giá trị phần trăm
      Bluetooth.print("|");
      delay(20);

      // In dữ liệu lên Serial Monitor để kiểm tra
      String dataToSend = String(tdsValue, 0) + "|" + String(pH_value - 13, 1)   + "|" + String(waterLevelPercentage, 0);
      Serial.println("Data sent: " + dataToSend);

  if(Bluetooth.available()) 
  {
    char cmd = Bluetooth.read();
    if (cmd == '1') // nếu nhận được ký tự '1' từ Bluetooth, bật đèn
    { 
      digitalWrite(ledPin1, HIGH);
    }
    else if (cmd == '2') // nếu nhận được ký tự '2' từ Bluetooth, tắt đèn
    { 
      digitalWrite(ledPin1, LOW);
    }
    else if (cmd == '3') 
    { 
      digitalWrite(ledPin2, HIGH);
    }
    else if (cmd == '4') 
    { 
      digitalWrite(ledPin2, LOW);
    }
    else if (cmd == '5') 
    { 
      digitalWrite(ledPin3, HIGH);
    }
    else if (cmd == '6') 
    {
      digitalWrite(ledPin3, LOW);  
    }
  }
}
# esp32_ble_oled
這是 ESP32 端程式，負責接收 Flutter App 透過藍牙連接傳送來的訊息，並顯示在OLED 螢幕上。

##  我的No+tion

https://www.notion.so/Esp32_ble_oled-28fcd2f379818098b837f54097551962?source=copy_link


##  函式庫：

🔹Adafruit SSD1306
🔹Adafruit GFX Library
🔹U8g2

### 🧩 Arduino IDE 開發環境
- **ESP32 開發板套件**  
  加入開發板網址：
  https://dl.espressif.com/dl/package_esp32_index.json

## 🧠 功能說明

1. 使用 ESP32 內建藍牙功能 (`BluetoothSerial`)，建立藍牙連線。
2. 透過手機或電腦的藍牙終端機傳送文字。
3. 顯示文字於 OLED 螢幕，支援：
   - 中文（使用 `u8g2_font_wqy12_t_gb2312` 字型）
   - 英文
   - 自動換行機制（超出螢幕寬度會自動折行）
4. 可即時更新畫面，不需重啟 ESP32。
5. 按下 Enter，會刷新顯示內容。

---

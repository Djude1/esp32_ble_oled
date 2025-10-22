#include <Wire.h>
#include <U8g2lib.h>
#include "BluetoothSerial.h"

// === OLED 使用 I2C 接法 ===
// SDA → GPIO21, SCL → GPIO22
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,
  /* reset=*/ U8X8_PIN_NONE,
  /* clock=*/ 22,
  /* data=*/ 21
);

// === 藍牙設定 ===
BluetoothSerial SerialBT;
String btMessage = "";

// === UTF-8 字元長度判斷函式 ===
int utf8CharLen(unsigned char c) {
  if ((c & 0x80) == 0) return 1;         // ASCII
  else if ((c & 0xE0) == 0xC0) return 2; // 2-byte
  else if ((c & 0xF0) == 0xE0) return 3; // 3-byte (中文多為此類)
  else if ((c & 0xF8) == 0xF0) return 4; // 4-byte emoji
  else return 1;
}

// === 自動換行顯示函式 ===
void drawWrappedText(const char* text, int x, int y, int lineHeight, int maxWidth) {
  int cursorX = x;
  int cursorY = y;

  while (*text) {
    int len = utf8CharLen((unsigned char)*text);
    char buf[5] = {0};
    strncpy(buf, text, len);

    int w = u8g2.getUTF8Width(buf);

    // 換行判斷
    if (cursorX + w > maxWidth || *text == '\n') {
      cursorX = x;
      cursorY += lineHeight;
      if (*text == '\n') {
        text += len;
        continue;
      }
    }

    u8g2.drawUTF8(cursorX, cursorY, buf);
    cursorX += w;
    text += len;
  }
}

void setup() {
  Serial.begin(115200);

  // 初始化 OLED
  u8g2.begin();
  u8g2.enableUTF8Print(); // ✅ 啟用 UTF-8 中文
  u8g2.setFont(u8g2_font_wqy12_t_gb2312); // ✅ 小中文字體
  u8g2.clearBuffer();
  u8g2.drawUTF8(0, 20, "等待藍牙連線...");
  u8g2.sendBuffer();

  // 初始化藍牙
  SerialBT.begin("ESP32_中文顯示_自動換行");
  Serial.println("✅ 藍牙啟動成功，請連線 ESP32_中文顯示_自動換行");
}

void loop() {
  if (SerialBT.available()) {
    char c = SerialBT.read();

    if (c == '\n' || c == '\r') {
      if (btMessage.length() > 0) {
        
        if (btMessage.equalsIgnoreCase("CLEAR") || btMessage.equalsIgnoreCase("RESET")) {
          u8g2.clearBuffer();
          u8g2.sendBuffer();
          // 在序列埠顯示，確認指令已收到
          Serial.println("🗑️ 收到重置指令，螢幕已清空。");
        } 
        else {
          Serial.print("📩 收到文字：");
          Serial.println(btMessage);

          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_wqy12_t_gb2312);
          drawWrappedText(btMessage.c_str(), 0, 16, 16, 128);
          u8g2.sendBuffer();
        }
        
        btMessage = ""; // 無論如何都清空緩衝區
      }
    } else {
      btMessage += c;
    }
  }
}
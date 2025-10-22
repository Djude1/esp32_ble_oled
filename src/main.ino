#include <Wire.h>
#include <U8g2lib.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// === OLED (與你原本的設定相同) ===
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,
  /* reset=*/ U8X8_PIN_NONE,
  /* clock=*/ 22,
  /* data=*/ 21
);

// === 全域變數 ===
String currentMessage = "Waiting Bluetooth Connection..."; // 用來儲存當前要顯示的訊息
bool shouldUpdateDisplay = true;          // 標記是否需要更新OLED

// === BLE 設定 ===
// 這是你的服務和特徵的 "地址"，App 會透過它來找到通訊的管道
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_RX "beb5483e-36e1-4688-b7f5-ea07361b26a8" // RX = 接收來自手機的訊息

// 特徵回呼類別：當 App 寫入資料時，這裡會被觸發
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String receivedText = pCharacteristic->getValue();

      if (receivedText.length() > 0) {
        Serial.print("📩 收到來自App的文字: ");
        Serial.println(receivedText);

        if (receivedText.equalsIgnoreCase("CLEAR") || receivedText.equalsIgnoreCase("RESET")) {
            currentMessage = ""; // 清空訊息
            Serial.println("🗑️ 收到重置指令，螢幕已清空。");
        } else {
            currentMessage = receivedText;
        }
        
        shouldUpdateDisplay = true; // 設定標記，讓 loop() 去更新螢幕
      }
    }
};

// === 中文處理函式 (與你原本的程式碼完全相同) ===
int utf8CharLen(unsigned char c) {
  if ((c & 0x80) == 0) return 1;
  else if ((c & 0xE0) == 0xC0) return 2;
  else if ((c & 0xF0) == 0xE0) return 3;
  else if ((c & 0xF8) == 0xF0) return 4;
  else return 1;
}

void drawWrappedText(const char* text, int x, int y, int lineHeight, int maxWidth) {
  int cursorX = x;
  int cursorY = y;
  while (*text) {
    int len = utf8CharLen((unsigned char)*text);
    char buf[5] = {0};
    strncpy(buf, text, len);
    int w = u8g2.getUTF8Width(buf);
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
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
  
  // --- 初始化 BLE ---
  Serial.println("🚀 正在啟動 BLE 模式...");
  
  // 1. 初始化 BLE 設備，並設定廣播名稱 (這就是App會掃描到的名字)
  BLEDevice::init("ESP32_中文顯示_自動換行");

  // 2. 建立 BLE 伺服器
  BLEServer *pServer = BLEDevice::createServer();
  
  // 3. 建立 BLE 服務
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 4. 在服務中建立一個特徵 (Characteristic)，讓 App 可以對它寫入資料
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  
  // 5. 綁定回呼函式，當 App 寫入資料時，MyCharacteristicCallbacks 會被呼叫
  pRxCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // 6. 啟動服務
  pService->start();

  // 7. 啟動廣播，讓 App 可以發現這個設備
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  
  Serial.println("✅ BLE 啟動成功！正在廣播，等待 App 連線...");
}

void loop() {
  // BLE 是事件驅動的，主要邏輯在回呼函式中
  // loop() 只負責根據標記來更新螢幕顯示，這樣更有效率
  if (shouldUpdateDisplay) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    // 使用 c_str() 將 Arduino String 轉為 C-style string 給函式使用
    drawWrappedText(currentMessage.c_str(), 0, 16, 16, 128);
    u8g2.sendBuffer();
    
    shouldUpdateDisplay = false; // 更新完畢，重置標記
  }

  // 可以在此處加入設備斷線重連等的邏輯，但我們先保持簡單
  delay(100); 
}

#include <Arduino.h>
#include <Wire.h>
// constants
constexpr uint8_t STATUS_LED_PIN = 25;
constexpr float FAULT_THRESHOLD_PA = 35.0f;
constexpr uint8_t SDA_PIN = 21; 
constexpr uint8_t SCL_PIN = 22; 


struct Reading{
  float pressure;
  uint32_t timestampMS;
  uint8_t sensorId;
};

QueueHandle_t queueA;
QueueHandle_t queueB;

void myTaskA(void *param) {
  TickType_t lastWake = xTaskGetTickCount();
  for (;;) {
    delay(30);
    Reading result = {101325.0f + random(-5, 6), millis(), 0};
    if(xQueueSend(queueA, &result, 0) != pdTRUE){
      Serial.print("A full at: ");
      Serial.print(result.timestampMS);
      Serial.print(" # of items: ");
      Serial.println(uxQueueMessagesWaiting(queueA));
    }
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(500));
  }
}

void myTaskB(void *param) {
  TickType_t lastWake = xTaskGetTickCount();
  for (;;) {
    delay(50);
    Reading result = {101355.0f + random(-5, 6), millis(), 1};
    if(xQueueSend(queueB, &result, 0) != pdTRUE){
      Serial.print("B full at:  ");
      Serial.print(result.timestampMS);
      Serial.print(" # of items: ");
      Serial.println(uxQueueMessagesWaiting(queueB));
    }
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(500));
  }
}

void comparatorTask(void *param){
  for(;;){
    Reading rA;
    if (xQueueReceive(queueA, &rA, portMAX_DELAY) != pdTRUE) {continue;}
    Reading rB;
    if (xQueueReceive(queueB, &rB, portMAX_DELAY) != pdTRUE) {continue;}
    int32_t dt = static_cast<int32_t>(rB.timestampMS - rA.timestampMS);
    float dp = rB.pressure - rA.pressure;
    if(fabsf(dp) > FAULT_THRESHOLD_PA){
      digitalWrite(STATUS_LED_PIN, HIGH);
    }
    else{
      digitalWrite(STATUS_LED_PIN, LOW);
    }
    Serial.print("tA=");
    Serial.print(rA.timestampMS);
    Serial.print(" tB=");
    Serial.print(rB.timestampMS);
    Serial.print(" dt=");
    Serial.print(dt);
    Serial.print(" pA=");
    Serial.print(rA.pressure);
    Serial.print(" pB=");
    Serial.print(rB.pressure);
    Serial.print(" dp=");
    Serial.println(dp);
  }
}

void scanI2C(){
  uint8_t count = 0;
  for (uint8_t address = 1; address < 127; ++address){
    Wire.beginTransmission(address);
    uint8_t result = Wire.endTransmission();
    if(result == 0){
      Serial.print("0x");
      Serial.println(address, HEX);
      ++count;
    }
  }
  if(count == 0){
    Serial.println("No I2C devices found - check wiring");
  }
  else{
    Serial.print("Found ");
    Serial.print(count);
    Serial.println(" device(s)");
  }
}

void setup(){
  Serial.begin(115200);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);
  scanI2C();

  queueA = xQueueCreate(10, sizeof(Reading));
  queueB = xQueueCreate(10, sizeof(Reading));
  xTaskCreate(myTaskA, "A", 4096, NULL, 1, NULL);
  xTaskCreate(myTaskB, "B", 4096, NULL, 1, NULL);
  xTaskCreate(comparatorTask, "C", 4096, NULL, 1, NULL);
}

void loop(){
  vTaskDelay(portMAX_DELAY);
}
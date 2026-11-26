#include <Arduino.h>
struct Reading{
  float pressure;
  uint32_t timestampMS;
  uint8_t sensorId;
};

QueueHandle_t queueA;

void myTaskA(void *param) {
  TickType_t lastWake = xTaskGetTickCount();
  for (;;) {
    delay(30);
    Reading result = {101325.0f, millis(), 0};
    if(xQueueSend(queueA, &result, 0) != pdTRUE){
      Serial.print("time: ");
      Serial.print(result.timestampMS);
      Serial.print(" # of items: ");
      Serial.println(uxQueueMessagesWaiting(queueA));
    }
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(500));
  }
}

void printerTask(void *param){
  for(;;){
    Reading output;
    if (xQueueReceive(queueA, &output, portMAX_DELAY) == pdTRUE) {
      Serial.print("id=");
      Serial.print(output.sensorId);
      Serial.print(" pressure=");
      Serial.print(output.pressure);
      Serial.print(" time=");
      Serial.println(output.timestampMS);
    }
  }
}

void setup(){
  Serial.begin(115200);
  queueA = xQueueCreate(10, sizeof(Reading));
  xTaskCreate(printerTask, "Printer Task", 4096, NULL, 1, NULL);
  xTaskCreate(myTaskA, "A", 4096, NULL, 1, NULL);
}

void loop(){
  vTaskDelay(portMAX_DELAY);
}
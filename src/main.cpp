#include <Arduino.h>
void myTaskA(void *param) {
  for (;;) {       // loop forever
    // do the job
    Serial.print("A: ");
    Serial.println(millis());
    // wait a bit
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void myTaskB(void *param) {
  for (;;) {       // loop forever
    // do the job
    Serial.print("B: ");
    Serial.println(millis());
    // wait a bit
    vTaskDelay(pdMS_TO_TICKS(700));
  }
}

void setup(){
  Serial.begin(115200);
  xTaskCreate(myTaskA, "A", 4096, NULL, 1, NULL);
  xTaskCreate(myTaskB, "B", 4096, NULL, 1, NULL);
}

void loop(){
vTaskDelay(portMAX_DELAY);
}
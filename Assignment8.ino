
#include<Arduino_FreeRTOS.h>
#include "queue.h"
unsigned long long buttonbounce[3], GreenDelay;
QueueHandle_t q;
TaskHandle_t Handle[3];
void setup() {
  Serial.begin(9600);
  DDRD |= B01110000;
  PORTD |= B01110000;
  q = xQueueCreate(10, sizeof(int));
  xTaskCreate(Sender, "f", 128, NULL, 1, NULL);
  xTaskCreate(Reciever, "f", 128, NULL, 1, NULL);
  xTaskCreate(GreenLed, "f", 128, NULL, 1, &Handle[0]);
  xTaskCreate(YellowLed, "f", 128, NULL, 1, &Handle[1]);
  xTaskCreate(RedLed, "f", 128, NULL, 1, &Handle[2]);
  for (int i = 0; i < 3; i++) vTaskSuspend(Handle[i]);
}
void Sender(void *param) {
  DDRB &= B11111000;
  PORTB |= B00000111;
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
  int input;
  while (1) {
    for (int i = 0; i < 3; i++) {
      input = PINB & (1 << i);
      if (!input && millis() - buttonbounce[i] <= 300) input = 1;
      else buttonbounce[i] = millis();
      if (!input) xQueueSend(q, &i, xTicksToWait);
    }
    vTaskDelay(1);
  }
}
void Reciever(void *param) {
  int pin;
  bool taskPin_1 = false; //false=suspended true=active
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
  BaseType_t qStatus;
  while (1) {
    qStatus = xQueueReceive(q, &pin, xTicksToWait);
    if (qStatus  == pdPASS) {
      Serial.println(pin);
      if (pin == 0) {
        if(GreenDelay>millis()) GreenDelay+=3000;
        else GreenDelay = millis()+3000;
        vTaskResume(Handle[0]);
      }
      else if (pin == 1) {
        taskPin_1 = 1 - taskPin_1;
        if (taskPin_1) vTaskResume(Handle[1]);
        else {
          vTaskSuspend(Handle[1]);
          PORTD |= (1 << 5);
        }
      }
      else if (pin == 2) {
        vTaskResume(Handle[2]);
      }
    }
  }
}
void GreenLed(void *param) {
  while (1) {
    PORTD &= ~(1 << 4);
    if (millis() >= GreenDelay) {
      PORTD |= (1 << 4);
      vTaskSuspend(Handle[0]);
    }
  }
}
void YellowLed(void *param) {
  while (1) {
    PORTD ^= (1 << 5);
    vTaskDelay(50);
  }
}
void RedLed(void *param) {
  while (1) {
    for (int i = 0; i < 6; i++) {
      PORTD ^= (1 << 6);
      vTaskDelay(50);
    }
    vTaskSuspend(Handle[2]);
  }
}
void loop() {}

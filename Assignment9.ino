#include<Arduino_FreeRTOS.h>
#include "semphr.h"
SemaphoreHandle_t YellowSemphr, GreenSemphr;
TaskHandle_t taskhandle[3];
QueueHandle_t q;
unsigned long RedDelay, YellowDelay, GreenDelay, ButtonBounce[3];
void setup() {
  Serial.begin(9600);
  for (int i = 2; i <= 4; i++) pinMode(i, OUTPUT);
  YellowSemphr = xSemaphoreCreateCounting(2, 2);
  GreenSemphr = xSemaphoreCreateBinary();
  q = xQueueCreate(10, sizeof(int));
  xSemaphoreGive(GreenSemphr);
  xTaskCreate(Sender, "Se", 128, NULL, 1, NULL);
  xTaskCreate(Reciever, "Re", 128, NULL, 1, NULL);
  xTaskCreate(Red, "R", 128, NULL, 1, &taskhandle[2]);
  xTaskCreate(Yellow, "Y", 128, NULL, 1, &taskhandle[1]);
  xTaskCreate(Green, "G", 128, NULL, 1, &taskhandle[0]);
  for (int i = 0; i < 3; i++) vTaskSuspend(taskhandle[i]);
}
void Sender(void *param) {
  pinMode(12, INPUT); //<-- pull down
  pinMode(11, INPUT); //<-- pull up
  pinMode(10, INPUT_PULLUP);
  const TickType_t xTicksToWait = pdMS_TO_TICKS(0);
  int input;
  while (1) {
    for (int i = 0; i < 3; i++) {
      input = digitalRead(i + 10);
      if (i == 2) input = 1 - input;
      if (!input && millis() - ButtonBounce[i] <= 300) input = 1;
      else ButtonBounce[i] = millis();
      if (!input) xQueueSend(q, &i, xTicksToWait);
    }
  }
}
void Reciever(void *param) {
  int pin;
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
  BaseType_t qStatus;
  while (1) {
    qStatus = xQueueReceive(q, &pin, xTicksToWait);
    if (qStatus  == pdPASS) {
      if (pin == 2) {
        if (RedDelay > millis()) {
          RedDelay = 0;
          digitalWrite(4, LOW);
        }
        else {
          RedDelay = millis() + 3000;
          vTaskResume(taskhandle[pin]);
        }
      }
      if (pin == 1) {
        vTaskResume(taskhandle[pin]);
      }
      if (pin == 0) {
        if (GreenDelay > millis()) {
          GreenDelay = 0;
          digitalWrite(2, LOW);
        }
        else {
          GreenDelay = millis() + 3000;
          vTaskResume(taskhandle[pin]);
        }
      }
    }
  }
}
void Red(void *param) {
  bool taken = false;
  while (1) {
    if (!taken) {
      Serial.println("Red!!!");
      if (xSemaphoreTake(YellowSemphr, 0) == pdTRUE && xSemaphoreTake(GreenSemphr, 0) == pdTRUE) {
        digitalWrite(4, HIGH);
        taken = true;
        //  Serial.println("Red taken");
      }
      else {
        Serial.println("Red Blocked");
        vTaskSuspend(taskhandle[2]);
      }
    }
    if (taken) {
      if (millis() >= RedDelay) {
        taken = false;
        xSemaphoreGive(YellowSemphr);
        xSemaphoreGive(GreenSemphr);
        //   Serial.println("Red give");
        digitalWrite(4, LOW);
        vTaskSuspend(taskhandle[2]);
      }
    }
  }
}
void Yellow(void *param) {
  bool taken = false;
  while (1) {
    if (!taken) {
      Serial.println("Yellow!!!");
      if (xSemaphoreTake(YellowSemphr, 0) == pdTRUE) {
        if (xSemaphoreTake(YellowSemphr, 0) == pdTRUE) {
          taken = true;
          xSemaphoreGive(YellowSemphr);
          xSemaphoreGive(YellowSemphr);
        }
        else {
          xSemaphoreGive(YellowSemphr);
          Serial.println("Yellow Blocked");
          vTaskSuspend(taskhandle[1]);
        }
      }
      else {
        Serial.println("Yellow Blocked");
        vTaskSuspend(taskhandle[1]);
      }
    }
    if (taken) {
      for (int i = 0; i < 4; i++) {
        digitalWrite(3, 1 - digitalRead(3));
        vTaskDelay(50);
      }
      taken = false;
      vTaskSuspend(taskhandle[1]);
    }
  }
}
void Green(void *param) {
  bool taken = false;
  while (1) {
    if (!taken) {
      Serial.println("Green!!!");
      if (xSemaphoreTake(GreenSemphr, 0) == pdTRUE && xSemaphoreTake(YellowSemphr, 0) == pdTRUE) {
        //Serial.println("Green taken");
        xSemaphoreGive(GreenSemphr);
        digitalWrite(2, HIGH);
        taken = true;
      }
      else {
        Serial.println("Green Blocked");
        vTaskSuspend(taskhandle[0]);
      }
    }
    if (taken) {
      if (millis() >= GreenDelay) {
        // Serial.println("Green give");
        taken = false;
        digitalWrite(2, LOW);
        xSemaphoreGive(YellowSemphr);
        vTaskSuspend(taskhandle[0]);
      }
    }
  }
}
void loop() {}

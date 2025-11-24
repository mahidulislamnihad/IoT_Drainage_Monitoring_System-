#include "system_tasks.h"

#define BLYNK_TEMPLATE_ID   "TMPL6BpZ6Au5d"
#define BLYNK_TEMPLATE_NAME "Drain System"
#define BLYNK_AUTH_TOKEN    "Q2hgH5SPbR5CCoZW6jHwMXAwPQKmnuxN"

char ssid[] = "Sophie";
char pass[] = "11223344";
char auth[] = BLYNK_AUTH_TOKEN;

void setup() {
    Serial.begin(9600);
    delay(150);

    gDataMutex = xSemaphoreCreateMutex();
    if (!gDataMutex) {
        Serial.println("Mutex create failed");
        while (1) delay(500);
    }

    Blynk.begin(auth, ssid, pass);

    xTaskCreate(SensorTask,   "SensorTask",   4096, NULL, 1, NULL);
    xTaskCreate(DecisionTask, "DecisionTask", 6144, NULL, 1, NULL);
    xTaskCreate(BlynkTask,    "BlynkTask",    6144, NULL, 1, NULL);

    Serial.println("System working");
}

void loop() {
    // All logic handled by FreeRTOS tasks.
}

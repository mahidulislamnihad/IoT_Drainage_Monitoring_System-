#include "system_tasks.h"
#include <ESP32Servo.h>
#include <BlynkSimpleEsp32.h>

// Pins
static const uint8_t PIN_D1 = 32;
static const uint8_t PIN_D2 = 33;
static const uint8_t PIN_D3 = 13;
static const uint8_t PIN_RAIN = 26;
static const uint8_t PIN_GAS = 34;
static const uint8_t PIN_FLOW = 35;

static const uint8_t PIN_GATE1 = 27;
static const uint8_t PIN_GATE2 = 14;

// Thresholds
static const int WATER_LIMIT = 30;
static const int GAS_LIMIT   = 500;
static const float FLOW_LIMIT = 0.05f;

SensorData gData;
SemaphoreHandle_t gDataMutex;

static Servo gate1;
static Servo gate2;

float simulateFlow() {
    int x = random(1, 10);
    return x / 100.0f;
}

void SensorTask(void *pv) {
    pinMode(PIN_D1, INPUT);
    pinMode(PIN_D2, INPUT);
    pinMode(PIN_D3, INPUT);
    pinMode(PIN_RAIN, INPUT);
    pinMode(PIN_GAS, INPUT);
    pinMode(PIN_FLOW, INPUT);

    for (;;) {
        int d1 = analogRead(PIN_D1);
        int d2 = analogRead(PIN_D2);
        int d3 = analogRead(PIN_D3);
        int rain = analogRead(PIN_RAIN);
        int gas = analogRead(PIN_GAS);
        float flow = simulateFlow();

        if (xSemaphoreTake(gDataMutex, 20)) {
            gData.drain1 = d1;
            gData.drain2 = d2;
            gData.drain3 = d3;
            gData.rain = rain;
            gData.gas = gas;
            gData.flow = flow;
            xSemaphoreGive(gDataMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void DecisionTask(void *pv) {
    gate1.attach(PIN_GATE1);
    gate2.attach(PIN_GATE2);

    for (;;) {
        int d1, d2, d3, rain, gas;
        float flow;
        bool g1, g2;

        if (!xSemaphoreTake(gDataMutex, 50)) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        d1 = gData.drain1;
        d2 = gData.drain2;
        d3 = gData.drain3;
        rain = gData.rain;
        gas = gData.gas;
        flow = gData.flow;
        g1 = gData.gate1Closed;
        g2 = gData.gate2Closed;

        xSemaphoreGive(gDataMutex);

        // Decision logic (same as your original logic but humanized)

        if (d3 > WATER_LIMIT && d2 < WATER_LIMIT) {
            if (!g2) {
                gate2.write(0);
                g2 = true;
                if (xSemaphoreTake(gDataMutex, 20)) {
                    gData.evtDrain3 = true;
                    gData.gate2Closed = true;
                    xSemaphoreGive(gDataMutex);
                }
            }
        } else {
            gate2.write(90);
            if (xSemaphoreTake(gDataMutex, 20)) {
                gData.gate2Closed = false;
                xSemaphoreGive(gDataMutex);
            }
        }

        if (d2 > WATER_LIMIT && d1 < WATER_LIMIT) {
            if (!g1) {
                gate1.write(0);
                g1 = true;
                if (xSemaphoreTake(gDataMutex, 20)) {
                    gData.evtDrain2 = true;
                    gData.gate1Closed = true;
                    xSemaphoreGive(gDataMutex);
                }
            }
        } else {
            gate1.write(90);
            if (xSemaphoreTake(gDataMutex, 20)) {
                gData.gate1Closed = false;
                xSemaphoreGive(gDataMutex);
            }
        }

        if (d1 > WATER_LIMIT) {
            gate1.write(90);
            gate2.write(90);

            if (xSemaphoreTake(gDataMutex, 20)) {
                gData.evtDrain1 = true;
                gData.gate1Closed = false;
                gData.gate2Closed = false;
                xSemaphoreGive(gDataMutex);
            }
        }

        if (rain > 500 && flow < FLOW_LIMIT) {
            if (xSemaphoreTake(gDataMutex, 20)) {
                gData.evtLowFlow = true;
                xSemaphoreGive(gDataMutex);
            }
        }

        if (gas > GAS_LIMIT) {
            if (xSemaphoreTake(gDataMutex, 20)) {
                gData.evtHighGas = true;
                xSemaphoreGive(gDataMutex);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void BlynkTask(void *pv) {
    TickType_t last = xTaskGetTickCount();

    for (;;) {
        Blynk.run();

        if (xTaskGetTickCount() - last >= pdMS_TO_TICKS(1000)) {
            last = xTaskGetTickCount();

            int d1, d2, d3, rain, gas;
            float flow;
            bool e1, e2, e3, lf, hg;
            bool g1, g2;

            if (xSemaphoreTake(gDataMutex, 50)) {
                d1 = gData.drain1;
                d2 = gData.drain2;
                d3 = gData.drain3;
                rain = gData.rain;
                gas = gData.gas;
                flow = gData.flow;

                e3 = gData.evtDrain3;
                e2 = gData.evtDrain2;
                e1 = gData.evtDrain1;
                lf = gData.evtLowFlow;
                hg = gData.evtHighGas;

                g1 = gData.gate1Closed;
                g2 = gData.gate2Closed;

                gData.evtDrain3 = false;
                gData.evtDrain2 = false;
                gData.evtDrain1 = false;
                gData.evtLowFlow = false;
                gData.evtHighGas = false;

                xSemaphoreGive(gDataMutex);
            }

            Blynk.virtualWrite(V1, d1);
            Blynk.virtualWrite(V2, d2);
            Blynk.virtualWrite(V3, d3);
            Blynk.virtualWrite(V4, rain);
            Blynk.virtualWrite(V5, gas);
            Blynk.virtualWrite(V6, flow);
            Blynk.virtualWrite(V7, g1 ? "Closed" : "Open");
            Blynk.virtualWrite(V8, g2 ? "Closed" : "Open");

            if (e3) Blynk.logEvent("drain3_overflow", "Gate2 Closed");
            if (e2) Blynk.logEvent("drain2_overflow", "Gate1 Closed");
            if (e1) Blynk.logEvent("drain1_overflow", "All Gates Open");
            if (lf) Blynk.logEvent("low_flow", "Flow low during rain");
            if (hg) Blynk.logEvent("high_gas", "Gas level high");
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

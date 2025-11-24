#ifndef SYSTEM_TASKS_H
#define SYSTEM_TASKS_H

#include <Arduino.h>
#include <semphr.h>

struct SensorData {
    int drain1, drain2, drain3;
    int rain, gas;
    float flow;

    bool evtDrain3, evtDrain2, evtDrain1;
    bool evtLowFlow, evtHighGas;

    bool gate1Closed;
    bool gate2Closed;
};

extern SensorData gData;
extern SemaphoreHandle_t gDataMutex;

void SensorTask(void *pv);
void DecisionTask(void *pv);
void BlynkTask(void *pv);

float simulateFlow();

#endif

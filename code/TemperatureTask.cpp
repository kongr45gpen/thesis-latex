#include <Logger.hpp>
#include <ServicePool.hpp>
#include "Tasks/TemperatureTask.hpp"

TemperatureTask::TemperatureTask(Parameter<float> &parameter,
                                 CallbackParameter<SystemParameters::TemperatureStatus> &statusParameter,
                                 uint8_t sensorI2c, PIO_PIN sensorPin, PIO_PIN buttonPin)
        : parameter(parameter), statusParameter(statusParameter), mcp9808(sensorI2c), sensorPin(sensorPin),
          buttonPin(buttonPin) {
    PIO_PinWrite(sensorPin, true);
}

void TemperatureTask::operator()() {
    taskHandle = xTaskGetCurrentTaskHandle();

    while (true) {
        wasSuspended = false;

        vTaskDelay(130);

        float temperature = 0;
        bool status = mcp9808.isIDok();

        if (status) {
            status = mcp9808.getTemp(temperature);
        }

        if (!status && wasSuspended) {
            // I2C timeout, but the task was suspended so the I2C peripheral didn't get enough information.
            // Retry again after a delay
            continue;
        }

        if (status) {
            statusParameter.setValue(SystemParameters::TemperatureStatus::Nominal);
        } else {
            statusParameter.setValue(SystemParameters::TemperatureStatus::Timeout);
        }

        if (!PIO_PinRead(buttonPin)) {
            temperature += 80;
        }

        if (bogusTemperatureTime != 0) {
            temperature += bellFunction();
        }

        // TODO: Set parameter on timeout?
        parameter.setValue(temperature);

        LOG_DEBUG << "T [" << pcTaskGetName(nullptr) << "]: " << parameter.getValue();
    }
}

void TemperatureTask::setOutput(bool output) {
    PIO_PinWrite(sensorPin, output);

    if (taskHandle != nullptr) {
        if (output) {
            wasSuspended = true;
            vTaskResume(taskHandle);
        } else {
            vTaskSuspend(taskHandle);
        }
    }
}

void TemperatureTask::restart() {
    if (statusParameter.getValue() != SystemParameters::TemperatureStatus::Disabled) {
        setOutput(false);
        vTaskDelay(100);
    }

    // Second check, in case the parameter was somehow set during the delay
    if (statusParameter.getValue() != SystemParameters::TemperatureStatus::Disabled) {
        setOutput(true);
    }
}

float TemperatureTask::bellFunction() {
    const float MaxBogusTemperature = 60;
    const float BogusDuration = 7000;

    // Apply error for t in (0, 2*BogusDuration)
    float t = xTaskGetTickCount() - bogusTemperatureTime;

    // Apply error for σt in (-BogusDuration, BogusDuration)
    float shiftedT = t - BogusDuration;

    if (abs(shiftedT) >= BogusDuration) {
        return 0;
    } else {
        return MaxBogusTemperature * expf(1) *
               expf(powf(BogusDuration, 2) / (powf(shiftedT, 2) - powf(BogusDuration, 2)));
    }
}

void TemperatureTask::addBogusTemperature(int32_t shift) {
    bogusTemperatureTime = xTaskGetTickCount() + shift;
}

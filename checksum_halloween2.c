#include "main.h"
#include "usart.h"
#include "gpio.h"

#define DEVICE_ID 0x01
#define UART_BUFFER_SIZE 100

// UART communication buffer
uint8_t uart_rx_buffer[UART_BUFFER_SIZE];

// Calculate checksum as XOR of all bytes
uint8_t CalculateChecksum(uint8_t *data, uint8_t length) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

// Function to send data frame over UART
void SendDataFrame(uint8_t dataType, uint16_t data) {
    uint8_t frame[5];
    frame[0] = DEVICE_ID;
    frame[1] = dataType;
    frame[2] = (data >> 8) & 0xFF;   // High byte of data
    frame[3] = data & 0xFF;          // Low byte of data
    frame[4] = CalculateChecksum(frame, 4); // Checksum
    HAL_UART_Transmit(&huart1, frame, sizeof(frame), HAL_MAX_DELAY);
}

// Error codes as per manual
typedef enum {
    ERR_NORMAL = 0,
    ERR_RESERVED = 1,
    ERR_BRAKE = 2,
    ERR_PAS_SENSOR = 3,
    ERR_CRUISE_6KMH = 4,
    ERR_REALTIME_CRUISE = 5,
    ERR_LOW_BATTERY = 6,
    ERR_MOTOR_FAILURE = 7,
    ERR_THROTTLE_FAILURE = 8,
    ERR_CONTROLLER_FAILURE = 9,
    ERR_COMM_RECEIVE_FAILURE = 10,
    ERR_COMM_SEND_FAILURE = 11,
    ERR_BMS_FAILURE = 12,
    ERR_LIGHT_FAILURE = 13
} ErrorCode;

// Function to send error code
void UpdateErrorCode(ErrorCode errorCode) {
    SendDataFrame(0x03, errorCode); // 0x03 for error code
}

// Send motor speed
void UpdateMotorSpeed(uint16_t speed) {
    SendDataFrame(0x01, speed); // 0x01 for motor speed
}

// Send battery level
void UpdateBatteryLevel(uint8_t batteryLevel) {
    SendDataFrame(0x02, batteryLevel); // 0x02 for battery level
}

// Send PAS level
void UpdatePASLevel(uint8_t pasLevel) {
    SendDataFrame(0x04, pasLevel); // 0x04 for PAS level
}

// Send distance (ODO and Trip)
void UpdateDistance(uint16_t odo, uint16_t trip) {
    SendDataFrame(0x05, odo); // 0x05 for ODO distance
    SendDataFrame(0x05, trip); // 0x05 for Trip distance
}

// Send voltage and current
void UpdateVoltageCurrent(uint16_t voltage, uint16_t current) {
    SendDataFrame(0x06, voltage); // 0x06 for Voltage
    SendDataFrame(0x06, current); // 0x06 for Current
}

// Send remaining mileage
void UpdateRemainingMileage(uint16_t remainingMileage) {
    SendDataFrame(0x07, remainingMileage); // 0x07 for remaining mileage
}

// Parameter setting functions (P00 to P19)
void SetParameter(uint8_t parameterID, uint16_t value) {
    SendDataFrame(parameterID, value);
}

// Functions to set each parameter (P00-P19) as per manual
void SetFactoryReset() { SetParameter(0x00, 10); }
void SetBackgroundLuminance(uint8_t level) { SetParameter(0x01, level); }
void SetMileageUnit(uint8_t unit) { SetParameter(0x02, unit); }
void SetVoltageGrade(uint8_t grade) { SetParameter(0x03, grade); }
void SetSleepTime(uint8_t time) { SetParameter(0x04, time); }
void SetPASGrades(uint8_t mode) { SetParameter(0x05, mode); }
void SetWheelSize(float size) { SetParameter(0x06, (uint16_t)(size * 10)); }
void SetSpeedMagnet(uint8_t magnet) { SetParameter(0x07, magnet); }
void SetSpeedLimit(uint8_t limit) { SetParameter(0x08, limit); }
void SetStartMode(uint8_t mode) { SetParameter(0x09, mode); }
void SetDriveMode(uint8_t mode) { SetParameter(0x0A, mode); }
void SetPASSensitivity(uint8_t sensitivity) { SetParameter(0x0B, sensitivity); }
void SetPASStartStrength(uint8_t strength) { SetParameter(0x0C, strength); }
void SetPASMagnetType(uint8_t type) { SetParameter(0x0D, type); }
void SetCurrentLimit(uint8_t current) { SetParameter(0x0E, current); }
void ResetODO() { SetParameter(0x10, 0); }
void SetAutoCruiseOption(uint8_t option) { SetParameter(0x11, option); }
void SetThrottleLevelOption(uint8_t option) { SetParameter(0x12, option); }
void SetCruiseThrottleDefinition(uint8_t option) { SetParameter(0x13, option); }

// Main program
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    uint16_t motorSpeed = 25;
    uint8_t batteryLevel = 80;
    uint8_t pasLevel = 3;
    uint16_t odoDistance = 1500;     // Total distance in km
    uint16_t tripDistance = 30;      // Trip distance in km
    uint16_t voltage = 48;           // Voltage in volts
    uint16_t current = 10;           // Current in amps
    uint16_t remainingMileage = 100; // Remaining mileage in km
    ErrorCode errorCode = ERR_NORMAL;

    // Parameter settings examples
    SetBackgroundLuminance(2);    // P01: Background luminance
    SetMileageUnit(0);            // P02: 0 for KM, 1 for Mile
    SetVoltageGrade(48);          // P03: Voltage level (24V, 36V, 48V)
    SetSleepTime(10);             // P04: Sleep time in minutes
    SetPASGrades(1);              // P05: PAS grades mode

    while (1) {
        // Periodically send each parameter to LCD
        UpdateMotorSpeed(motorSpeed);
        HAL_Delay(500);

        UpdateBatteryLevel(batteryLevel);
        HAL_Delay(500);

        UpdateErrorCode(errorCode);
        HAL_Delay(500);

        UpdatePASLevel(pasLevel);
        HAL_Delay(500);

        UpdateDistance(odoDistance, tripDistance);
        HAL_Delay(500);

        UpdateVoltageCurrent(voltage, current);
        HAL_Delay(500);

        UpdateRemainingMileage(remainingMileage);
        HAL_Delay(500);

        // Simulate changes in motor speed, battery level, PAS level, etc.
        motorSpeed = (motorSpeed + 1) % 60;
        batteryLevel = (batteryLevel - 1) % 100;
        pasLevel = (pasLevel % 5) + 1;

        // Simulate an error (example: throttle failure)
        if (motorSpeed > 50) {
            errorCode = ERR_THROTTLE_FAILURE;
        } else {
            errorCode = ERR_NORMAL;
        }
    }
}

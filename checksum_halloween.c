#include "main.h"
#include "usart.h"
#include "gpio.h"

#define DEVICE_ID 0x01
#define UART_BUFFER_SIZE 100

UART_HandleTypeDef huart1;

// Buffer for receiving UART data
char uart_rx_buffer[UART_BUFFER_SIZE];

// Function to calculate checksum (XOR of all bytes)
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
    frame[2] = (data >> 8) & 0xFF;  // High byte of data
    frame[3] = data & 0xFF;         // Low byte of data
    frame[4] = CalculateChecksum(frame, 4);  // Checksum

    HAL_UART_Transmit(&huart1, frame, sizeof(frame), HAL_MAX_DELAY);
}

// Functions to update various parameters
void UpdateMotorSpeed(uint16_t speed) { SendDataFrame(0x01, speed); }
void UpdateBatteryLevel(uint8_t level) { SendDataFrame(0x02, level); }
void UpdatePASLevel(uint8_t level) { SendDataFrame(0x04, level); }
void UpdateDistance(uint16_t odo, uint16_t trip) { 
    SendDataFrame(0x05, odo); 
    SendDataFrame(0x05, trip); 
}
void UpdateVoltageCurrent(uint16_t voltage, uint16_t current) { 
    SendDataFrame(0x06, voltage); 
    SendDataFrame(0x06, current); 
}
void UpdateRemainingMileage(uint16_t mileage) { SendDataFrame(0x07, mileage); }

// Error update function
void UpdateErrorCode(uint8_t errorCode) { SendDataFrame(0x03, errorCode); }

// Setting commands based on manual specifications
void SetParameter(uint8_t parameter, uint8_t value) {
    uint8_t frame[4];
    frame[0] = DEVICE_ID;
    frame[1] = parameter;
    frame[2] = value;
    frame[3] = CalculateChecksum(frame, 3);  // Checksum

    HAL_UART_Transmit(&huart1, frame, sizeof(frame), HAL_MAX_DELAY);
}

// Update parameter settings P00 - P19
void UpdateParameterSettings() {
    SetParameter(0x00, 10);  // P00: Restore Factory Settings
    SetParameter(0x01, 3);   // P01: Background luminance (3 - brightest)
    SetParameter(0x02, 0);   // P02: Unit of mileage (0 - KM)
    SetParameter(0x03, 48);  // P03: Voltage grade (48V)
    SetParameter(0x04, 5);   // P04: Sleep time (5 min)
    SetParameter(0x05, 1);   // P05: PAS grades mode (1 - 5 grades mode)
    SetParameter(0x06, 29);  // P06: Wheel size (29 inches)
    SetParameter(0x07, 32);  // P07: Speed measuring magnet count
    SetParameter(0x08, 25);  // P08: Speed limit (25 km/h)
    SetParameter(0x09, 0);   // P09: Zero start
    SetParameter(0x0A, 2);   // P10: Driving mode (PAS & Throttle)
    SetParameter(0x0B, 10);  // P11: PAS sensitivity
    SetParameter(0x0C, 3);   // P12: PAS start strength
    SetParameter(0x0D, 8);   // P13: PAS magnet type (8 magnets)
    SetParameter(0x0E, 15);  // P14: Controller current limit (15A)
    SetParameter(0x10, 1);   // P17: Auto Cruise Option (1 - Enabled)
    SetParameter(0x11, 1);   // P18: Throttle Level Option
    SetParameter(0x12, 0);   // P19: 6km/h Cruise Throttle Definition
}

// Main program
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    // Initial settings for e-bike parameters
    uint16_t motorSpeed = 0;
    uint8_t batteryLevel = 100;
    uint8_t pasLevel = 3;
    uint16_t odoDistance = 1200;  // Total distance in km
    uint16_t tripDistance = 25;   // Trip distance in km
    uint16_t voltage = 48;        // Voltage in volts
    uint16_t current = 10;        // Current in amps
    uint16_t remainingMileage = 80;  // Remaining mileage in km
    uint8_t errorCode = 0;        // No error initially

    // Set initial parameters
    UpdateParameterSettings();

    while (1) {
        // Periodic updates for each parameter
        UpdateMotorSpeed(motorSpeed);
        HAL_Delay(500);

        UpdateBatteryLevel(batteryLevel);
        HAL_Delay(500);

        UpdateErrorCode(errorCode);  // Send any active error code
        HAL_Delay(500);

        UpdatePASLevel(pasLevel);
        HAL_Delay(500);

        UpdateDistance(odoDistance, tripDistance);
        HAL_Delay(500);

        UpdateVoltageCurrent(voltage, current);
        HAL_Delay(500);

        UpdateRemainingMileage(remainingMileage);
        HAL_Delay(500);

        // Simulate changes for demonstration
        motorSpeed = (motorSpeed + 1) % 60;
        batteryLevel = (batteryLevel > 10) ? batteryLevel - 1 : 100;
        pasLevel = (pasLevel % 5) + 1;  // Cycle through PAS levels
    }
}

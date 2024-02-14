/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page:
 * https://arduinogetstarted.com/tutorials/arduino-uno-r4-led-matrix-displays-number-character
 */

uint8_t fonts[][8] = {
    {
        // 0
        0b000,
        0b111,
        0b101,
        0b101,
        0b101,
        0b111,
        0b000,
        0b000,
    },
    {
        // 1
        0b000,
        0b010,
        0b110,
        0b010,
        0b010,
        0b111,
        0b000,
        0b000,
    },
    {
        // 2
        0b000,
        0b111,
        0b001,
        0b111,
        0b100,
        0b111,
        0b000,
        0b000,
    },
    {
        // 3
        0b000,
        0b111,
        0b001,
        0b111,
        0b001,
        0b111,
        0b000,
        0b000,
    },
    {
        // 4
        0b000,
        0b101,
        0b101,
        0b111,
        0b001,
        0b001,
        0b000,
        0b000,
    },
    {
        // 5
        0b000,
        0b111,
        0b100,
        0b111,
        0b001,
        0b111,
        0b000,
        0b000,
    },
    {
        // 6
        0b000,
        0b111,
        0b100,
        0b111,
        0b101,
        0b111,
        0b000,
        0b000,
    },
    {
        // 7
        0b000,
        0b111,
        0b001,
        0b001,
        0b001,
        0b001,
        0b000,
        0b000,
    },
    {
        // 8
        0b000,
        0b111,
        0b101,
        0b111,
        0b101,
        0b111,
        0b000,
        0b000,
    },
    {
        // 9
        0b000,
        0b111,
        0b101,
        0b111,
        0b001,
        0b111,
        0b000,
        0b000,
    },
    {
        //  °
        0b000,
        0b011,
        0b011,
        0b000,
        0b000,
        0b000,
        0b000,
        0b000,
    },
    {
        // C
        0b000,
        0b111,
        0b100,
        0b100,
        0b100,
        0b111,
        0b000,
        0b000,
    },
};

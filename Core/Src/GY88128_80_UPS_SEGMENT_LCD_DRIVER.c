/**
 * LCD Driver for GY88128-80 UPS Display with HT1621
 * 
 * This driver provides functions to control the LCD segments and digits.
 * Include "GY88128_80_UPS_SEGMENT_LCD_DRIVER.h" in your project and call lcd_init() after HAL initialization.
 * Adjust HT1621 pin defines in lcd_driver.h to match your hardware.
 */

#include "GY88128_80_UPS_SEGMENT_LCD_DRIVER.h"
#include <stdint.h>
#include <stdbool.h>

// HT1621 RAM buffer (16 bytes for 128 bits)
static uint8_t ht1621_ram[16] = {0};

// Segment mappings: {address, bit} for HT1621 RAM
// Calculated as: seg = pin-1, com = com_num-1, addr = seg/2, bit = (seg%2)*4 + com
const uint8_t segment_map[34][2] = {
    {0,0}, // index 0 unused
    {6,3}, // S1: pin13 com4
    {6,2}, // S2: pin13 com3
    {4,7}, // S3: pin10 com4
    {11,7}, // S4: pin24 com4
    {6,0}, // S5: pin13 com1
    {5,4}, // S6: pin12 com1
    {6,1}, // S7: pin13 com2
    {10,3}, // S8: pin21 com4
    {0xFF,0xFF}, // S9: pin23 com4 (unused)
    {10,2}, // S10: pin21 com3
    {10,4}, // S11: pin22 com1
    {8,7}, // S12: pin18 com4 (confirmed)
    {10,0}, // S13: pin21 com1
    {9,4}, // S14: pin20 com1
    {10,1}, // S15: pin21 com2
    {1,5}, // S16: pin4 com2
    {1,4}, // S17: pin4 com1
    {2,0}, // S18: pin5 com1
    {2,1}, // S19: pin5 com2
    {2,2}, // S20: pin5 com3
    {2,3}, // S21: pin5 com4
    {0,1}, // S22: pin1 com2
    {0xFF,0xFF}, // S23: Not used - IRQ pin controls backlight via hardware command
    {11,6}, // S24: pin24 com3
    {11,5}, // S25: pin24 com2
    {1,6}, // S26: pin4 com3
    {0,6}, // S27: pin2 com3
    {0,5}, // S28: pin2 com2
    {0,4}, // S29: pin2 com1
    {1,0}, // S30: pin3 com1
    {1,1}, // S31: pin3 com2
    {1,2}, // S32: pin3 com3
    {1,3}  // S33: pin3 com4
};

// Digit segment mappings: digit[6][7] = {A,B,C,D,E,F,G} {address, bit}
const uint8_t digit_map[6][7][2] = {
    // Digit 1
    {{2,4}, {3,0}, {3,2}, {3,3}, {2,6}, {2,5}, {3,1}},
    // Digit 2
    {{3,4}, {4,0}, {4,2}, {4,3}, {3,6}, {3,5}, {4,1}},
    // Digit 3
    {{4,4}, {5,0}, {5,2}, {5,3}, {4,6}, {4,5}, {5,1}},
    // Digit 4
    {{6,4}, {7,0}, {7,2}, {7,3}, {6,6}, {6,5}, {7,1}},
    // Digit 5
    {{7,4}, {8,0}, {8,2}, {8,3}, {7,6}, {7,5}, {8,1}},
    // Digit 6
    {{8,4}, {9,0}, {9,2}, {9,3}, {8,6}, {8,5}, {9,1}}
};

static bool resolve_segment_entry(uint8_t seg, uint8_t *addr_out, uint8_t *bit_out);

// 7-segment digit patterns: A B C D E F G
static const bool digit_patterns[10][7] = {
    {1,1,1,1,1,1,0}, // 0
    {0,1,1,0,0,0,0}, // 1
    {1,1,0,1,1,0,1}, // 2
    {1,1,1,1,0,0,1}, // 3
    {0,1,1,0,0,1,1}, // 4
    {1,0,1,1,0,1,1}, // 5
    {1,0,1,1,1,1,1}, // 6
    {1,1,1,0,0,0,0}, // 7
    {1,1,1,1,1,1,1}, // 8
    {1,1,1,1,0,1,1}  // 9
};

// HT1621 functions (static)
#define HT1621_CMD_SYS_DIS         0x00
#define HT1621_CMD_SYS_EN          0x01
#define HT1621_CMD_LCD_OFF         0x02
#define HT1621_CMD_LCD_ON          0x03
#define HT1621_CMD_WDT_DIS         0x05
#define HT1621_CMD_TIMER_EN        0x07
#define HT1621_CMD_RC256K          0x18
#define HT1621_CMD_BIAS13_COM4     0x29  // Bias 1/3, Duty 1/4 (4 commons)

// Time base frequency selection for IRQ output
#define HT1621_CMD_TONE_2KHZ       0x80  // 2kHz tone output
#define HT1621_CMD_TONE_4KHZ       0xC0  // 4kHz tone output
#define HT1621_CMD_F1              0x40  // Time base: 2Hz
#define HT1621_CMD_F2              0x42  // Time base: 4Hz
#define HT1621_CMD_F4              0x44  // Time base: 8Hz
#define HT1621_CMD_F8              0x46  // Time base: 16Hz
#define HT1621_CMD_F16             0x48  // Time base: 32Hz
#define HT1621_CMD_F32             0x4A  // Time base: 64Hz (closest to 50Hz)
#define HT1621_CMD_F64             0x4C  // Time base: 128Hz
#define HT1621_CMD_F128            0x4E  // Time base: 256Hz

// IRQ output control
#define HT1621_CMD_IRQ_EN          0x88  // Enable IRQ output
#define HT1621_CMD_IRQ_DIS         0x80  // Disable IRQ output (default tone mode)

static void ht1621_send_bit(bool bit) {
    HAL_GPIO_WritePin(HT1621_DATA_PIN_GPIO_Port, HT1621_DATA_PIN, bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HT1621_WR_PIN_GPIO_Port, HT1621_WR_PIN, GPIO_PIN_RESET);
    // Short delay for HT1621 timing (~100ns at 64MHz)
    for (volatile int i = 0; i < 10; i++);
    HAL_GPIO_WritePin(HT1621_WR_PIN_GPIO_Port, HT1621_WR_PIN, GPIO_PIN_SET);
}

static void ht1621_send_command(uint8_t cmd) {
    HAL_GPIO_WritePin(HT1621_CS_PIN_GPIO_Port, HT1621_CS_PIN, GPIO_PIN_RESET);

    // Command mode prefix: 100 (MSB first)
    ht1621_send_bit(1);
    ht1621_send_bit(0);
    ht1621_send_bit(0);

    // 8-bit command word (MSB first)
    for (int i = 7; i >= 0; i--) {
        ht1621_send_bit((cmd >> i) & 1);
    }

    // Trailing 0 per HT1621 command protocol
    ht1621_send_bit(0);

    HAL_GPIO_WritePin(HT1621_CS_PIN_GPIO_Port, HT1621_CS_PIN, GPIO_PIN_SET);
}

static void ht1621_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = HT1621_CS_PIN;
    HAL_GPIO_Init(HT1621_CS_PIN_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = HT1621_WR_PIN;
    HAL_GPIO_Init(HT1621_WR_PIN_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = HT1621_DATA_PIN;
    HAL_GPIO_Init(HT1621_DATA_PIN_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(HT1621_CS_PIN_GPIO_Port, HT1621_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HT1621_WR_PIN_GPIO_Port, HT1621_WR_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HT1621_DATA_PIN_GPIO_Port, HT1621_DATA_PIN, GPIO_PIN_RESET);

    // HT1621 initialization sequence per AN0468E application note
    // 1. Disable system before configuration
    ht1621_send_command(HT1621_CMD_SYS_DIS);     // 0x00: System disable
    
    // 2. Configure oscillator source
    ht1621_send_command(HT1621_CMD_RC256K);      // 0x18: Internal RC 256kHz oscillator
    
    // 3. Configure bias and duty cycle
    ht1621_send_command(HT1621_CMD_BIAS13_COM4); // 0x29: 1/3 bias, 1/4 duty (4 COM)
    
    // 4. Enable system - MUST be done before LCD_ON
    ht1621_send_command(HT1621_CMD_SYS_EN);      // 0x01: System enable
    
    // 5. Turn on LCD bias generator
    ht1621_send_command(HT1621_CMD_LCD_ON);      // 0x03: LCD on
    
    // 6. Configure timer/IRQ (optional features, done after basic LCD setup)
    ht1621_send_command(HT1621_CMD_WDT_DIS);     // 0x05: Disable watchdog
    ht1621_send_command(HT1621_CMD_TIMER_EN);    // 0x07: Enable time base output
    // ht1621_send_command(HT1621_CMD_F32);      // 0x4A: Set time base to 64Hz
    ht1621_send_command(HT1621_CMD_F128);        // 0x4B: Set time base to 128Hz
    ht1621_send_command(HT1621_CMD_IRQ_EN);      // 0x88: Enable IRQ output pin
}

static void ht1621_update() {
    // Write all RAM to HT1621 using sequential burst starting at address 0
    HAL_GPIO_WritePin(HT1621_CS_PIN_GPIO_Port, HT1621_CS_PIN, GPIO_PIN_RESET);

    // Write mode prefix: 101
    ht1621_send_bit(1);
    ht1621_send_bit(0);
    ht1621_send_bit(1);

    // Start address 0x00
    for (int i = 5; i >= 0; i--) {
        ht1621_send_bit((0 >> i) & 1);
    }

    // Stream 4-bit nibbles for entire display memory (low nibble first per byte)
    for (uint8_t i = 0; i < sizeof(ht1621_ram); i++) {
        uint8_t value = ht1621_ram[i];
        for (int nibble = 0; nibble < 2; nibble++) {
            uint8_t data = (nibble == 0) ? (value & 0x0F) : (value >> 4);
            for (int bit = 3; bit >= 0; bit--) {
                ht1621_send_bit((data >> bit) & 1);
            }
        }
    }

    HAL_GPIO_WritePin(HT1621_CS_PIN_GPIO_Port, HT1621_CS_PIN, GPIO_PIN_SET);
}

// Initialize LCD (HT1621)
void lcd_init() {
    ht1621_init();
}

// Clear all segments and digits
void clear_all() {
    // Clear all segments
    for (uint8_t seg = 1; seg <= 33; seg++) {
        set_segment(seg, false);
    }
    // Clear all digits
    for (uint8_t d = 1; d <= 6; d++) {
        for (uint8_t s = 0; s < 7; s++) {
            set_digit_segment(d, s, false);
        }
    }
    ht1621_update();
}

// Turn on all unimplemented/unused segments (active-low: segments illuminate when bit is cleared)
void turn_on_unimplemented_segments(void) {
    // List of segments not used in any display functions (backlight handled separately)
    const uint8_t unused_segments[] = {9, 10, 11, 15, 26, 27};
    
    // Turn on each unused segment one by one with delay (active-low: false = on)
    for (uint8_t i = 0; i < sizeof(unused_segments); i++) {
        set_segment(unused_segments[i], false);
        ht1621_update();
        HAL_GPIO_TogglePin(SYSTEM_LED_PIN_GPIO_Port, SYSTEM_LED_PIN);
        HAL_Delay(250); // 250ms delay
        HAL_GPIO_TogglePin(SYSTEM_LED_PIN_GPIO_Port, SYSTEM_LED_PIN);
        HAL_Delay(250); // 250ms delay
    }
    
    // Keep all unused segments on (active-low)
    ht1621_update();
}

// Turn on all segments
void turn_on_all_segments(void) {
    // Turn on all discrete segments/icons
    for (uint8_t seg = 1; seg <= 33; seg++) {
        set_segment(seg, true);
    }

    // Turn on all digit segments (A-G for digits 1-6)
    for (uint8_t digit = 1; digit <= 6; digit++) {
        for (uint8_t seg = 0; seg < 7; seg++) {
            set_digit_segment(digit, seg, true);
        }
    }

    ht1621_update();
}

bool backlight_is_available(void) {
    // IRQ-based backlight is always available (hardware feature)
    return true;
}

bool backlight_set(bool on) {
    // Use HT1621 IRQ pin command to control backlight
    // IRQ pin outputs time base frequency when enabled (64Hz square wave)
    if (on) {
        ht1621_send_command(HT1621_CMD_IRQ_EN);  // 0x88: Enable IRQ output (64Hz pulsing)
    } else {
        ht1621_send_command(HT1621_CMD_IRQ_DIS); // 0x80: Disable IRQ output
    }
    return true;
}

bool backlight_on(void) {
    return backlight_set(true);
}

bool backlight_off(void) {
    return backlight_set(false);
}

// Sequentially scan each segment/digit to help identify wiring; use backlight_* helpers for S23 once mapped
void scan_all_segments(uint16_t dwell_ms) {
    if (dwell_ms == 0) {
        dwell_ms = 250;
    }

    clear_all();

    // Helper lambda-like macros to pulse the status LED
#define PULSE_LED()                                \
    do {                                           \
        HAL_GPIO_TogglePin(SYSTEM_LED_PIN_GPIO_Port, SYSTEM_LED_PIN); \
        HAL_Delay(dwell_ms);                       \
        HAL_GPIO_TogglePin(SYSTEM_LED_PIN_GPIO_Port, SYSTEM_LED_PIN); \
    } while (0)

    // Scan discrete segments/icons
    for (uint8_t seg = 1; seg <= 33; seg++) {
        set_segment(seg, true);
        ht1621_update();
        PULSE_LED();
        set_segment(seg, false);
        ht1621_update();
        HAL_Delay(50);
    }

    // Scan seven-segment digits
    for (uint8_t digit = 1; digit <= 6; digit++) {
        for (uint8_t seg = 0; seg < 7; seg++) {
            set_digit_segment(digit, seg, true);
            ht1621_update();
            PULSE_LED();
            set_digit_segment(digit, seg, false);
            ht1621_update();
            HAL_Delay(50);
        }
    }

#undef PULSE_LED

    clear_all();
}

static bool resolve_segment_entry(uint8_t seg, uint8_t *addr_out, uint8_t *bit_out) {
    if (seg < 1 || seg >= (sizeof(segment_map) / sizeof(segment_map[0]))) {
        return false;
    }

    uint8_t addr = segment_map[seg][0];
    uint8_t bit = segment_map[seg][1];
    if (addr == 0xFF || bit >= 8 || addr >= sizeof(ht1621_ram)) {
        return false;
    }

    if (addr_out) {
        *addr_out = addr;
    }
    if (bit_out) {
        *bit_out = bit;
    }
    return true;
}

// Set segment on/off
void set_segment(uint8_t seg, bool on) {
    uint8_t addr;
    uint8_t bit;
    if (!resolve_segment_entry(seg, &addr, &bit)) {
        return;
    }

    if (on) {
        ht1621_ram[addr] |= (1 << bit);
    } else {
        ht1621_ram[addr] &= ~(1 << bit);
    }
    // ht1621_update(); // Removed for efficiency, call separately
}

// Set digit segment on/off (digit 1-6, seg 0=A,1=B,...,6=G)
void set_digit_segment(uint8_t digit, uint8_t seg, bool on) {
    if (digit >= 1 && digit <= 6 && seg <= 6) {
        uint8_t addr = digit_map[digit-1][seg][0];
        uint8_t bit = digit_map[digit-1][seg][1];
        if (addr == 0xFF || bit >= 8 || addr >= sizeof(ht1621_ram)) {
            return; // Unmapped segment
        }
        if (on) {
            ht1621_ram[addr] |= (1 << bit);
        } else {
            ht1621_ram[addr] &= ~(1 << bit);
        }
        // ht1621_update(); // Removed for efficiency, call separately
    }
}

// Update display for a specific COM (not needed for HT1621)
// void update_display_com(uint8_t com) { ... }

// Display 3-digit input AC voltage on digits 1-3, with INPUT (S1) and VAC (S7) symbols
void input_vac_display(uint16_t voltage) {
    if (voltage > 999) voltage = 999; // Cap at 999

    // Turn on INPUT and VAC symbols
    set_segment(1, true); // S1: INPUT
    set_segment(7, true); // S7: VAC

    // Clear digits 1-3
    for (uint8_t d = 1; d <= 3; d++) {
        for (uint8_t s = 0; s < 7; s++) {
            set_digit_segment(d, s, false);
        }
    }

    // Extract digits
    uint8_t hundreds = voltage / 100;
    uint8_t tens = (voltage / 10) % 10;
    uint8_t units = voltage % 10;

    // Display hundreds on digit 1
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(1, s, digit_patterns[hundreds][s]);
    }

    // Display tens on digit 2
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(2, s, digit_patterns[tens][s]);
    }

    // Display units on digit 3
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(3, s, digit_patterns[units][s]);
    }

    ht1621_update();
}

// Display 3-digit input frequency on digits 1-3, with INPUT HZ (S6) symbol
void input_Hz_display(uint16_t hz) {
    if (hz > 999) hz = 999; // Cap at 999

    // Turn on INPUT HZ symbol
    set_segment(6, true); // S6: INPUT HZ

    // Clear digits 1-3
    for (uint8_t d = 1; d <= 3; d++) {
        for (uint8_t s = 0; s < 7; s++) {
            set_digit_segment(d, s, false);
        }
    }

    // Extract digits
    uint8_t hundreds = hz / 100;
    uint8_t tens = (hz / 10) % 10;
    uint8_t units = hz % 10;

    // Display hundreds on digit 1
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(1, s, digit_patterns[hundreds][s]);
    }

    // Display tens on digit 2
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(2, s, digit_patterns[tens][s]);
    }

    // Display units on digit 3
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(3, s, digit_patterns[units][s]);
    }

    ht1621_update();
}

// Display battery DC voltage with decimal point on digits 1-3, with BATTERY (S2) and VDC (S5) symbols
void battery_VDC_display(float voltage) {
    if (voltage > 99.9f) voltage = 99.9f; // Cap at 99.9
    if (voltage < 0.0f) voltage = 0.0f;

    // Turn on BATTERY and VDC symbols, and decimal point
    set_segment(2, true); // S2: BATTERY
    set_segment(5, true); // S5: VDC
    set_segment(3, true); // S3: DOT AFTER SEGMENT 2 (decimal point after digit 2)

    // Clear digits 1-3
    for (uint8_t d = 1; d <= 3; d++) {
        for (uint8_t s = 0; s < 7; s++) {
            set_digit_segment(d, s, false);
        }
    }

    // Extract digits: XX.X format
    uint8_t tens = (uint8_t)(voltage / 10.0f);
    uint8_t units = (uint8_t)voltage % 10;
    uint8_t tenths = (uint8_t)(voltage * 10.0f) % 10;

    // Display tens on digit 1
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(1, s, digit_patterns[tens][s]);
    }

    // Display units on digit 2
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(2, s, digit_patterns[units][s]);
    }

    // Display tenths on digit 3
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(3, s, digit_patterns[tenths][s]);
    }

    ht1621_update();
}

// Display battery level percentage with BATTERY (S2) and level indicators (S29-S33)
void battery_level_display(float current_voltage, float batthigh, float battlow) {
    // Calculate percentage
    float percentage = 0.0f;
    if (batthigh > battlow) {
        percentage = ((current_voltage - battlow) / (batthigh - battlow)) * 100.0f;
    }
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 100.0f) percentage = 100.0f;
    uint8_t level = (uint8_t)percentage;

    // Turn on BATTERY symbol
    set_segment(2, true); // S2: BATTERY

    // Turn off all level indicators first
    set_segment(29, false); // S29: BATTERY 0%
    set_segment(30, false); // S30: BATTERY 25%
    set_segment(31, false); // S31: BATTERY 50%
    set_segment(32, false); // S32: BATTERY 75%
    set_segment(33, false); // S33: BATTERY 100%

    // Turn on the appropriate level indicator
    if (level <= 12) {
        set_segment(29, true); // 0%
    } else if (level <= 37) {
        set_segment(30, true); // 25%
    } else if (level <= 62) {
        set_segment(31, true); // 50%
    } else if (level <= 87) {
        set_segment(32, true); // 75%
    } else {
        set_segment(33, true); // 100%
    }

    ht1621_update();
}

// Display 3-digit output AC voltage on digits 4-6, with OUTPUT (S8) and VAC (S7) symbols
void output_VAC_display(uint16_t voltage) {
    if (voltage > 999) voltage = 999; // Cap at 999

    // Turn on OUTPUT and VAC symbols
    set_segment(8, true); // S8: OUTPUT
    set_segment(7, true); // S7: VAC

    // Clear digits 4-6
    for (uint8_t d = 4; d <= 6; d++) {
        for (uint8_t s = 0; s < 7; s++) {
            set_digit_segment(d, s, false);
        }
    }

    // Extract digits
    uint8_t hundreds = voltage / 100;
    uint8_t tens = (voltage / 10) % 10;
    uint8_t units = voltage % 10;

    // Display hundreds on digit 4
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(4, s, digit_patterns[hundreds][s]);
    }

    // Display tens on digit 5
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(5, s, digit_patterns[tens][s]);
    }

    // Display units on digit 6
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(6, s, digit_patterns[units][s]);
    }

    ht1621_update();
}

// Display 3-digit output frequency on digits 4-6, with OUTPUT HZ (S14) symbol
void output_Hz_display(uint16_t hz) {
    if (hz > 999) hz = 999; // Cap at 999

    // Turn on OUTPUT HZ symbol
    set_segment(14, true); // S14: OUTPUT HZ

    // Clear digits 4-6
    for (uint8_t d = 4; d <= 6; d++) {
        for (uint8_t s = 0; s < 7; s++) {
            set_digit_segment(d, s, false);
        }
    }

    // Extract digits
    uint8_t hundreds = hz / 100;
    uint8_t tens = (hz / 10) % 10;
    uint8_t units = hz % 10;

    // Display hundreds on digit 4
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(4, s, digit_patterns[hundreds][s]);
    }

    // Display tens on digit 5
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(5, s, digit_patterns[tens][s]);
    }

    // Display units on digit 6
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(6, s, digit_patterns[units][s]);
    }

    ht1621_update();
}

// Display load KW with decimal point on digits 4-6, with KW (S13) symbol
void load_KW_display(float kw) {
    if (kw > 99.9f) kw = 99.9f; // Cap at 99.9
    if (kw < 0.0f) kw = 0.0f;

    // Turn on KW symbol and decimal point
    set_segment(13, true); // S13: KW
    set_segment(12, true); // S12: DOT AFTER SEGMENT 5 (decimal point after digit 5)

    // Clear digits 4-6
    for (uint8_t d = 4; d <= 6; d++) {
        for (uint8_t s = 0; s < 7; s++) {
            set_digit_segment(d, s, false);
        }
    }

    // Extract digits: XX.X format
    uint8_t tens = (uint8_t)(kw / 10.0f);
    uint8_t units = (uint8_t)kw % 10;
    uint8_t tenths = (uint8_t)(kw * 10.0f) % 10;

    // Display tens on digit 4
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(4, s, digit_patterns[tens][s]);
    }

    // Display units on digit 5
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(5, s, digit_patterns[units][s]);
    }

    // Display tenths on digit 6
    for (uint8_t s = 0; s < 7; s++) {
        set_digit_segment(6, s, digit_patterns[tenths][s]);
    }

    ht1621_update();
}

// Display load level percentage with LOAD (S4) and level indicators (S17-S21)
void load_level_display(float current_load, float loadmax, float loadmin) {
    // Calculate percentage
    float percentage = 0.0f;
    if (loadmax > loadmin) {
        percentage = ((current_load - loadmin) / (loadmax - loadmin)) * 100.0f;
    }
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 100.0f) percentage = 100.0f;
    uint8_t level = (uint8_t)percentage;

    // Turn on LOAD symbol
    set_segment(4, true); // S4: LOAD

    // Turn off all level indicators first
    set_segment(17, false); // S17: LOAD 0%
    set_segment(18, false); // S18: LOAD 25%
    set_segment(19, false); // S19: LOAD 50%
    set_segment(20, false); // S20: LOAD 75%
    set_segment(21, false); // S21: LOAD 100%

    // Turn on the appropriate level indicator
    if (level <= 12) {
        set_segment(17, true); // 0%
    } else if (level <= 37) {
        set_segment(18, true); // 25%
    } else if (level <= 62) {
        set_segment(19, true); // 50%
    } else if (level <= 87) {
        set_segment(20, true); // 75%
    } else {
        set_segment(21, true); // 100%
    }

    ht1621_update();
}

// Display fault overload indicator (S16)
void fault_overload_display(void) {
    set_segment(16, true); // S16: FAULT OVERLOAD
    ht1621_update();
}

// Display fault battery low indicator (S28)
void fault_battlow_display(void) {
    set_segment(28, true); // S28: FAULT BATTERY LOW
    ht1621_update();
}

// Display UPS on indicator (S22)
void ups_on_display(void) {
    set_segment(22, true); // S22: UPS ON
    ht1621_update();
}

// Display line mode indicator (S24)
void line_mode_display(void) {
    set_segment(24, true); // S24: LINE MODE
    ht1621_update();
}

// Display battery mode indicator (S25)
void battery_mode_display(void) {
    set_segment(25, true); // S25: BATTERY MODE
    ht1621_update();
}

// Example usage (uncomment and call in your application):
// void example_display() {
//     // Set digits
//     // Digit 1: '1' -> B,C
//     set_digit_segment(1, 1, true); // B
//     set_digit_segment(1, 2, true); // C
//
//     // Digit 2: '2' -> A,B,D,E,G
//     set_digit_segment(2, 0, true); // A
//     set_digit_segment(2, 1, true); // B
//     set_digit_segment(2, 3, true); // D
//     set_digit_segment(2, 4, true); // E
//     set_digit_segment(2, 6, true); // G
//
//     // Digit 3: '3' -> A,B,C,D,G
//     set_digit_segment(3, 0, true); // A
//     set_digit_segment(3, 1, true); // B
//     set_digit_segment(3, 2, true); // C
//     set_digit_segment(3, 3, true); // D
//     set_digit_segment(3, 6, true); // G
//
//     // Turn on battery symbol (S2)
//     set_segment(2, true);
// }

// Main loop (not part of driver - implement in your application)
// int main(void) {
//     // HAL_Init();
//     // SystemClock_Config();
//     // MX_GPIO_Init();
//
//     lcd_init();
//     example_display();
//
//     while (1) {
//         HAL_Delay(1000);
//     }
//
//     return 0;
// }

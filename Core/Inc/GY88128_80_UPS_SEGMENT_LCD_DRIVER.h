#ifndef GY88128_80_UPS_SEGMENT_LCD_DRIVER_H
#define GY88128_80_UPS_SEGMENT_LCD_DRIVER_H

#include <stm32g0xx_hal.h>
#include <stdint.h>
#include <stdbool.h>

// HT1621 pins (adjust to your GPIO assignments)
#define HT1621_CS_PIN_GPIO_Port GPIOA
#define HT1621_CS_PIN            GPIO_PIN_0
#define HT1621_WR_PIN_GPIO_Port GPIOA
#define HT1621_WR_PIN            GPIO_PIN_1
#define HT1621_DATA_PIN_GPIO_Port GPIOA
#define HT1621_DATA_PIN          GPIO_PIN_2

#define SYSTEM_LED_PIN_GPIO_Port GPIOC
#define SYSTEM_LED_PIN            GPIO_PIN_13

// Function prototypes
void lcd_init(void);
void clear_all(void);
void set_segment(uint8_t seg, bool on);
void set_digit_segment(uint8_t digit, uint8_t seg, bool on);
void turn_on_unimplemented_segments(void);
void turn_on_all_segments(void);
void scan_all_segments(uint16_t dwell_ms);
// Backlight control via HT1621 IRQ pin (hardware-based, not segment RAM)
bool backlight_is_available(void);
bool backlight_set(bool on);
bool backlight_on(void);
bool backlight_off(void);
void input_vac_display(uint16_t voltage);
void input_Hz_display(uint16_t hz);
void battery_VDC_display(float voltage);
void load_KW_display(float kw);
void battery_level_display(float current_voltage, float batthigh, float battlow);
void output_VAC_display(uint16_t voltage);
void output_Hz_display(uint16_t hz);
void load_level_display(float current_load, float loadmax, float loadmin);
void fault_overload_display(void);
void fault_battlow_display(void);
void ups_on_display(void);
void line_mode_display(void);
void battery_mode_display(void);

#endif // GY88128_80_UPS_SEGMENT_LCD_DRIVER_H
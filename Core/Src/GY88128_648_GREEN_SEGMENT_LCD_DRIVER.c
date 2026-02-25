/**
 * @file GY88128_648_GREEN_SEGMENT_LCD_DRIVER.c
 * @brief Corrected LCD Driver for GY88128-648 Green UPS Display
 * @author Nazmul Huda (pulsestartechnology@gmail.com)
 * @cell +880 1715-298880
 */

#include "GY88128_648_GREEN_SEGMENT_LCD_DRIVER.h"
#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// HT1621 RAM Buffer
// 16 Bytes = 32 Nibbles (Addresses). Address N corresponds to SEG N.
// ============================================================================
static uint8_t ht1621_ram[16] = {0};

// ============================================================================
// HT1621 Command Definitions
// ============================================================================
#define HT1621_CMD_SYS_DIS 0x00
#define HT1621_CMD_SYS_EN 0x01
#define HT1621_CMD_LCD_OFF 0x02
#define HT1621_CMD_LCD_ON 0x03
#define HT1621_CMD_WDT_DIS 0x05
#define HT1621_CMD_TIMER_EN 0x07
#define HT1621_CMD_RC256K 0x18
#define HT1621_CMD_BIAS13_COM4 0x29

// ============================================================================
// Icon Definitions (Mapped to Datasheet Page 2 Table)
// Note: RAM Address N/2 holds High Nibble (Pin N) or Low Nibble (Pin N+1)
// depending on mapping. Here we use raw bytes.
// Pin N corresponds to Address N.
// Byte Index = N / 2.
// ============================================================================

// Helper Macros to map LCD Pin & COM to RAM Address & Bit
// PIN 1 = SEG 0.
// Pin X (1-based) -> Address (X-1).
// RAM Byte i contains: LowNibble=Addr(2*i), HighNibble=Addr(2*i+1)
// Address is the "SEG" index.
// We will access via helper SetIcon(Addr...) where Addr is the Pin Index.

// ICON MAP (Verified 5x against Master Table in segments.md)

// ICON MAP (Verified Bit-Perfect via Manual Hardware Scan)

// Main Labels (Left)
#define ICON_AC_TEXT_ADDR 0 // S0 COM0
#define ICON_AC_TEXT_MASK 0x01
#define ICON_PV2_TEXT_ADDR 0 // S0 COM1
#define ICON_PV2_TEXT_MASK 0x02
#define ICON_PV1_TEXT_ADDR 0 // S0 COM2
#define ICON_PV1_TEXT_MASK 0x04
#define ICON_INPUT_ADDR 1 // S1 COM0
#define ICON_INPUT_MASK 0x01
#define ICON_BATT_L_ADDR 1 // S1 COM1
#define ICON_BATT_L_MASK 0x02

// Main Labels (Right)
#define ICON_OUTPUT_ADDR 15 // S15 COM0
#define ICON_OUTPUT_MASK 0x01
#define ICON_BATT_R_ADDR 15 // S15 COM1
#define ICON_BATT_R_MASK 0x02
#define ICON_LOAD_TXT_ADDR 15 // S15 COM2
#define ICON_LOAD_TXT_MASK 0x04

// PV / MPPT / Grid Icons
#define ICON_ARROW_TO_GRID_ADDR 0 // S0 COM3 (Arrow <<)
#define ICON_ARROW_TO_GRID_MASK 0x08
#define ICON_MAINS_TEXT_ADDR 31 // S31 COM2 (MAINS text)
#define ICON_MAINS_TEXT_MASK 0x04
#define ICON_LINE_ICON_ADDR 31 // S31 COM3 (LINE text)
#define ICON_LINE_ICON_MASK 0x08
#define ICON_MPPT_TEXT_ADDR 28 // S28 COM3 (MPPT text)
#define ICON_MPPT_TEXT_MASK 0x08
#define ICON_INV_TEXT_ADDR 26 // S26 COM3 (INV text)
#define ICON_INV_TEXT_MASK 0x08
#define ICON_PV1_ICON_ADDR 31 // S31 COM0 (Panel 1)
#define ICON_PV1_ICON_MASK 0x01
#define ICON_PV2_ICON_ADDR 31 // S31 COM1 (Panel 2)
#define ICON_PV2_ICON_MASK 0x02
#define ICON_PV_BIG_PV1_ADDR 30 // S30 COM0 (Large PV Bit)
#define ICON_PV_BIG_PV1_MASK 0x01
#define ICON_PV_BIG_PV2_ADDR 30 // S30 COM1 (Large PV Bit)
#define ICON_PV_BIG_PV2_MASK 0x02

// Units (Left Group)
#define ICON_UNIT_K_L_ADDR 1 // S1 COM3
#define ICON_UNIT_K_L_MASK 0x08
#define ICON_LABEL_M_L_ADDR 1 // S1 COM2
#define ICON_LABEL_M_L_MASK 0x04
#define ICON_UNIT_W_L_ADDR 8 // S8 COM0
#define ICON_UNIT_W_L_MASK 0x01
#define ICON_UNIT_A_L_ADDR 8 // S8 COM1
#define ICON_UNIT_A_L_MASK 0x02
#define ICON_UNIT_V_L_ADDR 8 // S8 COM2
#define ICON_UNIT_V_L_MASK 0x04
#define ICON_UNIT_PCT_L_ADDR 8 // S8 COM3
#define ICON_UNIT_PCT_L_MASK 0x08
#define ICON_UNIT_H_L_ADDR 9 // S9 COM0
#define ICON_UNIT_H_L_MASK 0x01
#define ICON_UNIT_M_L_ADDR 9 // S9 COM1
#define ICON_UNIT_M_L_MASK 0x02
#define ICON_UNIT_h_L_ADDR 9 // S9 COM3
#define ICON_UNIT_h_L_MASK 0x08
#define ICON_UNIT_Hz_L_ADDR 7 // S7 COM3
#define ICON_UNIT_Hz_L_MASK 0x08

// Units (Right Group)
#define ICON_UNIT_K_R_ADDR 15 // S15 COM3
#define ICON_UNIT_K_R_MASK 0x08
#define ICON_UNIT_W_R_ADDR 22 // S22 COM0
#define ICON_UNIT_W_R_MASK 0x01
#define ICON_UNIT_A_R_ADDR 22 // S22 COM1
#define ICON_UNIT_A_R_MASK 0x02
#define ICON_UNIT_V_R_ADDR 22 // S22 COM2
#define ICON_UNIT_V_R_MASK 0x04
#define ICON_UNIT_PCT_R_ADDR 22 // S22 COM3
#define ICON_UNIT_PCT_R_MASK 0x08
#define ICON_UNIT_Hz_R_ADDR 21 // S21 COM3
#define ICON_UNIT_Hz_R_MASK 0x08

// Levels / Bar Graphs
#define ICON_LOAD_BAR_ADDR 25 // S25 bits 0-3
#define ICON_LOAD_BAR_MASK 0x0F
#define ICON_BATT_BAR_ADDR 29 // S29 bits 0-3
#define ICON_BATT_BAR_MASK 0x0F
#define ICON_LOAD_LVL_ADDR 24 // S24 COM0 (Text label "Load Level")
#define ICON_LOAD_LVL_MASK 0x01
#define ICON_BULB_ADDR 24 // S24 COM1 (Bulb icon)
#define ICON_BULB_MASK 0x02

// Battery Types
#define ICON_TYPE_ADDR 26
#define ICON_TYPE_USER_MASK 0x01
#define ICON_TYPE_FLD_MASK 0x02
#define ICON_TYPE_AGM_MASK 0x04

// Faults / Alerts
#define ICON_ERROR_ADDR 14 // S14 COM1
#define ICON_ERROR_MASK 0x02
#define ICON_WARNING_ADDR 14 // S14 COM0 (Triangle !)
#define ICON_WARNING_MASK 0x01
#define ICON_WRENCH_ADDR 14 // S14 COM2
#define ICON_WRENCH_MASK 0x04
#define ICON_OVERLOAD_ADDR 23 // S23 COM0
#define ICON_OVERLOAD_MASK 0x01
#define ICON_MUTE_ADDR 23 // S23 COM1
#define ICON_MUTE_MASK 0x02

// Flow Lines & Arrows
#define ICON_FLOW_BAT_ADDR 27 // S27 COM0 (BAT Source)
#define ICON_FLOW_BAT_MASK 0x01
#define ICON_FLOW_CHG_ADDR 27 // S27 COM1 (Battery Charging)
#define ICON_FLOW_CHG_MASK 0x02
#define ICON_FLOW_AC_OUT_ADDR 27 // S27 COM2 (Inverter Output Flow)
#define ICON_FLOW_AC_OUT_MASK 0x04
#define ICON_FLOW_MAINS_IN_ADDR 27 // S27 COM3 (Mains Input Flow)
#define ICON_FLOW_MAINS_IN_MASK 0x08

#define ICON_ARROW_TO_LOAD_ADDR 23 // S23 COM2 (Arrow >>)
#define ICON_ARROW_TO_LOAD_MASK 0x04
#define ICON_LINE_TO_LOAD_ADDR 24 // S24 COM3 (LINE > to load)
#define ICON_LINE_TO_LOAD_MASK 0x08
#define ICON_LINE_FOR_CHG_ADDR 30 // S30 COM3 (LINE > for charging)
#define ICON_LINE_FOR_CHG_MASK 0x08
#define ICON_ARROW_MPPT_BAT_ADDR 28 // S28 COM2 (PVT1 Arrow down)
#define ICON_ARROW_MPPT_BAT_MASK 0x04
#define ICON_JUNCTION_ADDR 24 // S24 COM2
#define ICON_JUNCTION_MASK 0x04
#define ICON_LINE_ADDR 23 // S23 COM3 (L2 Cell)
#define ICON_LINE_MASK 0x08
#define ICON_BORDER_ADDR 13 // S13 COM3 (L1 Cell)
#define ICON_BORDER_MASK 0x08
#define ICON_UNIT_h_ALT_ADDR 28 // S28 COM0
#define ICON_UNIT_h_ALT_MASK 0x01

// Misc
#define ICON_CLOCK_ADDR 14 // S14 COM3
#define ICON_CLOCK_MASK 0x08
#define ICON_BAT_ICON_ADDR 28 // S28 COM1
#define ICON_BAT_ICON_MASK 0x02

// S-Number specific mappings from charging_modes.md
#define ICON_SYS_SWITCH_ADDR 27 // S27 COM2 (S50)
#define ICON_SYS_SWITCH_MASK 0x04
#define ICON_HZ1_ADDR 7 // S7 COM3
#define ICON_HZ1_MASK 0x08

// Decimal Dots
#define ICON_DOT1_ADDR 3 // S3 COM3
#define ICON_DOT1_MASK 0x08
#define ICON_DOT2_ADDR 5 // S5 COM3
#define ICON_DOT2_MASK 0x08
#define ICON_DOT4_ADDR 17 // S17 COM3
#define ICON_DOT4_MASK 0x08
#define ICON_DOT5_ADDR 19 // S19 COM3
#define ICON_DOT5_MASK 0x08

/* * Hardware Layout from Datasheet Screenshot (GY88128_648_Green.PNG):
 *
 * Top Left (3 Digits):
 * Digit 1 (1xx): S2 (F,G,E,D) & S3 (A,B,C)
 * Digit 2 (x2x): S5 (F,G,E,D) & S4 (A,B,C)
 * Digit 3 (xx3): S6 (F,G,E,D) & S7 (A,B,C)
 *
 * Center (2 Digits):
 * Digit 4: S10 (F,G,E,D) & S11 (A,B,C,%)
 * Digit 5: S12 (F,G,E,D) & S13 (A,B,C,Clock)
 *
 * Top Right (3 Digits):
 * Digit 6: S16 (F,G,E,D) & S17 (A,B,C)
 * Digit 7: S18 (F,G,E,D) & S19 (A,B,C)
 * Digit 8: S20 (F,G,E,D) & S21 (A,B,C)
 *
 * segment index: 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G
 */

static const uint8_t digit_map[8][7][2] = {
    // Digit 1 (Top Left): S3(A,B,C), S2(D,E,F,G)
    {{3, 0}, {3, 1}, {3, 2}, {2, 3}, {2, 2}, {2, 0}, {2, 1}},
    // Digit 2 (Top Left 2): S5(A,B,C), S4(D,E,F,G). Corrected swap.
    {{5, 0}, {5, 1}, {5, 2}, {4, 3}, {4, 2}, {4, 0}, {4, 1}},
    // Digit 3 (Top Left 3): S7(A,B,C), S6(D,E,F,G)
    {{7, 0}, {7, 1}, {7, 2}, {6, 3}, {6, 2}, {6, 0}, {6, 1}},

    // Digit 4 (Center Left): S11(A,B,C), S10(D,E,F,G)
    {{11, 0}, {11, 1}, {11, 2}, {10, 3}, {10, 2}, {10, 0}, {10, 1}},
    // Digit 5 (Center Right): S13(A,B,C), S12(D,E,F,G)
    {{13, 0}, {13, 1}, {13, 2}, {12, 3}, {12, 2}, {12, 0}, {12, 1}},

    // Digit 6 (Right Left): S17(A,B,C), S16(D,E,F,G)
    {{17, 0}, {17, 1}, {17, 2}, {16, 3}, {16, 2}, {16, 0}, {16, 1}},
    // Digit 7 (Right Center): S19(A,B,C), S18(D,E,F,G)
    {{19, 0}, {19, 1}, {19, 2}, {18, 3}, {18, 2}, {18, 0}, {18, 1}},
    // Digit 8 (Right Right): S21(A,B,C), S20(D,E,F,G)
    {{21, 0}, {21, 1}, {21, 2}, {20, 3}, {20, 2}, {20, 0}, {20, 1}}};

// 7-segment digit patterns: A B C D E F G
static const bool digit_patterns[10][7] = {
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}  // 9
};

// ============================================================================
// HT1621 Low-Level Functions
// ============================================================================

// Forward declarations of low-level functions
static void ht1621_send_bit(bool bit);
static void ht1621_send_bits(uint16_t value, uint8_t count, bool msb_first);
static void ht1621_send_command(uint8_t cmd);

static void ht1621_send_bit(bool bit) {
  HAL_GPIO_WritePin(HT1621_648_DATA_GPIO_Port, HT1621_648_DATA_PIN,
                    bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(HT1621_648_WR_GPIO_Port, HT1621_648_WR_PIN, GPIO_PIN_RESET);
  for (volatile int i = 0; i < 50; i++)
    ; // Delay for HT1621 1.6uS pulse width
  HAL_GPIO_WritePin(HT1621_648_WR_GPIO_Port, HT1621_648_WR_PIN, GPIO_PIN_SET);
  for (volatile int i = 0; i < 50; i++)
    ;
}

/**
 * @brief Sends multiple bits to HT1621
 * @param value The value containing bits
 * @param count Number of bits to send
 * @param msb_first True if sending from MSB to LSB
 */
static void ht1621_send_bits(uint16_t value, uint8_t count, bool msb_first) {
  if (msb_first) {
    for (int i = count - 1; i >= 0; i--) {
      ht1621_send_bit((value >> i) & 1);
    }
  } else {
    for (int i = 0; i < count; i++) {
      ht1621_send_bit((value >> i) & 1);
    }
  }
}

static void ht1621_send_command(uint8_t cmd) {
  HAL_GPIO_WritePin(HT1621_648_CS_GPIO_Port, HT1621_648_CS_PIN, GPIO_PIN_RESET);
  ht1621_send_bit(1); // ID
  ht1621_send_bit(0);
  ht1621_send_bit(0);
  for (int i = 7; i >= 0; i--) {
    ht1621_send_bit((cmd >> i) & 1);
  }
  ht1621_send_bit(0);
  HAL_GPIO_WritePin(HT1621_648_CS_GPIO_Port, HT1621_648_CS_PIN, GPIO_PIN_SET);
}

static void ht1621_gpio_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  GPIO_InitStruct.Pin = HT1621_648_CS_PIN;
  HAL_GPIO_Init(HT1621_648_CS_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = HT1621_648_WR_PIN;
  HAL_GPIO_Init(HT1621_648_WR_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = HT1621_648_DATA_PIN;
  HAL_GPIO_Init(HT1621_648_DATA_GPIO_Port, &GPIO_InitStruct);

  HAL_GPIO_WritePin(HT1621_648_CS_GPIO_Port, HT1621_648_CS_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(HT1621_648_WR_GPIO_Port, HT1621_648_WR_PIN, GPIO_PIN_SET);
}

// ============================================================================
// Public API Functions
// ============================================================================

void GY88128_648_Init(void) {
  ht1621_gpio_init();
  ht1621_send_command(HT1621_CMD_SYS_DIS);
  ht1621_send_command(HT1621_CMD_RC256K);
  ht1621_send_command(HT1621_CMD_BIAS13_COM4);
  ht1621_send_command(HT1621_CMD_SYS_EN);
  ht1621_send_command(HT1621_CMD_LCD_ON);
  GY88128_648_Clear();
}

void GY88128_648_Clear(void) {
  for (int i = 0; i < sizeof(ht1621_ram); i++)
    ht1621_ram[i] = 0;
}

void GY88128_648_Refresh(void) {
  // Use Successive Write Mode: 101 + A5..A0 + D0..D3 (count times)
  // This is the most stable way to update the HT1621.
  HAL_GPIO_WritePin(HT1621_648_CS_GPIO_Port, HT1621_648_CS_PIN, GPIO_PIN_RESET);

  ht1621_send_bits(0x05, 3, true); // Write ID: 101
  ht1621_send_bits(0, 6, true);    // Start Address: 0x00

  for (uint8_t i = 0; i < 16; i++) {
    uint8_t data = ht1621_ram[i];
    // HT1621 expects 4-bit nibbles LSB first in successive mode
    ht1621_send_bits(data & 0x0F, 4, false); // Low nibble (Addr 2*i)
    ht1621_send_bits(data >> 4, 4, false);   // High nibble (Addr 2*i + 1)
  }

  HAL_GPIO_WritePin(HT1621_648_CS_GPIO_Port, HT1621_648_CS_PIN, GPIO_PIN_SET);

  // Optional but recommended: Small setup delay after CS high
  for (volatile int i = 0; i < 100; i++)
    ;
}

// Logic: Addr in map is 0-31.
// ram[Addr/2] contains the data.
// If Addr is even, it's low nibble. If Addr is odd, it's high nibble.
void GY88128_648_SetSegment(uint8_t seg, bool on) {
  // This function originally took a linear bit index (0-256).
  // Not commonly used with the structure below.
}

// New Helper to set a specific bit in our byte-packed RAM
static void set_ram_bit(uint8_t addr, uint8_t bit, bool on) {
  if (addr > 31)
    return;
  uint8_t byte_idx = addr / 2;
  bool is_high = (addr % 2);

  uint8_t mask = (1 << bit);
  if (is_high)
    mask <<= 4;
  else
    mask = mask; // Low nibble

  if (on)
    ht1621_ram[byte_idx] |= mask;
  else
    ht1621_ram[byte_idx] &= ~mask;
}

// MINIMAL TEST: Write directly to RAM to verify basic communication
// This bypasses all helper functions to isolate the problem
void GY88128_648_TestSingleDigit(void) {
  // Test 1: Clear and write 0x0F to first few bytes
  // This should light up some segments
  GY88128_648_Clear();
  HAL_Delay(500);

  // Write directly to RAM - each byte holds 2 nibbles (2 addresses)
  // ht1621_ram[0] = nibble for Addr 0 (low) + nibble for Addr 1 (high)
  // ht1621_ram[1] = nibble for Addr 2 (low) + nibble for Addr 3 (high)
  // ht1621_ram[2] = nibble for Addr 4 (low) + nibble for Addr 5 (high)
  // ...

  // Set bytes 2 and 3 to 0xFF to light up addresses 4,5,6,7
  // This should light up Digit 1 and parts of Digit 2
  ht1621_ram[2] = 0xFF; // Addr 4 & 5 all bits
  ht1621_ram[3] = 0xFF; // Addr 6 & 7 all bits
  GY88128_648_Refresh();
  HAL_Delay(2000);

  // Test 2: Try different bytes
  GY88128_648_Clear();
  HAL_Delay(500);

  // Set bytes for addresses matching S5 and S6 from datasheet
  // S5 = Address 5, S6 = Address 6
  // Address 5 is in ht1621_ram[2] high nibble
  // Address 6 is in ht1621_ram[3] low nibble
  ht1621_ram[2] = 0xF0; // Addr 5 all bits ON, Addr 4 all bits OFF
  ht1621_ram[3] = 0x0F; // Addr 6 all bits ON, Addr 7 all bits OFF
  GY88128_648_Refresh();
  HAL_Delay(2000);

  // Test 3: Single nibble test
  GY88128_648_Clear();
  HAL_Delay(500);

  // Just one nibble
  ht1621_ram[2] = 0x0F; // Only Addr 4 bits ON
  GY88128_648_Refresh();
}

void GY88128_648_SetDigitSegment(uint8_t digit, uint8_t seg, bool on) {
  if (digit < 1 || digit > 8 || seg > 6)
    return;
  uint8_t addr = digit_map[digit - 1][seg][0];
  uint8_t bit = digit_map[digit - 1][seg][1];
  set_ram_bit(addr, bit, on);
}

void GY88128_648_SetIcon(uint8_t addr, uint8_t mask, bool on) {
  // Here 'addr' is the Pin Index (0-31). 'mask' is the COM bit (0x01, 0x02,
  // 0x04, 0x08). Note: The mask passed in original code was 0x08 (Bit 3). We
  // need to convert mask back to bit position 0-3.
  uint8_t bit = 0;
  if (mask & 0x02)
    bit = 1;
  if (mask & 0x04)
    bit = 2;
  if (mask & 0x08)
    bit = 3;

  set_ram_bit(addr, bit, on);
}

static void display_digit(uint8_t digit_pos, uint8_t value) {
  if (value > 9)
    value = 9;
  for (uint8_t seg = 0; seg < 7; seg++) {
    GY88128_648_SetDigitSegment(digit_pos, seg, digit_patterns[value][seg]);
  }
}

void GY88128_648_PrintLeft(int value) {
  if (value > 999)
    value = 999;
  if (value < 0)
    value = 0;
  display_digit(1, (value / 100) % 10);
  display_digit(2, (value / 10) % 10);
  display_digit(3, value % 10);
}

// Direct RAM function for Center Digits (4 & 5)
// Digit 4: S11 (A,B,C), S10 (D,E,F,G)
// Digit 5: S13 (A,B,C, Border), S12 (D,E,F,G)
void GY88128_648_PrintCenter(int value) {
  if (value > 99)
    value = 99;
  if (value < 0)
    value = 0;

  uint8_t tens = (value / 10) % 10;
  uint8_t units = value % 10;

  // --- Digit 4 (Tens) ---
  // A (0), B (1), C (2) on S11 (Addr 11)
  uint8_t s11_val = 0;
  if (digit_patterns[tens][0])
    s11_val |= 0x01; // A
  if (digit_patterns[tens][1])
    s11_val |= 0x02; // B
  if (digit_patterns[tens][2])
    s11_val |= 0x04; // C

  // D (3), E (2), F (0), G (1) on S10 (Addr 10)
  uint8_t s10_val = 0;
  if (digit_patterns[tens][3])
    s10_val |= 0x08; // D (Bit 3)
  if (digit_patterns[tens][4])
    s10_val |= 0x04; // E (Bit 2)
  if (digit_patterns[tens][5])
    s10_val |= 0x01; // F (Bit 0)
  if (digit_patterns[tens][6])
    s10_val |= 0x02; // G (Bit 1)

  // Write S10 and S11
  // Addr 10 is low nibble of byte 5. Addr 11 is high nibble of byte 5.
  ht1621_ram[5] = (s11_val << 4) | s10_val;

  // --- Digit 5 (Units) ---
  // A (0), B (1), C (2) on S13 (Addr 13)
  // CAUTION: S13 Bit 3 is Border/L1.
  uint8_t s13_val = 0;
  if (digit_patterns[units][0])
    s13_val |= 0x01; // A
  if (digit_patterns[units][1])
    s13_val |= 0x02; // B
  if (digit_patterns[units][2])
    s13_val |= 0x04; // C

  // Preserve existing Bit 3 (Border) in S13
  uint8_t current_s13 = ht1621_ram[6] >> 4;
  if (current_s13 & 0x08)
    s13_val |= 0x08;

  // D (3), E (2), F (0), G (1) on S12 (Addr 12)
  uint8_t s12_val = 0;
  if (digit_patterns[units][3])
    s12_val |= 0x08; // D (Bit 3)
  if (digit_patterns[units][4])
    s12_val |= 0x04; // E (Bit 2)
  if (digit_patterns[units][5])
    s12_val |= 0x01; // F (Bit 0)
  if (digit_patterns[units][6])
    s12_val |= 0x02; // G (Bit 1)

  // Write S12 and S13
  // Addr 12 is low nibble of byte 6. Addr 13 is high nibble of byte 6.
  ht1621_ram[6] = (s13_val << 4) | s12_val;
}

void GY88128_648_PrintRight(int value) {
  if (value > 999)
    value = 999;
  if (value < 0)
    value = 0;
  // Use Indices 5,6,7 of digit_map (Right Group)
  display_digit(6, (value / 100) % 10);
  display_digit(7, (value / 10) % 10);
  display_digit(8, value % 10);
}

void GY88128_648_TurnOnAllSegments(void) {
  for (int i = 0; i < sizeof(ht1621_ram); i++)
    ht1621_ram[i] = 0xFF;
  GY88128_648_Refresh();
}

void GY88128_648_ScanAllSegments(uint16_t dwell_ms) {
  if (dwell_ms == 0)
    dwell_ms = 500;
  GY88128_648_Clear();
  for (uint8_t addr = 0; addr < 32; addr++) { // Scan Pins
    for (uint8_t bit = 0; bit < 4; bit++) {   // Scan COMs
      GY88128_648_Clear();
      set_ram_bit(addr, bit, true);
      GY88128_648_Refresh();
      HAL_Delay(dwell_ms);
    }
  }
  GY88128_648_Clear();
}

void GY88128_648_ScanByAddress(uint16_t dwell_ms) {
  if (dwell_ms == 0)
    dwell_ms = 1000;
  GY88128_648_Clear();
  for (uint8_t addr = 0; addr < 32; addr++) {
    GY88128_648_Clear();
    set_ram_bit(addr, 0, true);
    set_ram_bit(addr, 1, true);
    set_ram_bit(addr, 2, true);
    set_ram_bit(addr, 3, true);
    GY88128_648_Refresh();
    HAL_Delay(dwell_ms);
  }
  GY88128_648_Clear();
}

void GY88128_648_TestAddressRange(uint8_t start_addr, uint8_t end_addr,
                                  uint16_t dwell_ms) {
  if (dwell_ms == 0)
    dwell_ms = 500;
  if (end_addr > 31)
    end_addr = 31;
  GY88128_648_Clear();
  for (uint8_t addr = start_addr; addr <= end_addr; addr++) {
    for (uint8_t bit = 0; bit < 4; bit++) {
      GY88128_648_Clear();
      set_ram_bit(addr, bit, true);
      GY88128_648_Refresh();
      HAL_Delay(dwell_ms);
    }
  }
  GY88128_648_Clear();
}

void GY88128_648_CumulativeSnakeScan(uint16_t dwell_ms) {
  if (dwell_ms == 0)
    dwell_ms = 50;
  GY88128_648_Clear();
  for (uint8_t addr = 0; addr < 32; addr++) { // Scan Pins
    for (uint8_t bit = 0; bit < 4; bit++) {   // Scan COMs
      set_ram_bit(addr, bit, true);
      GY88128_648_Refresh();
      HAL_Delay(dwell_ms);
    }
  }
}

void GY88128_648_WriteRAM(uint8_t addr, uint8_t value) {
  if (addr < 32) {
    uint8_t byte_idx = addr / 2;
    bool is_high = (addr % 2);
    if (is_high) {
      ht1621_ram[byte_idx] =
          (ht1621_ram[byte_idx] & 0x0F) | ((value & 0x0F) << 4);
    } else {
      ht1621_ram[byte_idx] = (ht1621_ram[byte_idx] & 0xF0) | (value & 0x0F);
    }
    GY88128_648_Refresh();
  }
}

// ============================================================================
// Helper Functions for Screen Displays
// ============================================================================

static bool get_blink_state(void) {
  // Sync blink state with physical system LED on PC13
  return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET;
}

static void Update_Battery_Visuals(float volt, uint8_t type, bool charging) {
  bool blink = get_blink_state();

  // Always turn on the Battery Icon shell (S28 COM1)
  // Blinks on low battery warning as per charging_modes.md
  bool shell_on = (volt > 10.8) ? true : blink;
  GY88128_648_SetIcon(ICON_BAT_ICON_ADDR, ICON_BAT_ICON_MASK, shell_on);

  // Battery Level / Charging Bar (Horizontal, S29, bits 0-3: K5-K8)
  // Logic: Display up to the current level. If charging, blink the "next" or
  // "last" bar.
  uint8_t level = 0;
  if (volt > 11.0)
    level = 1;
  if (volt > 12.0)
    level = 2;
  if (volt > 12.6)
    level = 3;
  if (volt > 13.5)
    level = 4;

  for (int i = 0; i < 4; i++) {
    bool bar_on = (i < level);
    if (charging && i == level && level < 4) {
      bar_on = blink; // Blink the filling bar
    } else if (charging && i == 3 && level == 4) {
      bar_on = blink; // Blink the last bar if full and charging
    }

    // Low battery warning special case: K5 blinks
    if (volt <= 11.0 && i == 0) {
      bar_on = blink;
    }

    GY88128_648_SetIcon(ICON_BATT_BAR_ADDR, (1 << i), bar_on);
  }

  // Battery Type Labels (S26)
  GY88128_648_SetIcon(ICON_TYPE_ADDR, ICON_TYPE_FLD_MASK, (type == 0));
  GY88128_648_SetIcon(ICON_TYPE_ADDR, ICON_TYPE_USER_MASK, (type == 1));
  GY88128_648_SetIcon(ICON_TYPE_ADDR, ICON_TYPE_AGM_MASK, (type == 2));
}

static void Update_Load_Visuals(int load_percent) {
  // Always turn on the "Load Level" text label when load visuals are updated
  GY88128_648_SetIcon(ICON_LOAD_LVL_ADDR, ICON_LOAD_LVL_MASK, 1);

  // Bulb Icon (S24 COM1) is active if load > 1%
  GY88128_648_SetIcon(ICON_BULB_ADDR, ICON_BULB_MASK, (load_percent > 1));

  // Load Level Bar (Vertical, S25, bits 0-3)
  GY88128_648_SetIcon(ICON_LOAD_BAR_ADDR, 0x01, (load_percent > 10));
  GY88128_648_SetIcon(ICON_LOAD_BAR_ADDR, 0x02, (load_percent > 40));
  GY88128_648_SetIcon(ICON_LOAD_BAR_ADDR, 0x04, (load_percent > 70));
  GY88128_648_SetIcon(ICON_LOAD_BAR_ADDR, 0x08, (load_percent > 90));
}

static void Update_Power_Flow(SystemStatus_t *stat) {
  // Clear all flow segments before reassignment
  GY88128_648_SetIcon(ICON_FLOW_BAT_ADDR, ICON_FLOW_BAT_MASK, 0);
  GY88128_648_SetIcon(ICON_FLOW_CHG_ADDR, ICON_FLOW_CHG_MASK, 0);
  GY88128_648_SetIcon(ICON_FLOW_AC_OUT_ADDR, ICON_FLOW_AC_OUT_MASK, 0);
  GY88128_648_SetIcon(ICON_FLOW_MAINS_IN_ADDR, ICON_FLOW_MAINS_IN_MASK, 0);

  // 1. PV Path (PV1, PV2, MPPT, Arrow To Bat, Line To Bat)
  if (stat->is_solar_present) {
    GY88128_648_SetIcon(ICON_PV1_ICON_ADDR, ICON_PV1_ICON_MASK, 1);
    GY88128_648_SetIcon(ICON_PV_BIG_PV1_ADDR, ICON_PV_BIG_PV1_MASK, 1);
    GY88128_648_SetIcon(ICON_MPPT_TEXT_ADDR, ICON_MPPT_TEXT_MASK, 1);
    if (stat->is_charging) {
      GY88128_648_SetIcon(ICON_ARROW_MPPT_BAT_ADDR, ICON_ARROW_MPPT_BAT_MASK,
                          1);
      GY88128_648_SetIcon(ICON_LINE_FOR_CHG_ADDR, ICON_LINE_FOR_CHG_MASK, 1);
    }
  }

  // 2. Mains Path (MAINS, LINE text, Flow Mains In, Flow Charging, Junction)
  if (stat->is_grid_present) {
    GY88128_648_SetIcon(ICON_MAINS_TEXT_ADDR, ICON_MAINS_TEXT_MASK, 1);
    GY88128_648_SetIcon(ICON_LINE_ICON_ADDR, ICON_LINE_ICON_MASK, 1);
    GY88128_648_SetIcon(ICON_FLOW_MAINS_IN_ADDR, ICON_FLOW_MAINS_IN_MASK, 1);
    GY88128_648_SetIcon(ICON_JUNCTION_ADDR, ICON_JUNCTION_MASK, 1);
    if (stat->is_charging) {
      GY88128_648_SetIcon(ICON_FLOW_CHG_ADDR, ICON_FLOW_CHG_MASK, 1);
    }
  }

  // 3. Inverter Path (INV, Flow Bat source, Flow AC Out, Arrow To Load, Line To
  // Load)
  if (stat->output_active) {
    GY88128_648_SetIcon(ICON_INV_TEXT_ADDR, ICON_INV_TEXT_MASK, 1);
    GY88128_648_SetIcon(ICON_FLOW_AC_OUT_ADDR, ICON_FLOW_AC_OUT_MASK, 1);
    GY88128_648_SetIcon(ICON_ARROW_TO_LOAD_ADDR, ICON_ARROW_TO_LOAD_MASK, 1);
    GY88128_648_SetIcon(ICON_LINE_TO_LOAD_ADDR, ICON_LINE_TO_LOAD_MASK, 1);
    GY88128_648_SetIcon(ICON_SYS_SWITCH_ADDR, ICON_SYS_SWITCH_MASK, 1); // S50

    // If not charging but active, show output from battery
    if (!stat->is_charging) {
      GY88128_648_SetIcon(ICON_FLOW_BAT_ADDR, ICON_FLOW_BAT_MASK, 1);
    }
  }
}

// ============================================================================
// ============================================================================
// Screen Display Functions
// ============================================================================

static void Show_Screen_ID(int id) {
  GY88128_648_PrintCenter(id);
  GY88128_648_SetIcon(ICON_BORDER_ADDR, ICON_BORDER_MASK, 1); // L1
}

void Screen_Inv_Home(SystemStatus_t *stat) {
  GY88128_648_Clear();
  if (stat->is_grid_present) {
    GY88128_648_PrintLeft((int)stat->grid_volt);
    GY88128_648_SetIcon(ICON_INPUT_ADDR, ICON_INPUT_MASK, 1);
    GY88128_648_SetIcon(ICON_DOT2_ADDR, ICON_DOT2_MASK, 0);
  } else {
    GY88128_648_PrintLeft((int)(stat->batt_volt * 10));
    GY88128_648_SetIcon(ICON_BATT_L_ADDR, ICON_BATT_L_MASK, 1);
    GY88128_648_SetIcon(ICON_DOT2_ADDR, ICON_DOT2_MASK, 1);
  }
  GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);

  GY88128_648_PrintRight((int)stat->inv_out_volt);
  GY88128_648_SetIcon(ICON_OUTPUT_ADDR, ICON_OUTPUT_MASK, 1);
  GY88128_648_SetIcon(ICON_UNIT_V_R_ADDR, ICON_UNIT_V_R_MASK, 1);

  Update_Battery_Visuals(stat->batt_volt, stat->batt_type, stat->is_charging);
  Update_Load_Visuals(stat->inv_load_percent);
  Update_Power_Flow(stat);

  Update_Power_Flow(stat);

  // Turn on L2 Partition Line for data framing; L1 is for center digits only
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);

  Show_Screen_ID(0);
  GY88128_648_Refresh();
}

void Screen_Grid_Detail(SystemStatus_t *stat) {
  GY88128_648_Clear();
  GY88128_648_PrintLeft((int)stat->grid_volt);
  GY88128_648_SetIcon(ICON_INPUT_ADDR, ICON_INPUT_MASK, 1);
  GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);
  GY88128_648_SetIcon(ICON_AC_TEXT_ADDR, ICON_AC_TEXT_MASK, 1);
  GY88128_648_SetIcon(ICON_MAINS_TEXT_ADDR, ICON_MAINS_TEXT_MASK, 1);
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);
  GY88128_648_SetIcon(ICON_BORDER_ADDR, ICON_BORDER_MASK, 0);
  GY88128_648_PrintRight((int)(stat->grid_freq * 10));
  GY88128_648_SetIcon(ICON_UNIT_Hz_R_ADDR, ICON_UNIT_Hz_R_MASK, 1);
  GY88128_648_Refresh();
}

void Screen_Batt_Charging(SystemStatus_t *stat) {
  GY88128_648_Clear();
  GY88128_648_PrintLeft((int)(stat->batt_volt * 10));
  GY88128_648_SetIcon(ICON_BATT_L_ADDR, ICON_BATT_L_MASK, 1);
  GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);
  GY88128_648_PrintRight((int)(stat->chg_amp * 10));
  GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);
  Update_Battery_Visuals(stat->batt_volt, stat->batt_type, true);
  Update_Power_Flow(stat);
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);
  GY88128_648_SetIcon(ICON_BORDER_ADDR, ICON_BORDER_MASK, 0);
  GY88128_648_Refresh();
}

void Screen_Load_Detail(SystemStatus_t *stat) {
  GY88128_648_Clear();
  GY88128_648_PrintLeft((int)(stat->batt_volt * 10));
  GY88128_648_SetIcon(ICON_BATT_L_ADDR, ICON_BATT_L_MASK, 1);
  GY88128_648_PrintRight(stat->inv_load_watt);
  GY88128_648_SetIcon(ICON_UNIT_W_R_ADDR, ICON_UNIT_W_R_MASK, 1);
  GY88128_648_SetIcon(ICON_OUTPUT_ADDR, ICON_OUTPUT_MASK, 1);
  GY88128_648_SetIcon(ICON_LOAD_TXT_ADDR, ICON_LOAD_TXT_MASK, 1);
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);
  Update_Load_Visuals(stat->inv_load_percent);

  Show_Screen_ID(7);
  GY88128_648_Refresh();
}

void Screen_Solar_Home(SystemStatus_t *stat) {
  GY88128_648_Clear();
  GY88128_648_SetIcon(ICON_PV_BIG_PV1_ADDR, ICON_PV_BIG_PV1_MASK, 1);
  GY88128_648_PrintLeft((int)stat->solar_volt);
  GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);
  GY88128_648_PrintRight((int)(stat->solar_amp * 10));
  GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);
  Update_Battery_Visuals(stat->batt_volt, stat->batt_type, stat->is_charging);
  Update_Power_Flow(stat);
  Update_Power_Flow(stat);
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);

  Show_Screen_ID(1);
  GY88128_648_Refresh();
}

void Screen_Solar_Yield(SystemStatus_t *stat) {
  GY88128_648_Clear();
  GY88128_648_PrintLeft((int)(stat->day_kwh * 100)); // Use D1-D3 for daily
  GY88128_648_SetIcon(ICON_UNIT_K_L_ADDR, ICON_UNIT_K_L_MASK, 1);
  GY88128_648_SetIcon(ICON_UNIT_W_L_ADDR, ICON_UNIT_W_L_MASK, 1);
  GY88128_648_SetIcon(ICON_UNIT_h_L_ADDR, ICON_UNIT_h_L_MASK, 1);
  GY88128_648_PrintRight(stat->total_kwh);
  GY88128_648_SetIcon(ICON_UNIT_K_R_ADDR, ICON_UNIT_K_R_MASK, 1);
  GY88128_648_SetIcon(ICON_UNIT_W_R_ADDR, ICON_UNIT_W_R_MASK, 1);
  GY88128_648_PrintRight(stat->total_kwh);
  GY88128_648_SetIcon(ICON_UNIT_K_R_ADDR, ICON_UNIT_K_R_MASK, 1);
  GY88128_648_SetIcon(ICON_UNIT_W_R_ADDR, ICON_UNIT_W_R_MASK, 1);

  // Center: Screen ID 2 overrides Temp
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);
  Show_Screen_ID(2);
  GY88128_648_Refresh();
}

void Screen_Grid_Standby(SystemStatus_t *stat) {
  // Keeping as placeholder, redirected to Inverter_Only if needed
  Screen_Inverter_Only(stat);
}

void Screen_Inverter_Only(SystemStatus_t *stat) {
  GY88128_648_Clear();
  bool data_swap = (HAL_GetTick() / 2000) % 2 == 0; // 2s toggle
  bool led_on = (HAL_GetTick() / 1000) % 2 == 0;    // 1s toggle
  int cycle_state = (HAL_GetTick() / 2000) % 4;     // 4-state 2s cycle

  // ==========================================================================
  // 1. INPUT SIDE (Left)
  // ==========================================================================
  if (!stat->is_grid_present) {
    // Battery Voltage (Fixed in Grid-OFF)
    GY88128_648_PrintLeft((int)(stat->batt_volt * 10));
    GY88128_648_SetIcon(ICON_BATT_L_ADDR, ICON_BATT_L_MASK, 1);
    GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);
    GY88128_648_SetIcon(ICON_DOT2_ADDR, ICON_DOT2_MASK, 1);
  } else {
    // Alternate Volt / Hz every 2s (Grid-ON)
    if (data_swap) {
      GY88128_648_PrintLeft((int)stat->grid_volt);
      GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);
    } else {
      GY88128_648_PrintLeft((int)(stat->grid_freq * 10));
      GY88128_648_SetIcon(ICON_HZ1_ADDR, ICON_HZ1_MASK, 1);
      GY88128_648_SetIcon(ICON_DOT2_ADDR, ICON_DOT2_MASK, 1);
    }
  }
  GY88128_648_SetIcon(ICON_INPUT_ADDR, ICON_INPUT_MASK,
                      stat->is_grid_present); // S5
  GY88128_648_SetIcon(ICON_AC_TEXT_ADDR, ICON_AC_TEXT_MASK,
                      stat->is_grid_present); // S2
  GY88128_648_SetIcon(ICON_MAINS_TEXT_ADDR, ICON_MAINS_TEXT_MASK,
                      stat->is_grid_present);

  // ==========================================================================
  // 2. OUTPUT SIDE (Right)
  // ==========================================================================
  if (!stat->is_grid_present) {
    bool is_inv_on = stat->output_active;
    // 4-state cycle for Grid-OFF
    switch (cycle_state) {
    case 0: // OUTPUT V
      GY88128_648_PrintRight(is_inv_on ? (int)stat->inv_out_volt : 0);
      GY88128_648_SetIcon(ICON_OUTPUT_ADDR, ICON_OUTPUT_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_V_R_ADDR, ICON_UNIT_V_R_MASK, 1);
      break;
    case 1: // LOAD W
      GY88128_648_PrintRight(is_inv_on ? stat->inv_load_watt : 0);
      GY88128_648_SetIcon(ICON_LOAD_TXT_ADDR, ICON_LOAD_TXT_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_W_R_ADDR, ICON_UNIT_W_R_MASK, 1);
      break;
    case 2: // OUTPUT Hz
      GY88128_648_PrintRight(is_inv_on ? 500 : 0);
      GY88128_648_SetIcon(ICON_DOT5_ADDR, ICON_DOT5_MASK, 1);
      GY88128_648_SetIcon(ICON_OUTPUT_ADDR, ICON_OUTPUT_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_Hz_R_ADDR, ICON_UNIT_Hz_R_MASK, 1);
      break;
    case 3: // LOAD %
      GY88128_648_PrintRight(is_inv_on ? stat->inv_load_percent : 0);
      GY88128_648_SetIcon(ICON_LOAD_TXT_ADDR, ICON_LOAD_TXT_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_PCT_R_ADDR, ICON_UNIT_PCT_R_MASK, 1);
      break;
    }
  } else {
    // Alternate Bat Volt / Chg Amp every 2s for Grid-ON
    if (data_swap) {
      GY88128_648_PrintRight((int)(stat->batt_volt * 10));
      GY88128_648_SetIcon(ICON_UNIT_V_R_ADDR, ICON_UNIT_V_R_MASK, 1);
    } else {
      GY88128_648_PrintRight((int)(stat->chg_amp * 10));
      GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);
    }
    GY88128_648_SetIcon(ICON_DOT5_ADDR, ICON_DOT5_MASK, 1);
    GY88128_648_SetIcon(ICON_BATT_R_ADDR, ICON_BATT_R_MASK, 1);
  }

  // ==========================================================================
  // 3. CENTER & SHARED ICONS
  // ==========================================================================
  // Unified Dynamic Time Toggle (2s rate)
  bool show_hours = (cycle_state % 2 == 0);
  int total_min = stat->output_active ? stat->inv_run_time_total_min : 0;
  if (show_hours) {
    GY88128_648_PrintCenter(total_min / 60);
    GY88128_648_SetIcon(ICON_UNIT_H_L_ADDR, ICON_UNIT_H_L_MASK, 1);
    GY88128_648_SetIcon(ICON_UNIT_M_L_ADDR, ICON_UNIT_M_L_MASK, 0);
  } else {
    GY88128_648_PrintCenter(total_min % 60);
    GY88128_648_SetIcon(ICON_UNIT_H_L_ADDR, ICON_UNIT_H_L_MASK, 0);
    GY88128_648_SetIcon(ICON_UNIT_M_L_ADDR, ICON_UNIT_M_L_MASK, 1);
  }

  GY88128_648_SetIcon(ICON_CLOCK_ADDR, ICON_CLOCK_MASK, led_on); // Clock blink
  GY88128_648_SetIcon(ICON_BORDER_ADDR, ICON_BORDER_MASK, 1);    // L1 static ON
  GY88128_648_SetIcon(ICON_LOAD_LVL_ADDR, ICON_LOAD_LVL_MASK, 1);     // S30 ON
  GY88128_648_SetIcon(ICON_INV_TEXT_ADDR, ICON_INV_TEXT_MASK, 1);     // S36 ON
  GY88128_648_SetIcon(ICON_SYS_SWITCH_ADDR, ICON_SYS_SWITCH_MASK, 1); // S50 ON
  GY88128_648_SetIcon(ICON_LINE_ICON_ADDR, ICON_LINE_ICON_MASK,
                      stat->is_grid_present); // S45 LINE

  // ==========================================================================
  // 4. POWER FLOW
  // ==========================================================================
  if (!stat->is_grid_present) {
    GY88128_648_SetIcon(ICON_FLOW_BAT_ADDR, ICON_FLOW_BAT_MASK, 1); // S48 ON
    GY88128_648_SetIcon(ICON_ARROW_TO_LOAD_ADDR, ICON_ARROW_TO_LOAD_MASK,
                        stat->output_active); // S29
    GY88128_648_SetIcon(ICON_LINE_TO_LOAD_ADDR, ICON_LINE_TO_LOAD_MASK,
                        1); // S33 LINE >
    Update_Battery_Visuals(stat->batt_volt, stat->batt_type, false);
    Update_Load_Visuals(stat->output_active ? stat->inv_load_percent : 0);
  } else {
    GY88128_648_SetIcon(ICON_FLOW_MAINS_IN_ADDR, ICON_FLOW_MAINS_IN_MASK, 1);
    GY88128_648_SetIcon(ICON_FLOW_CHG_ADDR, ICON_FLOW_CHG_MASK, 1);
    GY88128_648_SetIcon(ICON_LINE_FOR_CHG_ADDR, ICON_LINE_FOR_CHG_MASK,
                        0); // S53 OFF
    GY88128_648_SetIcon(ICON_LINE_TO_LOAD_ADDR, ICON_LINE_TO_LOAD_MASK,
                        1); // S33 ON
    Update_Battery_Visuals(stat->batt_volt, stat->batt_type, stat->is_charging);
    Update_Load_Visuals(stat->inv_load_percent);
  }

  // Common overrides
  GY88128_648_SetIcon(ICON_BULB_ADDR, ICON_BULB_MASK, 1); // S31 forced ON
  GY88128_648_SetIcon(ICON_JUNCTION_ADDR, ICON_JUNCTION_MASK,
                      1);                                 // S32 forced ON
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1); // L2 Divider

  GY88128_648_Refresh();
}

void Screen_Fault_Overload(int load_percent) {
  GY88128_648_Clear();
  GY88128_648_SetIcon(ICON_WARNING_ADDR, ICON_WARNING_MASK, 1);
  GY88128_648_SetIcon(ICON_OVERLOAD_ADDR, ICON_OVERLOAD_MASK, 1);
  GY88128_648_PrintRight(load_percent);
  GY88128_648_SetIcon(ICON_UNIT_PCT_R_ADDR, ICON_UNIT_PCT_R_MASK, 1);
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);
  GY88128_648_SetIcon(ICON_BORDER_ADDR, ICON_BORDER_MASK, 0);
  Update_Load_Visuals(load_percent);
  GY88128_648_Refresh();
}

void Screen_Fault_LowBat(float batt_volt) {
  GY88128_648_Clear();
  GY88128_648_SetIcon(ICON_ERROR_ADDR, ICON_ERROR_MASK, 1);
  GY88128_648_SetIcon(ICON_BATT_L_ADDR, ICON_BATT_L_MASK, 1);
  GY88128_648_PrintLeft((int)(batt_volt * 10));
  GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);
  GY88128_648_SetIcon(ICON_BORDER_ADDR, ICON_BORDER_MASK, 0);
  GY88128_648_Refresh();
}

void Screen_Fault_OverHeat(int temp) {
  GY88128_648_Clear();
  GY88128_648_SetIcon(ICON_ERROR_ADDR, ICON_ERROR_MASK, 1);
  GY88128_648_PrintCenter(temp);
  GY88128_648_SetIcon(ICON_WRENCH_ADDR, ICON_WRENCH_MASK, 1);
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 0);
  GY88128_648_SetIcon(ICON_BORDER_ADDR, ICON_BORDER_MASK, 1);
  GY88128_648_Refresh();
}

/**
 * @brief MPPT Only Mode Display Scenarios
 * @param stat System status structure
 * @param scenario 1=Charging, 2=PV Idle, 3=No PV
 */
void Screen_MPPT_Only(SystemStatus_t *stat, int scenario) {
  GY88128_648_Clear();
  bool data_swap = (HAL_GetTick() / 2000) % 2 == 0; // 2s toggle

  // 1. Common Icons for MPPT Mode (Scenario 1 & 2)
  if (scenario == 1 || scenario == 2) {
    GY88128_648_SetIcon(ICON_PV1_ICON_ADDR, ICON_PV1_ICON_MASK, 1);
    GY88128_648_SetIcon(ICON_PV2_ICON_ADDR, ICON_PV2_ICON_MASK, 1);
    GY88128_648_SetIcon(ICON_PV1_TEXT_ADDR, ICON_PV1_TEXT_MASK,
                        1); // S4 PV1 Text
    GY88128_648_SetIcon(ICON_PV2_TEXT_ADDR, ICON_PV2_TEXT_MASK,
                        1); // S3 PV2 Text
    GY88128_648_SetIcon(ICON_PV_BIG_PV1_ADDR, ICON_PV_BIG_PV1_MASK, 1);
    GY88128_648_SetIcon(ICON_PV_BIG_PV2_ADDR, ICON_PV_BIG_PV2_MASK, 1);
    GY88128_648_SetIcon(ICON_MPPT_TEXT_ADDR, ICON_MPPT_TEXT_MASK, 1);
    GY88128_648_SetIcon(ICON_LINE_FOR_CHG_ADDR, ICON_LINE_FOR_CHG_MASK,
                        1); // S53
    GY88128_648_SetIcon(ICON_ARROW_MPPT_BAT_ADDR, ICON_ARROW_MPPT_BAT_MASK,
                        0); // PVT1 OFF
  }

  // 2. INPUT SIDE (Left) - PV Voltage
  if (scenario == 3) {
    GY88128_648_PrintLeft(110); // Specification says 110V for no PV
  } else {
    GY88128_648_PrintLeft((int)stat->solar_volt);
  }
  GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);
  GY88128_648_SetIcon(ICON_INPUT_ADDR, ICON_INPUT_MASK, 1);

  // 3. OUTPUT SIDE (Right) - Batt Volt / Charging Amp
  if (scenario == 1) {
    // Charging: Toggle between Volt and AMP every 2s
    if (data_swap) {
      GY88128_648_PrintRight((int)(stat->batt_volt * 10));
      GY88128_648_SetIcon(ICON_UNIT_V_R_ADDR, ICON_UNIT_V_R_MASK, 1);
      GY88128_648_SetIcon(ICON_DOT5_ADDR, ICON_DOT5_MASK, 1);
    } else {
      GY88128_648_PrintRight((int)(stat->chg_amp * 10));
      GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);
      GY88128_648_SetIcon(ICON_DOT5_ADDR, ICON_DOT5_MASK, 1);
    }
  } else {
    // Idle or No PV: Only BATT Voltage
    GY88128_648_PrintRight((int)(stat->batt_volt * 10));
    GY88128_648_SetIcon(ICON_UNIT_V_R_ADDR, ICON_UNIT_V_R_MASK, 1);
    GY88128_648_SetIcon(ICON_DOT5_ADDR, ICON_DOT5_MASK, 1);
  }
  GY88128_648_SetIcon(ICON_BATT_R_ADDR, ICON_BATT_R_MASK, 1);
  GY88128_648_SetIcon(ICON_OUTPUT_ADDR, ICON_OUTPUT_MASK, 1);

  // 4. BATTERY VISUALS
  // Bars blink in scenario 1, steady in 2 & 3
  Update_Battery_Visuals(stat->batt_volt, stat->batt_type, (scenario == 1));

  // 5. CENTER DISPLAY (Sync with System LED)
  GY88128_648_PrintCenter(scenario); // Show Scenario ID 1, 2, or 3
  GY88128_648_SetIcon(ICON_BORDER_ADDR, ICON_BORDER_MASK, 1); // L1 Frame

  // 6. CLEAR MAINS & LOAD ICONS
  GY88128_648_SetIcon(ICON_LOAD_LVL_ADDR, ICON_LOAD_LVL_MASK, 0); // S30 OFF
  GY88128_648_SetIcon(ICON_MAINS_TEXT_ADDR, ICON_MAINS_TEXT_MASK, 0);
  GY88128_648_SetIcon(ICON_AC_TEXT_ADDR, ICON_AC_TEXT_MASK, 0);
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1); // L2 divider

  GY88128_648_Refresh();
}

/**
 * @brief Hybrid Mode Display Scenarios
 * @param stat System status structure
 * @param scenario 1=Dual Charging, 2=Solar Priority, 3=Grid Bypass, 4=PV Only
 */
void Screen_Hybrid_Mode(SystemStatus_t *stat, int scenario) {
  GY88128_648_Clear();
  uint32_t current_tick = HAL_GetTick();
  bool data_swap = (current_tick / 2000) % 2 == 0;
  int cycle4 = (current_tick / 2000) % 4; // 4-state cycle for scenario 2

  // 1. DATA LAYOUT (Scenarios 1, 3, 4 use 2-state swap; Scenario 2 uses 4-state
  // cycle)
  if (scenario == 2) {
    switch (cycle4) {
    case 0: // State 0: PV Volt | Output Volt
      GY88128_648_PrintLeft((int)stat->solar_volt);
      GY88128_648_SetIcon(ICON_PV1_TEXT_ADDR, ICON_PV1_TEXT_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);

      GY88128_648_PrintRight((int)stat->inv_out_volt);
      GY88128_648_SetIcon(ICON_UNIT_V_R_ADDR, ICON_UNIT_V_R_MASK, 1);
      break;

    case 1: // State 1: PV Amp | MPPT Output Amp (36.0A)
      GY88128_648_PrintLeft((int)(stat->solar_amp * 10));
      GY88128_648_SetIcon(ICON_PV1_TEXT_ADDR, ICON_PV1_TEXT_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_A_L_ADDR, ICON_UNIT_A_L_MASK, 1);
      GY88128_648_SetIcon(ICON_DOT2_ADDR, ICON_DOT2_MASK, 1);

      // Uses MPPT to Inverter current as requested
      GY88128_648_PrintRight((int)(stat->mppt_out_amp * 10));
      GY88128_648_SetIcon(ICON_OUTPUT_ADDR, ICON_OUTPUT_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);
      GY88128_648_SetIcon(ICON_DOT5_ADDR, ICON_DOT5_MASK, 1);
      break;

    case 2: // State 2: PV kVA | MPPT Output kVA (1.71)
      float pv_kva = (stat->solar_volt * stat->solar_amp) / 1000.0f;
      GY88128_648_PrintLeft((int)(pv_kva * 100)); // 1.80
      GY88128_648_SetIcon(ICON_PV1_TEXT_ADDR, ICON_PV1_TEXT_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_K_L_ADDR, ICON_UNIT_K_L_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_A_L_ADDR, ICON_UNIT_A_L_MASK, 1);
      GY88128_648_SetIcon(ICON_DOT1_ADDR, ICON_DOT1_MASK, 1);
      GY88128_648_SetIcon(ICON_DOT2_ADDR, ICON_DOT2_MASK, 0);

      float out_kva = stat->mppt_out_watt / 1000.0f;
      GY88128_648_PrintRight((int)(out_kva * 100)); // 1.71
      GY88128_648_SetIcon(ICON_UNIT_K_R_ADDR, ICON_UNIT_K_R_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_V_R_ADDR, ICON_UNIT_V_R_MASK, 1);
      GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);
      GY88128_648_SetIcon(ICON_DOT4_ADDR, ICON_DOT4_MASK, 1);
      GY88128_648_SetIcon(ICON_DOT5_ADDR, ICON_DOT5_MASK, 0);
      break;

    case 3: // State 3: Batt Volt | Batt Amp
      GY88128_648_PrintLeft((int)(stat->batt_volt * 10));
      GY88128_648_SetIcon(ICON_BATT_L_ADDR, ICON_BATT_L_MASK, 1);
      GY88128_648_SetIcon(ICON_INPUT_ADDR, ICON_INPUT_MASK, 0); // S5 OFF
      GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);
      GY88128_648_SetIcon(ICON_DOT2_ADDR, ICON_DOT2_MASK, 1);

      GY88128_648_PrintRight((int)(stat->chg_amp * 10));
      GY88128_648_SetIcon(ICON_BATT_R_ADDR, ICON_BATT_R_MASK, 1);
      GY88128_648_SetIcon(ICON_OUTPUT_ADDR, ICON_OUTPUT_MASK, 0); // S19 OFF
      GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);
      GY88128_648_SetIcon(ICON_DOT5_ADDR, ICON_DOT5_MASK, 1);
      break;
    }
    // Set labels for non-battery states
    if (cycle4 != 3) {
      GY88128_648_SetIcon(ICON_INPUT_ADDR, ICON_INPUT_MASK, 1);
      GY88128_648_SetIcon(ICON_OUTPUT_ADDR, ICON_OUTPUT_MASK, 1);
    }
  } else {
    // 1. INPUT SIDE (Left) for scenarios 1, 3, 4
    switch (scenario) {
    case 1: // Dual Charge: Toggle AC/PV
      if (data_swap) {
        GY88128_648_PrintLeft((int)stat->grid_volt);
        GY88128_648_SetIcon(ICON_AC_TEXT_ADDR, ICON_AC_TEXT_MASK, 1);
      } else {
        GY88128_648_PrintLeft((int)stat->solar_volt);
        GY88128_648_SetIcon(ICON_PV1_TEXT_ADDR, ICON_PV1_TEXT_MASK, 1);
      }
      break;
    case 4: // PV Only charge
      GY88128_648_PrintLeft((int)stat->solar_volt);
      GY88128_648_SetIcon(ICON_PV1_TEXT_ADDR, ICON_PV1_TEXT_MASK, 1);
      break;
    case 3: // Grid Bypass: AC Only
      GY88128_648_PrintLeft((int)stat->grid_volt);
      GY88128_648_SetIcon(ICON_AC_TEXT_ADDR, ICON_AC_TEXT_MASK, 1);
      break;
    }
    GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);
    GY88128_648_SetIcon(ICON_INPUT_ADDR, ICON_INPUT_MASK, 1);

    // 2. OUTPUT SIDE (Right) for scenarios 1, 3, 4
    switch (scenario) {
    case 1: // Dual Charge: Batt V / Combined Chg Amp
      if (data_swap) {
        GY88128_648_PrintRight((int)(stat->batt_volt * 10));
        GY88128_648_SetIcon(ICON_UNIT_V_R_ADDR, ICON_UNIT_V_R_MASK, 1);
        GY88128_648_SetIcon(ICON_BATT_R_ADDR, ICON_BATT_R_MASK, 1);
      } else {
        GY88128_648_PrintRight((int)(stat->chg_amp * 10));
        GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);
      }
      break;
    case 3: // Grid Bypass: Load W / Chg Amp
      if (data_swap) {
        GY88128_648_PrintRight(stat->inv_load_watt);
        GY88128_648_SetIcon(ICON_UNIT_W_R_ADDR, ICON_UNIT_W_R_MASK, 1);
        GY88128_648_SetIcon(ICON_LOAD_TXT_ADDR, ICON_LOAD_TXT_MASK, 1);
      } else {
        GY88128_648_PrintRight((int)(stat->chg_amp * 10));
        GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);
      }
      break;
    case 4: // PV Only: Batt V / Chg Amp
      if (data_swap) {
        GY88128_648_PrintRight((int)(stat->batt_volt * 10));
        GY88128_648_SetIcon(ICON_UNIT_V_R_ADDR, ICON_UNIT_V_R_MASK, 1);
        GY88128_648_SetIcon(ICON_BATT_R_ADDR, ICON_BATT_R_MASK, 1);
      } else {
        GY88128_648_PrintRight((int)(stat->chg_amp * 10));
        GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);
      }
      break;
    }
    GY88128_648_SetIcon(ICON_DOT5_ADDR, ICON_DOT5_MASK, 1);
    GY88128_648_SetIcon(ICON_OUTPUT_ADDR, ICON_OUTPUT_MASK, 1);
  }

  // 3. ICONS & FLOWS
  Update_Battery_Visuals(stat->batt_volt, stat->batt_type, stat->is_charging);

  // Forced Icons (S30, S31, S33, MPPT, INV always ON as per user request)
  GY88128_648_SetIcon(ICON_LOAD_LVL_ADDR, ICON_LOAD_LVL_MASK, 1);         // S30
  GY88128_648_SetIcon(ICON_BULB_ADDR, ICON_BULB_MASK, 1);                 // S31
  GY88128_648_SetIcon(ICON_LINE_TO_LOAD_ADDR, ICON_LINE_TO_LOAD_MASK, 1); // S33
  GY88128_648_SetIcon(ICON_MPPT_TEXT_ADDR, ICON_MPPT_TEXT_MASK, 1);       // S40
  GY88128_648_SetIcon(ICON_INV_TEXT_ADDR, ICON_INV_TEXT_MASK, 1);         // S36

  // Load Visuals (Bars and Flow Arrows) only if Inverter/Output is active
  if (stat->output_active) {
    Update_Load_Visuals(stat->inv_load_percent);
    GY88128_648_SetIcon(ICON_ARROW_TO_LOAD_ADDR, ICON_ARROW_TO_LOAD_MASK,
                        1);                                               // S29
    GY88128_648_SetIcon(ICON_FLOW_AC_OUT_ADDR, ICON_FLOW_AC_OUT_MASK, 1); // S50
  } else {
    // Clear bars and output flow arrows, keep labels/bulb/inverter ON
    GY88128_648_SetIcon(ICON_LOAD_BAR_ADDR, ICON_LOAD_BAR_MASK, 0);
    GY88128_648_SetIcon(ICON_ARROW_TO_LOAD_ADDR, ICON_ARROW_TO_LOAD_MASK, 0);
    GY88128_648_SetIcon(ICON_FLOW_AC_OUT_ADDR, ICON_FLOW_AC_OUT_MASK, 0);
  }

  // System is active, always show Junction and Border
  GY88128_648_SetIcon(ICON_JUNCTION_ADDR, ICON_JUNCTION_MASK, 1); // S32
  GY88128_648_SetIcon(ICON_BORDER_ADDR, ICON_BORDER_MASK, 1);     // L1
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);         // L2 divider

  // Solar Icons (Common if solar present)
  if (stat->is_solar_present) {
    GY88128_648_SetIcon(ICON_PV1_ICON_ADDR, ICON_PV1_ICON_MASK, 1);
    GY88128_648_SetIcon(ICON_PV2_ICON_ADDR, ICON_PV2_ICON_MASK, 1);
    GY88128_648_SetIcon(ICON_PV_BIG_PV1_ADDR, ICON_PV_BIG_PV1_MASK, 1);
    GY88128_648_SetIcon(ICON_LINE_FOR_CHG_ADDR, ICON_LINE_FOR_CHG_MASK, 1);
    GY88128_648_SetIcon(ICON_ARROW_MPPT_BAT_ADDR, ICON_ARROW_MPPT_BAT_MASK, 1);
  }

  // Mains Icons
  if (stat->is_grid_present) {
    GY88128_648_SetIcon(ICON_MAINS_TEXT_ADDR, ICON_MAINS_TEXT_MASK, 1);
    GY88128_648_SetIcon(ICON_LINE_ICON_ADDR, ICON_LINE_ICON_MASK, 1);
    GY88128_648_SetIcon(ICON_FLOW_MAINS_IN_ADDR, ICON_FLOW_MAINS_IN_MASK, 1);
  }

  // Charging Flow and Source
  if (stat->is_charging) {
    GY88128_648_SetIcon(ICON_FLOW_CHG_ADDR, ICON_FLOW_CHG_MASK, 1); // S49
    GY88128_648_SetIcon(ICON_FLOW_BAT_ADDR, ICON_FLOW_BAT_MASK,
                        1); // S48 (Source)
  } else if (stat->output_active) {
    GY88128_648_SetIcon(ICON_FLOW_BAT_ADDR, ICON_FLOW_BAT_MASK,
                        1); // S48 (Source)
  }

  // Center Screen ID
  GY88128_648_PrintCenter(scenario);

  GY88128_648_Refresh();
}

void Screen_Charge_PV_Priority(SystemStatus_t *stat) {
  GY88128_648_Clear();

  // Header: PV Volts (D1-D3) + PV1 Icon + Unit V
  GY88128_648_PrintLeft((int)(stat->solar_volt * 10));
  GY88128_648_SetIcon(ICON_DOT2_ADDR, ICON_DOT2_MASK, 1);
  GY88128_648_SetIcon(ICON_PV1_TEXT_ADDR, ICON_PV1_TEXT_MASK, 1);
  GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);

  // Footer: Batt Charging current (D6-D8) + Unit A
  GY88128_648_PrintRight((int)(stat->chg_amp * 10));
  GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);

  // Center: Batt Volt (D4-D5)
  // Center: Screen ID 3 overrides Batt Volt
  // GY88128_648_PrintCenter((int)stat->batt_volt);

  Update_Battery_Visuals(stat->batt_volt, stat->batt_type, stat->is_charging);
  Update_Power_Flow(stat);

  // Forced elements for this specific mode
  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);
  Show_Screen_ID(3);
  GY88128_648_Refresh();
}

void Screen_Charge_Mains_Priority(SystemStatus_t *stat) {
  GY88128_648_Clear();

  // Header: AC Input Volt (D1-D3) + AC Text + Unit V
  GY88128_648_PrintLeft((int)stat->grid_volt);
  GY88128_648_SetIcon(ICON_AC_TEXT_ADDR, ICON_AC_TEXT_MASK, 1);
  GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);

  // Footer: Batt Charging current (D6-D8) + Unit A
  GY88128_648_PrintRight((int)(stat->chg_amp * 10));
  GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);

  // Center: Screen ID 4 overrides Batt Volt
  // GY88128_648_PrintCenter((int)stat->batt_volt);

  Update_Battery_Visuals(stat->batt_volt, stat->batt_type, stat->is_charging);
  Update_Power_Flow(stat);

  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);
  Show_Screen_ID(4);
  GY88128_648_Refresh();
}

void Screen_Charge_Hybrid(SystemStatus_t *stat) {
  GY88128_648_Clear();

  // Header: PV Volts
  GY88128_648_PrintLeft((int)(stat->solar_volt * 10));
  GY88128_648_SetIcon(ICON_DOT2_ADDR, ICON_DOT2_MASK, 1);
  GY88128_648_SetIcon(ICON_PV1_TEXT_ADDR, ICON_PV1_TEXT_MASK, 1);
  GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);

  // Footer: Combined Charging Current
  GY88128_648_PrintRight((int)(stat->chg_amp * 10));
  GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);

  Update_Battery_Visuals(stat->batt_volt, stat->batt_type, stat->is_charging);
  Update_Power_Flow(stat); // Handles dual flow if both flags set

  Update_Power_Flow(stat); // Handles dual flow if both flags set

  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);
  Show_Screen_ID(5);
  GY88128_648_Refresh();
}

void Screen_Charge_Only_Solar(SystemStatus_t *stat) {
  GY88128_648_Clear();

  // Force grid off for display logic
  bool original_grid_state = stat->is_grid_present;
  stat->is_grid_present = false;

  // Header: PV Volts (D1-D3) + PV1 Icon + Unit V
  GY88128_648_PrintLeft((int)(stat->solar_volt * 10));
  GY88128_648_SetIcon(ICON_DOT2_ADDR, ICON_DOT2_MASK, 1);
  GY88128_648_SetIcon(ICON_PV1_TEXT_ADDR, ICON_PV1_TEXT_MASK, 1);
  GY88128_648_SetIcon(ICON_UNIT_V_L_ADDR, ICON_UNIT_V_L_MASK, 1);

  // Footer: Batt Charging current (D6-D8) + Unit A
  GY88128_648_PrintRight((int)(stat->chg_amp * 10));
  GY88128_648_SetIcon(ICON_UNIT_A_R_ADDR, ICON_UNIT_A_R_MASK, 1);

  Update_Battery_Visuals(stat->batt_volt, stat->batt_type, stat->is_charging);
  Update_Power_Flow(stat);

  GY88128_648_SetIcon(ICON_LINE_ADDR, ICON_LINE_MASK, 1);
  Show_Screen_ID(6);
  GY88128_648_Refresh();

  stat->is_grid_present = original_grid_state; // Restore
}

void GY88128_648_TestCenter(void) {
  GY88128_648_Clear();
  // Light up S10, S11, S12, S13 (Addresses 10-13)
  // Addresses 10,11 are in Byte 5
  // Addresses 12,13 are in Byte 6
  ht1621_ram[5] = 0xFF;
  ht1621_ram[6] = 0xFF;
  GY88128_648_Refresh();
}

void GY88128_648_TestDemo(void) {
  SystemStatus_t demo_stat = {.grid_volt = 220.0f,
                              .grid_freq = 50.0f,
                              .batt_volt = 12.8f,
                              .chg_amp = 10.5f,
                              .inv_out_volt = 230.0f,
                              .inv_load_watt = 500,
                              .inv_load_percent = 45,
                              .solar_volt = 35.0f,
                              .solar_amp = 8.5f,
                              .day_kwh = 2.5f,
                              .total_kwh = 150,
                              .sys_temp = 42,
                              .is_charging = true,
                              .is_grid_present = true,
                              .is_solar_present = true,
                              .output_active = true,
                              .batt_type = 0};
  GY88128_648_TurnOnAllSegments();
  HAL_Delay(2000);
  GY88128_648_Clear();
  HAL_Delay(1000);
  Screen_Inv_Home(&demo_stat);
  HAL_Delay(3000);
  Screen_Grid_Detail(&demo_stat);
  HAL_Delay(3000);
  Screen_Batt_Charging(&demo_stat);
  HAL_Delay(3000);
  Screen_Load_Detail(&demo_stat);
  HAL_Delay(3000);
  Screen_Solar_Home(&demo_stat);
  HAL_Delay(3000);
  Screen_Solar_Yield(&demo_stat);
  HAL_Delay(3000);
  Screen_Charge_PV_Priority(&demo_stat);
  HAL_Delay(3000);
  Screen_Charge_Mains_Priority(&demo_stat);
  HAL_Delay(3000);
  Screen_Charge_Hybrid(&demo_stat);
  HAL_Delay(3000);
  Screen_Fault_Overload(150);
  HAL_Delay(2000);
  Screen_Fault_LowBat(10.5f);
  HAL_Delay(2000);
  Screen_Fault_OverHeat(85);
  HAL_Delay(2000);
  demo_stat.is_grid_present = false;
  Screen_Inv_Home(&demo_stat);
  HAL_Delay(3000);
  GY88128_648_Clear();
}

void GY88128_648_ManualTesterLoop(void) {
  // 1. Initialize PC6 as Input with Pull-up
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitStruct.Pin = HT1621_MANUAL_SCAN_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(HT1621_MANUAL_SCAN_GPIO_Port, &GPIO_InitStruct);

  uint8_t test_addr = 0;
  uint8_t test_com = 0;
  bool last_btn_state = true;
  bool update_required = true;

  while (1) {
    if (update_required) {
      // 2. Update Display
      GY88128_648_Clear();
      set_ram_bit(test_addr, test_com, true); // Light up ONLY current bit
      GY88128_648_PrintLeft(test_addr);       // Show S0-S31 on left
      GY88128_648_PrintCenter(test_com);      // Show C0-C3 in center
      GY88128_648_Refresh();
      update_required = false;
    }

    // 3. Wait for Button Press (Falling Edge)
    bool btn_state =
        HAL_GPIO_ReadPin(HT1621_MANUAL_SCAN_GPIO_Port, HT1621_MANUAL_SCAN_PIN);

    if (last_btn_state == true && btn_state == false) {
      // Button pressed - Debounce
      HAL_Delay(50);
      // Wait for release
      while (HAL_GPIO_ReadPin(HT1621_MANUAL_SCAN_GPIO_Port,
                              HT1621_MANUAL_SCAN_PIN) == false)
        ;
      HAL_Delay(50);

      // 4. Increment Logic
      test_com++;
      if (test_com > 3) {
        test_com = 0;
        test_addr++;
        if (test_addr > 31)
          test_addr = 0;
      }
      update_required = true;
    }
    last_btn_state = btn_state;
    HAL_Delay(10); // Polling delay
  }
}
void GY88128_648_FullTestSequence(void) {
  // 1. Clear display
  GY88128_648_Clear();
  GY88128_648_Refresh();
  HAL_Delay(500);

  // 2. All segments ON (2 seconds)
  GY88128_648_TurnOnAllSegments();
  HAL_Delay(2000);

  // 3. Digit cycle 0-9 (sync) - Slowed down to 800ms
  for (int i = 0; i <= 9; i++) {
    GY88128_648_Clear();
    GY88128_648_PrintLeft(i * 111);
    GY88128_648_PrintCenter(i * 11);
    GY88128_648_PrintRight(i * 111);
    GY88128_648_Refresh();
    HAL_Delay(800);
  }

  // 4. Cumulative Snake Scan (Segments stay ON)
  GY88128_648_CumulativeSnakeScan(20);

  // 5. Hold all segments ON for 2 seconds
  HAL_Delay(2000);

  // 6. Final Clear
  GY88128_648_Clear();
  GY88128_648_Refresh();
  HAL_Delay(500);
}

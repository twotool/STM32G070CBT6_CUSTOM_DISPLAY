/**
 * @file GY88128_648_GREEN_SEGMENT_LCD_DRIVER.h
 * @brief LCD Driver for GY88128-648 Green UPS Display with HT1621
 *
 * Pin Configuration:
 * PC7  (PIN31) - DATA
 * PA10 (PIN32) - WR
 * PA11 (PIN33) - CS
 *
 * Hardware Note:
 * This LCD uses a Split-Segment topology.
 * - Odd Pins (Seg N):   F, G, E, D
 * - Even Pins (Seg N+1): A, B, C, Icon
 */

#ifndef GY88128_648_GREEN_SEGMENT_LCD_DRIVER_H
#define GY88128_648_GREEN_SEGMENT_LCD_DRIVER_H

#include <stdbool.h>
#include <stdint.h>
#include <stm32g0xx_hal.h> // Ensure this matches your MCU

// ============================================================================
// HT1621 Pin Definitions
// ============================================================================
#define HT1621_648_CS_GPIO_Port GPIOA
#define HT1621_648_CS_PIN GPIO_PIN_11
#define HT1621_648_WR_GPIO_Port GPIOA
#define HT1621_648_WR_PIN GPIO_PIN_10
#define HT1621_648_DATA_GPIO_Port GPIOC
#define HT1621_648_DATA_PIN GPIO_PIN_7
#define HT1621_MANUAL_SCAN_GPIO_Port GPIOC
#define HT1621_MANUAL_SCAN_PIN GPIO_PIN_6

// ============================================================================
// System Status Data Structure
// ============================================================================
typedef struct {
  float grid_volt;            // Grid/Mains voltage
  float grid_freq;            // Grid frequency
  float batt_volt;            // Battery voltage
  float chg_amp;              // Charging current
  float inv_out_volt;         // Inverter output voltage
  int inv_load_watt;          // Load in watts
  float solar_volt;           // Solar/PV voltage
  float solar_amp;            // Solar current
  float mppt_out_amp;         // MPPT output current to Inv/Batt
  float mppt_out_watt;        // MPPT output power in watts
  float day_kwh;              // Daily energy yield
  int total_kwh;              // Total energy yield
  int sys_temp;               // System temperature
  int inv_load_percent;       // Load percentage (0-100)
  int inv_run_time_total_min; // Inverter cumulative run-time in minutes

  // Status Flags
  bool is_charging;
  bool is_grid_present;
  bool is_solar_present;
  bool output_active;
  uint8_t batt_type; // 0=FLD, 1=USER, 2=AGM
} SystemStatus_t;

// ============================================================================
// Initialization & Control Functions
// ============================================================================
void GY88128_648_Init(void);
void GY88128_648_Clear(void);
void GY88128_648_Refresh(void);
void GY88128_648_TurnOnAllSegments(void);
void GY88128_648_ScanAllSegments(uint16_t dwell_ms);
void GY88128_648_CumulativeSnakeScan(uint16_t dwell_ms);
void GY88128_648_ScanByAddress(uint16_t dwell_ms);
void GY88128_648_TestAddressRange(uint8_t start_addr, uint8_t end_addr,
                                  uint16_t dwell_ms);
void GY88128_648_WriteRAM(uint8_t addr, uint8_t value);
void GY88128_648_TestSingleDigit(void); // Debug function to test digit mapping

// ============================================================================
// Low-Level Segment Control
// ============================================================================
void GY88128_648_SetSegment(uint8_t seg, bool on);
void GY88128_648_SetDigitSegment(uint8_t digit, uint8_t seg, bool on);

// ============================================================================
// Number Display Functions
// ============================================================================
void GY88128_648_PrintLeft(int value);   // 3-digit display on left
void GY88128_648_PrintCenter(int value); // 2-digit display in center
void GY88128_648_PrintRight(int value);  // 3-digit display on right

// ============================================================================
// Icon Control Functions
// ============================================================================
void GY88128_648_SetIcon(uint8_t addr, uint8_t mask, bool on);

// ============================================================================
// Screen Display Functions
// ============================================================================
void Screen_Inv_Home(SystemStatus_t *stat);      // Inverter Home Screen
void Screen_Grid_Detail(SystemStatus_t *stat);   // Grid Voltage + Frequency
void Screen_Batt_Charging(SystemStatus_t *stat); // Battery Charging Screen
void Screen_Load_Detail(SystemStatus_t *stat);   // Load Detail Screen
void Screen_Solar_Home(SystemStatus_t *stat);    // Solar/MPPT Home
void Screen_Solar_Yield(SystemStatus_t *stat);   // Solar Yield (kWh)
void Screen_Charge_PV_Priority(SystemStatus_t *stat);
void Screen_Charge_Mains_Priority(SystemStatus_t *stat);
void Screen_Charge_Hybrid(SystemStatus_t *stat);
void Screen_Charge_Only_Solar(SystemStatus_t *stat);
void Screen_Grid_Standby(SystemStatus_t *stat);  // Standby / OFF Screen
void Screen_Inverter_Only(SystemStatus_t *stat); // Detailed Inverter Only Mode
void Screen_MPPT_Only(SystemStatus_t *stat,
                      int scenario); // MPPT Only Mode Scenarios
void Screen_Hybrid_Mode(SystemStatus_t *stat,
                        int scenario); // Hybrid Mode Scenarios

// ============================================================================
// Fault Display Functions
// ============================================================================
void Screen_Fault_Overload(int load_percent);
void Screen_Fault_LowBat(float batt_volt);
void Screen_Fault_OverHeat(int temp);

// ============================================================================
// Test Function
// ============================================================================
void GY88128_648_TestDemo(void);
void GY88128_648_TestCenter(void);
void GY88128_648_FullTestSequence(void);
void GY88128_648_ManualTesterLoop(void);

#endif // GY88128_648_GREEN_SEGMENT_LCD_DRIVER_H

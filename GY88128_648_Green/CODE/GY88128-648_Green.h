#ifndef SCREENS_H
#define SCREENS_H

#include <stdint.h>
#include <stdbool.h>

// Data structure to hold all system state (populated by your main loop)
typedef struct {
    float grid_volt;     // e.g., 220.0
    float grid_freq;     // e.g., 50.0
    float batt_volt;     // e.g., 12.6
    float chg_amp;       // e.g., 10.5
    float inv_out_volt;  // e.g., 220
    int   inv_load_watt; // e.g., 150
    float solar_volt;    // e.g., 35.0
    float solar_amp;     // e.g., 5.5
    float day_kwh;       // e.g., 1.2
    int   total_kwh;     // e.g., 150
    int   sys_temp;      // e.g., 35
    
    // Status Flags
    bool  is_charging;
    bool  is_grid_present;
    bool  is_solar_present;
    bool  output_active;
    uint8_t batt_type;   // 0=FLD (Tubular), 1=USER (Li/Other), 2=AGM
} SystemStatus_t;

// Screen Functions
void Screen_Inv_Home(SystemStatus_t *stat);      // Replaces Screen 0, 6
void Screen_Grid_Detail(SystemStatus_t *stat);   // Replaces Screen 4
void Screen_Batt_Charging(SystemStatus_t *stat); // Replaces Screen 3
void Screen_Load_Detail(SystemStatus_t *stat);   // Replaces Screen 7
void Screen_Solar_Home(SystemStatus_t *stat);    // Replaces MPPT Screen 0, 1
void Screen_Solar_Yield(SystemStatus_t *stat);   // Replaces MPPT Screen 2, 3
void Screen_Solar_Input(SystemStatus_t *stat);   // Replaces MPPT Screen 4

// Fault Overrides (Call these immediately when a fault occurs)
void Screen_Fault_Overload(int load_percent);
void Screen_Fault_LowBat(float batt_volt);
void Screen_Fault_OverHeat(int temp);

#endif
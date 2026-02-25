#include "screens.h"
#include "GY88128.h" // Your driver file

// ============================================================================
// EXTENDED ICON DEFINITIONS (Flow Arrows & Modes)
// ============================================================================
// Add these to GY88128.h if not present. Derived from Datasheet pg 3-4.
// Note: Addresses are hypothetical based on pinout; verify with hardware test.

#define ICON_ARROW_AC_LOAD    25, 0x08  // AC -> Load flow
#define ICON_ARROW_BAT_LOAD   27, 0x08  // Batt -> Load flow
#define ICON_ARROW_PV_BAT     30, 0x08  // PV -> Batt flow (Charging)
#define ICON_ARROW_AC_BAT     31, 0x08  // AC -> Batt flow (Charging)

#define ICON_TYPE_AGM         35, 0x01  // AGM Text
#define ICON_TYPE_FLD         35, 0x02  // FLD Text (Tubular)
#define ICON_TYPE_USER        35, 0x04  // USER Text (Lithium)

#define ICON_OVERLOAD_TXT     26, 0x04  // "OVER LOAD" text
#define ICON_KWH              22, 0x04  // "kWh" text

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Updates the Battery Icon and Bar Graph based on voltage
static void Update_Battery_Visuals(float volt, uint8_t type, bool charging) {
    LCD_SetIcon(ICON_BAT_OUT, 0x08, 1); // Frame always on
    
    // Bar Graph Logic (Approximate for 12V system)
    LCD_SetIcon(ICON_BAT_BAR1, 0x01, (volt > 11.0));
    LCD_SetIcon(ICON_BAT_BAR2, 0x02, (volt > 12.0));
    LCD_SetIcon(ICON_BAT_BAR3, 0x04, (volt > 12.6));
    LCD_SetIcon(ICON_BAT_BAR4, 0x08, (volt > 13.5));

    // Battery Type Text
    LCD_SetIcon(ICON_TYPE_FLD, 0xFF, 0); // Clear all types first
    if(type == 0) LCD_SetIcon(ICON_TYPE_FLD, 0x02, 1);  // TUB -> FLD
    else if(type == 1) LCD_SetIcon(ICON_TYPE_USER, 0x04, 1); // LI -> USER
    else LCD_SetIcon(ICON_TYPE_AGM, 0x01, 1);
}

// Updates Flow Arrows based on power direction
static void Update_Power_Flow(SystemStatus_t *stat) {
    // Grid to Load (Bypass Mode)
    if(stat->is_grid_present && stat->output_active) {
        LCD_SetIcon(ICON_AC_GRID, 0x08, 1);
        LCD_SetIcon(ICON_ARROW_AC_LOAD, 0x08, 1); 
    }
    
    // Battery to Load (Inverter Mode)
    if(!stat->is_grid_present && stat->output_active) {
        LCD_SetIcon(ICON_ARROW_BAT_LOAD, 0x08, 1);
    }

    // Charging Flows
    if(stat->is_charging) {
        if(stat->is_solar_present) LCD_SetIcon(ICON_ARROW_PV_BAT, 0x08, 1);
        if(stat->is_grid_present)  LCD_SetIcon(ICON_ARROW_AC_BAT, 0x08, 1);
    }
}

// ============================================================================
// SCREEN IMPLEMENTATIONS
// ============================================================================

// ----------------------------------------------------------------------------
// Screen 0 & 6: Inverter Home / Idle
// Displays: Grid V (if avail) or Batt V (Left) | Output V (Right) | Batt Level
// ----------------------------------------------------------------------------
void Screen_Inv_Home(SystemStatus_t *stat) {
    GY88128_Clear();
    
    // Left: Priority to Grid Voltage, else Battery Voltage
    if(stat->is_grid_present) {
        LCD_PrintLeft((int)stat->grid_volt);
        LCD_SetIcon(ICON_INPUT, 0x08, 1);
        LCD_SetIcon(ICON_UNIT_V_L, 0x08, 1);
    } else {
        LCD_PrintLeft((int)(stat->batt_volt * 10)); // Show 12.6 as 126
        // Note: Manually enable DOT2 for decimal here if needed
        LCD_SetIcon(ICON_BATT_TXT, 0x08, 1);
        LCD_SetIcon(ICON_UNIT_V_L, 0x08, 1);
    }

    // Right: Output Voltage
    LCD_PrintRight((int)stat->inv_out_volt);
    LCD_SetIcon(ICON_OUTPUT, 0x08, 1);
    LCD_SetIcon(ICON_UNIT_V_R, 0x08, 1);

    Update_Battery_Visuals(stat->batt_volt, stat->batt_type, stat->is_charging);
    Update_Power_Flow(stat);
    GY88128_Refresh();
}

// ----------------------------------------------------------------------------
// Screen 4: Grid Detail
// Displays: Grid Voltage (Left) | Grid Frequency (Right)
// ----------------------------------------------------------------------------
void Screen_Grid_Detail(SystemStatus_t *stat) {
    GY88128_Clear();

    // Left: Grid Voltage
    LCD_PrintLeft((int)stat->grid_volt);
    LCD_SetIcon(ICON_INPUT, 0x08, 1);
    LCD_SetIcon(ICON_UNIT_V_L, 0x08, 1);
    LCD_SetIcon(ICON_AC_GRID, 0x08, 1);

    // Right: Frequency
    LCD_PrintRight((int)(stat->grid_freq * 10)); // Show 50.0 as 500
    // Enable DOT5 manually for 50.0
    LCD_SetIcon(ICON_UNIT_HZ, 0x08, 1);

    GY88128_Refresh();
}

// ----------------------------------------------------------------------------
// Screen 3: Battery Charging Detail
// Displays: Batt Voltage (Left) | Charging Amps (Right)
// ----------------------------------------------------------------------------
void Screen_Batt_Charging(SystemStatus_t *stat) {
    GY88128_Clear();

    // Left: Battery Voltage
    LCD_PrintLeft((int)(stat->batt_volt * 10));
    LCD_SetIcon(ICON_BATT_TXT, 0x08, 1);
    LCD_SetIcon(ICON_UNIT_V_L, 0x08, 1);

    // Right: Charging Current
    // Note: No "A" unit icon exists, just show the number
    LCD_PrintRight((int)(stat->chg_amp * 10)); // 10.5A -> 105

    // Visuals: Show charging arrows
    Update_Battery_Visuals(stat->batt_volt, stat->batt_type, true);
    Update_Power_Flow(stat); // Will light up arrows into battery

    GY88128_Refresh();
}

// ----------------------------------------------------------------------------
// Screen 7: Load Detail
// Displays: Batt Voltage (Left) | Load Watts (Right)
// ----------------------------------------------------------------------------
void Screen_Load_Detail(SystemStatus_t *stat) {
    GY88128_Clear();

    // Left: Battery Voltage (Always good context for load)
    LCD_PrintLeft((int)(stat->batt_volt * 10));
    LCD_SetIcon(ICON_BATT_TXT, 0x08, 1);

    // Right: Load in kW
    // 150W = 0.15kW. Display logic needs to handle decimal place.
    // Assuming we print "015" and light DOT6 (decimal)
    LCD_PrintRight(stat->inv_load_watt); 
    LCD_SetIcon(ICON_UNIT_KW, 0x08, 1);
    LCD_SetIcon(ICON_OUTPUT, 0x08, 1);
    LCD_SetIcon(ICON_LOAD, 0x08, 1);

    GY88128_Refresh();
}

// ----------------------------------------------------------------------------
// MPPT Screen 0: Solar Home
// Displays: PV Voltage (Left) | Charging Amps (Right) | Solar Icons
// ----------------------------------------------------------------------------
void Screen_Solar_Home(SystemStatus_t *stat) {
    GY88128_Clear();

    LCD_SetIcon(ICON_PV1, 0x08, 1); // Turn on Solar Panel Icon
    LCD_SetIcon(ICON_PV2, 0x08, 1);

    // Left: PV Volts
    LCD_PrintLeft((int)stat->solar_volt); 
    LCD_SetIcon(ICON_INPUT, 0x08, 1);
    LCD_SetIcon(ICON_UNIT_V_L, 0x08, 1); // "PV" text is part of PV icon usually

    // Right: Solar Amps
    LCD_PrintRight((int)(stat->solar_amp * 10));

    Update_Battery_Visuals(stat->batt_volt, stat->batt_type, stat->is_charging);
    GY88128_Refresh();
}

// ----------------------------------------------------------------------------
// MPPT Screen 2/3: Solar Yield
// Displays: Day kWh (Left) | Total kWh (Right)
// ----------------------------------------------------------------------------
void Screen_Solar_Yield(SystemStatus_t *stat) {
    GY88128_Clear();

    // Left: Day Yield (e.g., 1.20 kWh)
    LCD_PrintLeft((int)(stat->day_kwh * 100)); // 1.20 -> 120
    // Manually light decimal point for 1.xx
    LCD_SetIcon(ICON_INPUT, 0x08, 0); // Hide INPUT text to reduce clutter

    // Right: Total Yield (e.g. 0150 kWh)
    LCD_PrintRight(stat->total_kwh);
    LCD_SetIcon(ICON_KWH, 0x08, 1);

    // Center: Solar Heat
    LCD_PrintCenter(stat->sys_temp);
    LCD_SetIcon(ICON_UNIT_C, 0x08, 1);

    GY88128_Refresh();
}

// ============================================================================
// FAULT OVERRIDES (No SystemStatus struct needed, just raw urgency)
// ============================================================================

void Screen_Fault_Overload(int load_percent) {
    GY88128_Clear();
    
    // Blink Effect needs to be handled by caller (calling this repeatedly with/without text)
    // Here we just set the static frame
    
    LCD_SetIcon(ICON_ERROR, 0x08, 1);        // Triangle
    LCD_SetIcon(ICON_OVERLOAD_TXT, 0x04, 1); // "OVER LOAD" Text

    // Show Load % on right
    LCD_PrintRight(load_percent);
    // Assuming there is a % icon, otherwise just number
    // LCD_SetIcon(ICON_PERCENT, 1); 

    GY88128_Refresh();
}

void Screen_Fault_LowBat(float batt_volt) {
    GY88128_Clear();

    LCD_SetIcon(ICON_ERROR, 0x08, 1);
    
    // Show Empty Battery
    LCD_SetIcon(ICON_BAT_OUT, 0x08, 1); 
    // Do NOT light any bars

    // Blink voltage on left
    LCD_PrintLeft((int)(batt_volt * 10));
    LCD_SetIcon(ICON_BATT_TXT, 0x08, 1);

    GY88128_Refresh();
}

void Screen_Fault_OverHeat(int temp) {
    GY88128_Clear();

    LCD_SetIcon(ICON_ERROR, 0x08, 1);
    
    // Show high temp in center (most prominent for Temp)
    LCD_PrintCenter(temp);
    LCD_SetIcon(ICON_UNIT_C, 0x08, 1);

    GY88128_Refresh();
}
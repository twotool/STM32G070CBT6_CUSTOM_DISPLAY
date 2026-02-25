## Hybrid Mode Operation Specifcation

Hybrid mode combines both AC Mains (Inverter Mode) and PV (MPPT Mode) functions simultaneously. This document outlines the expected display behavior for common hybrid state combinations.

### General Rules
1. **Prioritization**: PV/Solar information is prioritised on the INPUT side when PV is available.
2. **Icons**: Icons for both PV (`PV1`, `PV2`, `PVL1`, `PVL2`, `MPPT`) and Mains (`MAINS`, `LINE`) should be active based on physical availability.
3. **Power Flow**: `S32` (Junction) and `S50` (System Switch) are generally ON when the system is active.
4. **Load Level**: `S30` (Load Level Box) is ON whenever the inverter is active or mains bypass is operational.

---

### Scenario 1: Dual Source Charging (AC Mains ON, PV ON, Charging)
*   **Description**: Both AC grid and Solar are charging the battery.
*   **INPUT Side (Left)**: Toggles between AC Voltage (223V) and PV Voltage (280V) every 2 seconds.
*   **OUTPUT Side (Right)**: Toggles between Battery Voltage (45.8V) and Combined Charging Amps every 2 seconds.
*   **Icons**: `PV1`, `PV2`, `MAINS`, `MPPT`, `LINE`, `S53` (Solar Line), `S49` (Mains Flow to Bat).
*   **Blink**: Battery bars blink indicating charging.

### Scenario 2: Solar Priority / Blended Power (AC Mains Standby, PV ON, Inverter ON)
*   **Description**: Solar is powering the load and charging the battery; Grid is in standby.
*   **INPUT Side (Left)**: Shows PV Voltage (280V).
*   **OUTPUT Side (Right)**: Toggles between Inverter Output (230V) and Load Watts.
*   **Icons**: `PV1`, `PV2`, `MPPT`, `INV`, `S48` (Bat/Solar to Inv), `S29` (Flow to Load).
*   **Visuals**: Load bars `K1`-`K4` active.

### Scenario 3: Grid Priority with Solar Assist
*   **Description**: Load is powered by Grid (Bypass), Solar is charging battery.
*   **INPUT Side (Left)**: Shows AC Voltage.
*   **OUTPUT Side (Right)**: Toggles between Load Watts and Charging Amps.
*   **Icons**: `MAINS`, `LINE`, `PV1`, `PV2`, `MPPT`, `S53` (Charging), `S24` (Load Bulb).

### Scenario 4: PV Only Charge (AC Mains OFF, PV ON, Inverter OFF)
*   **Description**: Grid is disconnected, Inverter is OFF, only Solar is charging.
*   **INPUT Side (Left)**: Shows PV Voltage.
*   **OUTPUT Side (Right)**: Toggles between Battery Voltage and Charging Amps.
*   **Icons**: `PV1`, `PV2`, `MPPT`, `S53`, `S40`.
*   **Note**: No AC icons or flow.

---

### Logic Implementation Note
The function `Screen_Hybrid_Mode(SystemStatus_t *stat, int hybrid_scenario)` should be developed to handle these transitions, using `HAL_GetTick()` for the 2-second synchronized data swap.

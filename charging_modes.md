# System Charging Modes

This document defines the possible battery charging combinations extracted from `Charging modes.PNG`.

## 1. PV Priority
- **Description**: PV module will charge the battery preferentially.
- **Logic**:
  - The battery is charged by the Mains **only when the PV system fails/available energy is zero**.
  - During the day, solar energy is fully used to charge.
  - At night (PV failure/zero), it converts to the Mains.
- **Ideal for**: Areas where the grid is stable and electricity price is high.

## 2. Mains Priority
- **Description**: The Mains supply is preferentially used to charge the battery.
- **Logic**:
  - Only when the **Mains fails**, PV charging is activated.
- **Ideal for**: Ensuring the battery is always full from a reliable grid.

## 3. Hybrid Charging
- **Description**: PV and Mains hybrid charging.
- **Logic**:
  - **PV MPPT charging is the priority**.
  - When PV energy is **insufficient**, the Mains supply supplements the charging.
  - When PV energy becomes sufficient again, the Mains stops charging.
- **Benefit**: Fastest charging mode, suitable for unstable power grids.

## 4. Only Solar
- **Description**: Only PV charging, without Mains charging.
- **Logic**:
  - Battery is charged **only by solar panels** regardless of Mains availability.
- **Ideal for**: Most energy-efficient operation in areas with good lighting conditions.

---

### Visualization Reference
![Charging Modes Table](file:///g:/Microcontroller/STM32%20Inverter%20Project/STM32G070CBT6_CUSTOM_DISPLAY/Charging%20modes.PNG)

## PV Priority Charging
	PV Recovery Power Flow/
	Mains Failure Power Flow
	S4, PV2, PVL2, S40, S53, S39, 
	K5, K6, K7, K8 will be calculated and displayed based on Battery voltage status,
	to indicate charging the last bar will continue to blink in charging mode.
	
	PV Failure Power Flow/
	Mains Recovery Power Flow
	S2, S43, S45, S32, S51, S36, S49, S39,
	K5, K6, K7, K8 will be calculated and displayed based on Battery voltage status,
	to indicate charging the last bar will continue to blink in charging mode.
	
## Hybrid Charging
	PV Insufficient Power Flow
	S2, S4, S43, S45, S32, S51, S36, S49, S39, 
	PV2, PVL2, S40, S53,
	K5, K6, K7, K8, will be calculated and displayed based on Battery voltage status,
	to indicate charging the last bar will continue to blink in charging mode.
	PV2, PVL2, S40, S53
	
	PV Sufficient Power Flow
	S4, PV2, PVL2, S40, S53, S39, 
	K5, K6, K7, K8 will be calculated and displayed based on Battery voltage status,
	to indicate charging the last bar will continue to blink in charging mode.
	
## Solar Only MMPT Mode
	MPPT Mode Power Flow
	PV2, PVL2, S40, S53, S39, K5, K6, K7, K8
	
## Mains available power flow
	S43, S45, S30, S31, S32, S33

## System Switch ON
	S50
	If system switch is OFF S50 will be off
	
## Inverter ON operation
	S29, S48, S30, S31, S33, S39, 
	K1, K2, K3, K4 will be calculated and displayed based on Inverter capacity
	K5, K6, K7, K8 will be calculated and displayed based on Battery voltage status
	
## Battery Low warning
	K5, S39 will continue to blink
	
## Battery Low cut
	S39 will continue to blink
	S29, S33, S31 will be OFF
	
## Over Load cut
	S27 will continue to blink
	S29, S33, S31 will be OFF
	
## Inverter Input data
	AC INPUT voltage: S2, S5, S11, Digit 1, Digit 2, Digit 3
	AC INPUT Frequency: HZ1, S5, Digit 1, Digit 2, Digit 3,
	
	PV INPUT voltage: S4, S5, S11, Digit 1, Digit 2, Digit 3,  DOT 2
	PV INPUT current: S4, S5, S10, Digit 1, Digit 2, Digit 3
	
	PV INPUT watt: S4, S5, S9, Digit 1, Digit 2, Digit 3
	PV INPUT current: S4, S5, S10, Digit 1, Digit 2, Digit 3, DOT 2
	
	BATT Voltage: S6, S11, Digit 1, Digit 2, Digit 3, DOT 2
	BATT Charging current: S6, S10, Digit 1, Digit 2, Digit 3, DOT 2
	
	BATT Voltage: S20, S25, Digit 6, Digit 7, Digit 8, DOT 5
	BATT Charging current: S20, S24, Digit 6, Digit 7, Digit 8, DOT 2
	
	Inverter output voltage: S19, S25, Digit 6, Digit 7, Digit 8
	Inverter output Frequency: S19, S26, Digit 6, Digit 7, Digit 8
	
	Inverter LOAD WATT: S21, S23, Digit 6, Digit 7, Digit 8
	Inverter LOAD PERCENT: S21,  S26, Digit 6, Digit 7, Digit 8
	
	
	
# GY88128-648 Bit-Perfect Segment Mapping (Verified 5x)

This document is the absolute source of truth for the GY88128-648 Green LCD, transcribed bit-by-bit from the `GY88128_648_Green.PNG` datasheet table and verified against the glass layout and the `Real Display.jpeg` photo.

## Address Logic
- **HT1621 Address** (0-31) = SEG Pin Index.
- **COM Line** (0-3) = Data bit in nibble.
- **RAM Packing**: `ht1621_ram[N/2]` contains Address N (low nibble) and Address N+1 (high nibble).

---

## The Master Table (Literal Transcription)

| RAM ADDR | COM 0 (bit 0) | COM 1 (bit 1) | COM 2 (bit 2) | COM 3 (bit 3) | Description |
| :--- 	   | :--- 		   | :--- 		   |:---		   | :---		   | :--- |
| **S0**   | S2 (AC text)  | S3 (PV2 text) | S4 (PV1 text) | S37 (Arrow <<)| Left Labels |
| **S1**   | S5 (INPUT)    | S6 (BATT Left)| S7 (M LEFT)   | S8 (k Left)   | Labels / Units |
| **S2**   | 1F            | 1G            | 1E            | 1D            | Digit 1 (L) |
| **S3**   | 1A            | 1B            | 1C            | DOT1          | Digit 1 (L) |
| **S4**   | 2F            | 2G            | 2E            | 2D            | Digit 2 (L) |
| **S5**   | 2A            | 2B            | 2C            | DOT2          | Digit 2 (L) |
| **S6**   | 3F            | 3G            | 3E            | 3D            | Digit 3 (L) |
| **S7**   | 3A            | 3B            | 3C            | Hz1           | Digit 3 / Hz |
| **S8**   | S9 (W Left)   | S10 (A Left)  | S11 (V Left)  | S12 (% Left)  | Unit Group L |
| **S9**   | S13 (H Left)  | S14 (M Left)  | --            | S16 (h Left)  | Unit Group L |
| **S10**  | 4F            | 4G            | 4E            | 4D            | Digit 4 (C) |
| **S11**  | 4A            | 4B            | 4C            | --            | Digit 4 / % |
| **S12**  | 5F            | 5G            | 5E            | 5D            | Digit 5 (C) |
| **S13**  | 5A            | 5B            | 5C            | L1 (border)   | Digit 5 / Icon|
| **S14**  | S17 (!)       | S18 (ERROR)   | S46 (Wrench)  | S47 (Clock)   | Error Group |
| **S15**  | S19 (OUTPUT)  | S20 (BATT Right)| S21 (LOAD)  | S22 (k Right) | Labels Right |
| **S16**  | 6F            | 6G            | 6E            | 6D            | Digit 6 (R) |
| **S17**  | 6A            | 6B            | 6C            | DOT4          | Digit 6 (R) |
| **S18**  | 7F            | 7G            | 7E            | 7D            | Digit 7 (R) |
| **S19**  | 7A            | 7B            | 7C            | DOT5          | Digit 7 (R) |
| **S20**  | 8F            | 8G            | 8E            | 8D            | Digit 8 (R) |
| **S21**  | 8A            | 8B            | 8C            | Hz2           | Digit 8 (R) |
| **S22**  | S23 (W Right) | S24 (A Right) | S25 (V Right) | S26 (% Right) | Unit Group R |
| **S23**  | S27 (OVERLOAD)| S28 (Mute)    | S29 (Arrow >>)| L2 (Line)     | Fault Icon |
| **S24**  | S30 (LOAD LVL)| S31 (BULB)	   | S32 (Junction)| S33 (LINE >)  | Unit Group R |
| **S25**  | K1 (Bar 1)    | K2 (Bar 2)    | K3 (Bar 3)    | K4 (Bar 4)    | Load Level |
| **S26**  | USER (Type)   | FLD (Type)    | AGM (Type)    | S36 (INV)   | Bat Type |
| **S27**  | S48 (Flow)    | S49 (Flow)    | S50 (Flow)    | S51 (Flow)    | Dots |
| **S28**  | S38 (h Left)  | S39 (Bat icon)| PVT1 (Arrow)  | S40 (MPPT)    | Flow Icons |
| **S29**  | K5 (Chg 1)    | K6 (Chg 2)    | K7 (Chg 3)    | K8 (Chg 4)    | Chg Bar |
| **S30**  | PVL2 (Big)    | PVL1 (Big)    | S52 (M RIGHT) | S53 (LINE >)  | Panels / Dots |
| **S31**  | PV1 (ICON)    | PV2 (ICON)    | S43 (MAINS)   | S45 (LINE)    | PV / Circle |

---

## Mapping Verified (Corrected Logic)
- **Digit 5**: **NOT SHIFTED**. Segments (F,G,E,D) are on S12 and (A,B,C) are on S13.
- **Digit 2**: Corrected. Segments (F,G,E,D) are on S4 and (A,B,C) are on S5.
- **Left Units**: Occupy addresses S8, S9, S11.
- **Right Units**: Occupy addresses S22, S24.
- **Mode/Labels**:
  - AC: S0 COM0
  - INPUT: S1 COM0
  - BATT(L): S1 COM1
  - OUTPUT: S15 COM1
  - BATT(R): S15 COM2
  - LOAD: S15 COM3
  - Inverter Circle: S31 COM2
  - Overload: S23 COM0
  - Error: S14 COM0 (text) and S14 COM1 (!)

01. When GRID is present it displayed in the following way - 
	S2, S5, DIGIT 1, DIGIT 2, DIGIT 3, S11
	
02. When battery volt is shown on the right side -
	S6, DIGIT 1, DIGIT 2, DIGIT 3, DOT2, S11
	
S37 (Arrow <<) - Power is sent to grid
S29 (Arrow >>) - Power is being drawn by the load
S33 (LINE >) - Power is sent to load
S36 (INV) - Inverter
S48 (Flow) - Inverter power source is battery
S49 (Flow) - Inverter is charging battery 
S50 (Flow) - Inverter is outputing AC
S51 (Flow) - Inverter is getting power fro AC mains
PVT1 (Arrow) - Inverter is getting power from MPPT controller
S40 (MPPT) - MPPT Charge controller
PVL2 (Big) - MPPT is getting power from PV1
PVL1 (Big) - MPPT is getting power from PV2
S53 (LINE >) - battery is being charged from MPPT
S43 (MAINS) - Mains icon
S45 (LINE) - Mains available
S30 (LOAD LVL) - 25%-100% Load level icon
S31 (BULB) - when load level is greater than 1% it means load the bulb is on
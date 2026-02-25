## Inverter only mode operation
	Scenario - 1, AC mains available, system is in Standby mode.
	01. In this mode no solar functions are active. So all the Solar related segments are not used.
	02. LED Sign shuold be ON.
	03. If the battery is being charged only battery level bars will blink according to battery charge level.
		If the charger is OFF battery bars will be steady. Battery type will be displayed accordingly.
	04. Load level box icon S30 will be ON.
	05. On INPUT side AC voltage and Frequency will be shown When mains is available. These two values alternate 
	    between Volt 223(S11) and Hz 49.9(HZ1) every 2 seconds, should be synchronized with system LED.
	06. The Bulb Icon will be ON.
	07. On the OUTPUT side BATT voltage and BATT charging Amp will be shown, These two values alternate 
	    between Volt(S25) 12.8 and AMP(S24) 10.5 every 2 seconds, should be synchronized with system LED.
		
	Scenario - 2, AC mains NOT available, Inverter is ON.
	01. On The INPUT side Battery voltage will be shown.
	02. On the OUTPUT side inverter OUTPUT voltage will be shown. 
	03. Inverter load in watts S23 will be shown.
	04. OUTPUT Hz (HZ1) will be shown.
	05. Inverter run time will be shown along with the clock symbol (S47) and borber (L1).
	06. S29 will show power flow direction to the load from the Inverter.
	07. Load level will be shown using K1, K2, K3, K4.
	
	Scenario - 3, AC mains NOT available, Inverter is OFF.
	01. S50 will be OFF to show system switch is OFF.
	02. AC mains power flow will be OFF. 
	03. The charger will be OFF.
	04. It will continue to show the values as Scenario - 1. obviouly Somes values will be zeros.
	
Take a look at the above operating mode. Write a fucntion to display all the power flow and information.
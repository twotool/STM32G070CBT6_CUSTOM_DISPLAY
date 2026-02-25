## MPPT only mode operation
	Scenario - 1,PV available, MPPT is Charinging the battery.
	01. In this mode no AC mains functions are active. So all the AC mains related segments are not used.
	02. PV1, PVL1, ,PV2, PVL2, S40, S53 should be on.
	03. If the battery is being charged only battery level bars will blink according to battery charge level.
		If the charger is OFF battery bars will be steady. Battery type will be displayed accordingly.
	04. Load level box icon S30 will be OFF.
	05. On INPUT side PV voltage will be shown When PV S3, S4 280 v is available. These PV voltage will update every 2 seconds, 
		should be synchronized with system LED.
	07. On the OUTPUT side BATT voltage and BATT charging Amp will be shown, These two values alternate 
	    between Volt(S25) 45.8 v and AMP(S24) 58 A every 2 seconds, should be synchronized with system LED.
		
	Scenario - 2, PV is available, MPPT is not Charinging the battery.
	01. In this mode no AC mains functions are active. So all the AC mains related segments are not used.
	02. PV1, PVL1, ,PV2, PVL2, S40, S53 should be on.
	03. If the battery is being charged only battery level bars will blink according to battery charge level.
		If the charger is OFF battery bars will be steady. Battery type will be displayed accordingly.
	04. Load level box icon S30 will be OFF.
	05. On INPUT side PV voltage will be shown When PV S3, S4 280 v is available. These PV voltage will update every 2 seconds, 
		should be synchronized with system LED.
	07. On the OUTPUT side only BATT voltage will be shown, this value will be updated every 2 seconds, 
		should be synchronized with system LED.
		
	Scenario - 3, PV is NOT available, MPPT is not Charinging the battery.
	01. On The OUTPUT side Battery voltage 45.8 v will be shown.
	02. On the INPUT side PV voltage 110 v will be shown. 
	03. Battery level will be shown using K5, K6, K7, K8.
	
Take a look at the above operating mode. Write a fucntion to display all the power flow and information.
#include <ESP8266WiFi.h> 
#include <FS.h> // Include the SPIFFS library 

// WiFi credentials (replace with your own) 
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD"; 

// Configuration file path 
const char* configFilePath = "/config.txt"; 

// Structure to hold your configuration data 
struct ConfigData {
	int value1; 
	float value2; 
	char stringValue[32]; // Example string, adjust size as needed 
};

ConfigData config; // Instance of the configuration data 

// Function to save the configuration data to SPIFFS 
bool saveConfig(){ 
	File configFile = SPIFFS.open(configFilePath, "w"); 
	if (!configFile) { 
		Serial.println("Failed to open config file for writing");
		return false; 
		}
	configFile.write((uint8_t*)&config, sizeof(config)); // Write the entire struct 
	configFile.close(); 
	Serial.println("Config saved successfully"); 
	return true; 
}

// Function to load the configuration data from SPIFFS 
bool loadConfig(){  
	if (!SPIFFS.exists(configFilePath)) {
		Serial.println("Config file does not exist, using default values"); 
		// Initialize with default values if the file doesn't exist. 
		config.value1 = 100; 
		config.value2 = 3.14; 
		strcpy(config.stringValue, "Default"); 
		saveConfig(); // Save the default values 
		return true; // Consider it successful since defaults were set.
		}
	File configFile = SPIFFS.open(configFilePath, "r"); 
	if (!configFile){ 
	Serial.println("Failed to open config file for reading"); 
	return false; 
	}
	configFile.read((uint8_t*)&config, sizeof(config)); //Read the entire struct 
	configFile.close(); 
	Serial.println("Config loaded successfully"); 
	return true; 
	}
	
void setup()
{
	Serial.begin(115200); 
	delay(10);


	// Initialize SPIFFS
	if (!SPIFFS.begin())
	{
	Serial.println("Failed to mount SPIFFS");
	return;
	}


	// Connect to WiFi
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
	delay(500);
	Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());


	// Load the configuration
	loadConfig();


	// Print the loaded configuration
	Serial.print("Value1:");
	Serial.println(config.value1);
	Serial.print("Value2: ");
	Serial.println(config.value2);
	Serial.print("String Value: ");
	Serial.println(config.stringValue);

	// Example: Modify the configuration 
	config.value1 = 200; 
	config.value2 = 2.71; 
	strcpy(config.stringValue, "Modified"); 
	// Save the modified configuration 
	saveConfig();
}
void loop(){ 
// Your main loop code here 
delay(1000); // Example delay
}
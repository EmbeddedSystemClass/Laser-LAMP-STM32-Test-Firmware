#ifndef __SPWF01_H
#define __SPWF01_H

/*"Console active												"
"Poweron (%s) 												"
"RESET 																"
"Watchdog Running 										"
"Heap too small 											"
"WiFi Hardware Failure: (%d) 					"
"Watchdog Terminating, reset pending	"
"SysTickConfigure 										"
"Hard Fault 													"
"StackOverflow 												"
"MallocFailed (%d/%d) 								"
"<error> 															"
"WiFi PS Mode Failure: %s: %d 				"
"<copyright information> 							"
"WiFi BSS Regained 										"
"WiFi Signal LOW (%d) 								"
"WiFi Signal OK (%d) 									"
"F/W update <state> 									"
"Keytype %d 													"
"WiFi Join: %m 												"
"JOINFAILED: %04x 										"
"WiFi Scanning												"
"SCANBLEWUP 													"
"SCANFAILED: %04x 										"
"WiFi Up: %i 													"
"WiFi Association with '%s' successful"
"WiFi Started AP with network “%d			"
"STARTFAILED: %04x 										"
"Station %m Associated: %d 						"
"DHCP reply for %i/%m 								"
"WiFi BSS Lost 												"
"WiFi EXCEPTION: <data> 							"
"WiFi Hardware Started 								"
"WiFi Network Lost 										"
"WiFi Unhandled Event: %d 						"
"Scan Complete:0x%x 									"
"WiFi UNHANDLED IND (%02x):<hexdata> 	"
"WiFi UNHANDLED (%d):<hexdata> 				"
"WiFi: Powered Down 									"
"HW in miniAP mode (GPIO7 Low) 				"
"WiFi Deauthentication: %d Radio: 		"
"WiFi Disassociation: %d"
"RX_MGMT: %04x"
"RX_DATA: %04x"
"RX_UNK: %04x"
"DOT11 AUTHILLEGAL"
"WPA: Crunching PSK..."
"WPA:%s"
"WPAC:%s"
"WPA:Terminated: %d 		"
"WPA Supplicant failed to initialize."
"WPA Handshake Complete"
"GPIO%d %d 							"
"Wakeup (GPIO6 High) 		"
"ETF %04d 							"
"Pending Data:%d:%d 		"
"Insert message to client:%d 	"
"<data>                                            "
"Socket Closed:%d                                  "
"Back to Command Mode                              "
"Now in Data Mode                                  "
"Incoming Socket Client:%i                         "
"Socket Client Gone:%i                             "
"Sockd Dropping Data:%d:%d                         "
"Sockd Pending Data:%c:%d:%e                       "
"HW Factory Reset (GPIO0 High)                     "
"Low Power mode enabled:%d                         "
"Going into Standby:%d                             "
"Resuming from Standby                             "
"Going into DeepSleep                              "
"Resuming from DeepSleep                           "
"DNS reply for %d                                  "
"Station %m Disassociated: %d                      "
"System Configuration Updated (Run AT&W to Save it)"
"Rejected found Network                            "
"Rejected Association:yyy                          "
"Authentication Timed Out                          "
"Association Timed Out                             "
"MIC Failure                                       "*/

// WIND commands
#define WIND_MSG_CONSOLE_ACTIVE	 						0		//Console task is running and can accept AT commands
#define WIND_MSG_POWERON				 						1		//Initial powerup indication, with f/w version
#define WIND_MSG_RESET					 						2		//System reset is being asserted/triggered
#define WIND_MSG_WATCHDOG_RUN		 						3		//Watchdog task initialized and running
#define WIND_MSG_HEAPTOOSMALL		 						4		//Selected heap allocation is too small for normal operation
#define WIND_MSG_HWR_FAILURE		 						5		//WiFi Radio Failure, reset pending
#define WIND_MSG_WATCHDOG_TERMINATING				6		//Watchdog reset asserted
#define WIND_MSG_SYSCLK_FAIL		 						7		//Failure to configure System Tick Clock
#define WIND_MSG_HARDFAULT			 						8		//OS hard fault detected
#define WIND_MSG_STACK_OVERFLOW	 						9		//OS stack overflow detected
#define WIND_MSG_MALLOC_FAILED						 10		//OS heap allocation failed (RequiredSize/FreeSpace)
#define WIND_MSG_ERROR										 11		//Radio Initialization failure
#define WIND_MSG_POWERSAVE_FAILED					 12		//Radio Failed to enter power saving state (%s=step, %d=state)
#define WIND_MSG_COPYRIGHT								 13		//Copyright information of SPWF01SX
#define WIND_MSG_RADIO_REGAINED						 14		//Radio regained association after loss
#define WIND_MSG_SIGNAL_LOW								 15		//Radio low signal threshold triggered
#define WIND_MSG_SIGNAL_OK								 16		//Radio signal level recovered
#define WIND_MSG_FIRMWARE_UPDATE					 17		//Firmware update in progress
#define WIND_MSG_KEYTYPE									 18		//Not implemented Encryption key type not recognized
#define WIND_MSG_WIFI_JOIN								 19		//BSS join successful, %m=BSSID
#define WIND_MSG_JOIN_FAILED							 20		//BSS join failed, %x = status code
#define WIND_MSG_WIFI_SCANNING						 21		/*Radio is scanning for a BSS that matches the currently configured SSID. (Note: WIND hidden when fast reconnect(1) is performed)*/
#define WIND_MSG_SCANBLEWUP								 22		//Radio failed to accept scan command
#define WIND_MSG_SCANFAILED								 23		//Radio failed to execute scan command
#define WIND_MSG_WIFIUP										 24		//Radio has successfully connected to a BSS and initialized the IP stack. %i=IP Address
#define WIND_MSG_WIFI_ASSOCIATION					 25		//Radio successfully associated to the “%s” BSS
#define WIND_MSG_WIFI_MINI_AP							 26		//Radio successfully started the Mini AP, where %d=network SSID
#define WIND_MSG_STARTFAILED							 27		//Radio failed to start the Mini AP,%x=status code
#define WIND_MSG_MINIAP_ASSOCIATION				 28		//Client associated to the module in Mini AP,%m=BSSID, %d=peers assoc status (0=default, 1=client reassociation)
#define WIND_MSG_DHCP_REPLY								 29		//DHCP reply sent for the client,%i = client IP address,%m = client MAC Address
#define WIND_MSG_BSS_LOST									 30		//Beacon missed from the BSS
#define WIND_MSG_WIFI_EXCEPTION						 31		//Radio reported an internal exception. Radio is nonfunctional from this point; User must reboot the module.
#define WIND_MSG_HWR_STARTED							 32		//Radio reports successful internal initialization
#define WIND_MSG_NETWORK_LOST							 33		//Connection to BSS lost due to excessive beacon misses
#define WIND_MSG_WIFI_UNHANDLED_EVENT			 34		//Unhandled internal event occurred,%d=identifier of the event occurred
#define WIND_MSG_SCAN_COMPLETE						 35		//Scan Complete indication,%x=result code (0: scan ok; 1: scan error). Note: WIND hidden when fast reconnect is performed).
#define WIND_MSG_UNHUNDLED_IND						 36		//Unparsed radio indication occurred
#define WIND_MSG_UNHUNDLED								 37		//Unhandled radio response message received
#define WIND_MSG_WIFI_POWERDOWN						 38		//Radio and radio thread shut down
#define WIND_MSG_HWR_MINIAP								 39		//Module started in miniAP mode (SSID = iwm-XX-YY-ZZ, where XXYYZZ are the last 6 digits of MAC Address)
#define WIND_MSG_WIFI_DEATHENTIFICATION		 40		//Access point sent deauthentication, :%d=reason code (802.11 Deauthentication Reason Code)
#define WIND_MSG_WIFI_DISASSOTIATION			 41		//Radio: Access point sent disassociation, :%d=reason code (802.11 Disassociation Reason Code)
#define WIND_MSG_RX_MGMT									 42		//Unhandled management frame subtype received
#define WIND_MSG_RX_DATA									 43		//Unhandled data frame subtype received
#define WIND_MSG_RX_UNK										 44		//Unhandled frame type received
#define WIND_MSG_DOT11_AUTHILLEGAL				 45		//Illegal authentication type detected
#define WIND_MSG_WPA_CRUNCHING						 46		//Creating PSK from PSK passphrase
#define WIND_MSG_WPA											 47		//Factory Debug
#define WIND_MSG_WPAC											 48		//Factory Debug
#define WIND_MSG_WPA_TERMINATED						 49		//WPA supplicant thread terminated
#define WIND_MSG_WPA_SUPPLICANT_FAIL			 50		//WPA supplicant thread initialization failed
#define WIND_MSG_WPA_HANDSHAKE_COMPLETE		 51		//WPA 4-way handshake successful
#define WIND_MSG_GPIO_CHANGED							 52		//GPIO line changed state (%d=GPIO changed, %d=GPIO logic state
#define WIND_MSG_WAKEUP										 53		//Device woken up from sleep from external signal
#define WIND_MSG_ETF											 54		//Factory Debug
#define WIND_MSG_PENDING_DATA							 55		//Pending data from the socket, %d =socket identifier:%d=pending byte available for reading
#define WIND_MSG_INSERT_MESSAGE						 56		//Input_demo indicator, displayed when the “input_demo.shtml” page is requested by a client, %d is the Nth input SSI into html page
#define WIND_MSG_DATA_IND									 57		//Firstset indicator, displayed during the remote configuration of the module
#define WIND_MSG_SOCKET_CLOSED						 58		//Socket closed, %d = identifier of the socket
#define WIND_MSG_COMMAND_MODE					 		 59		//Command mode is active (after the escape sequence)
#define WIND_MSG_DATA_MODE						 		 60		//Data mode is active
#define WIND_MSG_INCOMING_SOCKET					 61		//Socket client is connected to the module, %i = client IP address
#define WIND_MSG_SOCKET_DISCONNECTED			 62		//Socket client disconnected, %i = client IP address
#define WIND_MSG_DROPPING_DATA						 63		//Data dropped due to low memory, %d=bytes dropped, %d=free heap
#define WIND_MSG_SOCKET_PENDING						 64		//Data pending while module is in command mode, %c = number of message received, %d = bytes received in the last message, %e = tot bytes received 
#define WIND_MSG_HWR_FACTORY_RESET				 65		//Factory variables are restored via GPIO0
#define WIND_MSG_LOWPOWER									 66		//Power Save Mode enabled, %d = 1 for PS or 2 for FastPS 
#define WIND_MSG_STENDBY									 67		//Standby mode enabled, %d is time in sec
#define WIND_MSG_RESUMING_FROM_STDBY			 68		//Standby mode disabled
#define WIND_MSG_DEEPSLEEP								 69		//Sleep mode enabled
#define WIND_MSG_RESUMING_FROM_DEEPSLEEP	 70		//Sleep mode disabled
#define WIND_MSG_DNS_REPLY								 71		//DNS reply from MiniAP to the client, %d = client IP address
#define WIND_MSG_STATION_DIASSOCIATED			 72		//Client dissociated to the module in Mini AP, %m=BSSID, %d=reason code (802.11 Deauthentication Reason Code)
#define WIND_MSG_CONFIG_UPDATED						 73		//The configuration variables have been updated, it needs an AT&W to save it (this WIND is usually shown when an old FW version is updated) 
#define WIND_MSG_NETWORK_REJECT						 74		//A new scan needs to be scheduled due to a mismatch between SPWF configuration variables and Access Point configuration
#define WIND_MSG_REJECTED_ASSOCIATION			 75		//Indicates an association failure (yyy=low memory, reject status code)
#define WIND_MSG_AUTHENTIFICATION_TIMEOUT  76		//Indicates that the authentication process is timed out
#define WIND_MSG_ASSOCIATION_TIMEOUT       77		//Indicates that the association process is timed out
#define WIND_MSG_MIC_FAILURE               78		//Michael MIC error is detected by the local driver

/*// AT command list
char AT                [] = "AT\r\n\0";
char AT_CFUN           [] = "AT+CFUN\r\n\0";
char AT_S_HELP         [] = "AT+S.HELP\r\n\0";
char AT_S_GCFG         [] = "AT+S.GCFG";
char AT_S_SCFG         [] = "AT+S.SCFG\r\n\0";
char AT_S_SSIDTXT      [] = "AT+S.SSIDTXT\r\n\0";
char AT_V              [] = "AT&V\r\n\0";
char AT_F              [] = "AT&F\r\n\0";
char AT_W              [] = "AT&W\r\n\0";
char AT_S_NVW          [] = "AT+S.NVW\r\n\0";
char AT_S_STS          [] = "AT+S.STS\r\n\0";
char AT_S_PEERS        [] = "AT+S.PEERS\r\n\0";
char AT_S_PING         [] = "AT+S.PING\r\n\0";
char AT_S_SOCKON       [] = "AT+S.SOCKON\r\n\0";
char AT_S_SOCKOS       [] = "AT+S.SOCKOS\r\n\0";
char AT_S_SOCKW        [] = "AT+S.SOCKW\r\n\0";
char AT_S_SOCKQ        [] = "AT+S.SOCKQ\r\n\0";
char AT_S_SOCKR        [] = "AT+S.SOCKR\r\n\0";
char AT_S_SOCKC        [] = "AT+S.SOCKC\r\n\0";
char AT_S_SOCKD        [] = "AT+S.SOCKD\r\n\0";
char AT_S_             [] = "AT+S.\r\n\0";
char AT_S_HTTPGET      [] = "AT+S.HTTPGET\r\n\0";
char AT_S_HTTPPOST     [] = "AT+S.HTTPPOST\r\n\0";
char AT_S_FSC          [] = "AT+S.FSC\r\n\0";
char AT_S_FSA          [] = "AT+S.FSA\r\n\0";
char AT_S_FSD          [] = "AT+S.FSD\r\n\0";
char AT_S_FSL          [] = "AT+S.FSL\r\n\0";
char AT_S_FSP          [] = "AT+S.FSP\r\n\0";
char AT_S_MFGTEST      [] = "AT+S.MFGTEST\r\n\0";
char AT_S_PEMDATA      [] = "AT+S.PEMDATA\r\n\0";
char AT_S_WIFI         [] = "AT+S.WIFI\r\n\0";
char AT_S_ROAM         [] = "AT+S.ROAM\r\n\0";
char AT_S_GPIOC        [] = "AT+S.GPIOC\r\n\0";
char AT_S_GPIOR        [] = "AT+S.GPIOR\r\n\0";
char AT_S_GPIOW        [] = "AT+S.GPIOW\r\n\0";
char AT_S_FWUPDATE     [] = "AT+S.FWUPDATE\r\n\0";
char AT_S_HTTPDFSUPDATE[] = "AT+S.HTTPDFSUPDATE\r\n\0";
char AT_S_HTTPDFSERASE [] = "AT+S.HTTPDFSERASE \r\n\0";
char AT_S_HTTPD        [] = "AT+S.HTTPD\r\n\0";
char AT_S_SCAN         [] = "AT+S.SCAN\r\n\0";
char AT_S_ADC          [] = "AT+S.ADC\r\n\0";
char AT_S_DAC          [] = "AT+S.DAC\r\n\0";
char AT_S_PWM          [] = "AT+S.PWM\r\n\0";
char AT_S_TLSCERT      [] = "AT+S.TLSCERT\r\n\0";
char AT_S_TLSCERT2     [] = "AT+S.TLSCERT2\r\n\0";
char AT_S_TLSDOMAIN    [] = "AT+S.TLSDOMAIN\r\n\0";
char AT_S_SETTIME      [] = "AT+S.SETTIME\r\n\0";
char AT_S_RMPEER       [] = "AT+S.RMPEER\r\n\0";
char AT_S_HTTPREQ      [] = "AT+S.HTTPREQ\r\n\0";
char AT_S_FSR          [] = "AT+S.FSR\r\n\0";
char AT_S_HTTPDFSWRITE [] = "AT+S.HTTPDFSWRITE \r\n\0";*/

#endif

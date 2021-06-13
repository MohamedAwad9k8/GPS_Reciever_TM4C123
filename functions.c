#include "D:/Applications/Keil/EE319Kware/inc/tm4c123gh6pm.h"
#include "tm4c123.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "ports_init.h"
#include "functions.h"


// Defining Macros to ease the LCD Functions
#define LCD_CTRL GPIO_PORTC_DATA_R // RS is on PC7, R/W is on PC6, E is on PC5  
#define RS_MASK 0x80					 // RS Mask for pin 7
#define RW_MASK 0x40					 // RW Mask for pin 6
#define E_MASK 0x20 				   // E Mask for Pin5
#define LCD_DATA GPIO_PORTB_DATA_R       // DB0 to DB7 are on B0 to B7

// Defining Macros to ease Distance Calculations
#define pi 3.14159 // PI
#define R 6371 // Earth's Radius in Km


// LCD Functions
	// The function that takes commands to the LCD (like clear screen)
		void LCD_Command(char command){
			LCD_CTRL &= ~RS_MASK;						 // set RS = 0 for command
			LCD_CTRL &= ~RW_MASK; 					 // set R/W = 0 for write
			LCD_CTRL &= ~E_MASK; 						 // set E = 0 for initalize, make sure it's low.
			LCD_DATA = command; 	 // send the command to the LCD
			LCD_CTRL |= E_MASK;						 // Enable the latch, (low to high) so that LCD can Read the input
			delay (100);							 // Delay the microcontroller, to wait for the lower LCD clk speed
			LCD_CTRL &= ~E_MASK;						 // Turn off the Latch again
		}

	// The function that prints actual data on the LCD				
		void LCD_Data(char data){
			LCD_CTRL |= RS_MASK;						// set RS = 1 for data
			LCD_CTRL &= ~RW_MASK; 					// set R/W = 0 for write
			LCD_CTRL &= ~E_MASK; 						// set E = 0 for initalize, make sure it's low.
			LCD_DATA = data;	 							// send the command to the LCD
			LCD_CTRL |= E_MASK;						  // Enable the latch, (low to high) so that LCD can Read the input
			delay (100);							 			// Delay the microcontroller, to wait for the lower LCD clk speed
			LCD_CTRL &= ~E_MASK;						// Turn off the Latch again
		}
	
		//Allows writing strings on the LCD
			void LCD_String (char *str){
				int v;
				for(v=0;str[v]!='\0';v++)
				{
					LCD_Data(str[v]);
					if (v == 15){
						LCD_Command(0xC0); // Cursor starts at the second line
					}
				}
			}


		//Allows writing numbers on the LCD
			void LCD_Num(int num){
				LCD_Data((num/1000)+48); //Outputs thousands with ASCII offset
				num %= 1000;

				LCD_Data((num/100)+48); //Outputs hundreds with ASCII offset
				num %= 100;

				LCD_Data((num/10)+48); //Outputs tens with ASCII offset
				num %= 10;

				LCD_Data(num+48); //Outputs ones with ASCII offset
			}
	
// GPS Functions
			
	// Read a char form GPS
	char GPS_READ_CHAR(){
		char data_register;
		while (1){
			while ((UART7_FR_R & 0x10) != 0) {} //empty loop while the reciver FIFO is empty using RXFE (RXFE = 1 = Empty)
			data_register = (UART7_DR_R & 0xFF); // read the char from RX data register
			// We check if the char is valid, we use it. If it's not valid we discard it.
			if ((UART7_RSR_R & 0x01) == 1){} //If FE (Frame) flag is turned on, wait
			else if ((UART7_RSR_R & 0x04) == 1) {} //If BE (Break) flag is turned on, wait
			else if ((UART7_RSR_R & 0x08) == 1) {} //If OE (Over run) flag is turned on, wait
			else{	//If everything is alright, then read char from Data Register
				// This if conditional dodges non-ASCII Characters
				if ( data_register >= 0 && data_register <= 126){
					// This if conditional dodges Line Feed and Carriage Return
					if ( data_register == 0x0A || data_register == 0x0D){
					}
					else{
						return data_register; // return the 8 bits -> char
						break;
					}
				}
			}
		}
	}
	// Read a string from GPS
	void GPS_READ_STRING(char *raw_data){
		int i;
		// Saving the Data to the Memory
		for (i = 0; i <= 473 ; i++)
		{
			raw_data[i] = GPS_READ_CHAR(); //save characters from GPS to memory
			// Ending the string with the null terminator
			if (i == 473){
				raw_data[i] = '\0';
			}
		}
	}
	
	//Get the $GPGLL sentence from the NEMA Data
	void GET_GPGLL(char *raw_data, char *gpgll){
		int i,j;
		for (i = 0; i <= 472 ; i++){
			// Checking for $gpgll to parse it
			if (raw_data[i] == '$'){
				i += 3; // Skip GP
				if (raw_data[i] == 'G'){
					i++; // get next char
					if (raw_data[i] == 'L'){
						// Now We have the $gpgll and we will start saving inro memory
						gpgll[0] = '$';
						gpgll[1] = 'G';
						gpgll[2] = 'P';
						gpgll[3] = 'G';
						gpgll[4] = 'L';
						i ++; // Get next char, which should be C, note there's $gpgll , $GPRMB and $GPRMA, LCD will tell us if we got wrong sentence 
						for ( j = 5 ; j <= 51; j++){
							if (raw_data[i] == '$'){
								gpgll[j] = '\0';
								break;
							}
							gpgll[j] = raw_data[i];
							i++; //get next charachter
							// Ending the string with the null terminator
							if (i == 51){
								gpgll[j] = '\0';
							}
						}
						break;
					}
				}
			}
		}
	}


	// GPS PARSING FUNCTION SET UP TO GET LATTITUDE AND LONGITUDE FROM $GPGLL
	void parse_gps_string(char* gps_raw_data,char * latitude ,char * longitude){
		int counter = 0; //to track number of delimiters "," found in the string
		int i,k,j; // counters for loops

		// to loop on every character of string
		for(i=0;gps_raw_data[i]!='\0'; i++){
			//Check if the character is equal to the delimiter which is ','
			if (gps_raw_data[i]==','){
				counter++; //increase number of delimiters

				// check if reached 1st delimiter, GET LATTITUDE
				if (counter == 1){
					i++; //move gps_raw_data[i] to read first char after the 2nd delimiter
					//this is a loop to copy all characters between 2nd delimiter and 3rd delimiter
					for(k=0;1;k++){
						latitude[k] = gps_raw_data[i];
						//to get next char
						if(gps_raw_data[i]!=','){
							i++;
						}
						//stop when 3rd delimiter is found
						if(gps_raw_data[i] ==','){
							counter++; //increase number of delimiters
							latitude[k+1] = '\0'; // put the string terminator after last character stored in latitude
							break;
						}
					}
				}

				// check if reached 3rd delimiter , GET LONGITUDE
				if (counter == 3){
					i++; //move gps_raw_data[i] to read first char after the 4th delimiter
					//this is a loop to copy all characters between 4th delimiter and 5th delimiter
					for(j=0;1;j++){
						longitude[j] = gps_raw_data[i];
						//to get next char
						if(gps_raw_data[i]!=','){
							i++;
						}
						//stop when 5th delimiter is found
											if(gps_raw_data[i] ==','){
												 counter++;
												 longitude[j+1] = '\0'; // put the string terminator after last character stored in longitude
												 break;
											}
									}
					break;
				}
			}
		}
	}
	
	

// SWITCHES Functions
	int READ_SW1(void){
		int SW1_Pressed ;
		SW1_Pressed = (GPIO_PORTF_DATA_R & 0x10);
		return SW1_Pressed; // if SW1_Pressed = 0, then switch is pressed, else it's not
	}
	
	int READ_SW2(void){
		int SW2_Pressed ;
		SW2_Pressed = (GPIO_PORTF_DATA_R & 0x01);
		return SW2_Pressed; // if SW2_Pressed = 0, then switch is pressed, else it's not
	}
	
// LED Functions
	void LED_ON(void){
		GPIO_PORTF_DATA_R |= 0x02; // LED Turns On
	}

	void LED_OFF(void){
		GPIO_PORTF_DATA_R &= ~0x02; // LED Turns Off
	}
	


// Distance Functions
	  // CONVERTING DEGREES TO RADIANS 
		 double degreesToRadians (double degrees){

			 double rad = degrees * pi / 180;
			 return rad;
		}

   // GETTING DISTANCE BETWEEN TWO COORDINATES 	USING HAVERSINE FORMULA
		 double distanceCoordinates (double lat1, double lon1, double lat2, double lon2){
	
			double dLat;
			double dLon;
			double deg_lat1, deg_lat2;
			double a,a1,a2,a3,a4;
			double c;
			double dis;
			 
			dLat = degreesToRadians (lat2 - lat1);  // delta latitude
			dLon = degreesToRadians (lon2 - lon1);	// delta longitude
			deg_lat1 = degreesToRadians (lat1);					// latitude 1
			deg_lat2 = degreesToRadians (lat2);					// latitude 2

			// Note in haversine function, the longitude and latitude are given as angels to trig functions, we need to make them Radians
			//a = (sin (dLat / 2) * sin (dLat / 2)) + (sin (dLon / 2) * sin (dLon / 2)) * cos (deg_lat1) * cos (deg_lat2);
			 a1 = sin (dLat / 2);
			 a2 = sin (dLon / 2);
			 a3 = cos (deg_lat1);
			 a4 = cos (deg_lat2);
			 a = (a1 * a1) + ((a2 * a2) * a3 * a4);
			 c = 2 * atan2 (sqrt (a), sqrt (1 - a));
			
			dis = R * c * 1000;
			return dis;
		}

	//multiple of millisecond delay using periodic mode	
	void delay (int ms){
    int i,j;
		for (i=0; i < ms; i++){
			for(j=0; j < 3180; j++)
			{}
			}
	}


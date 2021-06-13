#include "tm4c123.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "ports_init.h"
#include "functions.h"

// Macros for the LCD Commands
#define CLEAR_SCREEN 0x01 //clears the LCD sreen
#define BLINK 0x0F 			 // makes the cursor blink
#define WAKE_UP 0x30			 // LCD wakeup
#define LCD_8BIT 0x38		 // uses the 2 lines, 8 bit on the LCD
#define LCD_INC 0x06			 // cursor position increment
#define LCD_HOME 0x02		 // cursor moves to home
#define LCD_Hide 0x0C // hides cursor
#define LCD_ROW1 0x80 // force cursor to the start of the 1st line
#define LCD_ROW2 0xC0 // force cursor to the start of the 2nd line

void SystemInit(void)
{ 
    /* Set floating point coprosessor access mode. */ 
		/* set CP10 Full Access and set CP11 Full Access */
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2) ); 
}


int main (){
	char raw_data[473];
	char gpgll[51];
	char lat_str[15];
	char lon_str[15];
	char dis_str[15];
	char *ptr; 			// un used pointer, needed by the strod function
	double initial_lat_double, initial_lon_double, current_lat_double, current_lon_double;
	char first_time_flag = '0';
	double dis_double = 0;
	int sw_reset = 1;
	
	// Initialize Ports
	initialize_ports();
	
	while(1){
		
		//If first iteration, Get Initial Position
		if(first_time_flag == '0'){
			// Read Raw Data From GPS
			GPS_READ_STRING(raw_data);
			//Parse the $GPGLL Sentence Only 
			GET_GPGLL(raw_data, gpgll);
			//Check If GPS is reading full data
			// Data Read Sucessfully from GPS
			if (strlen(gpgll) >= 48){
				//Parse the Latitude and Longitude
				parse_gps_string(gpgll, lat_str, lon_str);
				// Converting the string coordinates to doubles
				initial_lat_double = strtod(lat_str, &ptr);
				initial_lon_double = strtod(lon_str, &ptr);
				// printing Message on LCD
				LCD_Command(LCD_8BIT); // 2 line mode
				LCD_Command(LCD_INC); // inrement cursor
				LCD_Command(LCD_ROW1);	// prints on the first line
				LCD_Command(LCD_Hide); // Hide Cursor
				LCD_Command(CLEAR_SCREEN); // Clear
				LCD_String ("Initial Position Acquired !");
				delay(3000);				// wait 3 seconds
				//Set the Flag, to start calculating Current Position
				first_time_flag = '1';
			}
			//GPS IS not ready yet, Wait
			else{
				// printing Message on LCD
				LCD_Command(LCD_8BIT); // 2 line mode
				LCD_Command(LCD_INC); // inrement cursor
				LCD_Command(LCD_ROW1);	// prints on the first line
				LCD_Command(LCD_Hide); // Hide Cursor
				LCD_Command(CLEAR_SCREEN); // Clear
				LCD_String ("GPS LOOKING FOR SIGNAL ..."); //Prints Message
				delay(5000);				// wait 5 seconds
				// Just Testing The Switch
				sw_reset = READ_SW1();
				if (sw_reset == 0){
					// printing Message on LCD
					LCD_Command(LCD_8BIT); // 2 line mode
					LCD_Command(LCD_INC); // inrement cursor
					LCD_Command(LCD_ROW1);	// prints on the first line
					LCD_Command(LCD_Hide); // Hide Cursor
					LCD_Command(CLEAR_SCREEN); // Clear
					LCD_String ("SWITCH PRESSED"); //Prints Message
					delay(2000);				// wait 2 seconds
					sw_reset = 1;
				}
			}
		}
		//If Not First time, get current data, and Calculate distance
		else{
			// If Distance > 0, turn on Led
			if ((int)dis_double >= 100){
				LED_ON();
			}
			
			// Read Raw Data From GPS
			GPS_READ_STRING(raw_data);
			//Parse the $GPGLL Sentence Only 
			GET_GPGLL(raw_data, gpgll);
			//Parse the Latitude and Longitude
			parse_gps_string(gpgll, lat_str, lon_str);
			// Converting the string coordinates to doubles
			current_lat_double = strtod(lat_str, &ptr);
			current_lon_double = strtod(lon_str, &ptr);
			//Calculate Distance
			dis_double = distanceCoordinates (initial_lat_double/100, initial_lon_double/100, current_lat_double/100, current_lon_double/100); //Divide by 100 to make it in format Degrees.Minutes
			//Converts Double Distance to String Distance
			sprintf(dis_str,"%lf",dis_double);
			// printing Distance on LCD
			LCD_Command(LCD_8BIT); // 2 line mode
			LCD_Command(LCD_INC); // inrement cursor
			LCD_Command(LCD_ROW1);	// prints on the first line
			LCD_Command(LCD_Hide); // Hide Cursor
			LCD_Command(CLEAR_SCREEN); // Clear
			LCD_String ("Distance:");	//Prints Message
			LCD_Command(LCD_ROW2);	// prints on the second line
			LCD_String (dis_str);		//Prints the Actuall Distance
			LCD_String (" M");		//Prints the unit
			delay(3000);				// wait 3 seconds
			//USE THE SWITCH TO RESET
			sw_reset = READ_SW1();
			if (sw_reset == 0){
				// printing Message on LCD
				LCD_Command(LCD_8BIT); // 2 line mode
				LCD_Command(LCD_INC); // inrement cursor
				LCD_Command(LCD_ROW1);	// prints on the first line
				LCD_Command(LCD_Hide); // Hide Cursor
				LCD_Command(CLEAR_SCREEN); // Clear LCD
				LCD_String ("RESSETTING ..."); //Prints Message
				delay(2000);				// wait 2 seconds
				LED_OFF(); //Turn Off LED
				LCD_Command(CLEAR_SCREEN); // Clear LCD
				dis_double = 0; //Reset Distance Calculation
				//Reset Coordinates
				initial_lat_double = 0;
				initial_lon_double = 0;
				current_lat_double = 0;
				current_lon_double = 0;
				first_time_flag = '0'; //Reset First time flag
				sw_reset = 1; //Reset the switch flag
			}
		}
	}
}

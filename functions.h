#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

//put your function headers here
void LED_ON(void);
void LED_OFF(void);
int READ_SW1(void);
void LCD_Command(char command);
void LCD_Data(char data);
void LCD_String (char *str);
void LCD_Num(int num);
double distanceCoordinates (double lat1, double lon1, double lat2, double lon2);
char GPS_READ_CHAR();
void GPS_READ_STRING(char *raw_data);
void parse_gps_string(char* gps_raw_data,char * latitude ,char * longitude);
void GET_GPGLL(char *raw_data, char *gpgll);
void delay (int ms);

#endif

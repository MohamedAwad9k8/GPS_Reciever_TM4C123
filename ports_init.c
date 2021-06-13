#include"tm4c123.h"
#include "D:/Applications/Keil/EE319Kware/inc/tm4c123gh6pm.h"
#include"stdint.h"
#include "string.h"
#include "ports_init.h"

#define unlock 0x4C4F434B;

/*
Which pins to use, and how

	SWITCHES:
		PF0 -> SW2 (DIGITAL INPUT)
		PF4 -> SW1 (DIGITAL INPUT)
	LED:
    PF1 -> RED LED (DIGITAL OUTPUT)
	GPS:
    PE0 -> U7Rx // We only need RX to Recieve
	LCD:
    PC5 -> Enable (EN) 
		PC6 -> Read/Write (R/W)
    PC7 -> Register Select (RS) 
    PB0 -> DB0 
    PB1 -> DB1 
    PB2 -> DB2 
    PB3 -> DB3 
    PB4 -> DB4 
    PB5 -> DB5 
    PB6 -> DB6 
    PB7 -> DB7 
    */

void initialize_ports(void){
	
	// Activating the Ports B, C, D, E, F -> Mask 3E
		SYSCTL_RCGCGPIO_R |= 0x3E; //Initializes ports B, C, D, E ,F clks
		while ((SYSCTL_PRGPIO_R&0x2E)==0){}; //waiting for the ports' clocks to be activated
		
	// Activating the UART7 Clk -> Mask 80
		SYSCTL_RCGCUART_R |= 0x80; 
			
	// Unlocking the Ports C, D, F, E Port B doesnt need unlock 
		GPIO_PORTC_LOCK_R |= unlock;
		GPIO_PORTD_LOCK_R |= unlock;
		GPIO_PORTE_LOCK_R |= unlock;			
		GPIO_PORTF_LOCK_R |= unlock;
		
	// Unlocking The Pins that we will use
		GPIO_PORTC_CR_R |= 0xE0;	// unlocks pins PC5, PC6, PC7
		GPIO_PORTD_CR_R |= 0xFF;	// unlocks pins PD0 to PD7
		GPIO_PORTF_CR_R |= 0x13; 	// unlocks pins PF0, PF1, PF4
		GPIO_PORTE_CR_R |= 0x01;	// unlocks pin PE0
			
	// Initialize SW1, PF4 -> Mask 0x10
		GPIO_PORTF_DIR_R &= ~0x10; 				// Setting SW1 as Input, Input = 0
		GPIO_PORTF_DEN_R |= 0x10; 				// Enabling Digital
		GPIO_PORTF_AMSEL_R &= ~0x10; 			//Disable Analog
		GPIO_PORTF_AFSEL_R &= ~0x10;			// Disable the Alternate Function
		GPIO_PORTF_PCTL_R &= ~0x000F0000; // Clear the bits of PMC
		GPIO_PORTF_PUR_R |= 0x10; 				// Enable Pull Up Resistance			
	
	// Initialize SW2, PF0 -> Mask 0x01
		GPIO_PORTF_DIR_R &= ~0x01; 				// Setting SW2 as Input, Input = 0
		GPIO_PORTF_DEN_R |= 0x01;					// Enabling Digital
		GPIO_PORTF_AMSEL_R &= ~0x01; 			// Disable Analog
		GPIO_PORTF_AFSEL_R &= ~0x01; 			// Disable the Alternate Function
		GPIO_PORTF_PCTL_R &= ~0x0000000F; // Clear the bits of PMC
		GPIO_PORTF_PUR_R |= 0x01; 				// Enable Pull Up Resistance
			
	// Initialize RED LED, PF1 -> Mask 0x02
		GPIO_PORTF_DIR_R |= 0x02; 				// Setting Red Led as Output, Output = 1
		GPIO_PORTF_DEN_R |= 0x02;					// Enabling Digital
		GPIO_PORTF_AMSEL_R &= ~0x02; 			// Disable Analog
		GPIO_PORTF_AFSEL_R &= ~0x02; 			// Disable the Alternate Function
		GPIO_PORTF_PCTL_R &= ~0x000000F0; // Clear the bits of PMC
		GPIO_PORTF_DATA_R &= ~0x02; 			// Initialize With Zero
		
	// Initialize GPS Pins, PE0 -> Mask 0x01
		GPIO_PORTE_DIR_R &= ~0x01; 				// Setting PE0, RX as Input, Input = 0
		GPIO_PORTE_DEN_R |= 0x01;					// Enable Digital
		GPIO_PORTE_AMSEL_R &= ~0x01; 			// Disable Analog
		GPIO_PORTE_AFSEL_R |= 0x01; 			// Enable the Alternate Function
		GPIO_PORTE_PCTL_R |= 0x00000001;  // Set the PMC bits for UART mode
	
	// UART Configuration
		UART7_CTL_R &= ~0x01; //disables UART control
		UART7_IBRD_R = 104; // Desired BaudRate = 9600, Integer BaudRate Divisor = 104
		UART7_FBRD_R = 11; // Fractional Baud Rate Divisor = 21
		UART7_LCRH_R = 0x70; // 8-bit length, FIFO 16 bit enabled, No parity, 1 stop bit
		UART7_CTL_R |= 0x201; // Enables the UART, also works as RX
	
	// Initialize LCD Pins
		// Port C pins, PC5, PC6, PC7 -> Mask 0xE0
			GPIO_PORTC_DIR_R |= 0xE0; 				// Setting pins Output, Output = 1
			GPIO_PORTC_DEN_R |= 0xE0;					// Enabling Digital
			GPIO_PORTC_AMSEL_R &= ~0xE0; 			// Disable Analog
			GPIO_PORTC_AFSEL_R &= ~0xE0; 			// Disable the Alternate Function
			GPIO_PORTC_PCTL_R &= ~0xFFF00000; // Clear the bits of PMC
			GPIO_PORTC_DATA_R &= ~0xE0; 			// Initialize With Zero
		
		// port B pins, PD0 to PD7 -> Mask 0xFF
			GPIO_PORTB_DIR_R |= 0xFF; 				// Setting pins Output, Output = 1
			GPIO_PORTB_DEN_R |= 0xFF;					// Enabling Digital
			GPIO_PORTB_AMSEL_R &= ~0xFF; 			// Disable Analog
			GPIO_PORTB_AFSEL_R &= ~0xFF; 			// Disable the Alternate Function
			GPIO_PORTB_PCTL_R &= ~0xFFFFFFFF; // Clear the bits of PMC
			GPIO_PORTB_DATA_R &= ~0xFF; 			// Initialize With Zero

}



#include <unistd.h>
#include <stdio.h>
#include "initGPIO.h"
#include <wiringPi.h>
#include <stdlib.h>

void init_GPIO(unsigned int* gpioPtr, int code);
void clear_GPIO9(unsigned int* gpioPtr);
void clear_GPIO11(unsigned int* gpioPtr);


void Write_Latch(unsigned int* gpioPtr);
void Write_Clock(unsigned int* gpioPtr);
unsigned short Read_SNES(unsigned int* gpioPtr);
unsigned int Read_DATA(unsigned int* gpioPtr);
void Print_Message(int index, int index2);

#define BUTTON_B         (1 << 0)
#define BUTTON_Y         (1 << 1)
#define BUTTON_SEL       (1 << 2)
#define BUTTON_START     (1 << 3)
#define BUTTON_UP        (1 << 4)
#define BUTTON_DOWN      (1 << 5)
#define BUTTON_LEFT      (1 << 6)
#define BUTTON_RIGHT     (1 << 7)
#define BUTTON_A         (1 << 8)
#define BUTTON_X         (1 << 9)
#define BUTTON_L         (1 << 10)
#define BUTTON_R         (1 << 11)

/*
Init_GPIO: the subroutine initializes a GPIO line, the line number and function code
must be passed as parameters. The subroutine need not be general: it just needs
to work for the three SNES lines.
Write_Latch: writes a bit to the GPIO latch line
Write_Clock: writes to the GPIO Clock line
Read_Data: reads a bit from the GPIO data line
Wait: waits for a time interval, passed as a parameter.
Read_SNES: main SNES subroutine that reads input (buttons pressed) from a
SNES controller. Returns the code of a pressed button in a register.
Print_Message: prints an appropriate message.
*/


int main()
{
	// get gpio pointer
    unsigned int *gpioPtr = getGPIOPtr();  
	int latch = 0;
    int data = 1;
    int clock = 2;
    int button_combos[11] = {1, 256, 2, 512, 4, 32, 64, 128, 16, 1024, 2048};
    //const char * buttons[] = {"B", "Y", "SELECT", "UP", "DOWN", "LEFT", "RIGHT", "A", "X", "LEFT-TOP", "RIGHT-TOP"};

	
	//print out the program creator
    printf("Created by Sanjita Mitra and Monica Nguyen\n");

    //set GPIO pin 9 latch line to output
    init_GPIO(gpioPtr, latch);

    //set GPIO pin 10 data to input
    init_GPIO(gpioPtr, data);

    //set GPIO pin 11 clock to output
    init_GPIO(gpioPtr, clock);

    // Clear the LATCH line (GPIO 9) to low
    clear_GPIO9(gpioPtr);

    // Set CLOCK line (GPIO 11) to high
    Write_Clock(gpioPtr);

    //Request to press a button
    printf("Press a Button on the controller:\n");

    while(1){
        unsigned int button = Read_SNES(gpioPtr);
       // delayMicroseconds(1500);
        int noMulti = 20;
        int i,j;
       for (i=0; i<11; i++){
           if (button == BUTTON_START){
            printf("You have pressed START. Terminating Program.\n");
            exit(0);
        }
            if (button == button_combos[i]){
               Print_Message(i, noMulti);
               break;
           }
       }

        // JOYPAD with X
        if((button & BUTTON_LEFT)== BUTTON_LEFT && (button & BUTTON_X)== BUTTON_X){
                    printf("You have pressed JOYPAD LEFT AND X \n");
              }
        if((button & BUTTON_RIGHT)== BUTTON_RIGHT && (button & BUTTON_X)== BUTTON_X){
                    printf("You have pressed JOYPAD RIGHT AND X \n");
        }
        if((button & BUTTON_UP)== BUTTON_UP && (button & BUTTON_X)== BUTTON_X){
                    printf("You have pressed JOYPAD UP AND X \n");
        }   
        if((button & BUTTON_DOWN)== BUTTON_DOWN && (button & BUTTON_X)== BUTTON_X){
                    printf("You have pressed JOYPAD DOWN AND X \n");
        }   
        
        // JOYPAD with Y
        if((button & BUTTON_LEFT)== BUTTON_LEFT && (button & BUTTON_Y)== BUTTON_Y){
                    printf("You have pressed JOYPAD LEFT AND Y \n");
              }
        if((button & BUTTON_RIGHT)== BUTTON_RIGHT && (button & BUTTON_Y)== BUTTON_Y){
                    printf("You have pressed JOYPAD RIGHT AND Y \n");
        }
        if((button & BUTTON_UP)== BUTTON_UP && (button & BUTTON_Y)== BUTTON_Y){
                    printf("You have pressed JOYPAD UP AND Y \n");
        }   
        if((button & BUTTON_DOWN)== BUTTON_DOWN && (button & BUTTON_Y)== BUTTON_Y){
                    printf("You have pressed JOYPAD DOWN AND Y \n");
        } 

        //JOYPAD with B
        if((button & BUTTON_LEFT)== BUTTON_LEFT && (button & BUTTON_B)== BUTTON_B){
                    printf("You have pressed JOYPAD LEFT AND B \n");
              }
        if((button & BUTTON_RIGHT)== BUTTON_RIGHT && (button & BUTTON_Y)== BUTTON_B){
                    printf("You have pressed JOYPAD RIGHT AND B \n");
        }
        if((button & BUTTON_UP)== BUTTON_UP && (button & BUTTON_Y)== BUTTON_B){
                    printf("You have pressed JOYPAD UP AND B \n");
        }   
        if((button & BUTTON_DOWN)== BUTTON_DOWN && (button & BUTTON_Y)== BUTTON_B){
                    printf("You have pressed JOYPAD DOWN AND B \n");
        }         

        // JOYPAD with LEFT
        if((button & BUTTON_LEFT)== BUTTON_LEFT && (button & BUTTON_L)== BUTTON_L){
                    printf("You have pressed JOYPAD LEFT AND LEFT \n");
              }
        if((button & BUTTON_RIGHT)== BUTTON_RIGHT && (button & BUTTON_L)== BUTTON_L){
                    printf("You have pressed JOYPAD RIGHT AND LEFT \n");
        }
        if((button & BUTTON_UP)== BUTTON_UP && (button & BUTTON_L)== BUTTON_L){
                    printf("You have pressed JOYPAD UP AND LEFT \n");
        }   
        if((button & BUTTON_DOWN)== BUTTON_DOWN && (button & BUTTON_L)== BUTTON_L){
                    printf("You have pressed JOYPAD DOWN AND LEFT \n");
        }     
        
        // JOYPAD with RIGHT
        if((button & BUTTON_LEFT)== BUTTON_LEFT && (button & BUTTON_R)== BUTTON_R){
                    printf("You have pressed JOYPAD LEFT AND RIGHT \n");
              }
        if((button & BUTTON_RIGHT)== BUTTON_RIGHT && (button & BUTTON_R)== BUTTON_R){
                    printf("You have pressed JOYPAD RIGHT AND RIGHT \n");
        }
        if((button & BUTTON_UP)== BUTTON_UP && (button & BUTTON_R)== BUTTON_R){
                    printf("You have pressed JOYPAD UP AND RIGHT \n");
        }   
        if((button & BUTTON_DOWN)== BUTTON_DOWN && (button & BUTTON_R)== BUTTON_R){
                    printf("You have pressed JOYPAD DOWN AND RIGHT \n");
        }                       

        //BUTTONS XYAB with LEFT
        if((button & BUTTON_X)== BUTTON_X && (button & BUTTON_L)== BUTTON_L){
                    printf("You have pressed X AND LEFT \n");
              }
        if((button & BUTTON_Y)== BUTTON_Y && (button & BUTTON_L)== BUTTON_L){
                    printf("You have pressed Y AND LEFT \n");
        }
        if((button & BUTTON_A)== BUTTON_A && (button & BUTTON_L)== BUTTON_L){
                    printf("You have pressed A AND LEFT \n");
        }   
        if((button & BUTTON_B)== BUTTON_B && (button & BUTTON_L)== BUTTON_L){
                    printf("You have pressed B AND LEFT \n");
        }   

        //BUTTONS XYAB with RIGHT
        if((button & BUTTON_X)== BUTTON_X && (button & BUTTON_R)== BUTTON_R){
                    printf("You have pressed X AND RIGHT \n");
              }
        if((button & BUTTON_Y)== BUTTON_Y && (button & BUTTON_R)== BUTTON_R){
                    printf("You have pressed Y AND RIGHT \n");
        }
        if((button & BUTTON_A)== BUTTON_A && (button & BUTTON_R)== BUTTON_R){
                    printf("You have pressed A AND RIGHT \n");
        }   
        if((button & BUTTON_B)== BUTTON_B && (button & BUTTON_R)== BUTTON_R){
                    printf("You have pressed B AND RIGHT \n");
        }         

/*
        if (button == BUTTON_A){
            printf("You have pressed A : %d \n", button);
        }
        if (button == BUTTON_B){
            printf("You have pressed B : %d \n", button);
        }
        if (button == BUTTON_X){
            printf("You have pressed X : %d \n", button);
        }
        if (button == BUTTON_Y){
            printf("You have pressed Y : %d \n", button);
        }
        if (button == BUTTON_L){
            printf("You have pressed LEFT : %d \n", button);
        }
        if (button == BUTTON_R){
            printf("You have pressed RIGHT : %d \n", button);
        }
        if (button == BUTTON_LEFT){
            printf("You have pressed JOYPAD LEFT : %d \n", button);
        }
        if (button == BUTTON_RIGHT){
            printf("You have pressed JOYPAD RIGHT : %d \n", button);
        }
        if (button == BUTTON_UP){
            printf("You have pressed JOYPAD UP : %d \n", button);
        }
        if (button == BUTTON_DOWN){
            printf("You have pressed JOYPAD DOWN : %d \n", button);
        }
        if (button == BUTTON_SEL){
            printf("You have pressed SELECT : %d \n", button);
        }
         if (button == BUTTON_START){
            printf("You have pressed START. Terminating Program.\n");
            exit(0);
        }


        



*/
    }
  return 0;
}

//**************************PRINT MESSAGE************************

void Print_Message(int index, int index2){
char * buttons[11] = {"B", "A", "Y", "X", "SELECT", "JOYPAD DOWN", "JOYPAD LEFT", "JOYPAD RIGHT", "JOYPAD UP", "LEFT","RIGHT"};
char * mbuttons[11] = {"RIGHT", "LEFT", "JOYPAD UP", "JOYPAD RIGHT", "JOYPAD LEFT", "JOYPAD DOWN", "SELECT", "X", "Y", "A","B"};
if (index2 == 20){
printf("You have pressed %s\n", buttons[index]);
}
else{
printf("You have pressed %s and %s\n", buttons[index], mbuttons[index2]);
}

}
//**************************INITGPIO************************
//Funcion which initializes a GPIO line. The function is general for lines 9, 10 and 11
//Accepts 2 paramaters:
//   1) Line Number (r0)
//   2) Function Code (r1)
//Returns nothing
// Determines which function select register needs to be used as well as the passed in GPIO pin's offset
//Pin offset from GPIO Base address

void init_GPIO(unsigned int* gpioPtr, int code)
{
    
    unsigned int r;

    if (code == 0)
    {
    r = *(gpioPtr+((9)/10));

    // Clear bits 27 - 29. This is the field FSEL9, which maps to GPIO pin 9.
    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(7 << 27);

    // Set the field FSEL9 to 001, which sets pin 9 to an output pin.
    // We do so by ORing the bit pattern 001 into the field.
    r |= (1 << 27);

    // Write the modified bit pattern back to the
    // GPIO Function Select Register 0
    *(gpioPtr+((9)/10)) = r;
    }

    if (code ==1)
    {
    r = *(gpioPtr+((11)/10));
   
    // Clear bits 3 - 5. This is the field FSEL11, which maps to GPIO pin 11.
    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(7 << 3);

    // Set the field FSEL11 to 001, which sets pin 9 to an output pin.
    // We do so by ORing the bit pattern 001 into the field.
    r |= (1 << 3);
    *(gpioPtr+((11)/10)) = r;
    }

    if (code == 2)
    {

    unsigned int r;
    r = *(gpioPtr+((10)/10));
    // Clear bits 0 - 2. This is the field FSEL10, which maps to GPIO pin 10.
    // We clear the bits by ANDing with a 000 bit pattern in the field. This
    // sets the pin to be an input pin.
    r &= ~(0x7 << 0);
    }
}


//***************************WRITE LATCH*********************************
//Function which writes to GPIO latch line
//Accepts 1 parameter:
// Bit to be written (0,1)
//Returns nothing
// Function to set the value of GPIO pin 9 (Latch)

void Write_Latch(unsigned int* gpioPtr)
{
    register unsigned int r;
    // Put a 1 into the SET9 field of the GPIO Pin Output Set Register 0
   r = *(gpioPtr+7);
   r |= (0x1 << 9); // SET0 @ offset 0x1C, 28 / sizeof(unsigne int), 28/4 = 7
   *(gpioPtr+7) = r;
  //  *GPSET0 = gpioPtr;
}

//***************************WRITE CLOCK*********************************
//Function which writes to GPIO clock line
//Accepts 1 parameter
//Bit to be written (0,1)
//Returns nothing
void Write_Clock(unsigned int* gpioPtr)
{
   
     register unsigned int r;
    r = *(gpioPtr+7);
    r |= (0x1 << 11); // offset 0x28, 40 / 4 = 10
    // Put a 1 into the SET11 field of the GPIO Pin Output Set Register 0
    *(gpioPtr+7) = r;
}

//**************************READ SNES************************
//Read_SNES:  main  SNES  subroutine  that  reads  input  (buttons  pressed)  from  a 
//SNES controller. Returns the code of a pressed button in a register

unsigned short Read_SNES(unsigned int* gpioPtr){
    register unsigned int data;
    unsigned short button = 0;
    Write_Latch(gpioPtr); //write 1 to latch
    delayMicroseconds(125);
    clear_GPIO9(gpioPtr);
    delayMicroseconds(125);

    int i;
        for (i = 0; i < 16; i++){
            delayMicroseconds(125);
            data = Read_DATA(gpioPtr);
            if ( (data & (1 << 10)) == 0){
                button |= (1 << i);
              //  delayMicroseconds(750);
        }
            clear_GPIO11(gpioPtr);
            delayMicroseconds(10000);
            Write_Clock(gpioPtr);
        }

    return button;

    }

//**************************READ DATA************************

unsigned int Read_DATA(unsigned int* gpioPtr){
    register unsigned int r;
    r = *(gpioPtr+(13));
    return r;
}

//***************************CLEAR FUNCTIONS*********************************

void clear_GPIO9(unsigned int* gpioPtr)
{
    // Put a 1 into the CLR9 field of the GPIO Pin Output Clear Register 0
    register unsigned int r;
    r = *(gpioPtr+10);
    r |= (0x1 << 9); // offset 0x28, 40 / 4 = 10
    *(gpioPtr+10) = r;
}

void clear_GPIO11(unsigned int* gpioPtr)
{
    register unsigned int r;
    r = *(gpioPtr+10);
    r |= (0x1 << 11); // offset 0x28, 40 / 4 = 10
    // Put a 1 into the SET11 field of the GPIO Pin Output Set Register 0
    *(gpioPtr+10) = r;
}


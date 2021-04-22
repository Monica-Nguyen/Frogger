#include <unistd.h>
#include <stdio.h>
#include "initGPIO.h"
#include <wiringPi.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdbool.h>
#include "framebuffer.h"
#include "bomb.h"
#include "frog.h"
#include "log.h"
#include "truckright.h"
#include "lilypad.h"
#include "heart.h"
#include "froggerQuit.h"
#include "froggerStart.h"
#include "carleft.h"
#include "star.h"
#include "QuitPause.h"
#include "RestartPause.h"
#include "PlayAgainGameOver.h"
#include "QuitGameOver.h"

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


/* Definitions */
typedef struct {
	short int color;
	int x, y;
	int step;
} Pixel;

struct fbs framebufferstruct;
void drawPixel(Pixel *pixel);
void drawStartFrogger(Pixel *pixel);
void drawQuitFrogger(Pixel *pixel);
void drawGameBackground(Pixel *pixel);
void drawFrog(Pixel *pixel, int xPos, int yPos);
void drawQuitPause(Pixel *pixel);
void drawRestartPause(Pixel *pixel);
void clearScreen(Pixel *pixel);

int main()
{
	// get gpio pointer
    unsigned int *gpioPtr = getGPIOPtr();  
	int latch = 0;
    int data = 1;
    int clock = 2;
    //int button_combos[11] = {1, 256, 2, 512, 4, 32, 64, 128, 16, 1024, 2048};

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

	/* initialize + get FBS */
	framebufferstruct = initFbInfo();
	
	//short int *heartPtr=(short int *) heartImage.pixel_data;
	// short int *bombPtr=(short int *) bombImage.pixel_data;
	// short int *carLeftPtr=(short int *) carLeftImage.pixclerael_data;
	//short int *frogPtr=(short int *) frogImage.pixel_data;
	//short int *landPtr=(short int *) landImage.pixel_data;
	// short int *lilyPadPtr=(short int *) lilyPadImage.pixel_data;
	// short int *logPtr=(short int *) logImage.pixel_data;
	// short int *truckRightPtr=(short int *) truckRightImage.pixel_data;
	//short int *starPtr=(short int *) starImage.pixel_data;
	
	/* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));
    bool startFlag = false;
    int xPos = 20;
    int yPos = 21;
    int start = 1; // 0 is quit, 1 is start
    
    while(1){
        unsigned int button = Read_SNES(gpioPtr);
       // delayMicroseconds(1500);

        if (startFlag == false){
            drawStartFrogger(pixel);

            while(1){
                unsigned int button = Read_SNES(gpioPtr);
                if (button == BUTTON_RIGHT){
                    drawQuitFrogger(pixel); 
                    start = 0;                
                } 

                if (button == BUTTON_LEFT){
                    drawStartFrogger(pixel);
                    start = 1;                        
                }

                if (button == BUTTON_A && start == 1){
                    startFlag = true;  
                    drawGameBackground(pixel);
                    drawFrog(pixel, xPos, yPos);   
                    break;                 
                }

                if (button == BUTTON_A && start == 0){
                    clearScreen(pixel);
                    return(0);
                    break;
                }
            }
        }

        
        if (button == BUTTON_LEFT && xPos > 0){         
            xPos = xPos - 1;
            drawGameBackground(pixel);
            drawFrog(pixel, xPos, yPos); 
        }

        if (button == BUTTON_RIGHT && xPos < 39){
            xPos = xPos + 1;
            drawGameBackground(pixel);
            drawFrog(pixel, xPos, yPos); 
        }        

        if (button == BUTTON_UP && yPos < 22){
            yPos = yPos - 1;
            drawGameBackground(pixel);
            drawFrog(pixel, xPos, yPos); 

        }

        if (button == BUTTON_DOWN && yPos > 0){
            yPos = yPos + 1;
            drawGameBackground(pixel);
            drawFrog(pixel, xPos, yPos); 
        }  

        bool pause = false;
        if (button == BUTTON_START){
            pause = true;
            drawRestartPause(pixel);
            while(1){
                unsigned int button = Read_SNES(gpioPtr);
                if (button == BUTTON_DOWN){
                    drawQuitPause(pixel);
                }
                if (button == BUTTON_UP){
                    drawRestartPause(pixel);
                }
            }

        }

    }

    /* free pixel's allocated memory */
	free(pixel);
	pixel = NULL;
	munmap(framebufferstruct.fptr, framebufferstruct.screenSize);

    pthread_exit(NULL);

    return 0;
}

/* Draw a pixel */
void drawPixel(Pixel *pixel){
	long int location =(pixel->x +framebufferstruct.xOff) * (framebufferstruct.bits/8) +
                       (pixel->y+framebufferstruct.yOff) * framebufferstruct.lineLength;
	*((unsigned short int*)(framebufferstruct.fptr + location)) = pixel->color;
}

//**************************PRINT MESSAGE************************

void Print_Message(int index, int index2){
char * buttons[11] = {"B", "A", "Y", "X", "SELECT", "JOYPAD DOWN", "JOYPAD LEFT", "JOYPAD RIGHT", "JOYPAD UP", "LEFT","RIGHT"};
//char * mbuttons[11] = {"RIGHT", "LEFT", "JOYPAD UP", "JOYPAD RIGHT", "JOYPAD LEFT", "JOYPAD DOWN", "SELECT", "X", "Y", "A","B"};
    if (index2 == 20){
    printf("You have pressed %s\n", buttons[index]);
    }
    // else{
    // printf("You have pressed %s and %s\n", buttons[index], mbuttons[index2]);
    // }

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

void drawStartFrogger(Pixel *pixel){

    short int *startPtr=(short int *) froggerStart.pixel_data;
    
    int i=0;
	for (int y = 0; y < 720; y++) // height
	{
		for (int x = 0; x < 1280; x++) //width
		{	
				pixel->color = startPtr[i];
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;		
		}

	}
}

void drawQuitFrogger(Pixel *pixel){

    short int *quitPtr=(short int *) froggerQuit.pixel_data;
    
    int i=0;
	for (int y = 0; y < 720; y++) // height
	{
		for (int x = 0; x < 1280; x++) //width
		{	
				pixel->color = quitPtr[i];
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;		
		}

	}
}

void clearScreen(Pixel *pixel){

	for (int y = 0; y < 720; y++) // height
	{
		for (int x = 0; x < 1280; x++) //width
		{	
				pixel->color = 0x000000;
				pixel->x = x;
				pixel->y = y;
				drawPixel(pixel);		
		}

	}
}


void drawGameBackground(Pixel *pixel){

	short int *heartPtr=(short int *) heartImage.pixel_data;
	// short int *bombPtr=(short int *) bombImage.pixel_data;
	// short int *carLeftPtr=(short int *) carLeftImage.pixclerael_data;
	//short int *landPtr=(short int *) landImage.pixel_data;
	// short int *lilyPadPtr=(short int *) lilyPadImage.pixel_data;
	// short int *logPtr=(short int *) logImage.pixel_data;
	// short int *truckRightPtr=(short int *) truckRightImage.pixel_data;
	//short int *starPtr=(short int *) starImage.pixel_data;

	// Bottom Score and Stats Keeping bar
	//unsigned int quarter,byte,word;
	for (int y = 672; y < 720; y++) // height
	{
		for (int x = 0; x < 1280; x++) //width
		{	
				pixel->color = 0x000000;
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
		}
	}


	int lives = 4;
	// Lives 
	while (lives > 0){
		int i=0;
		for (int y = 685; y < 700; y++) // height
		{
			for (int x = 1150 + (lives % 5) * 16; x < 1166 + (lives % 5) * 16; x++) //width
			{	
					pixel->color = heartPtr[i];
					pixel->x = x;
					pixel->y = y;
					drawPixel(pixel);
					i++;
					
			}
		}
		lives = lives - 1; // Change the condition to remove them based on the game conditions
	}

	int grassPatches = 0;
	// Drawing the grass patches
	while(grassPatches < 5){
		for (int y = 640 - (grassPatches * 160); y < 672 - (grassPatches * 160); y++) // height
		{
			for (int x = 0; x < 1280; x++) //width
			{	
					pixel->color = 0x006400; // dark green
					pixel->x = x;
					pixel->y = y;
					drawPixel(pixel);
			}
		}
		grassPatches = grassPatches + 1;
	}

	int channels = 0;
	// Drawing the channels
	while(channels < 4){

		for (int y = 512 - channels * 160; y < 640 - channels * 160; y++) // height
		{
			for (int x = 0; x < 1280; x++) //width
			{	
                    if (channels == 1 || channels == 3){
                        pixel->color = 0x0000FF;
                    }
                    else{ 
					    pixel->color = 0x000000;
                    }

					pixel->x = x;
					pixel->y = y;
					drawPixel(pixel);	
			}

		}
		channels = channels + 1;
	}


}

void drawFrog(Pixel *pixel, int xPos, int yPos){

	short int *frogPtr=(short int *) frogImage.pixel_data;
    // The frog
	int i=0;
	for (int y = yPos*32-32; y < yPos*32; y++) // height
	{
		for (int x = xPos*32; x < xPos*32+32; x++) //width
		{	
				pixel->color = frogPtr[i];
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;		
		}

	}
}

void drawQuitPause(Pixel *pixel){
	short int *QuitPtr=(short int *) quitPause.pixel_data;

    int i=0;
	for (int y = 115; y < 605; y++) // height
	{
		for (int x = 393; x < 888; x++) //width
		{	
				pixel->color = QuitPtr[i];
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;		
		}

	}
}

void drawRestartPause(Pixel *pixel){

    short int *restartPtr=(short int *) restartPause.pixel_data;

    int i=0;
	for (int y = 115; y < 605; y++) // height
	{
		for (int x = 393; x < 888; x++) //width
		{	
				pixel->color = restartPtr[i];
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;		
		}

	}
}

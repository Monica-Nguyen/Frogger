#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "initGPIO.h"
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
void drawStar(Pixel *pixel, int starX, int starY);

//logic functions
void checkCollisions(int n);
void createGameArray();
void placeFrogger(int n, int m);
void resetFrogger(int n, int m);
void generateCarLocation();
void moveCarLocation();
void displayBoard();
void *mainRun();
void *runner (void *carID);
void placeCar(long car, int pos);

struct Game {
	long state;				//0 - is main menu, 1 - is playing game, 2 - game is paused
	int level;
	int timeremaining;
	int lives;
	int score;
	int moves;
	int startTime;
	bool gameOver;
	bool win;
	bool lose;
	bool displayOn;
	int boardArray[21][40];
    int carNum;
    int car[3];
}; 
struct Game g;


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
    int frogX = 20;
    int frogY = 19;
    int prevFX;
    int prevFY;
    int start = 1; // 0 is quit, 1 is start
    g.carNum = 1;
    int score = 0;
    
    while(1){
        pthread_t main_thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&main_thread, &attr, mainRun, NULL);
        //After main thread and threads finish
        pthread_exit(NULL);

        unsigned int button = Read_SNES(gpioPtr);
       // delayMicroseconds(1500);

        if (startFlag == false){
            xPos = 20;
            yPos = 21;
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
                    placeFrogger(frogX, frogY);  
                  //  displayBoard(); 
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
            prevFX = frogX;
            prevFY = frogY;
            xPos = xPos - 1;
            frogY = frogY - 1;
            drawGameBackground(pixel);
            drawFrog(pixel, xPos, yPos);
            resetFrogger(prevFX, prevFY);
            placeFrogger(frogX, frogY); 
            // long b = 0;
            // testPlaceCar(b, 39);
           // displayBoard();   
        }

        if (button == BUTTON_RIGHT && xPos < 39){
            prevFX = frogX;
            prevFY = frogY;
            xPos = xPos + 1;
            frogY = frogY + 1;
            drawGameBackground(pixel);
            drawFrog(pixel, xPos, yPos);
            resetFrogger(prevFX, prevFY);
            placeFrogger(frogX, frogY); 
            // long b = 0;
            // testPlaceCar(b, 39);
           // displayBoard();   
        }        


        if (button == BUTTON_UP && yPos < 22 && yPos > 1){
            prevFX = frogX;
            prevFY = frogY;
            yPos = yPos - 1;
            frogX = frogX - 1;
            drawGameBackground(pixel);
            drawFrog(pixel, xPos, yPos);
            resetFrogger(prevFX, prevFY);
            placeFrogger(frogX, frogY); 
            // long b = 0;
            // testPlaceCar(b, 39);
           // displayBoard();   

        }

        if (button == BUTTON_DOWN && yPos >= 0 && yPos < 21){
            prevFX = frogX;
            prevFY = frogY;
            yPos = yPos + 1;
            frogX = frogX + 1;
            drawGameBackground(pixel);
            drawFrog(pixel, xPos, yPos);
            resetFrogger(prevFX, prevFY);
            placeFrogger(frogX, frogY); 
            // long b = 0;
            // testPlaceCar(b, 39);
           // displayBoard();   
        }  

        int pause = 0; // 0 is restart, 1 is quit
        if (button == BUTTON_START){
            drawRestartPause(pixel);
            while(1){
                unsigned int button = Read_SNES(gpioPtr);
                if (button == BUTTON_DOWN){ // go to quit
                    drawQuitPause(pixel);
                    pause = 1;
                }
                if (button == BUTTON_UP){ // go to restart
                    drawRestartPause(pixel);                    
                    pause = 0;
                }
                if (button == BUTTON_START){ // exit menu 
                    drawGameBackground(pixel);
                    drawFrog(pixel, xPos, yPos);                     
                    break;
                }
                if (button == BUTTON_A && pause == 0){  // restart position 
                    int xPos = 20;
                    int yPos = 21;
                    drawGameBackground(pixel);
                    drawFrog(pixel, xPos, yPos);
                    break;
                }
                if (button == BUTTON_A && pause == 1){
                    int xPos = 20;
                    int yPos = 21;
                    drawStartFrogger(pixel);
                    startFlag = false;
                    break;
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


//**************************LOGIC FUNCTIONS***********************
void createGameArray() {
    int i, j;
    for (i = 0; i < 21; i++){
        for (j = 0; j < 40; j++){
            g.boardArray[i][j] = 0;                                 
        }
    }
}


void displayBoard(){
    int i, j;
    for (i=0; i<21; i++){
        for (j=0; j<40; j++){
            printf(" %d ", g.boardArray[i][j]);
}
        printf("\n");
        printf("-----------------------------------------------------------------------------------------------------------------------\n");
    }
    printf("\nEnd of Cycle\n");
}

//will update struct stats 
void placeFrogger(int n, int m){
	g.boardArray[n][m] = 5;
}

//will update struct stats 
void resetFrogger(int n, int m){
	g.boardArray[n][m] = 0;
}

void testPlaceCar(long car, int pos){
     if (car == 0){
        g.boardArray[18][pos] = 3;
    }

    if (car == 1){
        g.boardArray[19][pos] = 3;
    }

    if (car == 2){
        g.boardArray[20][pos] = 3;
    }
    
}

void placeCar(long car, int pos){
    if (car == 0){
           if (pos < 40){
                g.boardArray[18][pos] = 0;
            }
        g.boardArray[18][pos-1] = 3;
    }

    if (car == 1){
           if (pos < 40){
                g.boardArray[19][pos] = 0;
            }
        g.boardArray[19][pos-1] = 3;
    }

    if (car == 2){
           if (pos < 40){
                g.boardArray[20][pos] = 0;
            }
        g.boardArray[20][pos-1] = 3;
    }
    
 

    displayBoard();
}

//**************************MAIN THREAD**********************
void *mainRun()
    {
        //main thread waits until all horses finish the race
        //periodically displays the progress of each horse as an arrow
        //main thread should boot up home/pause screen
        g.startTime = time(NULL);
        g.gameOver = false;
        pthread_t carLogic[g.carNum];

		//Start state of game 
		long i = 1;
        for (i = 0; i < 2; i++){
        pthread_create(&carLogic[i], NULL, runner, (void *)i);
        }
	// while (!g.gameOver) {
    //         displayBoard();
	// 	}
	
  	//join threads so the main thread can gather the progress
      long p;
      for (p = 0; p < g.carNum; p++){
     pthread_join(carLogic[p], NULL);
	// pthread_join(gameDisplay, NULL); 
    } 	
}

void *runner (void *carID){
    int i;
    long h = (long) carID;
    for (i = 40; i > 0; i--){
        g.car[h]= i;
        sleep(2);
        placeCar(h, i);
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

	int channel = 0;
	// Drawing the channels
	while(channel < 4){

		for (int y = 512 - channel * 160; y < 640 - channel * 160; y++) // height
		{
			for (int x = 0; x < 1280; x++) //width
			{	
                    if (channel == 1 || channel == 3){
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
        
        srand(time(NULL));
        int starY;
        int starX;
        
        if (channel == 0){
            starY = rand() % 15;
            starY = starY / 4 + 17;
            starX = rand() % 39;
            drawStar(pixel, starX, starY);
        }

        else if (channel == 0){
            
        }

        else if (channel == 2){

        }
        
        else {

        }

        drawStar(pixel, starX, starY);

		channel = channel + 1;
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

void drawStar(Pixel *pixel, int starX, int starY){
	short int *starPtr=(short int *) starImage.pixel_data;

    int i=0;
	for (int y = starY * 32 - 32; y < starY * 32; y++) // height
	{
		for (int x = starX * 32; x < 32 + starX * 32; x++) //width
		{	
				pixel->color = starPtr[i];
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;		
		}

	}
}
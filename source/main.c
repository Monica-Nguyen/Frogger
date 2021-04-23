//CPSC 359 PROJECT PART 2
//SANJITA MITRA UCID: 10023967 and MONICA NGUYEN UCID: 30022256

#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <timeText.h>
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
#include "gameoverlost.h"
#include "gameoverwon.h"
#include "points.h"
#include "frogFoot.h"
#include "RestartPause.h"
#include "castle.h"
#include "pointsText.h"

//TO IDENTIFY BUTTONS PRESSED ON SNES CONTROLLER
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

//SNES CONTROLLER DRIVER FUNCTIONS
void init_GPIO(unsigned int* gpioPtr, int code);
void clear_GPIO9(unsigned int* gpioPtr);
void clear_GPIO11(unsigned int* gpioPtr);
void Write_Latch(unsigned int* gpioPtr);
void Write_Clock(unsigned int* gpioPtr);
unsigned short Read_SNES(unsigned int* gpioPtr);
unsigned int Read_DATA(unsigned int* gpioPtr);
void Print_Message(int index, int index2);

//STRUCT DEFINITIONS TO CONTROL GAME FLOW
typedef struct {
	short int color;
	int x, y;
	int step;
} Pixel;

//Initialize Draw Functions 
struct fbs framebufferstruct;
void drawPixel(Pixel *pixel);
void drawStartFrogger(Pixel *pixel);
void drawQuitFrogger(Pixel *pixel);
void drawGameBackground(Pixel *pixel);
void drawFrog(Pixel *pixel);
void drawQuitPause(Pixel *pixel);
void drawRestartPause(Pixel *pixel);
void clearScreen(Pixel *pixel);
void *drawStar(void *idk);
void drawCar(Pixel *pixel, int num);
void drawBomb(Pixel *pixel, int num);
void drawPad(Pixel *pixel, int num);
void drawLog(Pixel *pixel, int num);
void reDraw(Pixel *pixel);

//Initializa Logic Functions
void checkCarCollisions(int n);
void checkBombCollisions(int n);
void checkPadCollisions(int n, int m);
void checkLogCollisions(int n);
void createGameArray();
void placeFrogger(int n, int m);
void resetFrogger(int n, int m);
void generateCarLocation();
void moveCarLocation();
void displayBoard();
void *mainRun();
void *runner (void *carID);
void *brunner (void *bID);
void *prunner (void *pID);
void *lrunner (void *lID);
void *timeTicking (void *garbage);
void placeCar(long car, int pos);
void placeBomb(long bomb, int pos);
void placePad(long pad, int pos);
void placeLog(long log, int pos);


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
    int starX;
    int starY;
}; 
struct Game g;

struct Bomb {
    int bombX[3];
    int bombY[3];
    int progress[3];
}; 
struct Bomb b;

struct Car {
    int carX[3];
    int carY[3];
    int progress[3];
}; 
struct Car c;

struct Pad {
    int padX[4];
    int padY[4];
    int progress[4];
}; 
struct Pad p;

struct Log {
    int logX[4];
    int logY[4];
    int progress[4];
}; 
struct Log l;

struct frog {
    int xPos;
    int yPos;
};
struct frog f;

//Main Function will start the main thread
int main()
{

    pthread_t main_thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&main_thread, &attr, mainRun, NULL);
    //After main thread and threads finish
    pthread_exit(NULL);

}

//Main thread contains driver code and will branch to logic functions as needed
//Additionally, it will start the periphery worker threads for the objects
//**************************MAIN THREAD**********************
void *mainRun()
    {
    // get gpio pointer
    unsigned int *gpioPtr = getGPIOPtr();  
	int latch = 0;
    int data = 1;
    int clock = 2;

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
	
	/* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));
    bool startFlag = false;
    f.xPos = 20;
    f.yPos = 21;
    g.lose = false;
    g.win = false;
    g.lives = 4;
    g.moves = 92; // the frog foot will gradually get covered up by another pixel for every 2 moves 
    g.score = 0;
    g.starY = rand() % 21 + 1;
    g.starX = rand() % 39;    
    g.timeremaining = 120; // 120 seconds    
    int frogX = 20;
    int frogY = 20;
    int prevFX;
    int prevFY;
    int start = 1; // 0 is quit, 1 is start
    g.carNum = 3;

        //START OF THREADING
    g.gameOver = false;
    //declare the car threads
    pthread_t carLogic[g.carNum];
    //declare the bomb threads
    pthread_t bombLogic[3];
    //declare the pad threads
    pthread_t padLogic[4];
    //declare the log threads
    pthread_t logLogic[4];
    pthread_t drawingStar;

    //Thread to start a timer 
    pthread_t timeRemaining;

    //while loop to gather user input, decide start/stop state
    while(1){
        unsigned int button = Read_SNES(gpioPtr);
        
        if (startFlag == false){
            //default position of the frog
            f.xPos = 20;
            f.yPos = 21;
            g.lose = false;
            g.win = false;
            g.lives = 4;
            g.moves = 92; // the frog foot will gradually get covered up by another pixel for every 2 moves 
            g.score = 0;
            g.timeremaining = 120; // 120 seconds              
            
            //starting position of the frog
            drawStartFrogger(pixel);
            //this loop will gather user input
            while(1){
                unsigned int button = Read_SNES(gpioPtr);
                if (button == BUTTON_RIGHT){ //RIGHT BUTTON = QUIT SCREEN ON START MENU
                    drawQuitFrogger(pixel); 
                    start = 0;                
                } 

                if (button == BUTTON_LEFT){   //LEFT BUTTON = START GAME ON START MENU
                    drawStartFrogger(pixel);  
                    start = 1;                        
                }

                if (button == BUTTON_A && start == 0){ 
                    clearScreen(pixel);
                    return(0);
                    break;
                }

                //START GAME: kicks off the game environment
                if (button == BUTTON_A && start == 1){
                    startFlag = true;  
                    drawGameBackground(pixel);
                    drawFrog(pixel);
                    placeFrogger(frogX, frogY);
                    // //displayBoard();
                    // drawStar(pixel); // START A THREAD HERE AND MAKE IT SLEEP 30s

                    int idk;
                    pthread_create(&drawingStar, NULL, drawStar, (void *)idk);

                    int garbage;
                    pthread_create(&timeRemaining, NULL, timeTicking, (void *)garbage);

                    //create car threads with thread function: RUNNER
                    long i = 1;
                    for (i = 0; i < 3; i++){
                    pthread_create(&carLogic[i], NULL, runner, (void *)i);
                    // int delay = rand()%3;
                    // sleep(delay);
                    } 
                    //create bomb threads with thread function: BRUNNER
                    for (i = 0; i < 3; i++){
                    pthread_create(&bombLogic[i], NULL, brunner, (void *)i);
                    // int delay = rand()%3;
                    // sleep(delay);
                    } 
                    //create pad threads with thread function: PRUNNER 
                    for (i = 0; i < 4; i++){
                    pthread_create(&padLogic[i], NULL, prunner, (void *)i);
                    // int delay = rand()%3;
                    // sleep(delay);
                    } 
                    //create log threads with thread function: LRUNNER
                    for (i = 0; i < 4; i++){
                    pthread_create(&logLogic[i], NULL, lrunner, (void *)i);
                    // int delay = rand()%3;
                    // sleep(delay);
                    } 

                    //loops to the threads as they begin
                    for (int num = 0; num < 3; num++){
                    drawCar(pixel, num);
                    drawBomb(pixel, num);
                    }
                    for (int num = 0; num < 4; num++){
                    drawPad(pixel, num);
                    drawLog(pixel, num);
                    }
                    break;                 
                }
            }
        }   //end of start setup
       
       //the game is in progress and buttons will dictate additional steps
       //each move is tied in with back-end logic where an array tracks all positions
       //if left is pressed and x is in range
        if (button == BUTTON_LEFT && f.xPos > 0){       
            //saves previous frogger position to reset previous positions 
            prevFX = frogX;
            prevFY = frogY;
            //updates position with the left button (backend)
            f.xPos = f.xPos - 1;
            frogY = frogY - 1;
            g.moves = g.moves - 1;
            if(f.yPos > 11 && f.yPos < 16){
                if (g.boardArray[frogX][frogY] == 0){
                    g.lives = g.lives - 1;

                }
            }
            if(f.yPos > 1 && f.yPos < 6){
                if (g.boardArray[frogX][frogY] == 0){
                    g.lives = g.lives - 1;

                }
            }            
            //redraw all components
            reDraw(pixel);
            //erase previous position on backend array
            resetFrogger(prevFX, prevFY);
            //check if we are not colliding with this step
            // printf("\n CURRENT FROGX: %d, CURRENT FROG Y: %d", frogX, frogY);  
            // printf("\n CURRENT f.xPos: %d, CURRENT f.yPosY: %d", f.xPos, f.yPos); 
            checkFroggerCollisions(frogX, frogY);
            placeFrogger(frogX, frogY);
        }

        //if RIGHT is pressed and x is in range
        if (button == BUTTON_RIGHT && f.xPos < 39){
            //saves previous frogger position to reset previous positions 
            prevFX = frogX;
            prevFY = frogY;
            //updates position with the left button (backend)
            f.xPos = f.xPos + 1;
            frogY = frogY + 1;
            g.moves = g.moves - 1;    
            if(f.yPos > 11 && f.yPos < 16){
                if (g.boardArray[frogX][frogY] == 0){
                    g.lives = g.lives - 1;
                   
                }
            }
            if(f.yPos > 1 && f.yPos < 6){
                if (g.boardArray[frogX][frogY] == 0){
                    g.lives = g.lives - 1;
                 
                }
            }                    
            //redraw all components
            reDraw(pixel);
            //erase previous position on backend array  
            resetFrogger(prevFX, prevFY);
            //check if we are not colliding with this step
            checkFroggerCollisions(frogX, frogY);
            placeFrogger(frogX, frogY);  
        }        

        //if UP is pressed and y is in range
        if (button == BUTTON_UP && f.yPos < 22 && f.yPos > 1){
            //saves previous frogger position to reset previous positions 
            prevFX = frogX;
            prevFY = frogY;
            //updates position with the left button (backend)
            f.yPos = f.yPos - 1;
            frogX = frogX - 1;
            g.moves = g.moves - 1;            
            if(f.yPos > 11 && f.yPos < 16){
                if (g.boardArray[frogX][frogY] == 0){
                    g.lives = g.lives - 1;
                     
                }
            }
            if(f.yPos > 1 && f.yPos < 6){
                if (g.boardArray[frogX][frogY] == 0){
                    g.lives = g.lives - 1;
                     
                }
            }
            //redraw all components
            reDraw(pixel);  
            //erase previous position on backend array  
            resetFrogger(prevFX, prevFY);
            //check if we are not colliding with this step
            checkFroggerCollisions(frogX, frogY);
            placeFrogger(frogX, frogY); 
        }

        //if DOWN is pressed and y is in range
        if (button == BUTTON_DOWN && f.yPos >= 0 && f.yPos < 21){
            //saves previous frogger position to reset previous positions
            prevFX = frogX;
            prevFY = frogY;
            //updates position with the left button (backend)
            f.yPos = f.yPos + 1;
            frogX = frogX + 1;
            g.moves = g.moves - 1;     
            if(f.yPos > 11 && f.yPos < 16){
                if (g.boardArray[frogX][frogY] == 0){
                    g.lives = g.lives - 1;
                     
                }
            }
            if(f.yPos > 1 && f.yPos < 6){
                if (g.boardArray[frogX][frogY] == 0){
                    g.lives = g.lives - 1;
                    
                }
            }                   
            //redraw all components
            reDraw(pixel); 
            //erase previous position on backend array   
            resetFrogger(prevFX, prevFY);
            checkFroggerCollisions(frogX, frogY);
            placeFrogger(frogX, frogY); 
        }
        //continually refreshed for all components and their updated positions
        reDraw(pixel);  

        // This pops up a restart menu when hitting select and allows the user to choose between restart or quitting 
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
                    drawFrog(pixel);                     
                    break;
                }
                if (button == BUTTON_A && pause == 0){  // restart position 
                    f.xPos = 20;
                    f.yPos = 21;
                    drawGameBackground(pixel);
                    drawFrog(pixel);
                    break;
                }
                if (button == BUTTON_A && pause == 1){
                    f.xPos = 20;
                    f.yPos = 21;
                    pthread_cancel(carLogic);
                    pthread_cancel(bombLogic);
                    pthread_cancel(logLogic);
                    pthread_cancel(padLogic);
                    pthread_cancel(timeRemaining);                           
                    drawStartFrogger(pixel);
                    startFlag = false;
                    break;
                }

            }

        }

        //This is to determine the wins and losses 
        if (g.lives == 0 || g.timeremaining == 0 || g.moves == 0) {
            pthread_cancel(carLogic);
            pthread_cancel(bombLogic);
            pthread_cancel(logLogic);
            pthread_cancel(padLogic);
            pthread_cancel(timeRemaining);                
            g.lose = true;
        }

        if (f.yPos == 0){
            pthread_cancel(carLogic);
            pthread_cancel(bombLogic);
            pthread_cancel(logLogic);
            pthread_cancel(padLogic);
            pthread_cancel(timeRemaining);    
            g.win = true;
        }

        if (g.lose == true){
            short int *gameoverlostPtr=(short int *) gameoverlostImage.pixel_data;
            
            //draw game over lost message 
            int i=0;
            for (int y = 0; y < 720; y++) // height
            {
                for (int x = 0; x < 1280; x++) //width
                {	
                        pixel->color = gameoverlostPtr[i];
                        pixel->x = x;
                        pixel->y = y;
            
                        drawPixel(pixel);
                        i++;		
                }

            }

            while(1){
                //draw game over lost message 
                int i=0;
                for (int y = 0; y < 720; y++) // height
                {
                    for (int x = 0; x < 1280; x++) //width
                    {	
                            pixel->color = gameoverlostPtr[i];
                            pixel->x = x;
                            pixel->y = y;
                
                            drawPixel(pixel);
                            i++;		
                    }

                }

                //hit any button to continue 
                if (button == BUTTON_A || button == BUTTON_B || button == BUTTON_L || button == BUTTON_LEFT || button == BUTTON_R || button == BUTTON_RIGHT || button == BUTTON_SEL || button == BUTTON_START || button == BUTTON_UP || button == BUTTON_Y || button == BUTTON_X){                  
                    drawStartFrogger(pixel);
                    startFlag = false;
                    g.lose = false;
                    break;                    

                }


            }

        }

        // if the user wins display the message and allow escape based on any button input
        if (g.win == true){
            short int *gameoverwinPtr=(short int *) gameoverwonImage.pixel_data;
            int i=0;
            for (int y = 0; y < 720; y++) // height
            {
                for (int x = 0; x < 1280; x++) //width
                {	
                        pixel->color = gameoverwinPtr[i];
                        pixel->x = x;
                        pixel->y = y;
            
                        drawPixel(pixel);
                        i++;		
                }

            }
            while(1){
                int i=0;
                for (int y = 0; y < 720; y++) // height
                {
                    for (int x = 0; x < 1280; x++) //width
                    {	
                            pixel->color = gameoverwinPtr[i];
                            pixel->x = x;
                            pixel->y = y;
                
                            drawPixel(pixel);
                            i++;		
                    }

                }

                //hit any button to continue 
                if (button == BUTTON_A || button == BUTTON_B || button == BUTTON_L || button == BUTTON_LEFT || button == BUTTON_R || button == BUTTON_RIGHT || button == BUTTON_SEL || button == BUTTON_START || button == BUTTON_UP || button == BUTTON_Y || button == BUTTON_X){                   
                    drawStartFrogger(pixel);
                    startFlag = false;
                    g.win = false;                    
                    break;
                }                
                
            }            
        }

    }

    /* free pixel's allocated memory */
	free(pixel);
	pixel = NULL;
	munmap(framebufferstruct.fptr, framebufferstruct.screenSize);
    return 0;	
}

//*********************************************************************************
//**************************LOGIC FUNCTIONS*****************************************
//creates a 21x40 backend array to match screen
void createGameArray() {
    int i, j;
    for (i = 0; i < 21; i++){
        for (j = 0; j < 40; j++){
            g.boardArray[i][j] = 0;                                 
        }
    }
}
//print function to display board (for troubleshooting)
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

//places frogger in [n][m] location
//will update struct stats 
//Frogger backend encoding 5
void placeFrogger(int n, int m){
	g.boardArray[n][m] = 5;
}

//will update struct stats for previous position
void resetFrogger(int n, int m){
	g.boardArray[n][m] = 0;
}

//CAR backend encoding: 3
//place car function will update car thread location
//should start on the right and move left across the screen
//iterations will start at index 39
void placeCar(long car, int pos){
    
    /* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));

    //car thread ID 0: top most card thread
    if (car == 0){
            //1 step made = can wipe previous position
            if (pos < 39){
                //starting cell wiped
                g.boardArray[17][pos+1] = 0;
            }
            //current position entered into backend array
            //backend array most right cell is [n][39]
            //checks if the current position has the frogger == collision
            if (g.boardArray[17][pos] == 5){
                g.lives = g.lives - 1;
                f.xPos = 20;
                f.yPos = 21;
                placeFrogger(20,20);  
                reDraw(pixel);                 
                printf("\n You have decreased a life\n");
                printf("\n Lives Remaining: %d\n", g.lives);
            }
            //else it is fine for car to move into location

            g.boardArray[17][pos-1] = 3;
            c.carX[0] = pos;
            
    }
    //car thread ID = 1 = MIDDLE CAR
    //similar to other car threads
    if (car == 1){
            if (pos < 39){
                g.boardArray[18][pos+1] = 0;
            }
            if (g.boardArray[18][pos] == 5){
                g.lives = g.lives - 1;
                f.xPos = 20;
                f.yPos = 21;   
                reDraw(pixel);                
                placeFrogger(20,20);                               
                printf("\n You have decreased a life\n");
                printf("\n Lives Remaining: %d\n", g.lives);
            }

            g.boardArray[18][pos-1] = 3;
            c.carX[1] = pos;
            
    }
     //car thread ID = 2 = BOTTOM
    //similar to other car threads
    if (car == 2){
            if (pos < 39){
                g.boardArray[19][pos+1] = 0;
            }
            if (g.boardArray[19][pos] == 5){
                g.lives = g.lives - 1;
                f.xPos = 20;
                f.yPos = 21;      
                reDraw(pixel);                 
                placeFrogger(20,20);                             
                printf("\n You have decreased a life\n");
                printf("\n Lives Remaining: %d\n", g.lives);
        }

            g.boardArray[19][pos] = 3;
            c.carX[2] = pos;
         
    }
}

//Bomb backend encoding: 8
//place bomb function will update bomb thread location
//should start on the right and move left across the screen
//iterations will start at index 39
void placeBomb(long bomb, int pos){
    /* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));
    //first bomb thread
    if (bomb == 0){
            //has already moved one space and should wipe prev position
           if (pos < 39){
                g.boardArray[7][pos+1] = 0;
            }
            //check if it is about to collide with frogger (5)
            if (g.boardArray[7][pos] == 5){
                g.lives = g.lives - 1;
                f.xPos = 20;
                f.yPos = 21;   
                reDraw(pixel);                 
                placeFrogger(20,20);                                    
                printf("\n You have decreased a life\n");
                printf("\n Lives Remaining: %d\n", g.lives);
            }
            //ok for bomb to keep moving
                g.boardArray[7][pos] = 8;
                b.bombX[0] = pos;
    }
    //second bomb thread
    if (bomb == 1){
           if (pos < 39){
                g.boardArray[8][pos+1] = 0;
            }
            if (g.boardArray[8][pos] == 5){
                g.lives = g.lives - 1;
                f.xPos = 20;
                f.yPos = 21;  
                reDraw(pixel);                  
                placeFrogger(20,20);                                     
                printf("\n You have decreased a life\n");
                printf("\n Lives Remaining: %d\n", g.lives);
            }
                g.boardArray[8][pos] = 8;
                b.bombX[1] = pos;
    }
    //third bomb thread
    if (bomb == 2){
           if (pos < 39){
                g.boardArray[9][pos+1] = 0;
            }
            if (g.boardArray[9][pos] == 5){
                g.lives = g.lives - 1;
                f.xPos = 20;
                f.yPos = 21;   
                reDraw(pixel);                   
                placeFrogger(20,20);                                   
                printf("\n You have decreased a life\n");
                printf("\n Lives Remaining: %d\n", g.lives);
            }
                g.boardArray[9][pos] = 8;
                b.bombX[2] = pos;
    }
}

//Pad backend encoding: 7
//Pad thread works differently where it will hope for collision
//it will decrement life if not collided
//pads will move left to right on screen
void placePad(long pad, int pos){
    /* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));
    if (pad == 0){
        //pad has already taken a step and can wipe prev step
           if (pos > 0){
                g.boardArray[11][pos-1] = 0;
            }
            //update current pad position with gui and backend
                g.boardArray[11][pos] = 7;
                p.padX[0] = pos;
    }
    //second thread
    if (pad == 1){
           if (pos > 0){
                g.boardArray[12][pos-1] = 0;
            }
   
            //update current pad position with gui and backend
                g.boardArray[12][pos] = 7;
                p.padX[1] = pos;
    }

    //third thread
    if (pad == 2){
           if (pos > 0){
                g.boardArray[13][pos-1] = 0;
            }

            //update current pad position with gui and backend
            g.boardArray[13][pos] = 7;
            p.padX[2] = pos;
    }

    //fourth thread
    if (pad == 3){
           if (pos > 0){
                g.boardArray[14][pos-1] = 0;
            }

            //update current pad position with gui and backend
                g.boardArray[14][pos] = 7;
                p.padX[3] = pos;
    }
}

//Log encoding on backend: 4
//Log thread works differently where it will hope for collision
//it will decrement life if not collided
//pads will move left to right on screen
//similar logic to pad
void placeLog(long log, int pos){
    /* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));
    //first thread   
    if (log == 0){
           if (pos > 0){
                g.boardArray[1][pos-1] = 0;
            }

                g.boardArray[1][pos] = 4;
                l.logX[0] = pos;
    }
    //second thread
    if (log == 1){
           if (pos > 0){
                g.boardArray[2][pos-1] = 0;
            }

                g.boardArray[2][pos] = 4;
                l.logX[1] = pos;
    }
    //third thread
    if (log == 2){
           if (pos > 0){
                g.boardArray[3][pos-1] = 0;
            }

                g.boardArray[3][pos] = 4;
                l.logX[2] = pos;
    }
    //fourth thread
    if (log == 3){
           if (pos > 0){
                g.boardArray[4][pos-1] = 0;
            }

                g.boardArray[4][pos] = 4;
                l.logX[3] = pos;
    }
}

/*
//check collisions
void checkCarCollisions(int n){
 if ((g.boardArray[17][n-1] == 5) || (g.boardArray[18][n-1] == 5) ||  (g.boardArray[19][n-1] == 5)){
	 	g.lives = g.lives - 1;
        printf("\n You have decreased a life\n");
        printf("\n Lives Remaining: %d\n", g.lives);
 }
}

//check collisions
void checkBombCollisions(int n){
 if ((g.boardArray[7][n-1] == 5) || (g.boardArray[8][n-1] == 5) ||  (g.boardArray[9][n-1] == 5)){
	 	g.lives = g.lives - 1;
        printf("\n You have decreased a life\n");
        printf("\n Lives Remaining: %d\n", g.lives);
 }
}

//check collisions
void checkPadCollisions(int n, int m){
 if ((g.boardArray[n][m] == 0) || (g.boardArray[n][m] == 0) ||  (g.boardArray[n][m] == 0) || (g.boardArray[n][m] == 0)){
	 	g.lives = g.lives - 1;
        printf("\n You have decreased a life\n");
        printf("\n Lives Remaining: %d\n", g.lives);
 }
}

//check collisions
void checkLogCollisions(int n){
 if ((g.boardArray[1][n] == 5) || (g.boardArray[2][n] == 5) ||  (g.boardArray[3][n] == 5)|| (g.boardArray[4][n] == 5)){
	 	g.lives--;
        printf("\n You have decreased a life\n");
        printf("\n Lives Remaining: %d\n", g.lives);
 }
}
*/
//check collisions
void checkFroggerCollisions(int n, int m){
 if (!g.boardArray[n][m]==0){
	 	g.lives--;
        printf("\n You have decreased a life\n");
        printf("\n Lives Remaining: %d\n", g.lives);
 }
}


//**************************************************************************************
//***********************  THREAD FUNCTIONS FOR CAR, BOMB, PAD, LOG **********************
//RUNNER THREAD FOR CAR
//START ON RIGHT MOVE TO LEFT
void *runner (void *carID){
    //.carX = 39;
    c.carY[0] = 18;
    c.carY[1] = 19;
    c.carY[2] = 20;
    Pixel *cpixel;
	cpixel = malloc(sizeof(Pixel));
    int i;
    long h = (long) carID;
    for (i = 39; i > -1; i--){
        g.car[h]= i;
        placeCar(h, i);
        sleep(2);   //sleep to slow down the path
    }
    free(cpixel);
	cpixel = NULL;
}

//BRUNNER THREAD FOR BOMB
//START ON RIGHT MOVE TO LEFT
void *brunner (void *bID){
    b.bombY[0] = 8;
    b.bombY[1] = 9;
    b.bombY[2] = 10;
    Pixel *bpixel;
	bpixel = malloc(sizeof(Pixel));
    int i;
    long h = (long) bID;
    for (i = 39; i > -1; i--){
        placeBomb(h, i);
        sleep(2);
    }
    free(bpixel);
	bpixel = NULL;
}

//PRUNNER THREAD FOR PAD
//START ON LEFT MOVE TO RIGHT
void *prunner (void *pID){
    p.padY[0] = 12;
    p.padY[1] = 13;
    p.padY[2] = 14;
    p.padY[3] = 15;
    Pixel *ppixel;
	ppixel = malloc(sizeof(Pixel));
    int i;
    long h = (long) pID;
    for (i = 0; i < 40; i++){
        placePad(h, i);
        sleep(2);
    }
    free(ppixel);
	ppixel = NULL;
}

//PRUNNER THREAD FOR PAD
//START ON LEFT MOVE TO RIGHT
void *lrunner (void *lID){
    l.logY[0] = 2;
    l.logY[1] = 3;
    l.logY[2] = 4;
    l.logY[3] = 5;
    Pixel *lpixel;
	lpixel = malloc(sizeof(Pixel));
    int i;
    long h = (long) lID;
    for (i = 0; i < 40; i++){
        placeLog(h, i);
        sleep(2);
    }
    free(lpixel);
	lpixel = NULL;
}

// This will run as the program is running to tell how much time is remaining in the game 
void *timeTicking (void *garbage){
    while(1){
        sleep(1);
        g.timeremaining = g.timeremaining - 1;
        if (g.timeremaining == 0){
            g.lose = true;
        }
    }

}


//**************************************************************************
//****************************** DRAW FUNCTIONS *****************************

/* Draw a pixel */
void drawPixel(Pixel *pixel){
	long int location =(pixel->x +framebufferstruct.xOff) * (framebufferstruct.bits/8) +
                       (pixel->y+framebufferstruct.yOff) * framebufferstruct.lineLength;
	*((unsigned short int*)(framebufferstruct.fptr + location)) = pixel->color;
}

void reDraw(Pixel *pixel){
        drawGameBackground(pixel);
        for (int num = 0; num < 3; num++){
            drawCar(pixel, num);
            drawBomb(pixel, num);
        }
        for (int num = 0; num < 4; num++){
            drawPad(pixel, num);
            drawLog(pixel, num);
        }
        drawFrog(pixel);
        //drawCar(pixel, 0);  
        //drawCar(pixel, 1);
        //drawCar(pixel, 2);
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

    // Frog Foot to represent the number of moves left
    short int *frogFootPtr=(short int *) frogFootImage.pixel_data;

    int i=0;
    for (int y = 678; y < 713; y++) // height
    {
        for (int x = 25; x < 46 + 25; x++) //width
        {	
                pixel->color = frogFootPtr[i];
                pixel->x = x;
                pixel->y = y;
                drawPixel(pixel);
                i++;
                
        }
    }
    
    // Gradually cover up the frog foot as the lives decrement
    int movesHelper = (92 - g.moves) / 2;
    for (int y = 678; y < 713; y++) // height
    {
        for (int x = 25; x < 25 + movesHelper; x++) //width
        {	
                pixel->color = 0x000000;
                pixel->x = x;
                pixel->y = y;
                drawPixel(pixel);        
        }
    }

    // Time text that will show a bar showing how much time there is 
    short int *timeTextPtr=(short int *) timeTextImage.pixel_data;
    i = 0;
    for (int y = 681; y < 711; y++) // height
    {
        for (int x = 135; x < 135 + 52; x++) //width
        {	
                pixel->color = timeTextPtr[i];
                pixel->x = x;
                pixel->y = y;
                drawPixel(pixel);
                i++;        
        }
    }

    // Time bar
    for (int y = 688; y < 704; y++) // height
    {
        for (int x = 65+135; x < 135 + 65 + g.timeremaining; x++) //width
        {	
                pixel->color = 0xFFFFFF;
                pixel->x = x;
                pixel->y = y;
                drawPixel(pixel);        
        }
    }
    
	// Lives 
    int lifeHelper = g.lives;
	short int *heartPtr=(short int *) heartImage.pixel_data;    
	while (lifeHelper > 0){
		int i=0;
		for (int y = 685; y < 700; y++) // height
		{
			for (int x = 1150 + (lifeHelper % 5) * 16; x < 1166 + (lifeHelper % 5) * 16; x++) //width
			{	
					pixel->color = heartPtr[i];
					pixel->x = x;
					pixel->y = y;
					drawPixel(pixel);
					i++;
					
			}
		}
		lifeHelper = lifeHelper - 1; // Change the condition to remove them based on the game conditions
	}


    //points text
    short int *pointsTextPtr=(short int *) pointsTextImage.pixel_data;    
    i=0;
    int gap = 0;
    for (int y = 681; y < 711; y++) // height
    {
        for (int x = 465; x < 555; x++) //width
        {	
                pixel->color = pointsTextPtr[i];
                pixel->x = x;
                pixel->y = y;
                drawPixel(pixel);
                i++;
                gap = gap + 5;
                
        }
    }


    // score 
    g.score = g.timeremaining / 10 + g.moves / 10 + g.lives * 1; // Every 10 seconds is 10 points, every 10 moves left is 10 points, every life is 10 points 
    int scoreHelper = g.score;
	short int *pointsPtr=(short int *) pointsImage.pixel_data;    
	while (scoreHelper > 0){
		i=0;
		for (int y = 686; y < 706; y++) // height
		{
			for (int x = 565 + scoreHelper * 20; x < 585 + scoreHelper * 20; x++) //width
			{	
					pixel->color = pointsPtr[i];
					pixel->x = x;
					pixel->y = y;
					drawPixel(pixel);
					i++;
					
			}
		}
		scoreHelper = scoreHelper - 1; // Change the condition to remove them based on the game conditions
	}

    // Castle 
    short int *castlePtr=(short int *) castleImage.pixel_data;   
    i=0;
    for (int y = 0; y < 32; y++) // height
    {
        for (int x = 640; x < 672 ; x++) //width
        {	
                pixel->color = castlePtr[i];
                pixel->x = x;
                pixel->y = y;
                drawPixel(pixel);
                i++;
        }
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

		channel = channel + 1;
	}
}

void drawFrog(Pixel *pixel){

	short int *frogPtr=(short int *) frogImage.pixel_data;
    // The frog
	int i=0;
	for (int y = f.yPos*32-32; y < f.yPos*32; y++) // height
	{
		for (int x = f.xPos*32; x < f.xPos*32+32; x++) //width
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

void* drawStar(void *idk){
	/* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));
    
    short int *starPtr=(short int *) starImage.pixel_data;

    int i=0;
	for (int y = g.starY * 32; y < g.starY * 32 + 32; y++) // height
	{
		for (int x = g.starX * 32; x < 32 + g.starX * 32; x++) //width
		{	
				pixel->color = starPtr[i];
				pixel->x = x;
				pixel->y = y;
				drawPixel(pixel);
				i++;		
		}

	}

}

void drawCar(Pixel *pixel, int num){ 
	short int *carPtr=(short int *) carLeftImage.pixel_data;
     int i=0;
    
    if (num == 0){
    for (int y = c.carY[0] * 32 - 32; y < c.carY[0] * 32; y++) // height
    {
        for (int x = c.carX[0] * 32; x < 32 + c.carX[0] * 32; x++) //width
        {
                pixel->color = carPtr[i];
                pixel->x = x;
                pixel->y = y;

                drawPixel(pixel);
                i++;
        }

	}
    }

    if (num == 1){
        for (int y = c.carY[1] * 32 - 32; y < c.carY[1] * 32; y++) // height
        {
            for (int x = c.carX[1] * 32; x < 32 + c.carX[1] * 32; x++) //width
            {
                    pixel->color = carPtr[i];
                    pixel->x = x;
                    pixel->y = y;

                    drawPixel(pixel);
                    i++;
            }

        }
        }

    if (num == 2){
        for (int y = c.carY[2] * 32 - 32; y < c.carY[2] * 32; y++) // height
        {
            for (int x = c.carX[2] * 32; x < 32 + c.carX[2] * 32; x++) //width
            {
                    pixel->color = carPtr[i];
                    pixel->x = x;
                    pixel->y = y;

                    drawPixel(pixel);
                    i++;
            }

        }
        }
 
}

void drawBomb(Pixel *pixel, int num){ 
	short int *bombPtr=(short int *) bombImage.pixel_data;
     int i=0;
    
    if (num == 0){
    for (int y = b.bombY[0] * 32 - 32; y < b.bombY[0] * 32; y++) // height
    {
        for (int x = b.bombX[0] * 32; x < 32 + b.bombX[0] * 32; x++) //width
        {
                pixel->color = bombPtr[i];
                pixel->x = x;
                pixel->y = y;

                drawPixel(pixel);
                i++;
        }

	}
    }

    if (num == 1){
        for (int y = b.bombY[1] * 32 - 32; y < b.bombY[1] * 32; y++) // height
        {
            for (int x = b.bombX[1] * 32; x < 32 + b.bombX[1] * 32; x++) //width
            {
                    pixel->color = bombPtr[i];
                    pixel->x = x;
                    pixel->y = y;

                    drawPixel(pixel);
                    i++;
            }

        }
        }

    if (num == 2){
        for (int y = b.bombY[2] * 32 - 32; y < b.bombY[2] * 32; y++) // height
        {
            for (int x = b.bombX[2] * 32; x < 32 + b.bombX[2] * 32; x++) //width
            {
                    pixel->color = bombPtr[i];
                    pixel->x = x;
                    pixel->y = y;

                    drawPixel(pixel);
                    i++;
            }

        }
        }
 
}

void drawPad(Pixel *pixel, int num){ 
	short int *padPtr=(short int *) lilypadImage.pixel_data;
     int i=0;
    
    if (num == 0){
    for (int y = p.padY[0] * 32 - 32; y < p.padY[0] * 32; y++) // height
    {
        for (int x = p.padX[0] * 32; x < 32 + p.padX[0] * 32; x++) //width
        {
                pixel->color = padPtr[i];
                pixel->x = x;
                pixel->y = y;

                drawPixel(pixel);
                i++;
        }

	}
    }

    if (num == 1){
        for (int y = p.padY[1] * 32 - 32; y < p.padY[1] * 32; y++) // height
        {
            for (int x = p.padX[1] * 32; x < 32 + p.padX[1] * 32; x++) //width
            {
                    pixel->color = padPtr[i];
                    pixel->x = x;
                    pixel->y = y;

                    drawPixel(pixel);
                    i++;
            }

        }
        }

    if (num == 2){
        for (int y = p.padY[2] * 32 - 32; y < p.padY[2] * 32; y++) // height
        {
            for (int x = p.padX[2] * 32; x < 32 + p.padX[2] * 32; x++) //width
            {
                    pixel->color = padPtr[i];
                    pixel->x = x;
                    pixel->y = y;

                    drawPixel(pixel);
                    i++;
            }

        }
        }

    if (num == 3){
        for (int y = p.padY[3] * 32 - 32; y < p.padY[3] * 32; y++) // height
        {
            for (int x = p.padX[3] * 32; x < 32 + p.padX[3] * 32; x++) //width
            {
                    pixel->color = padPtr[i];
                    pixel->x = x;
                    pixel->y = y;

                    drawPixel(pixel);
                    i++;
            }

        }
        }
 
}

void drawLog(Pixel *pixel, int num){ 
	short int *logPtr=(short int *) logImage.pixel_data;
     int i=0;
    
    if (num == 0){
    for (int y = l.logY[0] * 32 - 32; y < l.logY[0] * 32; y++) // height
    {
        for (int x = l.logX[0] * 32; x < 32 + l.logX[0] * 32; x++) //width
        {
                pixel->color = logPtr[i];
                pixel->x = x;
                pixel->y = y;

                drawPixel(pixel);
                i++;
        }

	}
    }

    if (num == 1){
        for (int y = l.logY[1] * 32 - 32; y < l.logY[1] * 32; y++) // height
        {
            for (int x = l.logX[1] * 32; x < 32 + l.logX[1] * 32; x++) //width
            {
                    pixel->color = logPtr[i];
                    pixel->x = x;
                    pixel->y = y;

                    drawPixel(pixel);
                    i++;
            }

        }
        }

    if (num == 2){
        for (int y = l.logY[2] * 32 - 32; y < l.logY[2] * 32; y++) // height
        {
            for (int x = l.logX[2] * 32; x < 32 + l.logX[2] * 32; x++) //width
            {
                    pixel->color = logPtr[i];
                    pixel->x = x;
                    pixel->y = y;

                    drawPixel(pixel);
                    i++;
            }

        }
        }

    if (num == 3){
        for (int y = l.logY[3] * 32 - 32; y < l.logY[3] * 32; y++) // height
        {
            for (int x = l.logX[3] * 32; x < 32 +l.logX[3] * 32; x++) //width
            {
                    pixel->color = logPtr[i];
                    pixel->x = x;
                    pixel->y = y;

                    drawPixel(pixel);
                    i++;
            }

        }
        }
 
}

//********************************  DRIVER FUNCTIONS ************************************
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

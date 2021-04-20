#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "framebuffer.h"
#include "bomb.h"
#include "frog.h"
#include "land.h"
#include "log.h"
#include "truckright.h"
#include "lilypad.h"
#include "heart.h"
#include "carleft.h"
#include "star.h"
#include "mainmenu.h"


/* Definitions */
typedef struct {
	short int color;
	int x, y;
	int step = 32;
} Pixel;

struct fbs framebufferstruct;
void drawPixel(Pixel *pixel);

/* main function */
int main(){

	/* initialize + get FBS */
	framebufferstruct = initFbInfo();
	
	short int *heartPtr=(short int *) heartImage.pixel_data;
	// short int *bombPtr=(short int *) bombImage.pixel_data;
	// short int *carLeftPtr=(short int *) carLeftImage.pixclerael_data;
	short int *frogPtr=(short int *) frogImage.pixel_data;
	//short int *landPtr=(short int *) landImage.pixel_data;
	// short int *lilyPadPtr=(short int *) lilyPadImage.pixel_data;
	// short int *logPtr=(short int *) logImage.pixel_data;
	// short int *truckRightPtr=(short int *) truckRightImage.pixel_data;
	short int *mainMenuPtr=(short int *) mainMenu.pixel_data;
	//short int *starPtr=(short int *) starImage.pixel_data;
	
	/* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));
	
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
					pixel->color = 0x000000;
					pixel->x = x;
					pixel->y = y;
					drawPixel(pixel);	
			}

		}
		channels = channels + 1;
	}

	// The frog
	int i=0;
	for (int y = 640; y < 672; y++) // height
	{
		for (int x = 624; x < 656; x++) //width
		{	
				pixel->color = frogPtr[i];
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;		
		}

	}


	/* free pixel's allocated memory */
	free(pixel);
	pixel = NULL;
	munmap(framebufferstruct.fptr, framebufferstruct.screenSize);
	
	return 0;
}


/* Draw a pixel */
void drawPixel(Pixel *pixel){
	long int location =(pixel->x +framebufferstruct.xOff) * (framebufferstruct.bits/8) +
                       (pixel->y+framebufferstruct.yOff) * framebufferstruct.lineLength;
	*((unsigned short int*)(framebufferstruct.fptr + location)) = pixel->color;
}



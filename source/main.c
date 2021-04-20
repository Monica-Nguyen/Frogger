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
} Pixel;

struct fbs framebufferstruct;
void drawPixel(Pixel *pixel);

/* main function */
int main(){

	/* initialize + get FBS */
	framebufferstruct = initFbInfo();
	
	// short int *heartPtr=(short int *) heartImage.pixel_data;
	// short int *bombPtr=(short int *) bombImage.pixel_data;
	// short int *carLeftPtr=(short int *) carLeftImage.pixclerael_data;
	// short int *frogPtr=(short int *) frogImage.pixel_data;
	short int *landPtr=(short int *) landImage.pixel_data;
	// short int *lilyPadPtr=(short int *) lilyPadImage.pixel_data;
	// short int *logPtr=(short int *) logImage.pixel_data;
	// short int *truckRightPtr=(short int *) truckRightImage.pixel_data;
	// short int *mainMenuPtr=(short int *) mainMenu.pixel_data;
	short int *starPtr=(short int *) starImage.pixel_data;
	
	/* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));
	int i=0;
	//unsigned int quarter,byte,word;
	for (int y = 0; y < 16; y++) // height
	{
		for (int x = 0; x < 1280; x++) //width
		{	
				pixel->color = 0x000000;
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;
				
		}
	}

	i=0;

	for (int y = 16; y < 48; y++) // height
	{
		for (int x = 0; x < 1280; x++) //width
		{	
				pixel->color = landPtr[i]; 
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


#ifndef FRAMEBUFF
#define FRAMEBUFF

/* definitions */
struct fbs initFbInfo(void) {
	char *fptr;		// framebuffer pointer
	int xOff; 					// x offset
	int yOff = 150;					// y offset
	int bits;					// bits per pixel
	int lineLength;				// Line Length
	float screenSize;			// Screen Size
};


#endif

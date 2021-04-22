#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main(){
    srand(time(NULL));
    int starY;
    int starX;

    for(int i = 0; i < 100; i++){

        starY = rand() % 16;
        starY = starY / 4;
        starX = rand() % 40;
        printf("%d y HERE %d x HERE\n", starY, starX);
    }

}



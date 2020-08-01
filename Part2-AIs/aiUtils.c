//add includes
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "aiUtils.h"

//add defines
#define a 16807
#define c 0
#define m (2^31 - 1)


//declare local functions

unsigned int random();


//initialise functions 

//return a random number
unsigned int random() {
    //set X as a random number
    static unsigned int X;
    srand(X)
    X = 0 ? srand(X) : X;
    X = (a * X + c) % m;
    return X;
}

int randomRange(int arrayLen) {
    // call random and get range
    return random() % (arrayLen+1);
}

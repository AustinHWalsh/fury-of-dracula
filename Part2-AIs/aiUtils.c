//add includes
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "aiUtils.h"

//add defines
#define a 16807
#define c 56123
#define m ((2^31) - 1)


//declare local functions

unsigned int randomN();


//initialise functions 

//return a random number
unsigned int randomN() {
    //set X as a random number
    static unsigned int X;
    srand(X);
    X == 0 ? srand(X) : X;
    X = (a * X + c) % m;
    return X;
}

int randomRange(int arrayLen) {
    // call random and get range
    return randomN() % (arrayLen+1);
}

char *convertAbbrev(const char *abbrev) {
    // cast to char *
    return (char *)abbrev;
}
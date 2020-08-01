//add includes
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "aiUtils.h"

//add defines

//initialise functions 

int randomRange(int arrayLen) {
    // call random and get range
    return random() % (arrayLen+1);
}

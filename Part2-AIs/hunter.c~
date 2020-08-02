////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"
#include "aiUtils.h"

void decideHunterMove(HunterView hv)
{
	////////////////////////////////////////////////////////
	// For whoever builds this tomorrow morning,
	// (0.) Check if round is 0, if so place the hunter in 
	// a city of your choice, it doesnt matter
	// (1.) Call getvalidmoves to find where drac can go
	// if the pointer passed into valid moves is 0:
	// registerBestPlay to be TP (teleport)
	// (2.) Call hvwherecanigo and find the len of the array
	// if len is 0, registerbestplay to a rand loc on map
	// (only happens if the hunter hasnt moved yet)
	// (3.) Call randomRange from aiUtils.h, passing in len
	// of the hvwherecanigo array. Get output array index and 
	// call registerbestplay passing in the city in the
	// hvwehrecanigo array using the index from randomRange
	// -----------------------------------------------------
	// dont forget to create the arrays for steps (1.) & (2.)
	// and make the pointer for (1.)
	// also as dvwherecanigo returns placeids, make sure to
	// convert it to abbrev before passing it into register
	// bestplay
	// -----------------------------------------------------
	// MOST IMPORTANTLY: READ WHAT EVERY FUNCTION DOES, 
	// REGISTERBESTPLAY PLAYER.C etc etc.
	////////////////////////////////////////////////////////

	// explaining what randomRange does
	int arrayLen = 6;
	// the newArrayPos will always be < arrayLen
	int newArrayPos = randomRange(arrayLen);

	registerBestPlay("TO", "Have we nothing Toulouse?");
}

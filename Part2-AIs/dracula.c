////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// dracula.c: your "Fury of Dracula" Dracula AI
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include "aiUtils.h"

void decideDraculaMove(DraculaView dv)
{
	if (dv->round == 0) {
		dv->playerLocation[PLAYER_DRACULA] == _______ //(decide what location to put for dracula)
	}
	int *numReturnedMoves; // not sure if this should be initialised or not
	PlaceId *validMoves = malloc(NUM_REAL_PLACES * sizeof(PlaceId));
	validMoves = DvGetValidMoves(dv, numReturnedMoves);

	if (validMoves == 0) {
		registerBestPlay(TP, _____ ); //fill out message, not sure what 
	}
	
	// not sure if this is right
	PlaceId *reachable = GvGetReachableByType(dv->gv, PLAYER_DRACULA, 1,
	                                          DvWhereAmI(dv), road, false,
	                                          boat, &numReachable); 
	
	// not sure what the value for numReturnedLocs should be 
	reachable = DvWhereCanIGo(dv, numReturnedLocs); 
	int moveNum = randomRange(len(reachable));
	if (len(reachable)) == 0) {
		registerBestPlay(validMoves[moveNum], ______) // enter message 
	}
	
	registerBestPlay(reachable[moveNum], ______) // enter message 
	
	////////////////////////////////////////////////////////
	// For whoever builds this tomorrow morning,
	// (0.) Check if round is 0, if so place dracula in 
	// a city of your choice, it doesnt matter
	// (1.) Call getvalidmoves to find where drac can go
	// if the pointer passed into valid moves is 0:
	// registerBestPlay to be TP (teleport)
	// (2.) Call dvwherecanigo and find the len of the array
	// if len is 0, use any of the other moves from (1.)
	// in registerBestPlay, i.e. HI, D1, D2, etc
	// (3.) Call randomRange from aiUtils.h, passing in len
	// of the dvwherecanigo array. Get output array index and 
	// call registerbestplay passing in the city in the
	// dvwehrecanigo array using the index from randomRange
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
	
	registerBestPlay("CD", "Mwahahahaha");
}

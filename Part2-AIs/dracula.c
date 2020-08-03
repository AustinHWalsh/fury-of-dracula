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

#include <stdio.h>

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include "aiUtils.h"
#define DRAC_NEXT_MOVE reachable[moveNum] 

void decideDraculaMove(DraculaView dv)
{
	// first round
	Round roundNum = DvGetRound(dv);
	if (roundNum == 0) {
		registerBestPlay("MI", "start!"); //milan
		return;
	}
	
	int numOfReach = 0, count;
	PlaceId *validMoves = DvGetValidMoves(dv, &numOfReach);

	// no valid moves must teleport
	if (numOfReach == 0) 
		registerBestPlay("TP", "haha!"); 
	
	numOfReach = 0;
	PlaceId *reachable = DvWhereCanIGo(dv, &numOfReach);

	int moveNum = randomRange(numOfReach);
	// no places to go, just double back/hide
	if (numOfReach == 0) {
		registerBestPlay(placeIdToAbbrev(validMoves[numOfReach]), "haha!"); 
		return;
	}

	// if dracula has less than 10 health, try to get him to move
	// away from the sea
	if (DvGetHealth(dv, PLAYER_DRACULA) <= 10) {
		// find a move in his reachable that isnt the sea
		while (placeIdToType(DRAC_NEXT_MOVE) == SEA) {
			if (moveNum+count > numOfReach) {
					
			}
		}
	}
	

	// if the location is in the hunter's reachable, randomise again
	for (int i = 0; i < NUM_PLAYERS-1; i++) {
		int locs = 0;
		Player player = PLAYER_LORD_GODALMING+i;
		PlaceId *hunterMoves = DvWhereCanTheyGo(dv, player, &locs);
		// test each reachable 
		for (int j = 0; j < locs; j++) {
			if (DRAC_NEXT_MOVE == hunterMoves[j]) {
				moveNum = randomRange(numOfReach);
				break;
			}
		}
	}

	//if random location is already occupied by hunter, randomise again
	for (int i = PLAYER_LORD_GODALMING; i < PLAYER_DRACULA; i++) {
		if (DvGetPlayerLocation(dv, i) == DRAC_NEXT_MOVE) {
			moveNum = randomRange(numOfReach);
			break;
		} 
	}

	// move to random location
	registerBestPlay(placeIdToAbbrev(DRAC_NEXT_MOVE), "ha!"); 
}

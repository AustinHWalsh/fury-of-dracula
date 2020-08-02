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

void decideDraculaMove(DraculaView dv)
{
	// first round
	if (DvGetRound(dv) == 0) {
		registerBestPlay("MI", "start!"); //milan
		return;
	}
		
	int num = 0;
	PlaceId *validMoves = DvGetValidMoves(dv, &num);

	// no valid moves must teleport
	if (num == 0) 
		registerBestPlay("TP", "haha!"); 
	
	num = 0;
	PlaceId *reachable = DvWhereCanIGo(dv, &num);

	int moveNum = randomRange(num);
	char *loc = PLACES[reachable[moveNum]].abbrev;
	// no places to go, just double back/hide
	if (num == 0) {
		registerBestPlay((char *)placeIdToAbbrev(validMoves[num]), "haha!"); 
		return;
	}
		
	//if random location is already occupied by hunter, randomise again
	for (int i = PLAYER_LORD_GODALMING; i < PLAYER_DRACULA; i++) {
		if (DvGetPlayerLocation(dv, i) == reachable[moveNum]) {
			moveNum = randomRange(num);
			loc = PLACES[reachable[moveNum]].abbrev;
			break;
		}
	}
	// move to random location
	registerBestPlay(loc, "ha!"); 
}

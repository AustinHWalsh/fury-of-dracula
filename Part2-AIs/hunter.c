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

#include <stdio.h>

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"
#include "aiUtils.h"

void decideHunterMove(HunterView hv)
{
	// first round
    if (HvGetRound(hv) == 0) {
		registerBestPlay("LV", "start!");
		return;
	}

	int num = 0;

	// find where the hunter can go
	PlaceId *reachable = HvWhereCanIGo(hv, &num);
	
	// no reachable locations, move to liverpool
	if (num == 0) 
		registerBestPlay("LV", "move random!");
	else {
		// get random location
		int moveNum = randomRange(num);
		char *loc = PLACES[reachable[moveNum]].abbrev;
		registerBestPlay(loc, "moving :)"); // enter message 
	}
}

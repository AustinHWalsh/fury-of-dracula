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
#include <string.h>

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"
#include "aiUtils.h"

void decideHunterMove(HunterView hv)
{
	// first round
    if (HvGetRound(hv) == 0) {
		if (HvGetPlayer(hv) == PLAYER_LORD_GODALMING)
			registerBestPlay("MN", "start!"); //manchester
		else if (HvGetPlayer(hv) == PLAYER_DR_SEWARD)
			registerBestPlay("FR", "start!"); //frankfurt
		else if (HvGetPlayer(hv) == PLAYER_VAN_HELSING)
			registerBestPlay("SR", "start!"); //saragossa
		else if (HvGetPlayer(hv) == PLAYER_MINA_HARKER)
			registerBestPlay("SZ", "start!"); //szeged
		return;
	}

	/*
	int currPlayer = HvGetPlayer(hv);

	int DracLocation = HvGetPlayerLocation(hv, PLAYER_DRACULA);

	//if dracula enters location of curr hunter, hunter stays where they are
	if (currPlayer != PLAYER_DRACULA
		//dracula's current location is revealed
		&& DracLocation != NOWHERE
		&& DracLocation != CITY_UNKNOWN
		&& DracLocation != SEA_UNKNOWN) {

		if (HvGetPlayerLocation(hv, currPlayer) == HvGetPlayerLocation(hv, PLAYER_DRACULA)) {
			//const char *currPlayerLocAbbrev = placeIdToAbbrev(HvGetPlayerLocation(hv, currPlayer));
			//char *currPlayerLoc = malloc(sizeof(currPlayerLocAbbrev));
			//strcpy(currPlayerLoc, currPlayerLocAbbrev);
			registerBestPlay(placeIdToAbbrev(HvGetPlayerLocation(hv, currPlayer)), "Die, Dracula!");
			return;
		}
	}

	int shortestPathLen;
	PlaceId *shortestPath;

	//hunters head towards the first revealed location of Dracula
	//Round 1
	if (currPlayer != PLAYER_DRACULA && HvGetRound(hv) == 1) {
		shortestPath = HvGetShortestPathTo(hv,currPlayer, DracLocation, &shortestPathLen);
		//follow Drac if he is within 5 moves
		if (shortestPathLen <= 5) {
			if (shortestPathLen == 1)
				registerBestPlay(placeIdToAbbrev(DracLocation), "You were next to me all along!");
			else
				registerBestPlay(placeIdToAbbrev(shortestPath[0]), "Behind you, Dracula.");
			return;
		}
	}

	int lastRevealedRound;
	PlaceId lastKnownDracLoc = HvGetLastKnownDraculaLocation(hv, &lastRevealedRound);

	//head towards Dracula after round 1
	if (currPlayer != PLAYER_DRACULA && HvGetRound(hv) > 1) {
		//follow Drac if last location is within 5 moves
		if (shortestPathLen <= 5) {
			shortestPath = HvGetShortestPathTo(hv,currPlayer, lastKnownDracLoc, &shortestPathLen);
			if (shortestPathLen == 1)
				registerBestPlay(placeIdToAbbrev(lastKnownDracLoc), "Reached Drac's last known location.");
			else
				registerBestPlay(placeIdToAbbrev(shortestPath[0]), "Behind you, Dracula.");
			return;
		}
	}
	*/
	int num = 0;

	// find where the hunter can go
	PlaceId *reachable = HvWhereCanIGo(hv, &num);
	
	// no reachable locations, move to liverpool
	if (num == 0) 
		registerBestPlay("LV", "move random!");
	else {
		// get random location
		int moveNum = randomRange(num);

		//if random location is already occupied by another hunter, randomise again
		for (int i = PLAYER_LORD_GODALMING; i < PLAYER_DRACULA; i++) {
			if (HvGetPlayerLocation(hv, i) == reachable[moveNum] && i != HvGetPlayer(hv)) {
				moveNum = randomRange(num);
				break;
			}
		}
		registerBestPlay(placeIdToAbbrev(reachable[moveNum]), "moving :)"); // enter message 
	}
}

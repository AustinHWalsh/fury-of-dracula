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
#include <stdbool.h>

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"
#include "aiUtils.h"

//move to Castle Dracula
void moveToCD(int currPlayer, HunterView hv);

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

	/**/
	bool goToCD = true;

	int currPlayer = HvGetPlayer(hv); //current hunter

	int DracLocation = HvGetPlayerLocation(hv, PLAYER_DRACULA);

	int shortestPathLen;
	PlaceId *shortestPath;

	int whereDracCanGoLen;
	PlaceId *whereDracCanGo;

	int lastRevealedRound;

	if (HvGetPlayerLocation(hv, currPlayer) == CASTLE_DRACULA
		|| HvGetLastKnownDraculaLocation(hv, &lastRevealedRound) != CASTLE_DRACULA
		|| HvGetPlayerLocation(hv, currPlayer) == HvGetPlayerLocation(hv, PLAYER_DRACULA)) {
		//stop hunters from going to CD if currPlayer has already reached it
		//or Dracula has a new revealed location
		//or hunter encounters Dracula
		goToCD = false;
	}

	if (goToCD == true) {
		//continue heading to CD
		moveToCD(currPlayer, hv);
		return;
	}

	//hunters head towards the first revealed location of Dracula
	//Round 1
	if (HvGetRound(hv) == 1) {
		moveToCD(currPlayer, hv);
		return;
		/*shortestPath = HvGetShortestPathTo(hv,currPlayer, DracLocation, &shortestPathLen);
		//follow Drac if he is within 5 moves
		if (shortestPathLen <= 5) {
			if (shortestPathLen == 1)
				registerBestPlay(placeIdToAbbrev(DracLocation), "You were next to me all along!");
			else
				registerBestPlay(placeIdToAbbrev(shortestPath[0]), "Behind you, Dracula.");
			return;
		}*/
	}

	//if drac's current loc is revealed
	//use whereCanTheyGo on drac
	//find the location furthest away from hunter
	//use shortest path to that location

	
	if (//dracula's current location is revealed
		DracLocation != NOWHERE
		&& DracLocation != CITY_UNKNOWN
		&& DracLocation != SEA_UNKNOWN) {
		//if dracula enters location of curr hunter, hunter stays where they are
		if (HvGetPlayerLocation(hv, currPlayer) == HvGetPlayerLocation(hv, PLAYER_DRACULA)) {
			//const char *currPlayerLocAbbrev = placeIdToAbbrev(HvGetPlayerLocation(hv, currPlayer));
			//char *currPlayerLoc = malloc(sizeof(currPlayerLocAbbrev));
			//strcpy(currPlayerLoc, currPlayerLocAbbrev);
			registerBestPlay(placeIdToAbbrev(HvGetPlayerLocation(hv, currPlayer)), "Die, Dracula!");
			return;
		}
		whereDracCanGo = HvWhereCanTheyGo(hv, PLAYER_DRACULA, &whereDracCanGoLen);
		//dracula has nowhere to go, next move is teleport to CD
		if (whereDracCanGoLen == 0)
			moveToCD(currPlayer, hv);
		else {
			//find the path of location furthest away from hunter and follow it
			int longestPath = -1;
			for (int i = 0; i < whereDracCanGoLen; i++) {
				shortestPath = HvGetShortestPathTo(hv, currPlayer, whereDracCanGo[i], &shortestPathLen);
				if (shortestPathLen > longestPath)
					longestPath = shortestPathLen;
			}
			for (int i = 0; i < whereDracCanGoLen; i++) {
				shortestPath = HvGetShortestPathTo(hv, currPlayer, whereDracCanGo[i], &shortestPathLen);
				if (shortestPathLen == longestPath)
					break;
			}
			registerBestPlay(placeIdToAbbrev(shortestPath[0]), "Predicting Drac's path.");
		}
		return;
	}

	
	PlaceId lastKnownDracLoc = HvGetLastKnownDraculaLocation(hv, &lastRevealedRound);
	shortestPath = HvGetShortestPathTo(hv,currPlayer, lastKnownDracLoc, &shortestPathLen);
	
	//head towards Dracula last known location after round 1
	//follow Drac if last location is within 5 moves
	if (shortestPathLen <= 5 && shortestPathLen != 0) {
		
		if (shortestPathLen == 1)
			registerBestPlay(placeIdToAbbrev(lastKnownDracLoc), "Reached Drac's last known location.");
		else
			registerBestPlay(placeIdToAbbrev(shortestPath[0]), "Behind you, Dracula.");
		return;
	}

	/**/
	int num = 0;

	// find where the hunter can go
	PlaceId *reachable = HvWhereCanIGo(hv, &num);
    if (num != 0)
	    num--;
    
	// no reachable locations, move to liverpool

	// get random location
	int moveNum = randomRange(num);
	//if random location is already occupied by another hunter, randomise again
	for (int i = PLAYER_LORD_GODALMING; i < PLAYER_DRACULA; i++) {
		if (HvGetPlayerLocation(hv, i) == reachable[moveNum] && i != HvGetPlayer(hv)) {
			moveNum = randomRange(num);
			break;
		}
	}
		
	registerBestPlay(placeIdToAbbrev(reachable[moveNum]), "Rand Move"); // enter message 
}

//find shortest path to CD and follow it
void moveToCD(int currPlayer, HunterView hv) {
	int shortestPathLen;
	PlaceId *shortestPath = HvGetShortestPathTo(hv, currPlayer, CASTLE_DRACULA, &shortestPathLen);
	if (shortestPathLen == 1)
		registerBestPlay("CD", "Reach Castle Dracula.");
	else
		registerBestPlay(placeIdToAbbrev(shortestPath[0]), "Heading to Castle Dracula.");
	return;
}
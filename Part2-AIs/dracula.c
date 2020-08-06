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
#define DRAC_NEXT_MOVE validMoves[moveNum] 



void decideDraculaMove(DraculaView dv)
{
	// first round
	Round roundNum = DvGetRound(dv);
	if (roundNum == 0) {
		registerBestPlay("MI", "start!"); //milan
		return;
	}
	
	int numOfReach = 0, count = 0;
	PlaceId *validMoves = DvGetValidMoves(dv, &numOfReach);

	// no valid moves must teleport
	if (numOfReach == 0) {
		registerBestPlay("TP", "haha!");
		return; 
	}
			
	int moveNum = randomRange(numOfReach);
	
	// no places to go, just double back/hide
	for (int i = 0; i < NUM_PLAYERS-1; i++) {
		int locs = 0;
		Player player = PLAYER_LORD_GODALMING+i;
		PlaceId *hunterMoves = DvWhereCanTheyGo(dv, player, &locs);
		// test each reachable 
		for (int j = 0; j < locs; j++) {
			if (DRAC_NEXT_MOVE == hunterMoves[j]) {
				for (int k = 0; k < numOfReach; k++) {
					if (validMoves[k] == HIDE) {
						registerBestPlay("HI", "LATER!");
						return;
					}
				}
				
			}
		}
	}
	
	//drac does double back when he sees that hunters are getting close 
	//but are not in his wherecanigo locs and have less health
	//so that he can go back and lay more traps in his trail

	// check if hunters are in range of whereCanIGo array of drac
	 for (int i = 0; i < NUM_PLAYERS-1; i++) {
		int locs = 0;
		Player player = PLAYER_LORD_GODALMING+i;
		PlaceId *hunterMoves = DvWhereCanTheyGo(dv, player, &locs);
		for (int j = 0; j < locs; j++) {
			if (DRAC_NEXT_MOVE == hunterMoves[j]) {
				moveNum = randomRange(numOfReach);
				break;
			}
			else {
				int hunterHealth = DvGetHealth(dv, player);
				if (hunterHealth <= 4) {
					for (int k = 0; k < locs; k++) {
						PlaceId dracCurrLoc = validMoves[k];
						if (dracCurrLoc == hunterMoves[j]) {
							if (validMoves[k] == DOUBLE_BACK_1) {
								registerBestPlay("D1", "haha!");
								return;
							}
							else if (validMoves[k] == DOUBLE_BACK_2) {
								registerBestPlay("D2", "haha!");
								return;
							}
							else if (validMoves[k] == DOUBLE_BACK_3) {
								registerBestPlay("D3", "haha!");
								return;
							}
							else if (validMoves[k] == DOUBLE_BACK_4) {
								registerBestPlay("D4", "haha!");
								return;
							}
							else if (validMoves[k] == DOUBLE_BACK_5) {
								registerBestPlay("D5", "haha!");
								return;
							}
						}
					}
				}
			}
		}
	} 
	//if true, check if their health is less than 4, if yes then go to 
	//the nearest place of that hunter w a double back and repeat 
	//else, if hunters are close but health is more than 4, then 
	
	// if dracula has less than 10 health, try to get him to move
	// away from the sea
	if (DvGetHealth(dv, PLAYER_DRACULA) <= 10) {
		// find a move in his reachable that isnt the sea
		while (placeIdToType(DRAC_NEXT_MOVE) == SEA) {
			moveNum++;
			// restrict index to be within array
			if (moveNum >= numOfReach) {
				// reset moveNum to 0 and increase count
				moveNum = 0;
				// stops infinite loops from occuring
				count++;
			}
			// if this while loop has occurred twice without breaking
			// break it manually
			if (count == 2) {
				// reset moveNum
				moveNum = randomRange(numOfReach);
				break;
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

	
    if (numOfReach == moveNum) {
        moveNum--;
    }
        
    
	// move to random location
	registerBestPlay(placeIdToAbbrev(DRAC_NEXT_MOVE), "ha!"); 
}

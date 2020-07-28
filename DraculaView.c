////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.c: the DraculaView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
// add your own #includes here

// TODO: ADD YOUR OWN STRUCTS HERE
typedef struct playerInfo {
	Player name;				 // player's name
	int health;					 // health of the player
	PlaceId currLocation;		 // the player's location
	PlaceId *prevMoves; 		 // players past moves in a dynamic array
} PlayerInfo;

struct draculaView {
	// TODO: ADD FIELDS HERE
	Round roundNum;
	int gameScore;
	char *pastPlays;
	Player currPlayer;
	Map m;
	PlayerInfo *allPlayers;
};

// declare your own functions here
void completePlayerTrails(GameView gv, char *startId, Player player);
void completePastPlays(GameView gv, char *pastPlays);

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	// create new DraculaView
	DraculaView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}
    
    // set up values
	new->roundNum = (strlen(pastPlays)+1) / 8;
	new->gameScore = GAME_START_SCORE;
	new->allPlayers = malloc(NUM_PLAYERS * sizeof(PlayerInfo));
	if (new->allPlayers == NULL) {
		fprintf(stderr, "Couldn't allocate allPlayers!\n");
		exit(EXIT_FAILURE);
	}

	// copy the pastplays string
	new->pastPlays = malloc(strlen(pastPlays) * sizeof(char));
	memcpy(new->pastPlays, pastPlays, strlen(pastPlays));
		
	new->m = MapNew();
	
	// initialise most of the player info in the arrays
	for (int i = 0; i < NUM_PLAYERS; i++) {
		// set name
		new->allPlayers[i].name = PLAYER_LORD_GODALMING + i;
        
		// create space for previous trails and fill them with 
		new->allPlayers[i].prevMoves = malloc((new->roundNum+7) * sizeof(Place));
		if (new->allPlayers[i].prevMoves == NULL) {
			fprintf(stderr, "Couldn't allocate trail!\n");
			exit(EXIT_FAILURE);
		}
		for (int j = 0; j < (new->roundNum+7); j++)
			new->allPlayers[i].prevMoves[j] = TBA_LOCATION;

		// set player heatlh
		if (new->allPlayers[i].name != PLAYER_DRACULA)
			new->allPlayers[i].health = GAME_START_HUNTER_LIFE_POINTS;
		else
			new->allPlayers[i].health = GAME_START_BLOOD_POINTS;
		
	}
	
	// fill in trails and calculate game scores and health
	completePastPlays(new, pastPlays);
	
	return new;
}

void DvFree(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	
	// loop through each player in the all players array
	for(int i = 0; i < NUM_PLAYERS; i++) 
		free(dv->allPlayers[i].prevMoves);
	
	free(dv->pastPlays);
	free(dv->allPlayers);
	
	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return (dv->roundNum * 7)/35;
}

int DvGetScore(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return dv->gameScore;
}

int DvGetHealth(DraculaView dv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return dv->allPlayers[player].health;
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	//player has not had a turn
	if (dv->allPersons[player].currLocation.name == NULL)
	    return NOWHERE;
	    
    //player has a current location
    return dv->allPlayers[player].currLocation;
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	int r = (GvGetRound(gv) - 1) % 13;
	//an immature vampire exists
	if (0 <= r && r < 6) {
		//temp copy of pastPlays for strtok
		char *pastPlayCpy = malloc(strlen(gv->pastPlays) * sizeof(char));
		strcpy(pastPlayCpy, gv->pastPlays);
		//find location of vamp in pastPlay
		char *currStr = strtok(pastPlayCpy, "\n");
		// go to correct roundNum line to search for V
		for (int i = 0; i < GvGetRound(gv) - r - 1; i++) {
			currStr = strtok(pastPlayCpy, "\n");
		}
		char *prevStr;
		char vampLoc[LOCATION_ABBREVIATION_MAX];
		//if no lines were skipped, read first playermove in the line
		if (r == 0) {
		    currStr = strtok(pastPlayCpy, ". ");
		}
		//read Dracula location when V was spawned and assign it as vampire's location
		while (currStr != NULL) {
			if (strcmp(currStr, "V") == 0) {
				vampLoc[0] = prevStr[1];
		    	vampLoc[1] = prevStr[2];
		    	
		    	break;
			}
			prevStr = currStr;
			currStr = strtok(NULL, ". ");
		}
		//vampire cannot exist in the sea
		if (placeIdToType(placeAbbrevToId(vampLoc)) == SEA) 
			return NOWHERE;
		//city is not revealed
		/*if (strcmp(vampLoc, "C?") == 0 )
			return CITY_UNKNOWN;*/
		//search if hunters have visited vampire's city
		char *hunterLoc = &currStr[1];
		while (currStr != NULL) {
			currStr = strtok(NULL, ". \n");
			hunterLoc = &currStr[1];
			if (currStr[0] != 'D' && strcmp(hunterLoc, vampLoc) == 0)
				//vampire has been vanquished
				return NOWHERE;
		}
		//hunters have not visited city the vampire is in
		return placeAbbrevToId(vampLoc);
	}
	//vampire not spawned, not immature or has been vanquished
	return NOWHERE;
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numTraps = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	return NULL;
}

PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

// initialise the trails of each person in the allPersons array
// using the pastPlays string
void completePlayerTrails(GameView gv, char *startId, Player player) {
	// create the abbreviation of the city from the paststring
	char cityAbbrev[3] = {startId[0], startId[1], '\0'};
	PlaceId cityId = placeAbbrevToId(cityAbbrev);

	for (int i = 0; i < MIN_TRAIL; i++) {
		// first empty stop in trail
		if (gv->allPlayers[player].prevMoves[i] == TBA_LOCATION) 
			gv->allPlayers[player].prevMoves[i] = cityId;
			
	}

	// reset that player's current position to the first in the trail
	gv->allPlayers[player].currLocation = cityId; 
} 

// clean up and complete the remaining required gv elements,
// such as current score and player health
// dependencies on the pastPlays string
void completePastPlays(GameView gv, char *pastPlays) {
	
	// when its the first round
	if (gv->roundNum == 0) {
		gv->currPlayer = PLAYER_LORD_GODALMING;
		for (int i = 0; i < NUM_PLAYERS; i++) {
			Player roundPlayer = PLAYER_LORD_GODALMING + i;
			// set all player's trail to NOWHERE
			gv->allPlayers[roundPlayer].prevMoves[0] = NOWHERE;
			gv->allPlayers[roundPlayer].currLocation = NOWHERE;

			// set remaining info
			gv->allPlayers[i].name = roundPlayer;
			if (roundPlayer != PLAYER_DRACULA)
				gv->allPlayers[i].health = GAME_START_HUNTER_LIFE_POINTS;
			else
				gv->allPlayers[i].health = GAME_START_BLOOD_POINTS;
		}
	}

	// do actions from each round
	for (int i = 0; i < gv->roundNum; i++) {
		// array position of the start of the round (players name)
		int startOfRound = i * ROUND_DIFF;
		
		// get current round's player
		Player roundPlayer;
		switch(pastPlays[startOfRound]) {
			case 'G':
				roundPlayer = PLAYER_LORD_GODALMING;
				gv->currPlayer = PLAYER_DR_SEWARD;
				break;
			case 'S':
				roundPlayer = PLAYER_DR_SEWARD;
				gv->currPlayer = PLAYER_VAN_HELSING;
				break;
			case 'H':
				roundPlayer = PLAYER_VAN_HELSING;
				gv->currPlayer = PLAYER_MINA_HARKER;
				break;
			case 'M':
				roundPlayer = PLAYER_MINA_HARKER;
				gv->currPlayer = PLAYER_DRACULA;
				break;
			case 'D':
				roundPlayer = PLAYER_DRACULA;
				gv->currPlayer = PLAYER_LORD_GODALMING;
				// dracula's turn reduces point by 1
				gv->gameScore -= SCORE_LOSS_DRACULA_TURN;
				break;
		}
        
		// add the player's current location to the trail
		completePlayerTrails(gv, &pastPlays[startOfRound+1], roundPlayer);

		// not dracula
		if (roundPlayer != PLAYER_DRACULA) {
			// if they are in the hospital because of losing lifepoints
			if (GvGetPlayerLocation(gv, roundPlayer) == ST_JOSEPH_AND_ST_MARY && gv->allPlayers[roundPlayer].health <= 0)
				gv->allPlayers[roundPlayer].health = GAME_START_HUNTER_LIFE_POINTS;

			// check each action in the round
			for (int i = 3; pastPlays[startOfRound+i] != '.'; i++) {
				switch(pastPlays[startOfRound+i]) {
					case 'T': // encountered trap
						gv->allPlayers[roundPlayer].health -= LIFE_LOSS_TRAP_ENCOUNTER;
						break;
					case 'D': // endcountered dracula
						gv->allPlayers[roundPlayer].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
						gv->allPlayers[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
						break;
					case 'V': {// encountered vampire
						char *tmpPastPlays = malloc(strlen(gv->pastPlays)*sizeof(char));
						strcpy(tmpPastPlays, gv->pastPlays);
						tmpPastPlays[startOfRound+i] = '.';
						strcpy(gv->pastPlays, tmpPastPlays);
						free(tmpPastPlays);
						break;
					}
				}
			}

			// when the hunter loses all their health, determine what to
			if (gv->allPlayers[roundPlayer].health <= 0) {
				gv->allPlayers[roundPlayer].health = 0;
				gv->gameScore -= SCORE_LOSS_HUNTER_HOSPITAL;
				return;
			}

			// when the player didnt move and didnt just got sent to hospital
			if (gv->allPlayers[roundPlayer].prevMoves[0] == gv->allPlayers[roundPlayer].prevMoves[1]
				&& gv->allPlayers[roundPlayer].health <= 0)
				gv->allPlayers[roundPlayer].health += LIFE_GAIN_REST;

			// reduce hunters health to max if it is over
			if (gv->allPlayers[roundPlayer].health > GAME_START_HUNTER_LIFE_POINTS)
				gv->allPlayers[roundPlayer].health = GAME_START_HUNTER_LIFE_POINTS;

		} else { // player is dracula
			// check each action in the round
			for (int i = 0; i < 4; i++) {
				if (pastPlays[startOfRound+i] == 'V')
						gv->gameScore -= SCORE_LOSS_VAMPIRE_MATURES;
			}

			// use draculas current location to test if he is at sea
			dracLocationDetail(gv, true);

			if (gv->allPlayers[roundPlayer].currLocation == CASTLE_DRACULA)
				gv->allPlayers[roundPlayer].health += LIFE_GAIN_CASTLE_DRACULA;
		}
	}
	
}

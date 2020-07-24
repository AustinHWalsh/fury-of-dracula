////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// GameView.c: GameView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "Places.h"

// add your own #includes here
#define LOCATION_ABBREVIATION_MAX 3
#define PLACE_POS 6
#define PLAYER_POS 7
#define ROUND_DIFF 8

// TODO: ADD YOUR OWN STRUCTS HERE
typedef struct playerInfo {
	Player name;				 // player's name
	int health;					 // health of the player
	Place currLocation;			 // the player's location
	Place *prevMoves;			 // the player's previous trail
} PlayerInfo;

struct gameView {
	Round roundNum;							// number of rounds that have passed
	int gameScore;							// current game score
	char *pastPlays; 						// string of past plays !!!THIS NEEDS TO BE TALKED ABOUT!!!
	Player currPlayer;						// the "name" of the current player
	Map m;									// graph of the locations
	PlayerInfo *allPlayers;					// array of player informations
};

// declare your own functions here
void completePlayerTrails(GameView gv, char *startId, Player player);
void completePastPlays(GameView gv, char *pastPlays);

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

GameView GvNew(char *pastPlays, Message messages[])
{
	// create new game view
	GameView new = malloc(sizeof(struct gameView));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate GameView!\n");
		exit(EXIT_FAILURE);
	}
	
	// set up values
	new->roundNum = strlen(pastPlays) / 7;
	new->gameScore = GAME_START_SCORE;
	new->allPlayers = malloc(NUM_PLAYERS * sizeof(PlayerInfo));
	if (new->allPlayers == NULL) {
		fprintf(stderr, "Couldn't allocate allPlayers!\n");
		exit(EXIT_FAILURE);
	}

	///////////////////////////////////////////
	//				NOT SURE				 //
	//				ABOUT THIS				 //
	///////////////////////////////////////////
	memcpy(new->pastPlays, pastPlays, strlen(pastPlays));
		
	new->m = MapNew();
	
	// initialise most of the player info in the arrays
	for (int i = 0; i < NUM_PLAYERS; i++) {
		// set name
		new->allPlayers[i].name = PLAYER_LORD_GODALMING + i;

		// create space for previous trails and fill them with NULL pointers
		new->allPlayers[i].prevMoves = malloc(TRAIL_SIZE * sizeof(Place));
		if (new->allPlayers[i].prevMoves == NULL) {
			fprintf(stderr, "Couldn't allocate trail!\n");
			exit(EXIT_FAILURE);
		}
		for (int j = 0; j < TRAIL_SIZE; j++) 
			new->allPlayers[i].prevMoves[j].name = NULL;

		// set player heatlh
		if (new->allPlayers[i].name != PLAYER_DRACULA)
			new->allPlayers[i].health = GAME_START_HUNTER_LIFE_POINTS;
		else
			new->allPlayers[i].health = GAME_START_BLOOD_POINTS;
		
		// set current location of player
		new->allPlayers[i].currLocation.id = GvGetPlayerLocation(new, new->allPlayers[i].name);
		new->allPlayers[i].currLocation.name = placeIdToName(new->allPlayers[i].currLocation.id);
		new->allPlayers[i].currLocation.abbrev = placeIdToAbbrev(new->allPlayers[i].currLocation.id);
		new->allPlayers[i].currLocation.type = placeIdToType(new->allPlayers[i].currLocation.id);
	}
	
	// fill in trails and calculate game scores and health
	completePastPlays(new, pastPlays);

	return new;
}

void GvFree(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	for(int i = 0; i <= NUM_PLAYERS; i++) {
		free(gv->allPlayers[i].prevMoves);
	}
	
	free(gv->allPlayers);
		
	free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->roundNum;
}

Player GvGetPlayer(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->currPlayer;
}

int GvGetScore(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->gameScore;
}

int GvGetHealth(GameView gv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->allPlayers[player].health;
}

PlaceId GvGetPlayerLocation(GameView gv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	for (int i = 0; i < NUM_PLAYERS; i++) {
		if (gv->allPlayers[i].name == player) {
			//player is a hunter
			if (gv->allPlayers[i].name != PLAYER_DRACULA) {
				//hunter is in hospital if zero health
				if (gv->allPlayers[i].health == 0) 
					return ST_JOSEPH_AND_ST_MARY;
				else 
					return gv->allPlayers[i].currLocation.id;
			//player is Dracula
			} else {
				//Dracula's location is revealed (not unknown)
				if (gv->allPlayers[PLAYER_DRACULA].currLocation.id != CITY_UNKNOWN
					&& gv->allPlayers[PLAYER_DRACULA].currLocation.id != SEA_UNKNOWN) {
					return gv->allPlayers[PLAYER_DRACULA].currLocation.id;
				//Dracula's location unknown
				} else {
					//in the sea
					if (gv->allPlayers[PLAYER_DRACULA].currLocation.type == SEA) {
						return SEA_UNKNOWN;
					}
					//on land (in city)
					else if (gv->allPlayers[PLAYER_DRACULA].currLocation.type == LAND) {
						return CITY_UNKNOWN;
					}
				}
			}
		}
	}
	return NOWHERE;
}

PlaceId GvGetVampireLocation(GameView gv)
{
	// Gets the location of the sleeping immature vampire.
	
	int r = (gv->roundNum - 1) % 13;
	//an immature vampire exists
	if (0 <= r && r < 6) {
		//temp copy of pastPlays for strtok
		char pastPlayCpy[strlen(pastPlays)];
		strcpy(pastPlayCpy, gv->pastPlays);
		//find location of vamp in pastPlay
		char *currStr = strtok(pastPlayCpy, "\n");
		for (int i = 0; i < gv->roundNum - r - 1; i++) { // go to correct roundNum line to search for V
			currStr = strtok(pastPlayCpy, "\n");
		}
		char *prevStr;
		char vampLoc[LOCATION_ABBREVIATION_MAX];
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
		if (strcmp(vampLoc, "S?") == 0) 
			return NOWHERE;
		//city is not revealed
		if (strcmp(vampLoc, "C?") == 0 )
			return CITY_UNKNOWN;
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

PlaceId *GvGetTrapLocations(GameView gv, int *numTraps)
{

 ///////////////////// PSEUDO CODE /////////////////////////
 /*
 
 initialise array for trap locations 
 go through all locations of the map and check if they are land
 compare these with locations in dracula's trail 
 if they match, then add location to traplocations array and increment numTraps
 else continue 
 also, need to check after every dracula move to remove traps once the location is no longer in the trail

 loop through player moves for current round + 1 ()
 check if theyve set off a trap, if yes remove from list 
 if multiple traps, check player health to check if all traps are destoryed or not 

 */
 ///////////////////////////////////////////////////////////
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    PlaceId *trapLocations = malloc(TRAIL_SIZE * (sizeof(PlaceId)));
    *numTraps = 0;
    
    int n = gv->roundNum - TRAIL_SIZE;
    if (n < 0) n = 0;

    for (int i = n; i < gv->roundNum; i++) {
        if (gv->allPlayers[PLAYER_DRACULA].prevMoves[i].name != NULL && gv->allPlayers[PLAYER_DRACULA].prevMoves[i].type == LAND) {
            trapLocations[*numTraps] = gv->allPlayers[PLAYER_DRACULA].prevMoves[i].id;
            (*numTraps)++;
        } 
		
    } 
    
    //*numTraps = 0;
    return NULL;
}
////////////////////////////////////////////////////////////////////////
// Game History

PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	*canFree = false;
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
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
	for (int i = 0; i < TRAIL_SIZE; i++) {
		// first empty stop in trail
		if (gv->allPlayers[player].prevMoves[i].name == NULL) {
			gv->allPlayers[player].prevMoves[i].id = cityId;
			gv->allPlayers[player].prevMoves[i].name = placeIdToName(cityId);
			gv->allPlayers[player].prevMoves[i].type = placeIdToType(cityId);
			gv->allPlayers[player].prevMoves[i].abbrev = placeIdToAbbrev(cityId);
			return;
		}
	}
	
	// if no empty stops where found, push the array along and remove last from trail
	// might make this a QUEUE
	PlaceId cityIdTemp = gv->allPlayers[player].prevMoves[0].id;
	PlaceId cityIdTemp2 = gv->allPlayers[player].prevMoves[1].id;
	// set the first place in the array
	gv->allPlayers[player].prevMoves[0].id = cityId;
	gv->allPlayers[player].prevMoves[0].name = placeIdToName(cityId);
	gv->allPlayers[player].prevMoves[0].type = placeIdToType(cityId);
	gv->allPlayers[player].prevMoves[0].abbrev = placeIdToAbbrev(cityId);
	// set the remaining in the trail
	for (int i = 1; i < TRAIL_SIZE; i++) {
		cityIdTemp2 = gv->allPlayers[player].prevMoves[i].id;
		gv->allPlayers[player].prevMoves[i].id = cityIdTemp;
		gv->allPlayers[player].prevMoves[i].name = placeIdToName(cityIdTemp);
		gv->allPlayers[player].prevMoves[i].type = placeIdToType(cityIdTemp);
		gv->allPlayers[player].prevMoves[i].abbrev = placeIdToAbbrev(cityIdTemp);
		cityIdTemp = cityIdTemp2;
	}
} 

// clean up and complete the remaining required gv elements,
// such as current score and player health
// dependencies on the pastPlays string
void completePastPlays(GameView gv, char *pastPlays) {

	// do actions from each round
	for (int i = 0; i < gv->roundNum; i++) {
		// array position of the start of the round (players name)
		int startOfRound = pastPlays[i * ROUND_DIFF];

		// get current round's player
		Player roundPlayer;
		switch(startOfRound) {
			case 'G':
				roundPlayer = PLAYER_LORD_GODALMING;
				break;
			case 'S':
				roundPlayer = PLAYER_DR_SEWARD;
				break;
			case 'H':
				roundPlayer = PLAYER_VAN_HELSING;
				break;
			case 'M':
				roundPlayer = PLAYER_MINA_HARKER;
				break;
			case 'D':
				roundPlayer = PLAYER_DRACULA;
				// dracula's turn reduces point by 1
				gv->gameScore -= SCORE_LOSS_DRACULA_TURN;
				break;
		}

		gv->currPlayer = roundPlayer;
		// add the player's current location to the trail
		completePlayerTrails(gv, &pastPlays[startOfRound+1], roundPlayer);

		// not dracula
		if (roundPlayer != PLAYER_DRACULA) {
			// check each action in the round
			for (int i = 0; pastPlays[startOfRound+i] != '.'; i++) {
				switch(startOfRound+i) {
					case 'T':
						gv->allPlayers[roundPlayer].health -= LIFE_LOSS_TRAP_ENCOUNTER;
						break;
					case 'D':
						gv->allPlayers[roundPlayer].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
						gv->allPlayers[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
						break;
				}
			}

			if (gv->allPlayers[roundPlayer].health <= 0) {
				/* set trail to hospital */
				// TODO
				gv->allPlayers[roundPlayer].health = 0;
				gv->gameScore -= SCORE_LOSS_HUNTER_HOSPITAL;
			}

			// when the player didnt move and didnt just get sent to hospital
			if (gv->allPlayers[roundPlayer].prevMoves[0].name == gv->allPlayers[roundPlayer].prevMoves[1].name
				&& gv->allPlayers[roundPlayer].health > 0)
				gv->allPlayers[roundPlayer].health += LIFE_GAIN_REST;

			if (gv->allPlayers[roundPlayer].health > GAME_START_HUNTER_LIFE_POINTS)
				gv->allPlayers[roundPlayer].health = GAME_START_HUNTER_LIFE_POINTS;

		} else { // player is dracula
			// check each action in the round
			for (int i = 0; i < 4; i++) {
				if (pastPlays[startOfRound+i] == 'V')
						gv->gameScore -= SCORE_LOSS_VAMPIRE_MATURES;
			}

			// if drac just moved to the sea
			/* i think this is enough? */
			if (gv->allPlayers[roundPlayer].currLocation.id == SEA_UNKNOWN)
				gv->allPlayers[roundPlayer].health -= LIFE_LOSS_SEA;

			else if (gv->allPlayers[roundPlayer].currLocation.id == CASTLE_DRACULA)
				gv->allPlayers[roundPlayer].health += LIFE_GAIN_CASTLE_DRACULA;
		}
	}
}

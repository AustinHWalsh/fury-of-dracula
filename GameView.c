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
#define PLACE_POS 6
#define PLAYER_POS 7
#define ROUND_DIFF 8

// TODO: ADD YOUR OWN STRUCTS HERE
typedef struct playerInfo {
	Player name;				 // player's name
	int health;					 // health of the player
	Place currLocation;			 // the player's location
	Place prevMoves[TRAIL_SIZE]; // the player's previous trail
} PlayerInfo;

struct gameView {
	Round roundNum;							// number of rounds that have passed
	int gameScore;							// current game score
	char *pastPlays; 						// string of past plays !!!THIS NEEDS TO BE TALKED ABOUT!!!
	Player currPlayer;						// the "name" of the current player
	Map m;									// graph of the locations
	PlayerInfo allPlayers[NUM_PLAYERS];		// array of player informations
};

// declare your own functions here
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
	
	int playLen = strlen(pastPlays);
	new->roundNum = playLen / 7;
	new->gameScore = GAME_START_SCORE;

	///////////////////////////////////////////
	//				NOT SURE				 //
	//				ABOUT THIS				 //
	///////////////////////////////////////////
	memcpy(new->pastPlays, pastPlays, playLen);
		
	new->m = MapNew();
	
	// initialise most of the player info in the arrays
	for (int i = 0; i < NUM_PLAYERS; i++) {
		new->allPlayers[i].name = PLAYER_LORD_GODALMING + i;
		if (new->allPlayers[i].name != PLAYER_DRACULA)
			new->allPlayers[i].health = GAME_START_HUNTER_LIFE_POINTS;
		else
			new->allPlayers[i].health = GAME_START_BLOOD_POINTS;
		new->allPlayers[i].currLocation.id = GvGetPlayerLocation(new, new->allPlayers[i].name);
		new->allPlayers[i].currLocation.name = placeIdToName(new->allPlayers[i].currLocation.id);
		new->allPlayers[i].currLocation.abbrev = placeIdToAbbrev(new->allPlayers[i].currLocation.id);
		new->allPlayers[i].currLocation.type = placeIdToType(new->allPlayers[i].currLocation.id);
	}
	
	///////////////////////////////////////////
	//				NEED TO ADD				 //
	//				TRAILS NEXT				 //
	///////////////////////////////////////////
	completePastPlays(new, pastPlays);

	return new;
}

void GvFree(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
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
				//Dracula's location is revealed
				if (/*location is in pastPlays*/ gv) {
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
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION

	//there is an immature vampire
	if (0 <= (gv->roundNum - 1 % 13) && (gv->roundNum - 1 % 13) < 6) {
		//find location of vamp in pastPlay or Dracula's prevMoves
		//if hunters have not visited the city vamp is spawned in,
			if (/*Drac's CITY is revealed in pastPlay when V is spawned*/gv) 
				return /*Drac's CITY when V is spawned*/CITY_UNKNOWN;
			else
				return CITY_UNKNOWN;
	}
	//vampire not immature, not spawned or vanquished
	return NOWHERE;
}

PlaceId *GvGetTrapLocations(GameView gv, int *numTraps)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numTraps = 0;
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
				/* move player to the hospital */
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
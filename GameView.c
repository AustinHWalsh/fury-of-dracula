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
#define NOT_MEMBER 0
#define IS_MEMBER 1
#define LOCATION_ABBREVIATION_MAX 3
#define TBA_LOCATION -3
#define MIN_TRAIL (gv->roundNum + 7)

#define PLACE_POS 6
#define PLAYER_POS 7
#define ROUND_DIFF 8

// TODO: ADD YOUR OWN STRUCTS HERE
typedef struct playerInfo {
	Player name;				 // player's name
	int health;					 // health of the player
	PlaceId currLocation;		 // the player's location
	PlaceId *prevMoves; 		 // players past moves in a dynamic array
} PlayerInfo;

struct gameView {
	Round roundNum;							// number of total actions that have passed
	int gameScore;							// current game score
	char *pastPlays; 						// string of past plays 
	Player currPlayer;						// the "name" of the current player
	Map m;									// graph of the locations
	PlayerInfo *allPlayers;					// array of player informations
};

// declare your own functions here
void completePlayerTrails(GameView gv, char *startId, Player player);
void completePastPlays(GameView gv, char *pastPlays);
void removeTrapFromDrac(GameView gv, int HunterTrapPos);
int isReachableMember(PlaceId *reachable, PlaceId w);

char convertToPlayer(Player player);
void recurAddRail(GameView gv, ConnList reachList, PlaceId *reachArray, int *railDistance,
int *numReturnedLocs, int visitedLocs[NUM_REAL_PLACES]);
PlaceId dracLocationDetail(GameView gv, bool updateHealth);

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
         
		//set all currLocation to NOWHERE
		new->allPlayers[i].currLocation = NOWHERE;

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

void GvFree(GameView gv)
{
	// loop through each player in the all players array
	for(int i = 0; i < NUM_PLAYERS; i++) 
		free(gv->allPlayers[i].prevMoves);
	
	free(gv->pastPlays);
	free(gv->allPlayers);
		
	free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv)
{
	return (gv->roundNum * 7)/35;
}

Player GvGetPlayer(GameView gv)
{
	return gv->currPlayer;
}

int GvGetScore(GameView gv)
{
	return gv->gameScore;
}

int GvGetHealth(GameView gv, Player player)
{
	return gv->allPlayers[player].health;
}

PlaceId GvGetPlayerLocation(GameView gv, Player player)
{
	//player is a hunter
	if (player != PLAYER_DRACULA) {
		//hunter is in hospital if zero health
		if (gv->allPlayers[player].health == 0) 
			return ST_JOSEPH_AND_ST_MARY;
		else 
			return gv->allPlayers[player].currLocation;
	//player is Dracula
	} else {
		return dracLocationDetail(gv, false);
	}
	return NOWHERE;
}

PlaceId GvGetVampireLocation(GameView gv)
{
	// Gets the location of the sleeping immature vampire.
	
	int r = (GvGetRound(gv) - 1) % 13;
	//an immature vampire exists
	if (0 <= r && r < 6) {
		
		//temp copy of pastPlays for strtok
		char *pastPlayCpy = malloc(strlen(gv->pastPlays) * sizeof(char));
		strcpy(pastPlayCpy, gv->pastPlays);
		//find location of vamp in pastPlay
		char *currStr;
		//if immature vampire is in the first round
		if (GvGetRound(gv) < 14) {
		    currStr = strtok(pastPlayCpy, ". ");
		} else {
		// go to correct roundNum line to search for V
			currStr = strtok(pastPlayCpy, "\n");
			for (int i = 0; i < GvGetRound(gv) - r - 1; i++)
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
			if (currStr == NULL) 
				break;
			hunterLoc = &currStr[1];
			if (hunterLoc != NULL && strlen(hunterLoc) > 2)
				hunterLoc[2] = '\0';
				
			if (currStr != NULL && currStr[0] != 'D' && strcmp(hunterLoc, vampLoc) == 0)
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

    PlaceId *trapLocations = malloc(TRAIL_SIZE * (sizeof(PlaceId)));
    *numTraps = 0;
	// drac hasnt made the rth move yet
    for (int i = 0; i < GvGetRound(gv); i++) {
		// position of trap in pastPlay string
		int checkTrap = 35 + (5 * ROUND_DIFF * i);
		if (gv->pastPlays[checkTrap] != 'T')
			continue;
		
		char cityAbbrev[3] = {gv->pastPlays[checkTrap-2], gv->pastPlays[checkTrap-1], '\0'};
		PlaceId cityId = placeAbbrevToId(cityAbbrev);
        // checking if the 
		trapLocations[(*numTraps)++] = cityId;
    } 
	    
    return trapLocations;
}
////////////////////////////////////////////////////////////////////////
// Game History

PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree)
{
		
	PlaceId *MoveHistory = malloc(gv->roundNum * sizeof(PlaceId));
	*numReturnedMoves = 0;
	
	char playerChar = convertToPlayer(player);
	for (int i = 0; i < gv->roundNum; i++) {
		if (gv->pastPlays[i * ROUND_DIFF] == playerChar) {
			char abbrev[3] = {gv->pastPlays[i * ROUND_DIFF+1], gv->pastPlays[i * ROUND_DIFF+2], '\0'};
			MoveHistory[*numReturnedMoves] = placeAbbrevToId(abbrev);
			(*numReturnedMoves)++;
		}
	}
	
	MoveHistory = realloc(MoveHistory, *numReturnedMoves * sizeof(PlaceId));
	
	*canFree = true;
	return MoveHistory;
}

PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{
	PlaceId *lastNMoves = malloc(numMoves * sizeof(PlaceId));
	*numReturnedMoves = 0;
	char playerChar = convertToPlayer(player);
	
	// for each round in the string
	for (int i = 0; i < gv->roundNum; i++) {
		if (*numReturnedMoves < numMoves)
			break;
		// check if current round involves the player
		if (gv->pastPlays[i * ROUND_DIFF] == playerChar) {
			// convert the location to an abbrev and add it the array
			char abbrev[3] = {gv->pastPlays[i * ROUND_DIFF+1], gv->pastPlays[i * ROUND_DIFF+2], '\0'};
			lastNMoves[*numReturnedMoves] = placeAbbrevToId(abbrev);
			(*numReturnedMoves)++;
		}
	}
	
	*canFree = true;
	return lastNMoves;
}


PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{
		
	PlaceId *LocationsHistory = malloc( gv->roundNum * (sizeof(PlaceId)));

	//check if input is valid
    assert(gv != NULL);
    assert(player >= 0 && player <= 5);
    assert(gv->allPlayers != NULL);
	
	//insert player location from (total moves - numLocs) to total moves
	for (int j = 0; gv->allPlayers[player].prevMoves[j] != TBA_LOCATION; j++) {
		PlaceId currPlace = gv->allPlayers[player].prevMoves[j];
		int doubleVal = 0;
		// if the currLocation is a double back/hide
		while (currPlace > SEA_UNKNOWN) {
			// test the current location
			switch(currPlace) {
				case DOUBLE_BACK_1:
					doubleVal++;
					break;
				case DOUBLE_BACK_2:
					doubleVal = doubleVal+2;
					break;
				case DOUBLE_BACK_3:
					doubleVal = doubleVal+3;
					break;
				case DOUBLE_BACK_4:
					doubleVal = doubleVal+4;
					break;
				case DOUBLE_BACK_5:
					doubleVal = doubleVal+5;
					break;
				case HIDE:
					doubleVal++;
					break;
				default:
					doubleVal = 0;
					break;
			}
			// update and test again
			currPlace = gv->allPlayers[player].prevMoves[j-doubleVal];
		}
		LocationsHistory[(*numReturnedLocs)++] = currPlace;
	}

	*canFree = true;
	return LocationsHistory;
}

PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	PlaceId *lastLocations = malloc(numLocs * (sizeof(PlaceId)));
	*canFree = true;

	int j = gv->roundNum - numLocs + 1;
	//insert player location from (total moves - numLocs) to total moves
	while (gv->allPlayers[player].prevMoves[j] != TBA_LOCATION && 
		j <= MIN_TRAIL) {
		lastLocations[*numReturnedLocs] = gv->allPlayers[player].prevMoves[j];
		(*numReturnedLocs)++;
		j++;
	}	
	return lastLocations;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{
		
	//count max num of connections to from
	int connectionNum = 0;
	for (int i = 0; CONNECTIONS[i].v != UNKNOWN_PLACE; i++) {
		if (CONNECTIONS[i].v == from) {
			connectionNum++;
		}
	}

	PlaceId *reachable = malloc(connectionNum*sizeof(PlaceId));
	
	//set all fields to UNKNOWN_PLACE
    for (int i = 0; i <= connectionNum; i++) {
	    reachable[i] = UNKNOWN_PLACE;
    }
	
	//array includes the given location
	reachable[0] = from;
	int j = 1;
	//player is a hunter
	if (player != PLAYER_DRACULA) {
		//int railDistance = (player + round) % 4;
		for (int i = 0; CONNECTIONS[i].v != UNKNOWN_PLACE; i++) {
			if (CONNECTIONS[i].v == from && isReachableMember(reachable, CONNECTIONS[i].w) == NOT_MEMBER) {
				if (CONNECTIONS[i].t == RAIL /*&& not in range of railDistance*/) {
					continue;
				}
				reachable[j] = CONNECTIONS[i].w;
				j++;
			}
		}
		*numReturnedLocs = j;
		return reachable;
	}
	//player is Dracula
	for (int i = 0; CONNECTIONS[i].v != UNKNOWN_PLACE; i++) {
		if (CONNECTIONS[i].v == from && CONNECTIONS[i].w != ST_JOSEPH_AND_ST_MARY && CONNECTIONS[i].t != RAIL 
			&& isReachableMember(reachable, CONNECTIONS[i].w) == NOT_MEMBER) {
			reachable[j] = CONNECTIONS[i].w;
			j++;
		}
	}
	*numReturnedLocs = j;
	return reachable;
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{

	// adjacency list of the connections leaving the from location
	ConnList currentReach = MapGetConnections(gv->m, from);
	ConnList curr = currentReach;

	// array of reachable locations
	PlaceId *reachableConn = malloc((MapNumPlaces(gv->m)) * sizeof(PlaceId));

	// create an array of visited places, ensure no doubleups in returned array
	// used only when the hunter moves, because of the rail algorithm
	int visitedLocations[NUM_REAL_PLACES] = {0};
	*numReturnedLocs = 0;
	reachableConn[(*numReturnedLocs)++] = from;
	visitedLocations[from]++;
	
	// Dracula can only move to specific locations
	if (player == PLAYER_DRACULA) {
		// iterate through the list
		while (curr != NULL) {
			// test the location can be added
			if (curr->type != RAIL && curr->p != ST_JOSEPH_AND_ST_MARY) {
				// test bools to add to array
				if (road && curr->type == ROAD && 
					visitedLocations[curr->p] != 0)  
					reachableConn[(*numReturnedLocs)++] = currentReach->p;
				else if (boat && curr->type == BOAT) 
					reachableConn[(*numReturnedLocs)++] = curr->p;
			}
			curr = curr->next;
		}

		// teleport if no moves possible
		if (*numReturnedLocs == 0)
			reachableConn[(*numReturnedLocs)++] = TELEPORT;

	} else { // hunters move
		while (curr != NULL) {
			if (visitedLocations[curr->p] == 0) {
				int railCount = (round + player) % 4;
				int *railDistance = &railCount;
				// determine which type of connection can be added
				if (rail && curr->type == RAIL)
					recurAddRail(gv, curr, reachableConn, railDistance, 
						numReturnedLocs, visitedLocations);
				else if (road && curr->type == ROAD) {
					reachableConn[(*numReturnedLocs)++] = curr->p;
					visitedLocations[curr->p]++;
				} else if (boat && curr->type == BOAT) {
					reachableConn[(*numReturnedLocs)++] = curr->p; 
					visitedLocations[curr->p]++;
				}
			}	
			curr = curr->next;
		}		
	}

	return reachableConn;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

//determines whether a location is already a member of reachable (locations)
int isReachableMember(PlaceId *reachable, PlaceId w) {
	for (int i = 0; reachable[i] != UNKNOWN_PLACE; i++) {
		if (reachable[i] == w) {
			return IS_MEMBER;
		}
	}
	return NOT_MEMBER;
}

// initialise the trails of each person in the allPersons array
// using the pastPlays string
void completePlayerTrails(GameView gv, char *startId, Player player) {
	// create the abbreviation of the city from the paststring
	char cityAbbrev[3] = {startId[0], startId[1], '\0'};
	PlaceId cityId = placeAbbrevToId(cityAbbrev);

	for (int i = 0; i < MIN_TRAIL; i++) {
		// first empty stop in trail
		if (gv->allPlayers[player].prevMoves[i] == TBA_LOCATION) {
			gv->allPlayers[player].prevMoves[i] = cityId;
			break;
		}
					
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
			if (GvGetPlayerLocation(gv, roundPlayer) == ST_JOSEPH_AND_ST_MARY 
				&& gv->allPlayers[roundPlayer].health <= 0)
				gv->allPlayers[roundPlayer].health = GAME_START_HUNTER_LIFE_POINTS;

			// check each action in the round
			for (int i = 3; pastPlays[startOfRound+i] != '.'; i++) {
				switch(pastPlays[startOfRound+i]) {
					case 'T': {// encountered trap
						if (gv->allPlayers[roundPlayer].health <= 0) break;
						removeTrapFromDrac(gv, startOfRound+i);
						gv->allPlayers[roundPlayer].health -= LIFE_LOSS_TRAP_ENCOUNTER;
						break;
					}
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
			if (gv->allPlayers[roundPlayer].prevMoves[0] == 
				gv->allPlayers[roundPlayer].prevMoves[1]
				&& gv->allPlayers[roundPlayer].health <= 0)
				gv->allPlayers[roundPlayer].health += LIFE_GAIN_REST;

			// reduce hunters health to max if it is over
			if (gv->allPlayers[roundPlayer].health > GAME_START_HUNTER_LIFE_POINTS)
				gv->allPlayers[roundPlayer].health = GAME_START_HUNTER_LIFE_POINTS;

		} else { // player is dracula
			// check if vamp matured
			if (pastPlays[startOfRound+5] == 'V')
				gv->gameScore -= SCORE_LOSS_VAMPIRE_MATURES; 
			// use draculas current location to test if he is at sea
			PlaceId lastPos = dracLocationDetail(gv, true);

			if (lastPos == CASTLE_DRACULA)
				gv->allPlayers[roundPlayer].health += LIFE_GAIN_CASTLE_DRACULA;
		}
	}
	
}

// convert a Player to a char and return it
char convertToPlayer(Player player) {
	char playerChar;
	assert(player >= 0 && player <= 4);
	switch(player) {
		case PLAYER_LORD_GODALMING:
			playerChar = 'G';
			break;
		case PLAYER_DR_SEWARD:
			playerChar = 'S';
			break;
		case PLAYER_VAN_HELSING:
			playerChar = 'H';
			break;
		case PLAYER_MINA_HARKER:
			playerChar = 'M';
			break;
		case PLAYER_DRACULA:
			playerChar = 'D';				
			break;
	}

	return playerChar;
}

// add all possible rail locations to the reacharray, only while the hunter
// can move through another rail. 
void recurAddRail(GameView gv, ConnList reachList, PlaceId *reachArray, 
	int *railDistance, int *numReturnedLocs, int visitedLocs[NUM_REAL_PLACES]) {
	// base case when the hunter has run out of rail moves
	
	if (*railDistance < 1)
		return;
	else {
		// when the passed location is not a previously added one
		if (visitedLocs[reachList->p] == 0) {
			// add the current vertice to the list
			reachArray[(*numReturnedLocs)++] = reachList->p; 
			// make sure it isnt visited again
			visitedLocs[reachList->p]++;
			// reduce the number of rail trips left by 1
			(*railDistance)--;

			// get all the connections of the current vertice
			ConnList curr = MapGetConnections(gv->m, reachList->p);
			// loop through each one and recur if rail connection
			while (curr != NULL) {
				if (curr->type == RAIL)
					recurAddRail(gv, curr, reachArray, railDistance, 
						numReturnedLocs, visitedLocs);
				curr = curr->next;
			}
		} 
	}

}

// determine if dracula is current at sea, including
// double moves
PlaceId dracLocationDetail(GameView gv, bool updateHealth) {
	int doubleVal = 0, healthLoss = 0, len;
	PlaceId currId = gv->allPlayers[PLAYER_DRACULA].currLocation;
	// find the len of the array upto currLocation
	for (len = 1; len < MIN_TRAIL; len++) {
		if (gv->allPlayers[PLAYER_DRACULA].prevMoves[len] == currId) {
			break;
		} 
	}
	
	// determine what the current location is
	switch(currId) {
		case DOUBLE_BACK_1:
			doubleVal++;
			break;
		case DOUBLE_BACK_2:
			doubleVal = doubleVal+2;
			break;
		case DOUBLE_BACK_3:
			doubleVal = doubleVal+3;
			break;
		case DOUBLE_BACK_4:
			doubleVal = doubleVal+4;
			break;
		case DOUBLE_BACK_5:
			doubleVal = doubleVal+5;
			break;
		case SEA_UNKNOWN:
			healthLoss++;
			currId = SEA_UNKNOWN;
			break;
		case CITY_UNKNOWN:
			currId = CITY_UNKNOWN;
			break;
		case HIDE:
			doubleVal++;
			break;
		default:
			currId = gv->allPlayers[PLAYER_DRACULA].currLocation;
			if (placeIdToType(currId) == SEA) 
				healthLoss++;
			break;
	}
	
	// check the locations of the double back/hide
	while (doubleVal != 0) {	
		switch(gv->allPlayers[PLAYER_DRACULA].prevMoves[len-doubleVal]) {
			case DOUBLE_BACK_1:
				doubleVal++;
				break;
			case DOUBLE_BACK_2:
				doubleVal = doubleVal+2;
				break;
			case DOUBLE_BACK_3:
				doubleVal = doubleVal+3;
				break;
			case DOUBLE_BACK_4:
				doubleVal = doubleVal+4;
				break;
			case DOUBLE_BACK_5:
				doubleVal = doubleVal+5;
				break;
			case SEA_UNKNOWN:
				healthLoss++;
				doubleVal = 0;
				currId = SEA_UNKNOWN;
				break;
			case CITY_UNKNOWN:
				doubleVal = 0;
				currId = CITY_UNKNOWN;
				break;
			case HIDE:
				doubleVal++;
				break;
			default:
				currId = gv->allPlayers[PLAYER_DRACULA].prevMoves[len-doubleVal];
				if (placeIdToType(currId) == SEA) 
					healthLoss++;
				doubleVal = 0;
				
				break;
		}
	}

	// lose health when at sea
	if (healthLoss > 0 && updateHealth) 
		gv->allPlayers[PLAYER_DRACULA].health -= LIFE_LOSS_SEA;
		
	return currId;
}

// when a hunter encounters a trap remove the trap from draculas trail
// in the pastPlays string
void removeTrapFromDrac(GameView gv, int HunterTrapPos) {
	char cityAbbrev[3] = {gv->pastPlays[HunterTrapPos-2], gv->pastPlays[HunterTrapPos-1], '\0'};
	// location which trap was found by hunter
	PlaceId trapCity = placeAbbrevToId(cityAbbrev);

	int i;
	// find location in pastPlays string
	for (i = 0; i < GvGetRound(gv); i++) {
		// position of trap in pastPlay string
		int checkTrap = 35 + (5 * ROUND_DIFF * i);
		if (gv->pastPlays[checkTrap] != 'T') 
			continue;

		// check the location of the trap is the same as the one the hunter found
		char dracCity[3] = {gv->pastPlays[checkTrap-2], gv->pastPlays[checkTrap-1], '\0'};
		PlaceId dracLoc = placeAbbrevToId(dracCity);
		if (dracLoc != trapCity)
			continue;
		
		// remove 'T' from string
		char *tmpPastPlays = malloc(strlen(gv->pastPlays)*sizeof(char));
		strcpy(tmpPastPlays, gv->pastPlays);
		tmpPastPlays[checkTrap] = '.';
		strcpy(gv->pastPlays, tmpPastPlays);
		free(tmpPastPlays);
	}
}
////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// HunterView.c: the HunterView ADT implementation
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
#include "HunterView.h"
#include "Map.h"
#include "Places.h"

#include "Graph.h"

// add your own #includes here
#define NOT_MEMBER 0
#define IS_MEMBER 1
#define LOCATION_ABBREVIATION_MAX 3
#define TBA_LOCATION -3
#define MIN_TRAIL (hv->roundNum + 7)

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

struct hunterView {
	Round roundNum;
	int gameScore;
	char *pastPlays;
	Player currPlayer;
	Map m;
	PlayerInfo *allPlayers;
};

void completePlayerTrailsHv(HunterView hv, char *startId, Player player);
void completePastPlaysHv(HunterView hv, char *pastPlays);
PlaceId dracLocationDetailHv(HunterView hv, bool updateHealth);
void recurAddRailHv(HunterView hv, ConnList reachList, PlaceId *reachArray, 
int *railDistance, int *numReturnedLocs, int visitedLocs[NUM_REAL_PLACES]);
Graph makeRailGraph();
////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

HunterView HvNew(char *pastPlays, Message messages[])
{
	HunterView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate HunterView!\n");
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
	completePastPlaysHv(new, pastPlays);

	return new;
}

void HvFree(HunterView hv)
{
	for(int i = 0; i < NUM_PLAYERS; i++) 
		free(hv->allPlayers[i].prevMoves);
	
	free(hv->pastPlays);
	free(hv->allPlayers);
		
	free(hv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round HvGetRound(HunterView hv)
{
	return (hv->roundNum * 7)/35;
}

Player HvGetPlayer(HunterView hv)
{
	return hv->currPlayer;
}

int HvGetScore(HunterView hv)
{
	return hv->gameScore;
}

int HvGetHealth(HunterView hv, Player player)
{
	return hv->allPlayers[player].health;
}

PlaceId HvGetPlayerLocation(HunterView hv, Player player)
{
	//player is a hunter
	if (player != PLAYER_DRACULA) {
		//hunter is in hospital if zero health
		if (hv->allPlayers[player].health == 0) 
			return ST_JOSEPH_AND_ST_MARY;
		else 
			return hv->allPlayers[player].currLocation;
	//player is Dracula
	} else {
		return dracLocationDetailHv(hv, false);
	}
	return NOWHERE;
}

PlaceId HvGetVampireLocation(HunterView hv)
{
	// Gets the location of the sleeping immature vampire.
	
	int r = (HvGetRound(hv) - 1) % 13;
	//an immature vampire exists
	if (0 <= r && r < 6) {
		
		//temp copy of pastPlays for strtok
		char *pastPlayCpy = malloc(strlen(hv->pastPlays) * sizeof(char));
		strcpy(pastPlayCpy, hv->pastPlays);
		//find location of vamp in pastPlay
		char *currStr;
		//if immature vampire is in the first round
		if (HvGetRound(hv) < 14) {
		    currStr = strtok(pastPlayCpy, ". ");
		} else {
		// go to correct roundNum line to search for V
			currStr = strtok(pastPlayCpy, "\n");
			for (int i = 0; i < HvGetRound(hv) - r - 1; i++)
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

////////////////////////////////////////////////////////////////////////
// Utility Functions

PlaceId HvGetLastKnownDraculaLocation(HunterView hv, Round *round)
{
	// get the last location
	PlaceId lastLoc = NOWHERE;
	
	// find the last real location in the prevMoves
	int i;
	for (i = 0; i < MIN_TRAIL; i++) {
		if (hv->allPlayers[PLAYER_DRACULA].prevMoves[i] <= MAX_REAL_PLACE &&
			hv->allPlayers[PLAYER_DRACULA].prevMoves[i] >= MIN_REAL_PLACE) {
			lastLoc = hv->allPlayers[PLAYER_DRACULA].prevMoves[i];
		}
	}

	// check a new location was found and updating
	if (lastLoc != NOWHERE)
		*round = *round + i/7;
	
	return lastLoc;
}

PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest,
                             int *pathLength)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	//*pathLength = 0;

	PlaceId *path = malloc(NUM_REAL_PLACES * sizeof(PlaceId));

	Graph map = newGraph(NUM_REAL_PLACES);
	
	int railDistance = (hunter + HvGetRound(hv)) % 4;
    for (int i = 0; CONNECTIONS[i].v != UNKNOWN_PLACE; i++) {
        insertEdge(map, CONNECTIONS[i].v, CONNECTIONS[i].w);
		//if src and dest are connected by RAIL
		if (CONNECTIONS[i].v == hv->allPlayers[hunter].currLocation
			&& CONNECTIONS[i].w == dest && CONNECTIONS[i].t == RAIL) {
			//exclude if outside raildistance
			if (railDistance == 0)
				continue;
			//if railDistance > 0, loop will add adjacent cities by rail 
		}
    }

	*pathLength = findPathLength(map, hv->allPlayers[hunter].currLocation, dest);

	//shift one place to the left to remove source location
	for (int i = 0; i <= *pathLength; i++)
		path[i] = path[i + 1];

	PlaceId *tmpPath = malloc(NUM_REAL_PLACES * sizeof(PlaceId));

	Graph railGraph = makeRailGraph();
	if (railDistance > 1 && railDistance <= 3) {
		int pathLen;
		for (int i = 0; i < MAX_REAL_PLACE; i++) {
			if (PLACES[i].type == LAND && placeIdToType(hv->allPlayers[hunter].currLocation) == LAND) {
				pathLen = findPathLength(railGraph, hv->allPlayers[hunter].currLocation, PLACES[i].id);
				if (pathLen > 1 && pathLen <= 3) {
					for (int j = 1; j <= pathLen; j++) {
						path[pathLen] = tmpPath[j];
						pathLen++;
					}
				}
			}
		}
	}
	dropGraph(railGraph);
	dropGraph(map);
	return path;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *HvWhereCanIGo(HunterView hv, int *numReturnedLocs)
{	
	return HvWhereCanIGoByType(hv, true, true, true, numReturnedLocs);
}

PlaceId *HvWhereCanIGoByType(HunterView hv, bool road, bool rail,
                             bool boat, int *numReturnedLocs)
{
	// adjacency list of the connections leaving the currentLoc
	PlaceId from = hv->allPlayers[hv->currPlayer].currLocation;
	ConnList currentReach = MapGetConnections(hv->m, from);
	ConnList curr = currentReach;

	// array of reachable locations
	PlaceId *reachableConn = malloc((MapNumPlaces(hv->m)) * sizeof(PlaceId));

	// create an array of visited places, ensure no doubleups in returned array
	// used only when the hunter moves, because of the rail algorithm
	int visitedLocations[NUM_REAL_PLACES] = {0};
	*numReturnedLocs = 0;
	reachableConn[(*numReturnedLocs)++] = from;
	visitedLocations[from]++;
	
	// Dracula can only move to specific locations
	if (hv->currPlayer == PLAYER_DRACULA) {
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
				int railCount = (HvGetRound(hv) + hv->currPlayer) % 4;
				int *railDistance = &railCount;
				// determine which type of connection can be added
				if (rail && curr->type == RAIL)
					recurAddRailHv(hv, curr, reachableConn, railDistance, 
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

PlaceId *HvWhereCanTheyGo(HunterView hv, Player player,
                          int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *HvWhereCanTheyGoByType(HunterView hv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

Graph makeRailGraph() {
    Graph railGraph = newGraph(NUM_REAL_PLACES);
    for (int i = 0; CONNECTIONS[i].v != UNKNOWN_PLACE; i++) {
        if (CONNECTIONS[i].t == RAIL)
            insertEdge(railGraph, CONNECTIONS[i].v, CONNECTIONS[i].w);
    }
	return railGraph;
}

// initialise the trails of each person in the allPersons array
// using the pastPlays string
void completePlayerTrailsHv(HunterView hv, char *startId, Player player) {
	// create the abbreviation of the city from the paststring
	char cityAbbrev[3] = {startId[0], startId[1], '\0'};
	PlaceId cityId = placeAbbrevToId(cityAbbrev);

	for (int i = 0; i < MIN_TRAIL; i++) {
		// first empty stop in trail
		if (hv->allPlayers[player].prevMoves[i] == TBA_LOCATION) {
			hv->allPlayers[player].prevMoves[i] = cityId;
			break;
		}		
	}

	// reset that player's current position to the first in the trail
	hv->allPlayers[player].currLocation = cityId; 
} 

// clean up and complete the remaining required hv elements,
// such as current score and player health
// dependencies on the pastPlays string
void completePastPlaysHv(HunterView hv, char *pastPlays) {
	
	// when its the first round
	if (hv->roundNum == 0) {
		hv->currPlayer = PLAYER_LORD_GODALMING;
		for (int i = 0; i < NUM_PLAYERS; i++) {
			Player roundPlayer = PLAYER_LORD_GODALMING + i;
			// set all player's trail to NOWHERE
			hv->allPlayers[roundPlayer].prevMoves[0] = NOWHERE;
			hv->allPlayers[roundPlayer].currLocation = NOWHERE;

			// set remaining info
			hv->allPlayers[i].name = roundPlayer;
			if (roundPlayer != PLAYER_DRACULA)
				hv->allPlayers[i].health = GAME_START_HUNTER_LIFE_POINTS;
			else
				hv->allPlayers[i].health = GAME_START_BLOOD_POINTS;
		}
	}

	// do actions from each round
	for (int i = 0; i < hv->roundNum; i++) {
		// array position of the start of the round (players name)
		int startOfRound = i * ROUND_DIFF;
		
		// get current round's player
		Player roundPlayer;
		switch(pastPlays[startOfRound]) {
			case 'G':
				roundPlayer = PLAYER_LORD_GODALMING;
				hv->currPlayer = PLAYER_DR_SEWARD;
				break;
			case 'S':
				roundPlayer = PLAYER_DR_SEWARD;
				hv->currPlayer = PLAYER_VAN_HELSING;
				break;
			case 'H':
				roundPlayer = PLAYER_VAN_HELSING;
				hv->currPlayer = PLAYER_MINA_HARKER;
				break;
			case 'M':
				roundPlayer = PLAYER_MINA_HARKER;
				hv->currPlayer = PLAYER_DRACULA;
				break;
			case 'D':
				roundPlayer = PLAYER_DRACULA;
				hv->currPlayer = PLAYER_LORD_GODALMING;
				// dracula's turn reduces point by 1
				hv->gameScore -= SCORE_LOSS_DRACULA_TURN;
				break;
		}
        
		// add the player's current location to the trail
		completePlayerTrailsHv(hv, &pastPlays[startOfRound+1], roundPlayer);

		// not dracula
		if (roundPlayer != PLAYER_DRACULA) {
			// if they are in the hospital because of losing lifepoints
			if (HvGetPlayerLocation(hv, roundPlayer) == ST_JOSEPH_AND_ST_MARY 
				&& hv->allPlayers[roundPlayer].health <= 0)
				hv->allPlayers[roundPlayer].health = GAME_START_HUNTER_LIFE_POINTS;

			// check each action in the round
			for (int i = 3; pastPlays[startOfRound+i] != '.'; i++) {
				switch(pastPlays[startOfRound+i]) {
					case 'T': {// encountered trap
						char *tmpPastPlays = malloc(strlen(hv->pastPlays)*sizeof(char));
						strcpy(tmpPastPlays, hv->pastPlays);
						tmpPastPlays[startOfRound+i] = '.';
						strcpy(hv->pastPlays, tmpPastPlays);
						free(tmpPastPlays);
						hv->allPlayers[roundPlayer].health -= LIFE_LOSS_TRAP_ENCOUNTER;
						break;
					}
					case 'D': // endcountered dracula
						hv->allPlayers[roundPlayer].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
						hv->allPlayers[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
						break;
					case 'V': {// encountered vampire
						char *tmpPastPlays = malloc(strlen(hv->pastPlays)*sizeof(char));
						strcpy(tmpPastPlays, hv->pastPlays);
						tmpPastPlays[startOfRound+i] = '.';
						strcpy(hv->pastPlays, tmpPastPlays);
						free(tmpPastPlays);
						break;
					}
				}
			}

			// when the hunter loses all their health, determine what to
			if (hv->allPlayers[roundPlayer].health <= 0) {
				hv->allPlayers[roundPlayer].health = 0;
				hv->gameScore -= SCORE_LOSS_HUNTER_HOSPITAL;
				return;
			}

			// when the player didnt move and didnt just got sent to hospital
			if (hv->allPlayers[roundPlayer].prevMoves[0] == 
				hv->allPlayers[roundPlayer].prevMoves[1]
				&& hv->allPlayers[roundPlayer].health <= 0)
				hv->allPlayers[roundPlayer].health += LIFE_GAIN_REST;

			// reduce hunters health to max if it is over
			if (hv->allPlayers[roundPlayer].health > GAME_START_HUNTER_LIFE_POINTS)
				hv->allPlayers[roundPlayer].health = GAME_START_HUNTER_LIFE_POINTS;

		} else { // player is dracula
			// check if vamp matured
			if (pastPlays[startOfRound+5] == 'V')
				hv->gameScore -= SCORE_LOSS_VAMPIRE_MATURES; 

			// use draculas current location to test if he is at sea
			PlaceId lastPos = dracLocationDetailHv(hv, true);

			if (lastPos == CASTLE_DRACULA)
				hv->allPlayers[roundPlayer].health += LIFE_GAIN_CASTLE_DRACULA;
		}
	}
	
}

// convert a Player to a char and return it
char convertToPlayerHv(Player player) {
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

// determine if dracula is current at sea, including
// double moves
PlaceId dracLocationDetailHv(HunterView hv, bool updateHealth) {
	int doubleVal = 0, healthLoss = 0, len;
	PlaceId currId = hv->allPlayers[PLAYER_DRACULA].currLocation;
	// find the len of the array upto currLocation
	for (len = 1; len < MIN_TRAIL; len++) {
		if (hv->allPlayers[PLAYER_DRACULA].prevMoves[len] == currId) {
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
			currId = hv->allPlayers[PLAYER_DRACULA].currLocation;
			if (placeIdToType(currId) == SEA) 
				healthLoss++;
			break;
	}
	
	// check the locations of the double back/hide
	while (doubleVal != 0) {	
		switch(hv->allPlayers[PLAYER_DRACULA].prevMoves[len-doubleVal]) {
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
				currId = hv->allPlayers[PLAYER_DRACULA].prevMoves[len-doubleVal];
				if (placeIdToType(currId) == SEA) 
					healthLoss++;
				doubleVal = 0;
				
				break;
		}
	}

	// lose health when at sea
	if (healthLoss > 0 && updateHealth) 
		hv->allPlayers[PLAYER_DRACULA].health -= LIFE_LOSS_SEA;
		
	return currId;
}

// add all possible rail locations to the reacharray, only while the hunter
// can move through another rail. 
void recurAddRailHv(HunterView hv, ConnList reachList, PlaceId *reachArray, 
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
			ConnList curr = MapGetConnections(hv->m, reachList->p);
			// loop through each one and recur if rail connection
			while (curr != NULL) {
				if (curr->type == RAIL)
					recurAddRailHv(hv, curr, reachArray, railDistance, 
						numReturnedLocs, visitedLocs);
				curr = curr->next;
			}
		} 
	}

}
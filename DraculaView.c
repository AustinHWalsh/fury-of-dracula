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

#include <string.h>

#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "Graph.h"

// add your own #includes here
#define NOT_MEMBER 0
#define IS_MEMBER 1
#define LOCATION_ABBREVIATION_MAX 3
#define TBA_LOCATION -3
#define MIN_TRAIL (dv->roundNum + 7)
//#define MAX_RAIL_CONN 43

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
void completePlayerTrailsDv(DraculaView dv, char *startId, Player player);
void completePastPlaysDv(DraculaView dv, char *pastPlays);
void removeTrapFromDracDv(DraculaView dv, int HunterTrapPos);
PlaceId dracLocationDetailDv(DraculaView dv, bool updateHealth);
Graph makeRailGraph();
////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	// create new game view
	DraculaView new = malloc(sizeof(struct draculaView));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView!\n");
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
	completePastPlaysDv(new, pastPlays);

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
	if (dv->allPlayers[player].currLocation == NOWHERE)
	    return NOWHERE;
	    
    //player has a current location
    return dv->allPlayers[player].currLocation;
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	int r = (DvGetRound(dv) - 1) % 13;
	//an immature vampire exists
	if (0 <= r && r < 6) {
		//temp copy of pastPlays for strtok
		char *pastPlayCpy = malloc(strlen(dv->pastPlays) * sizeof(char));
		strcpy(pastPlayCpy, dv->pastPlays);
		//find location of vamp in pastPlay
		char *currStr;
		//if immature vampire is in the first round
		if (DvGetRound(dv) < 14) {
		    currStr = strtok(pastPlayCpy, ". ");
		} else {
		// go to correct roundNum line to search for V
			currStr = strtok(pastPlayCpy, "\n");
			for (int i = 0; i < DvGetRound(dv) - r - 1; i++)
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
			if (currStr != NULL && currStr[0] != 'D' && strcmp(hunterLoc, vampLoc) == 0)
				//vampire has been vanquished
				return NOWHERE;
		}
		//hunters have not visited city the vampire is in
		return placeAbbrevToId(vampLoc);
	}
	//vampire not spawned or not immature
	return NOWHERE;
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	PlaceId *trapLocations = malloc(TRAIL_SIZE * (sizeof(PlaceId)));
    *numTraps = 0;
	// drac hasnt made the rth move yet
    for (int i = 0; i < DvGetRound(dv); i++) {
		// position of trap in pastPlay string
		int checkTrap = 35 + (5 * ROUND_DIFF * i);
		if (dv->pastPlays[checkTrap] != 'T')
			continue;
		
		char cityAbbrev[3] = {dv->pastPlays[checkTrap-2], dv->pastPlays[checkTrap-1], '\0'};
		PlaceId cityId = placeAbbrevToId(cityAbbrev);

		// accounting for double backs/hide
		switch(cityId) {
			case DOUBLE_BACK_1:
				cityId = trapLocations[*numTraps-1];
				break;
			case DOUBLE_BACK_2:
				cityId = trapLocations[*numTraps-2];
				break;
			case DOUBLE_BACK_3:
				cityId = trapLocations[*numTraps-3];
				break;
			case DOUBLE_BACK_4:
				cityId = trapLocations[*numTraps-4];
				break;
			case DOUBLE_BACK_5:
				cityId = trapLocations[*numTraps-5];
				break;
			case HIDE:
				cityId = trapLocations[*numTraps-1];
				break;
			default:
				break;
		}
		trapLocations[(*numTraps)++] = cityId;
    } 
    return trapLocations;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	//put NUM_REAL_PLACES for now
	PlaceId *validMoves = malloc(NUM_REAL_PLACES *sizeof(PlaceId));
	int validMovesNum = 0;
	validMoves[validMovesNum] = NOWHERE;
	//LOCATION move if there is an adjacent location by road or 
    //boat that drac has not been to in the last 5 prevmoves
    //not including ST_JOSEPH_AND_ST_MARY

    for (int i = 0; CONNECTIONS[i].v != UNKNOWN_PLACE; i++) {
        bool locationValid = true;
		//find each connected location that isn't by rail or the hospital
        if (CONNECTIONS[i].v == dv->allPlayers[PLAYER_DRACULA].currLocation
			&& CONNECTIONS[i].t != RAIL
			&& CONNECTIONS[i].w != ST_JOSEPH_AND_ST_MARY) {
			for (int j = MIN_TRAIL- 4; j <= MIN_TRAIL; j++) {
            //if connected location is in the last 5 moves, find next location
				if (dv->allPlayers[PLAYER_DRACULA].prevMoves[j] == CONNECTIONS[i].w) {
					locationValid = false;
                    break;
				}  
            }
			//add valid location to validMoves
            if (locationValid) {
                validMoves[validMovesNum] = CONNECTIONS[i].w;
                validMovesNum++;
            }
        }
    }
    
    //HIDE if he hasn't made HIDE move in the last 5 rounds
    // and not at sea
    //DOUBLE_BACK 1-5 cannot be made if done in last 5 rounds
    // aka last 5 prevmove locations do not have duplicates
    
    bool hide = true;
    bool doubleBack = true;
    
    int j = MIN_TRAIL-4;
    while (j <= MIN_TRAIL) {
        int k = j + 1;
        while (k <= MIN_TRAIL) {
            if (dv->allPlayers[PLAYER_DRACULA].prevMoves[j] == dv->allPlayers[PLAYER_DRACULA].prevMoves[k]
				&& dv->allPlayers[PLAYER_DRACULA].prevMoves[j] != TBA_LOCATION) {
                if (k == j + 1) {
				//dracula has stayed in the same location 2 times in a row
					hide = false;
					doubleBack = false;
				} else
                    doubleBack = false;
            }
            k++;
        }
        j++;
    }


    //Dracula can HIDE
    if (hide) 
        validMoves[validMovesNum++] = HIDE;  
	//Dracula can DOUBLE_BACK
	if (doubleBack) {
		//add DOUBLE_BACK 1 
		int locNum = validMovesNum;

		for (int i = 0; i < locNum; i++)
			validMoves[validMovesNum++] = DOUBLE_BACK_1+i;
	}

    *numReturnedMoves = validMovesNum;
	//No valid moves other than TELEPORT
	if (validMoves[0] == NOWHERE)
	    return NULL;
	//Dracula can make a move
	return validMoves;
}

PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	bool road = true;
	bool rail = true;
	bool boat = true;
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

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
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

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION

	//player hasn't made a move yet
	if (dv->allPlayers[player].currLocation == NOWHERE) {
		numReturnedLocs = 0;
		return NULL;
	}
	
	PlaceId *canGo;
	int canGoNum = 0;

	//player is a hunter
	if (player != PLAYER_DRACULA) {
		//calculate number of adjacent locations
		int connectionNum = 0;
		for (int i = 0; CONNECTIONS[i].v != UNKNOWN_PLACE; i++) {
			if (CONNECTIONS[i].v == dv->allPlayers[player].currLocation) {
				connectionNum++;
			}
		}
		canGo = malloc(connectionNum * sizeof (PlaceId));
		for (int i = 0; CONNECTIONS[i].v != UNKNOWN_PLACE; i++) {
			if (CONNECTIONS[i].v == dv->allPlayers[player].currLocation) {
				//int railDistance = (DvGetRound(dv) + player) % 4;
				if (CONNECTIONS[i].t == RAIL /*&& within rail distance*/) {
					canGo[canGoNum] = CONNECTIONS[i].w;
					canGoNum++;
				} else {
					canGo[canGoNum] = CONNECTIONS[i].w;
					canGoNum++;
				}
			}
		}
		*numReturnedLocs = connectionNum;
		return canGo;
	}
	//player is Dracula
	canGo = DvWhereCanIGo(dv, &canGoNum);
	*numReturnedLocs = canGoNum;
	return canGo;
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	
	//player hasn't made a move yet
	if (dv->allPlayers[player].currLocation == NOWHERE) {
		numReturnedLocs = 0;
		return NULL;
	}
	
	PlaceId *canGo;
	int canGoNum = 0;

	//player is a hunter
	if (player != PLAYER_DRACULA) {
		//calculate number of adjacent locations
		int connectionNum = 0;
		for (int i = 0; CONNECTIONS[i].v != UNKNOWN_PLACE; i++) {
			if (CONNECTIONS[i].v == dv->allPlayers[player].currLocation) {
				connectionNum++;
			}
		}
		canGo = malloc(connectionNum * sizeof (PlaceId));
		for (int i = 0; CONNECTIONS[i].v != UNKNOWN_PLACE; i++) {
			if (CONNECTIONS[i].v == dv->allPlayers[player].currLocation) {
				//int railDistance = (DvGetRound(dv) + player) % 4;
				if (CONNECTIONS[i].t == RAIL && rail == true /*&& within rail distance*/) {
					canGo[canGoNum] = CONNECTIONS[i].w;
					canGoNum++;
				} else {
					if ((CONNECTIONS[i].t == ROAD && road == true)
						|| (CONNECTIONS[i].t == BOAT && boat == true)) {
						canGo[canGoNum] = CONNECTIONS[i].w;
						canGoNum++;
					}
				}
			}
		}
		*numReturnedLocs = connectionNum;
		return canGo;
	}
	//player is Dracula
	canGo = DvWhereCanIGoByType(dv, road, boat, &canGoNum);
	*numReturnedLocs = canGoNum;
	return canGo;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

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
void completePlayerTrailsDv(DraculaView dv, char *startId, Player player) {
	// create the abbreviation of the city from the paststring
	char cityAbbrev[3] = {startId[0], startId[1], '\0'};
	PlaceId cityId = placeAbbrevToId(cityAbbrev);

	for (int i = 0; i < MIN_TRAIL; i++) {
		// first empty stop in trail
		if (dv->allPlayers[player].prevMoves[i] == TBA_LOCATION) {
			dv->allPlayers[player].prevMoves[i] = cityId;
			break;
		}
					
	}

	// reset that player's current position to the first in the trail
	dv->allPlayers[player].currLocation = cityId; 
} 

// clean up and complete the remaining required dv elements,
// such as current score and player health
// dependencies on the pastPlays string
void completePastPlaysDv(DraculaView dv, char *pastPlays) {
	
	// when its the first round
	if (dv->roundNum == 0) {
		dv->currPlayer = PLAYER_LORD_GODALMING;
		for (int i = 0; i < NUM_PLAYERS; i++) {
			Player roundPlayer = PLAYER_LORD_GODALMING + i;
			// set all player's trail to NOWHERE
			dv->allPlayers[roundPlayer].prevMoves[0] = NOWHERE;
			dv->allPlayers[roundPlayer].currLocation = NOWHERE;

			// set remaining info
			dv->allPlayers[i].name = roundPlayer;
			if (roundPlayer != PLAYER_DRACULA)
				dv->allPlayers[i].health = GAME_START_HUNTER_LIFE_POINTS;
			else
				dv->allPlayers[i].health = GAME_START_BLOOD_POINTS;
		}
	}

	// do actions from each round
	for (int i = 0; i < dv->roundNum; i++) {
		// array position of the start of the round (players name)
		int startOfRound = i * ROUND_DIFF;
		
		// get current round's player
		Player roundPlayer;
		switch(pastPlays[startOfRound]) {
			case 'G':
				roundPlayer = PLAYER_LORD_GODALMING;
				dv->currPlayer = PLAYER_DR_SEWARD;
				break;
			case 'S':
				roundPlayer = PLAYER_DR_SEWARD;
				dv->currPlayer = PLAYER_VAN_HELSING;
				break;
			case 'H':
				roundPlayer = PLAYER_VAN_HELSING;
				dv->currPlayer = PLAYER_MINA_HARKER;
				break;
			case 'M':
				roundPlayer = PLAYER_MINA_HARKER;
				dv->currPlayer = PLAYER_DRACULA;
				break;
			case 'D':
				roundPlayer = PLAYER_DRACULA;
				dv->currPlayer = PLAYER_LORD_GODALMING;
				// dracula's turn reduces point by 1
				dv->gameScore -= SCORE_LOSS_DRACULA_TURN;
				break;
		}
        
		// add the player's current location to the trail
		completePlayerTrailsDv(dv, &pastPlays[startOfRound+1], roundPlayer);

		// not dracula
		if (roundPlayer != PLAYER_DRACULA) {
			// if they are in the hospital because of losing lifepoints
			if (DvGetPlayerLocation(dv, roundPlayer) == ST_JOSEPH_AND_ST_MARY 
				&& dv->allPlayers[roundPlayer].health <= 0)
				dv->allPlayers[roundPlayer].health = GAME_START_HUNTER_LIFE_POINTS;

			// check each action in the round
			for (int i = 3; pastPlays[startOfRound+i] != '.'; i++) {
				switch(pastPlays[startOfRound+i]) {
					case 'T': {// encountered trap
						if (dv->allPlayers[roundPlayer].health <= 0) break;
						removeTrapFromDracDv(dv, startOfRound+i);
						dv->allPlayers[roundPlayer].health -= LIFE_LOSS_TRAP_ENCOUNTER;
						break;
					}
					case 'D': // endcountered dracula
						dv->allPlayers[roundPlayer].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
						dv->allPlayers[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
						break;
					case 'V': {// encountered vampire
						char *tmpPastPlays = malloc(strlen(dv->pastPlays)*sizeof(char));
						strcpy(tmpPastPlays, dv->pastPlays);
						tmpPastPlays[startOfRound+i] = '.';
						strcpy(dv->pastPlays, tmpPastPlays);
						free(tmpPastPlays);
						break;
					}
				}
			}

			// when the hunter loses all their health, determine what to
			if (dv->allPlayers[roundPlayer].health <= 0) {
				dv->allPlayers[roundPlayer].health = 0;
				dv->gameScore -= SCORE_LOSS_HUNTER_HOSPITAL;
				return;
			}

			// when the player didnt move and didnt just got sent to hospital
			if (dv->allPlayers[roundPlayer].prevMoves[0] == 
				dv->allPlayers[roundPlayer].prevMoves[1]
				&& dv->allPlayers[roundPlayer].health <= 0)
				dv->allPlayers[roundPlayer].health += LIFE_GAIN_REST;

			// reduce hunters health to max if it is over
			if (dv->allPlayers[roundPlayer].health > GAME_START_HUNTER_LIFE_POINTS)
				dv->allPlayers[roundPlayer].health = GAME_START_HUNTER_LIFE_POINTS;

		} else { // player is dracula
			// check if vamp matured
			if (pastPlays[startOfRound+5] == 'V')
					dv->gameScore -= SCORE_LOSS_VAMPIRE_MATURES; 
			// use draculas current location to test if he is at sea
			PlaceId lastPos = dracLocationDetailDv(dv, true);

			if (lastPos == CASTLE_DRACULA)
				dv->allPlayers[roundPlayer].health += LIFE_GAIN_CASTLE_DRACULA;
		}
	}
	
}
// determine if dracula is current at sea, including
// double moves
PlaceId dracLocationDetailDv(DraculaView dv, bool updateHealth) {
	int doubleVal = 0, healthLoss = 0, len;
	PlaceId currId = dv->allPlayers[PLAYER_DRACULA].currLocation;
	// find the len of the array upto currLocation
	for (len = 1; len < MIN_TRAIL; len++) {
		if (dv->allPlayers[PLAYER_DRACULA].prevMoves[len] == currId) {
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
			currId = dv->allPlayers[PLAYER_DRACULA].currLocation;
			if (placeIdToType(currId) == SEA) 
				healthLoss++;
			break;
	}
	
	// check the locations of the double back/hide
	while (doubleVal != 0) {	
		switch(dv->allPlayers[PLAYER_DRACULA].prevMoves[len-doubleVal]) {
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
				currId = dv->allPlayers[PLAYER_DRACULA].prevMoves[len-doubleVal];
				if (placeIdToType(currId) == SEA) 
					healthLoss++;
				doubleVal = 0;
				
				break;
		}
	}

	// lose health when at sea
	if (healthLoss > 0 && updateHealth) 
		dv->allPlayers[PLAYER_DRACULA].health -= LIFE_LOSS_SEA;
		
	return currId;
}

// when a hunter encounters a trap remove the trap from draculas trail
// in the pastPlays string
void removeTrapFromDracDv(DraculaView dv, int HunterTrapPos) {
	char cityAbbrev[3] = {dv->pastPlays[HunterTrapPos-2], dv->pastPlays[HunterTrapPos-1], '\0'};
	// location which trap was found by hunter
	PlaceId trapCity = placeAbbrevToId(cityAbbrev);

	int i;
	// find location in pastPlays string
	for (i = 0; i < DvGetRound(dv); i++) {
		// position of trap in pastPlay string
		int checkTrap = 35 + (5 * ROUND_DIFF * i);
		if (dv->pastPlays[checkTrap] != 'T') 
			continue;

		// check the location of the trap is the same as the one the hunter found
		char dracCity[3] = {dv->pastPlays[checkTrap-2], dv->pastPlays[checkTrap-1], '\0'};
		PlaceId dracLoc = placeAbbrevToId(dracCity);
		if (dracLoc != trapCity)
			continue;
		
		// remove 'T' from string
		char *tmpPastPlays = malloc(strlen(dv->pastPlays)*sizeof(char));
		strcpy(tmpPastPlays, dv->pastPlays);
		tmpPastPlays[checkTrap] = '.';
		strcpy(dv->pastPlays, tmpPastPlays);
		free(tmpPastPlays);
	}
}

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
// follow van helsings path
PlaceId vanHelPathMove(PlaceId currLocation);

void decideHunterMove(HunterView hv)
{
	
	int currPlayer = HvGetPlayer(hv); //current hunter

	// first round
    if (HvGetRound(hv) == 0) {
		if (currPlayer == PLAYER_LORD_GODALMING)
			registerBestPlay("ZU", "start!"); //zurich
		else if (currPlayer == PLAYER_DR_SEWARD)
			registerBestPlay("ED", "start!"); //edingburgh
		else if (currPlayer == PLAYER_VAN_HELSING)
			registerBestPlay("SR", "start!"); //saragossa
		else if (currPlayer == PLAYER_MINA_HARKER)
			registerBestPlay("SZ", "start!"); //szeged
		return;
	}

	//bool goToCD = false;
	int DracLocation = HvGetPlayerLocation(hv, PLAYER_DRACULA);

	int shortestPathLen;
	PlaceId *shortestPath;

	int whereDracCanGoLen;
	PlaceId *whereDracCanGo;

	int lastRevealedRound;

	PlaceId vampLoc = HvGetVampireLocation(hv);
	PlaceId *vampPath;
	int vampPathLen = -1;

	//stop hunters from going to CD if currPlayer has already reached it
		//or Dracula has a new revealed location
		//or hunter encounters Dracula
	/*if (HvGetPlayerLocation(hv, currPlayer) == CASTLE_DRACULA
		|| HvGetLastKnownDraculaLocation(hv, &lastRevealedRound) != CASTLE_DRACULA
		|| HvGetPlayerLocation(hv, currPlayer) == HvGetPlayerLocation(hv, PLAYER_DRACULA)) {
		
		goToCD = false;
	}*/

	//continue heading to CD
	/*if (goToCD == true) {
		moveToCD(currPlayer, hv);
		return;
	}*/

	//hunters head towards the first revealed location of Dracula
	//Round 1
	/*if (HvGetRound(hv) == 1) {
		goToCD = true;
		moveToCD(currPlayer, hv);
		return;
	}*/

	//if drac's current loc is revealed
	//use whereCanTheyGo on drac
	//find the location furthest away from hunter
	//use shortest path to that location

	//if immature vampire exist, move there to vanquish it
	if (vampLoc != NOWHERE && vampLoc != CITY_UNKNOWN) {
		vampPath = HvGetShortestPathTo(hv, currPlayer, vampLoc, &vampPathLen);
		//move to vamp if its within 5 moves
		if (vampPathLen <= 5) {
			registerBestPlay(placeIdToAbbrev(vampPath[0]), "Heading to vampire.");
		}
		return;
	}
	
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
		if (whereDracCanGoLen == 0) {
			//goToCD = true;
			moveToCD(currPlayer, hv);
			return;
		} else {
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
			//set max path limit to follow
			if (longestPath <= 6) {
				registerBestPlay(placeIdToAbbrev(shortestPath[0]), "Predicting Drac's path.");
				return;
			}
		}
		
	}

	
	PlaceId lastKnownDracLoc = HvGetLastKnownDraculaLocation(hv, &lastRevealedRound);
	
	//head towards Dracula last known location after round 1
	if (HvGetRound(hv) > 1 && lastKnownDracLoc != NOWHERE) {
		//follow Drac if last location is within 5 moves
		shortestPath = HvGetShortestPathTo(hv,currPlayer, lastKnownDracLoc, &shortestPathLen);
		if (shortestPath[0] >= MIN_REAL_PLACE && shortestPath[0] <= MAX_REAL_PLACE && shortestPathLen > 0 && shortestPathLen <= 5) {
			if (shortestPathLen == 1)
				registerBestPlay(placeIdToAbbrev(lastKnownDracLoc), "Reached Drac's last known location.");
			else
				registerBestPlay(placeIdToAbbrev(shortestPath[0]), "Behind you, Dracula.");
			return;
		}
	}

	//Fixed hunter paths to cover max locations

	PlaceId currHunterLoc = HvGetPlayerLocation(hv, currPlayer);

	if (currPlayer == PLAYER_LORD_GODALMING) {

		if (currHunterLoc == PLYMOUTH) {
			registerBestPlay(placeIdToAbbrev(LONDON), "G-Fixed path.");
			return;
		} else if (currHunterLoc == LONDON) {
			registerBestPlay(placeIdToAbbrev(SWANSEA), "G-Fixed path.");
			return;
		} else if (currHunterLoc == SWANSEA) {
			registerBestPlay(placeIdToAbbrev(IRISH_SEA), "G-Fixed path.");
			return;
		} else if (currHunterLoc == IRISH_SEA) {
			registerBestPlay(placeIdToAbbrev(ATLANTIC_OCEAN), "G-Fixed path.");
			return;
		} else if (currHunterLoc == ATLANTIC_OCEAN) {
			registerBestPlay(placeIdToAbbrev(GALWAY), "G-Fixed path.");
			return;
		} else if (currHunterLoc == GALWAY) {
			registerBestPlay(placeIdToAbbrev(DUBLIN), "G-Fixed path.");
			return;
		} else if (currHunterLoc == DUBLIN) {
			registerBestPlay(placeIdToAbbrev(LIVERPOOL), "G-Fixed path.");
			return;
		} else if (currHunterLoc == LIVERPOOL) {
			registerBestPlay(placeIdToAbbrev(MANCHESTER), "G-Fixed path.");
			return;
		} else if (currHunterLoc == MANCHESTER) {
			registerBestPlay(placeIdToAbbrev(EDINBURGH), "G-Fixed path.");
			return;
		} else if (currHunterLoc == EDINBURGH) {
			registerBestPlay(placeIdToAbbrev(NORTH_SEA), "G-Fixed path.");
			return;
		} else if (currHunterLoc == NORTH_SEA) {
			registerBestPlay(placeIdToAbbrev(HAMBURG), "G-Fixed path.");
			return;
		} else if (currHunterLoc == HAMBURG) {
			registerBestPlay(placeIdToAbbrev(COLOGNE), "G-Fixed path.");
			return;
		} else if (currHunterLoc == COLOGNE) {
			registerBestPlay(placeIdToAbbrev(AMSTERDAM), "G-Fixed path.");
			return;
		} else if (currHunterLoc == AMSTERDAM) {
			registerBestPlay(placeIdToAbbrev(BRUSSELS), "G-Fixed path.");
			return;
		} else if (currHunterLoc == BRUSSELS) {
			registerBestPlay(placeIdToAbbrev(LE_HAVRE), "G-Fixed path.");
			return;
		} else if (currHunterLoc == LE_HAVRE) {
			registerBestPlay(placeIdToAbbrev(ENGLISH_CHANNEL), "G-Fixed path.");
			return;
		} else if (currHunterLoc == ENGLISH_CHANNEL) {
			registerBestPlay(placeIdToAbbrev(PLYMOUTH), "G-Fixed path.");
			return;
		}
	} else if (currPlayer == PLAYER_VAN_HELSING) {
		PlaceId nextLoc = vanHelPathMove(HvGetPlayerLocation(hv, PLAYER_VAN_HELSING));
		if (nextLoc != NOWHERE) {
			registerBestPlay(placeIdToAbbrev(nextLoc), "on path");
			return;
		}
	} else if (currPlayer == PLAYER_DR_SEWARD) {

		if (currHunterLoc == EDINBURGH) {
			registerBestPlay(placeIdToAbbrev(NORTH_SEA), "S-Fixed path.");
			return;
		} else if (currHunterLoc == NORTH_SEA) {
			registerBestPlay(placeIdToAbbrev(AMSTERDAM), "S-Fixed path.");
			return;
		} else if (currHunterLoc == AMSTERDAM) {
			registerBestPlay(placeIdToAbbrev(COLOGNE), "S-Fixed path.");
			return;
		} else if (currHunterLoc == COLOGNE) {
			registerBestPlay(placeIdToAbbrev(HAMBURG), "S-Fixed path.");
			return;
		} else if (currHunterLoc == HAMBURG) {
			registerBestPlay(placeIdToAbbrev(BERLIN), "S-Fixed path.");
			return;
		} else if (currHunterLoc == BERLIN) {
			registerBestPlay(placeIdToAbbrev(PRAGUE), "S-Fixed path.");
			return;
		} else if (currHunterLoc == PRAGUE) {
			registerBestPlay(placeIdToAbbrev(NUREMBURG), "S-Fixed path.");
			return;
		} else if (currHunterLoc == NUREMBURG) {
			registerBestPlay(placeIdToAbbrev(LEIPZIG), "S-Fixed path.");
			return;
		} else if (currHunterLoc == LEIPZIG) {
			registerBestPlay(placeIdToAbbrev(FRANKFURT), "S-Fixed path.");
			return;
		} else if (currHunterLoc == FRANKFURT) {
			registerBestPlay(placeIdToAbbrev(STRASBOURG), "S-Fixed path.");
			return;
		} else if (currHunterLoc == STRASBOURG) {
			registerBestPlay(placeIdToAbbrev(BRUSSELS), "S-Fixed path.");
			return;
		} else if (currHunterLoc == BRUSSELS) {
			registerBestPlay(placeIdToAbbrev(PARIS), "S-Fixed path.");
			return;
		} else if (currHunterLoc == PARIS) {
			registerBestPlay(placeIdToAbbrev(LE_HAVRE), "S-Fixed path.");
			return;
		} else if (currHunterLoc == LE_HAVRE) {
			registerBestPlay(placeIdToAbbrev(ENGLISH_CHANNEL), "S-Fixed path.");
			return;
		} else if (currHunterLoc == ENGLISH_CHANNEL) {
			registerBestPlay(placeIdToAbbrev(LONDON), "S-Fixed path.");
			return;
		} else if (currHunterLoc == LONDON) {
			registerBestPlay(placeIdToAbbrev(MANCHESTER), "S-Fixed path.");
			return;
		} else if (currHunterLoc == MANCHESTER) {
			registerBestPlay(placeIdToAbbrev(EDINBURGH), "S-Fixed path.");
			return;
		}
	}
	
	else if (currPlayer == PLAYER_MINA_HARKER) {

        if (currHunterLoc == SZEGED) {
            registerBestPlay(placeIdToAbbrev(BELGRADE), "G-Fixed path.");
            return;
        } else if (currHunterLoc == BELGRADE) {
            registerBestPlay(placeIdToAbbrev(BUCHAREST), "G-Fixed path.");
            return;
        } else if (currHunterLoc == BUCHAREST) {
            registerBestPlay(placeIdToAbbrev(KLAUSENBURG), "G-Fixed path.");
            return;
        } else if (currHunterLoc == KLAUSENBURG) {
            registerBestPlay(placeIdToAbbrev(CASTLE_DRACULA), "G-Fixed path.");
            return;
        } else if (currHunterLoc == CASTLE_DRACULA) {
            registerBestPlay(placeIdToAbbrev(GALATZ), "G-Fixed path.");
            return;
        } else if (currHunterLoc == GALATZ) {
            registerBestPlay(placeIdToAbbrev(BLACK_SEA), "G-Fixed path.");
            return;
        } else if (currHunterLoc == BLACK_SEA) {
            registerBestPlay(placeIdToAbbrev(CONSTANTA), "G-Fixed path.");
            return;
        } else if (currHunterLoc == CONSTANTA) {
            registerBestPlay(placeIdToAbbrev(VARNA), "G-Fixed path.");
            return;
        } else if (currHunterLoc == VARNA) {
            registerBestPlay(placeIdToAbbrev(SOFIA), "G-Fixed path.");
            return;
        } else if (currHunterLoc == SOFIA) {
            registerBestPlay(placeIdToAbbrev(SALONICA), "G-Fixed path.");
            return;
        } else if (currHunterLoc == SALONICA) {
            registerBestPlay(placeIdToAbbrev(IONIAN_SEA), "G-Fixed path.");
            return;
        } else if (currHunterLoc == IONIAN_SEA) {
            registerBestPlay(placeIdToAbbrev(ATHENS), "G-Fixed path.");
            return;
        } else if (currHunterLoc == ATHENS) {
            registerBestPlay(placeIdToAbbrev(VALONA), "G-Fixed path.");
            return;
        } else if (currHunterLoc == VALONA) {
            registerBestPlay(placeIdToAbbrev(SARAJEVO), "G-Fixed path.");
            return;
        } else if (currHunterLoc == SARAJEVO) {
            registerBestPlay(placeIdToAbbrev(ZAGREB), "G-Fixed path.");
            return;
        } else if (currHunterLoc == ZAGREB) {
            registerBestPlay(placeIdToAbbrev(VIENNA), "G-Fixed path.");
            return;
        } else if (currHunterLoc == VIENNA) {
            registerBestPlay(placeIdToAbbrev(BUDAPEST), "G-Fixed path.");
            return;
        }else if (currHunterLoc == BUDAPEST) {
            registerBestPlay(placeIdToAbbrev(SZEGED), "G-Fixed path.");
            return;
        }
	}

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
		
	registerBestPlay(placeIdToAbbrev(reachable[moveNum]), "Moving"); // enter message 
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

// get the next move on van helsings path
// or return to the path if in the hospital
PlaceId vanHelPathMove(PlaceId currLocation) {
	switch (currLocation) {
		case SARAGOSSA: return SANTANDER; break;
		case SANTANDER: return MADRID; break;
		case MADRID: return ALICANTE; break;
		case ALICANTE: return GRANADA; break;
		case GRANADA: return CADIZ; break;
		case CADIZ: return LISBON; break;
		case LISBON: return ATLANTIC_OCEAN; break;
		case ATLANTIC_OCEAN: return GALWAY; break;
		case GALWAY: return DUBLIN; break;
		case DUBLIN: return IRISH_SEA; break;
		case IRISH_SEA: return LIVERPOOL; break;
		case LIVERPOOL: return SWANSEA; break;
		case SWANSEA: return LONDON; break;
		case LONDON: return PLYMOUTH; break;
		case PLYMOUTH: return ENGLISH_CHANNEL; break;
		case ENGLISH_CHANNEL: return LE_HAVRE; break;
		case LE_HAVRE: return NANTES; break;
		case NANTES: return BAY_OF_BISCAY; break;
		case BAY_OF_BISCAY: return BORDEAUX; break;
		case BORDEAUX: return SARAGOSSA; break;
		case ST_JOSEPH_AND_ST_MARY: return ZAGREB; break;
		case ZAGREB: return MUNICH; break;
		case MUNICH: return ZURICH; break;
		case ZURICH: return GENEVA; break;
		case GENEVA: return CLERMONT_FERRAND; break;
		case CLERMONT_FERRAND: return BORDEAUX; break;	
		default: return NOWHERE; break;						
	}
}
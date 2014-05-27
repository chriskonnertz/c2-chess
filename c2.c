/*
	C2
	Version 0.8
	2013-2014
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <windows.h> // Windows API header

#define COLOR_DEFAULT 7
#define COLOR_ERROR 12
#define COLOR_INFO 14
#define COLOR_SLIGHT 8
#define COLOR_DEV 2
#define COLOR_PLAYER_WHITE 287
#define COLOR_PLAYER_WHITEA 15 // 415
#define COLOR_PLAYER_BLACK 285
#define COLOR_PLAYER_BLACKA 13 // 413
#define COLOR(c) SetConsoleTextAttribute(hConsole, c)

const char innerBoardMin = 21;
const char innerBoardMax = 98;
const char boardMax = 120;
const int valueCheckMate = 2000000000;
const char bookFile[] = "book.txt";

char debugMode = 0;
char maxDepth = 4; // Gibt die maximale Suchtiefe an. 0 = eine Etage. Empfohlener Maximalwert: 4
char autoMode = 0; // Wenn 1 und zwei KIs eingestellt, spielen sie das Spiel schnellstmoeglich aus.
char history[1024] = { 0 };
time_t timeStamp;
HANDLE hConsole;

void printError(const char *format, ...) {
	va_list arglist; // Liste der Argumente 
	va_start(arglist, format); // Argumentliste initialisieren

	COLOR(COLOR_ERROR);
	vprintf(format, arglist);
	COLOR(COLOR_DEFAULT);

 	va_end(arglist);
}

void printInfo(const char *format, ...) {
	va_list arglist; // Liste der Argumente 
	va_start(arglist, format); // Argumentliste initialisieren

	COLOR(COLOR_INFO);
	vprintf(format, arglist);
	COLOR(COLOR_DEFAULT);

 	va_end(arglist);
}

void printSlight(const char *format, ...) {
	va_list arglist; // Liste der Argumente 
	va_start(arglist, format); // Argumentliste initialisieren

	COLOR(COLOR_SLIGHT);
	vprintf(format, arglist);
	COLOR(COLOR_DEFAULT);

 	va_end(arglist);
}

void printDev(const char *format, ...) {
	va_list arglist; // Liste der Argumente 
	va_start(arglist, format); // Argumentliste initialisieren

	COLOR(COLOR_DEV);
	vprintf(format, arglist);
	COLOR(COLOR_DEFAULT);

 	va_end(arglist);
}

void printPlayerWhite(const char *format, ...) {
	va_list arglist; // Liste der Argumente 
	va_start(arglist, format); // Argumentliste initialisieren

	COLOR(COLOR_PLAYER_WHITE);
	vprintf(format, arglist);
	COLOR(COLOR_DEFAULT);

 	va_end(arglist);
}

void printPlayerWhiteA(const char *format, ...) {
	va_list arglist; // Liste der Argumente 
	va_start(arglist, format); // Argumentliste initialisieren

	COLOR(COLOR_PLAYER_WHITEA);
	vprintf(format, arglist);
	COLOR(COLOR_DEFAULT);

 	va_end(arglist);
} 

void printPlayerBlack(const char *format, ...) {
	va_list arglist; // Liste der Argumente 
	va_start(arglist, format); // Argumentliste initialisieren

	COLOR(COLOR_PLAYER_BLACK);
	vprintf(format, arglist);
	COLOR(COLOR_DEFAULT);

 	va_end(arglist);
}

void printPlayerBlackA(const char *format, ...) {
	va_list arglist; // Liste der Argumente 
	va_start(arglist, format); // Argumentliste initialisieren

	COLOR(COLOR_PLAYER_BLACKA);
	vprintf(format, arglist);
	COLOR(COLOR_DEFAULT);

 	va_end(arglist);
}

/**
* Erstellt ein leeres Schachbrett (nur gefuellt mit Nullen und ungueltigem Bereich).
*/
void createEmptyBoard(short *board) {
	for (char i = 0; i < boardMax; i++) {
		if (i > innerBoardMin && i <= innerBoardMax && (i % 10 != 0) && ((i + 1) % 10 != 0)) {
			*(board + i) = 0;
		} else {
			*(board + i) = 7;
		}
	}
}

/**
* Erstellt ein Schachbrett mit Startaufstellung.
*/
void createStartBoard(short *board) {
	createEmptyBoard(board);

	// Schwarz - Reihe oben
	*(board + 21) = -4;
	*(board + 22) = -2;
	*(board + 23) = -3;
	*(board + 24) = -5;
	*(board + 25) = -6;
	*(board + 26) = -3;
	*(board + 27) = -2;
	*(board + 28) = -4;

	// Schwarz - Reihe unten
	*(board + 31) = -1;
	*(board + 32) = -1;
	*(board + 33) = -1;
	*(board + 34) = -1;
	*(board + 35) = -1;
	*(board + 36) = -1;
	*(board + 37) = -1;
	*(board + 38) = -1;

	// Weiss - Reihe oben
	*(board + 81) = 1;
	*(board + 82) = 1;
	*(board + 83) = 1;
	*(board + 84) = 1;
	*(board + 85) = 1;
	*(board + 86) = 1;
	*(board + 87) = 1;
	*(board + 88) = 1;

	// Weiss - Reihe unten
	*(board + 91) = 4;
	*(board + 92) = 2;
	*(board + 93) = 3;
	*(board + 94) = 5;
	*(board + 95) = 6;
	*(board + 96) = 3;
	*(board + 97) = 2;
	*(board + 98) = 4;
}

/**
* Gibt die Darstellung (das Symbol) einer Schachfigur bzw. eines Feldes als Char zurueck.
*/
char getPieceSymbolAsChar(short i) {
	switch (i) {
		case -6: // Spieler 1 schwarz:
			return 'K';
		case -5:
			return 'D';
		case -4:
			return 'T';
		case -3:
			return 'L';
		case -2:
			return 'S';
		case -1:
			return 'B';
		case -0: // leeres Feld
			return ' ';
		case 1:  // Spieler 0 weiss:
			return 'b';
		case 2:
			return 's';
		case 3:
			return 'l';
		case 4:
			return 't';
		case 5:
			return 'd';
		case 6:
			return 'k';
		case 7: // Rand-Feld
			return '!';
		default:
			printError("Unbekanntes Figuren-Symbol: %i\n", i);
			exit(1);
	}
}

/**
* Konvertiert einen gueltigen Board-Index (1-D) zu einer Kooridnate (2-D).
* Die Coordinate wird im String coord gespeichert.
*/
void convertIndexToCoord(char index, char *coord) {
	if (index < innerBoardMin) {
		printError("Positions-Index zu klein: %u\n", index);
		exit(1);
	}
	if (index > innerBoardMax) {
		printError("Positions-Index zu gross: %u\n", index);
		exit(1);
	}

	// ASCII-Werte: 105 = h, 97 = a; 48 = 0, 57 = 9
	*coord 			= 96 + (index % 10);
	*(coord + 1 ) 	= 57 - ((char) ((index - 10) / 10));
}

/**
* Konvertiert einen gueltigen Board-Index (1-D) zu einer Kooridnate (2-D).
* Die Coordinate wird im String coord gespeichert.
*/
char convertCoordToIndex(char *coord) {
	if (*(coord + 2) != 0) {
		printError("Ungueltige Koordinate: fehlendes \\0!\n");
		exit(1);
	}

	char n1 = *(coord) - 96; // a - h
	char n2 = *(coord + 1) - 48; // 1 - 8

	if (n1 < 1 || n1 > 8 || n2 < 1 || n2 > 8) {
		printError("Ungueltige Koordinate: zu grosser/kleiner Wert!\n");
		exit(1);
	}

	return (10 - n2) * 10 + n1;
}

/**
* Gibt ein Schachbrett auf stdout aus.
* Ist showAll = 1 werden auch die Rand-Felder gezeigt.
* Ist showLabels = 1 werden die IDs der Felder angezeigt.
*/
void printBoard(short *board, short showAll, short showLabels) {
	char min = innerBoardMin;
	char max = innerBoardMax + 1;

	if (showAll) {
		min = 0;
		max = boardMax;
	}

	char coord[] = "??";
	if (showLabels == 0) printSlight("\n  abcdefgh\n\n");
	for (char i = min; i < max; i++) {
		if (i % 10 == 0) printf("\n");
		if (showLabels == 0 && i % 10 == 1) printSlight("%i ", 10 - i / 10);
		if (showAll || (i % 10 != 0) && ((i + 1) % 10 != 0)) {
			convertIndexToCoord(i, coord);
			if (showLabels) {
				printf("%i: ", i, getPieceSymbolAsChar(board[i]));
				//printf("%s: %c\t", coord, getPieceSymbolAsChar(board[i]));
				//printf("%s|%i:%c\t", coord, i, getPieceSymbolAsChar(board[i]));
			}
			if (board[i] > 0) {
				if ((i + ((int) i / 10)) % 2 == 0) {
					printPlayerWhite("%c", getPieceSymbolAsChar(board[i]));
				} else {
					printPlayerWhiteA("%c", getPieceSymbolAsChar(board[i]));
				}
			} else {
				if ((i + ((int) i / 10)) % 2 == 0) {
					printPlayerBlack("%c", getPieceSymbolAsChar(board[i]));
				} else {
					printPlayerBlackA("%c", getPieceSymbolAsChar(board[i]));
				}
			}
			if (showLabels) printf("\t");
		}
	}
	printf("\n");
}

/**
* Die derzeitige History ausgeben.
*/
void printHistory(void) {
	char from[] = "??";
	char to[] = "??";

	for (short i = 0; i < sizeof(history) && history[i] != 0; i += 2) {
		convertIndexToCoord(history[i], 	from);
		convertIndexToCoord(history[i + 1], to);
		printf("Halbzug %3i: von %s nach %s\n", i / 2, from, to);
	}
}

/**
* Struktur zum Speichern eines Spielzustandes (Schachbrett, Zugnummer).
*/
struct GameState {
	short board[120]; // die interne Schachbrettdarstellung inklusive Randfelder. 0/0 = links oben
	short turnNumber; // die Nummer des aktuellen Zugs, 0 - n. Bestimmt auch, welcher Spieler am Zug ist (Spieler 0 beginnt).
	char ai0; // gibt an, Spieler 1 ein KI-Spieler ist (> 0) - und welcher KI-Typ - oder nicht (0)
	char ai1; // gibt an, Spieler 2 ein KI-Spieler ist (> 0) - und welcher KI-Typ - oder oder nicht (0)
};

/**
* Setzt Werte fuer einen GameState. Dieser muss als Pointer ubergeben werden.
*/
void setGameState(struct GameState *gs, short *board, short turnNumber, char ai0, char ai1) {
	if (turnNumber < 0) {
		printError("turnNumber kann nicht kleiner als 0 sein: %i\n", turnNumber);
		exit(1);
	}
	if (ai0 < 0) {
		printError("ai0 kann nicht kleiner als 1 sein: %i\n", ai0);
		exit(1);
	}
	if (ai1 < 0) {
		printError("ai1 kann nicht kleiner als 1 sein: %i\n", ai1);
		exit(1);
	}

	(*gs).turnNumber = turnNumber;
	(*gs).ai0 = ai0;
	(*gs).ai1 = ai1;

	for (char i = 0; i < boardMax; i++) {
		(*gs).board[i] = *(board + i);
	}
}

/**
* Kopiert einen GameState. Sowohl Original als auch Kopie muessen als Pointer uebergeben werden.
*/
void copyGameState(struct GameState *gssource, struct GameState *gscopy) {
	(*gscopy).turnNumber = (*gssource).turnNumber;
	(*gscopy).ai0 = (*gssource).ai0 ;
	(*gscopy).ai1 = (*gssource).ai1 ;

	for (char i = 0; i < boardMax; i++) {
		(*gscopy).board[i] = (*gssource).board[i];
	}
}

/**
* Speichert, falls moeglich, ein Spiel. Der GameState muss als Pointer uebergeben werden.
* Wenn showMessages = 0 ist, werden keine Bildschirmausgaben gemacht.
*/
void saveGame(struct GameState *gs, char *filename, char showMessages) {
	char fullFilename[255];
	FILE *file;

	sprintf(fullFilename, "savegames/%s.sav", filename);
	
	file = fopen(fullFilename, "w"); // Datei oeffnen. w = write
	if (file == NULL) {
		if (showMessages) printError("Erstellen der Savegame-Datei fehlgeschlagen.\n");
		return;
	}

	size_t size;
	size = fwrite(gs, sizeof(*gs), 1, file);
	size = fwrite(history, sizeof(history) * sizeof(char), 1, file);

	fclose(file); // Datei schliessen.
	if (showMessages) printInfo("Spiel gespeichert.\n");
}

/**
* Laedt, falls moeglich, ein Spiel. Das Spiel wird im GameState gespeichert, dieser muss also ein Pointer sein.
*/
void loadGame(struct GameState *gs, char *filename) {
	char fullFilename[255];
	FILE *file;

	sprintf(fullFilename, "savegames/%s.sav", filename); // kompletten Dateinamen erzeugen

	file = fopen(fullFilename, "r"); // Datei oeffnen. r = read
	if (file == NULL) {
		printError("Oeffnen der Savegame-Datei fehlgeschlagen.\n");
		return;
	}

	char ai0 = (*gs).ai0;
	char ai1 = (*gs).ai1;
	size_t size1 = fread(gs, sizeof(*gs), 1, file);
	(*gs).ai0 = ai0;
	(*gs).ai1 = ai1;

	size_t size2 = fread(history, sizeof(history) * sizeof(char), 1, file);

	fclose(file); // Datei schliessen.

	if (size1 == 0 || size2 == 0) {
		printError("Laden fehlgeschlagen: Der Inhalt des Savegames ist inkompatibel.\n");
	} else {
		printInfo("Spiel geladen.\n");
	}
}

/**
* Erstellt einen Autosave-Spielstand.
*/
void autoSave(struct GameState *gs) {
	char filename[15] = "autosave";
	sprintf(filename, "autosave%i", (*gs).turnNumber);
	saveGame(gs, filename, 0);
}

/**
* Laedt einen Autosave-Spielstand.
*/
void loadAutoSave(struct GameState *gs) {
	if ((*gs).turnNumber == 0) {
		printError("Es wurde noch kein Zug gemacht.\n");
		return;
	}

	char filename[15] = "autosave";
	sprintf(filename, "autosave%i", (*gs).turnNumber - 1);
	loadGame(gs, filename);
}

/**
* Laedt einen Zug aus dem Eroeffnungsbuch.
* Dieser wird in from und to - die als Referenz uebergeben werden - gespeichert.
* Die Funktion gibt 0 zurueck, wenn kein Zug gefunden wurde, sonst 1.
*/
int loadOpeningBookMove(struct GameState *gs, char *from, char *to) {
	short i = 0;
	short len = 0;
	short counter = 0;
	short lineLength = 115;
	char line[lineLength];
	FILE *file;

	file = fopen(bookFile, "r"); // Datei oeffnen. r = read
	if (file == NULL) {
		printError("Oeffnen der Eroeffnungsbuch-Datei fehlgeschlagen.\n");
		return 0;
	}

	int size = (*gs).turnNumber * 5 + 1;
	char his[size];

	// Derzeitige History als ein String speichern, 
	// der dann mit den Zeilen aus der Datei verglichen werden kann:
	for (i = 0; i < (*gs).turnNumber; i++) {
		counter = i * 5;

		if (history[i * 2] == 0) {
			printError("Fehler beim Laden eines Zugs aus dem Eroeffnungsbuch: Keine History fuer Zug %i gefunden!", i);
			return 0;
		}

		convertIndexToCoord(history[i * 2], 	from);
		convertIndexToCoord(history[i * 2 + 1], to);

		his[counter]	 = from[0];
		his[counter + 1] = from[1];
		his[counter + 2] = to[0];
		his[counter + 3] = to[1];
		his[counter + 4] = ' ';
	}

	// Laenge des Strings speichern und \0 anhaengen:
	if ((*gs).turnNumber > 0) {
		len = i * 5 - 1;
		his[len] = '\0';
	}

	counter = 0;
	if ((*gs).turnNumber > 0) {
		// Alle Zeilen zaehlen die mit der bisherigen History uebereinstimmen und noch neue Zuege beinhalten:
		while (fgets(line, lineLength, file)) {
			if (strncmp(line, his, len) == 0 && strlen(line) > size) counter++;
		}
	} else {
		// Da noch kein Zug gemacht wurde, einfach nur die Anzahl der Zeilen zaehlen:
		while (fgets(line, lineLength, file)) {
			counter++;
		}
	}

	if (counter == 0) return 0; // leere Datei / keine moeglichen Zuege gefunden

	i = rand() % counter + 1; // eine Zeile (=Zug) zufaellig aus den verfuegbaren auswaehlen

	rewind(file); // Einlesen der Datei wieder ganz vorne starten

	counter = 0;
	while (fgets(line, lineLength, file)) {
		if ((*gs).turnNumber == 0 || (strncmp(line, his, len) == 0 && strlen(line) > size)) counter++;
		if (counter == i) { // die zuvor per Zufall ausgewaehlte Zeile wurde gefunden
			// Naechsten Zug aus der Zeile auslesen:
			for (i = 0; i <= strlen(line) - 45; i += 5) {
				from[0] = line[i];
				from[1] = line[i + 1];

				to[0] = line[i + 2];
				to[1] = line[i + 3];

				if (i / 5 == (*gs).turnNumber) return 1; // Zug gefunden!
			}
		}
	}

	return 0;
}

/**
* Gibt 1 zurueck, wenn der Spieler, der am Zug ist, eine KI ist, sonst 0.
*/
char isCurrentPlayerAi(struct GameState *gs) {
	char player = (*gs).turnNumber % 2;
	if ((player == 0 && (*gs).ai0 > 0) || (player == 1 && (*gs).ai1 > 0)) {
		return 1;
	} else {
		return 0;
	}
}

/**
* Gibt 1 zurueck, wenn der Spieler, der am Zug ist, 
* der Besitzer der Figur auf der Position index ist. (index muss auf ein Feld verweisen.)
* Sonst wid 0 zurueck gegeben. (Auch wenn das Feld leer/ungueltig ist.)
*/
char isCurrentPlayerOwner(struct GameState *gs, char index) {
	char player = (*gs).turnNumber % 2;

	if (player == 0) {
		return ((*gs).board[index] > 0 && (*gs).board[index] != 7);
	} else {
		return ((*gs).board[index] < 0);
	}
}

/**
* Gibt die Spielernummer (0 weiss/1 schwarz) zurueck, wenn eine Figur auf der Position index steht,
* sonst (leer/Rand) 2. (index muss auf ein Feld verweisen.)
*/
char getOwner(struct GameState *gs, char index) {
	if ((*gs).board[index] == 0 || (*gs).board[index] == 7) return 2;

	if ((*gs).board[index] > 0) return 0; // weiss
	if ((*gs).board[index] < 0) return 1; // schwarz
}

/**
* Berechnet alle moeglichen Zuege (genauer: Endpositionen) fuer die Figur auf der Position index.
* from ist ein Position index und muss auf ein existierendes Feld zeigen. Es ist egal, was sich auf dem Feld befindet.
* (Befindet sich bspw. keine Figur auf dem Feld, werden 0 gueltige Zuege zurueckgebeben.)
* (Es wird ignoriert, ob der Besitzer der Figur am Zug ist.)
* Die Rueckgabe erfolgt ueber den Parameter moves, der ein Zeiger auf einen Array von Position Indizes sein muss.
*/
void generatePieceMoves(struct GameState *gs, short from, char *moves) {
	if ((*gs).board[from] == 0 || (*gs).board[from] == 7) return;

	short piece = abs((*gs).board[from]); // Typ der Figur speichern
	char i;
	char counter = 0;
	short directions[8] = { 0 }; // setze alle Elemente auf 0
	char player = ((*gs).board[from] < 0);
	char owner = 0;

	if (piece == 6 || piece == 5 || piece == 4) { // horizontal + vertikal (fuer Koenig, Dame, Turm)
		directions[0] = -10; directions[1] = -1; directions[2] = +1; directions[3] = +10; 
		if (piece == 6 || piece == 5) { // Koenig und Dame - diagonal hinzufuegen:
			directions[4] = -11; directions[5] = -9; directions[6] = +9; directions[7] = +11; 
		}
	}
	// Laeufer - nur diagonal:
	if (piece == 3) {
		directions[0] = -11; directions[1] = -9; directions[2] = +9; directions[3] = +11; 
	}
	// Springer:
	if (piece == 2) {
		directions[0] = -21; directions[1] = -19; directions[2] = -12; directions[3] = -8;
		directions[4] = +8; directions[5] = +12; directions[6] = +19; directions[7] = +21; 
	}
	// Bauern - vertikal:
	if (piece == 1) {
		if (player == 0) { // wenn Spieler 0 (weiss)
			directions[0] = -10; directions[1] = -11; directions[2] = -9;
		} else {
			directions[0] = 10; directions[1] = 11; directions[2] = 9;
		}
	}

	for (char d = 0; d < 8 && directions[d] != 0; d++) {
		for (i = from + directions[d]; (*gs).board[i] != 7; i += directions[d]) {
			owner = getOwner(gs, i);

			if (owner == player) break;
			if (piece == 1) { 
				if (directions[d] != 10 && directions[d] != -10 && (*gs).board[i] == 0) break; // Bauer darf nur diagonal laufen wenn er schlaegt
				if ((directions[d] == 10 || directions[d] == -10) && (*gs).board[i] != 0) break; // Bauer darf nur vertikal laufen wenn er nicht
			}

			moves[counter] = i;
			counter++;

			if (owner == (1 - player)) break; // Kann gegnerische Figur schlagen. Aber nur eine pro Zug!
			if (piece == 6 || piece == 2) break; // Koenig Springer duerfen sich nur um 1 Schritt bewegen
			if (  // Bauer darf nur 1 Feld laufen ausser er steht noch auf der Grundlinie
				piece == 1 &&
				(
					(player == 0 && ((from < 81 || from > 88) || i < 71)) || 
					(player == 1 && ((from < 31 || from > 38) || i > 48))
				)
			) {
				break;	
			} 
		}
	}
}

/**
* Testet einen GameState auf Schach fuer den Spieler, der am Zug ist.
* Gibt den Typ (ohne Vorzeichen) der Schachfigur zurueck, die den Koenig unter Schach setzt.
*/
char isCheck(struct GameState *gs) {
	short directions[8];
	char player = (*gs).turnNumber % 2;
	char owner = 0;
	char from = 0;
	char found = 0;
	char pieceType = 0;
	char i;
	char moveCounter;

	for (i = innerBoardMin; i <= innerBoardMax; i++) {
		if (((*gs).board[i] == 6 && player == 0) || ((*gs).board[i] == -6 && player == 1)) {
			from = i;
			break;
		}
	}

	if (from == 0) { 
		printError("Fehler bei Bewertung: Kein Koenig gefunden - wurde er regelwidrig geschlagen?!\n"); // DEBUG
		printError("Stellung:\n");
		
		printBoard((*gs).board, 0, 0);
		exit(1);
	}

	directions[0] = -10; directions[1] = -1; directions[2] = +1; directions[3] = +10; 
	directions[4] = -11; directions[5] = -9; directions[6] = +9; directions[7] = +11; 

	for (char d = 0; d < 8 && directions[d] != 0; d++) {
		moveCounter = 0;
		for (i = from + directions[d]; (*gs).board[i] != 7; i += directions[d]) {
			moveCounter++;

			owner = getOwner(gs, i);

			if (owner == player) break; // eigene Figur gefunden
			if (owner == (1 - player)) { // gegnerische Figur gefunden. Pruefen, ob diese das Feld bedroht:
				pieceType = abs((*gs).board[i]);
				switch (pieceType) {
					case 6: // Koenig
						if (moveCounter == 1) return pieceType;
						break;
					case 5: // Dame
						return pieceType;
						break;
					case 4: // Turm
						if (directions[d] == 10 || directions[d] == -10 || directions[d] == 1 || directions[d] == -1) return pieceType;
						break;
					case 3: // Laeufer
						if (directions[d] == 11 || directions[d] == -11 || directions[d] == 9 || directions[d] == -9) return pieceType;
						break;
					case 2: // Springer
						break; // wird nachher seperat betrachtet
					case 1: // Bauer
						if (player == 0) { // weiss
							if ((directions[d] == -11 || directions[d] == -9) && moveCounter == 1) return pieceType;
						} else {
							if ((directions[d] == 11 || directions[d] == 9)  && moveCounter == 1) return pieceType;
						}
						break;
				}
				break; // Suche in diese Richtung abbrechen
			}
		}
	}

	// Springer ueberpruefen
	short sign = 2;
	if (player == 0) sign = -2; // bei weiss haben gegnerische Figuren ein Minus vor ihrem Wert
	if ((*gs).board[from - 21] == sign) return 2;
	if ((*gs).board[from - 19] == sign) return 2;
	if ((*gs).board[from - 12] == sign) return 2;
	if ((*gs).board[from - 8] == sign) return 2;
	if ((*gs).board[from + 8] == sign) return 2;
	if ((*gs).board[from + 12] == sign) return 2;
	if ((*gs).board[from + 19] == sign) return 2;
	if ((*gs).board[from + 21] == sign) return 2;

	return 0;
}

/**
* Generiert alle moeglichen Zuege zu einer Stellung (Board).
* Werden keine Zuege generiert, ist das Spiel in dieser Stellung zu Ende.
* moves muss ein 28 * 16 * 2 grossser Char-Array sein.
*/
void generateMoves(struct GameState *gs, char *moves, short *movesCounter) {
	char player = (*gs).turnNumber % 2;
	char pieceMoves[28];
	short counter = 0;

	for (char i = innerBoardMin; i <= innerBoardMax; i++) {
		if ((*gs).board[i] != 0 && (*gs).board[i] != 7) {
			if (getOwner(gs, i) == player) {
				memset(pieceMoves, 0, 28 * sizeof(pieceMoves[0])); // set all values to 0
				generatePieceMoves(gs, i, pieceMoves);
				for (char j = 0; j < 28 && pieceMoves[j] != 0; j++) {
					moves[counter] = i;
					moves[counter + 1] = pieceMoves[j];
					counter += 2;

					//char c1[] = "??"; char c2[] = "??";
					//convertIndexToCoord(i, c1); convertIndexToCoord(pieceMoves[j], c2);
					//printDev("Move %c from %s to %s.\n", getPieceSymbolAsChar((*gs).board[i]), c1, c2);
				}
			}
		}
	}
	if (counter == 0) {
		printError("Zuggenerator konnte keinen Zug generieren - dass sollte aber immer moeglich sein (da Schach ignoriert wird) !!\n"); // DEBUG
		exit(1);
	}
	*movesCounter = counter;
}

/**
* Bewertungsfunktion. Bewertet eine Stellung (Board) und gibt die Bewertung als int zurueck.
* Die Bewertung erfolgt immer aus Sicht des weissen Spielers relativ zum schwarzen Spieler.
* Haben beide Spieler Material von gleichem Wert auf dem Feld, ist die Bewertung 0.
* Hat weiss besseres Material, ist sie > 0. Hat schwarz besseres Material, ist sie < 0.
* board ist das Board-Array.
*/
int evaluateBoard(short *board) {
	int value = 0;
	for (char i = innerBoardMin; i <= innerBoardMax; i++) {
		switch (board[i]) {
			case 5: // Dame
				value += 900;
				break;
			case 4: // Turm
				value += 485;
				break;
			case 3: // Laeufer
				value += 325;
				break;
			case 2: // Springer
				value += 275;
				break;
			case 1: // Bauer
				value += 100 + 8 - (i / 10);
				break;
			case -5: // Dame
				value -= 900;
				break;
			case -4: // Turm
				value -= 485;
				break;
			case -3: // Laeufer
				value -= 325;
				break;
			case -2: // Springer
				value -= 275;
				break;
			case -1: // Bauer
				value -= 100 - 3 + (i / 10);
				break;
		}
	}
	return value;
}

/**
* Fuehrt einen Zug teilweise durch. Die Validitaet des Zuges wird nicht ueberprueft!!
*/
void doMovePartial(struct GameState *gs, char from, char to) {
	(*gs).board[to] 	= (*gs).board[from];
	(*gs).board[from] 	= 0;

	// Bauer in Dame umwandeln:
	if ((*gs).board[to] == 1  && to >= 21 && to <= 28) (*gs).board[to] = 5; 
	if ((*gs).board[to] == -1 && to >= 91 && to <= 98) (*gs).board[to] = -5; 
}

/**
* Fuehrt den Rest eines Zuges durch. Zuvor muss ein Testzug durchgefuhert worden sein!
* Die Validitaet des Zuges wird nicht ueberprueft!!
*/
void doMoveFinal(struct GameState *gs, char from, char to) {
	if ((*gs).turnNumber > 512) {
		printError("Fehler beim Finalisieren des Zugs: Es sind maximal 512 Zuege moeglich!");
		exit(1);
	}

	history[(*gs).turnNumber * 2] 		= from;
	history[(*gs).turnNumber * 2 + 1] 	= to;

	(*gs).turnNumber++;
}

/**
* Fuehrt einen Move durch (falls moeglich).
* Der GameState wird als Referenz uebergeben. from und to sind Board Indizes.
* Gibt 1 zurueck, wenn der Zug durchgefuhert werden konnte, sonst 0.
*/
char doUserMove(struct GameState *gs, char from, char to) {
	char player = (*gs).turnNumber % 2;
	if (isCurrentPlayerOwner(gs, from)) { // er muss eine Figur auf dem Startfeld besitzen
		if (isCurrentPlayerOwner(gs, to) == 0) { // aber auf dem Zielfeld darf er keine Figur besitzen
			char moves[28];
			generatePieceMoves(gs, from, moves);

			for (char i = 0; i < 28; i++) {
				if (moves[i] == 0) break;
				if (moves[i] == to) { // Zug durchfuehren:
					short pieceFrom = (*gs).board[from];
					short pieceTo = (*gs).board[to];

					doMovePartial(gs, from, to);

					char checkPiece = isCheck(gs);
					if (checkPiece) { // Schach?
						(*gs).board[from] = pieceFrom;
						(*gs).board[to] = pieceTo;
						printError("Zug nicht moeglich - der eigene Koenig kaeme unter Schach durch gegnerischen %c!\n", getPieceSymbolAsChar(checkPiece));
						return 0;
					} else {
						doMoveFinal(gs, from, to);
					}
					
					break;
				}
			}
			return 1;
		} else {
			printError("Der Spieler %i besitzt eine Figur auf dem Zielfeld!\n", player);
			return 0;	
		}
	} else {
		printError("Der Spieler %i besitzt keine Figur auf dem Ausgangsfeld!\n", player);
		return 0;
	}
}

/**
* KI macht einen zufaelligen Zug.
* Gibt 1 zurueck, wenn ein Zug ausgefuehrt wurde, sonst 0.
*/
char aiSimpleMove(struct GameState *gs) {
	char moves[28 * 16 * 2]; // keine Initialiserung!
	short movesCounter;

	generateMoves(gs, moves, &movesCounter);

	short found = 0;
	short move;
	for (move = 0; move < movesCounter; move += 2) {
		short pieceFrom = (*gs).board[moves[move]];
		short pieceTo = (*gs).board[moves[move + 1]];

		doMovePartial(gs, moves[move], moves[move + 1]);

		short check = isCheck(gs);

		(*gs).board[moves[move]] = pieceFrom;
		(*gs).board[moves[move + 1]] = pieceTo;

		if (check) { // Schach?
			moves[move] = 0;
		} else {
			found++;
		}
	}

	if (found == 0) {
		if (isCheck(gs)) {
			printInfo("KI ist Schachmatt!\n");	
		} else {
			printInfo("KI ist Patt gesetzt!\n");
		}
		return 0;
	}

	short r = rand() % found;
	found = 0;
	for (move = 0; move < movesCounter; move += 2) {
		if (moves[move] != 0) {
			if (found == r) {
				break;
			}
			found++;
		}
	}

	char c1[] = "??"; char c2[] = "??";
	convertIndexToCoord(moves[move], c1); convertIndexToCoord(moves[move + 1], c2);
	if ((*gs).board[moves[move + 1]] == 0) {
		printInfo("KI zieht mit %c von %s nach %s.\n", getPieceSymbolAsChar((*gs).board[moves[move]]), c1, c2);	
	} else {
		printInfo("KI zieht mit %c von %s nach %s und schlaegt %c.\n", getPieceSymbolAsChar((*gs).board[moves[move]]), c1, c2, getPieceSymbolAsChar((*gs).board[moves[move + 1]]));	
	}

	doMovePartial(gs, moves[move], moves[move + 1]);
	doMoveFinal(gs, moves[move], moves[move + 1]);

	return 1;
}

/**
* Ermittelt den besten Zug aus der uebergebenen Stellung.
*
* from = Poistionsindex des Start des Zugs, to = Ziel
* bestEval = die beste ermittelte Bewertung
* bestEvalAdding = zusätzliche Bewertung die genutzt werden kann, um gleichbewertete Zuege zu priorisieren
* depth = die aktuelle Zugtiefe
*/
void aiDeepSearch(struct GameState *gameState, char *from, char *to, int *bestEval, int *bestEvalAdding, char depth) {
	short movesCounter;
	char moves[28 * 16 * 2]; // keine Initialiserung!
	struct GameState gs;

	if (depth == 0 && autoMode) printBoard((*gameState).board, 0, 0);

	// Erzeuge lokale Kopie des GameState der beliebig veraendert werden kann:
	copyGameState(gameState, &gs); 

	// Erzeuge Zuege:
	generateMoves(&gs, moves, &movesCounter);

	// Zunaechst alle Zuege auf Gueltigkeit (Schach) untersuchen.
	// Ungueltige Zuege aussortieren.
	short move;
	short found = 0; // gefundene gueltige Zuege
	int eval = 0;
	int evalAdding = 0;
	*bestEval = -valueCheckMate; // valueCheckMate bedeutet: Der bewertende Spieler ist Schachmatt/Patt
	//*bestEvalAdding = 0;
	for (move = 0; move < movesCounter; move += 2) {
		short pieceFrom = gs.board[moves[move]];
		short pieceTo = gs.board[moves[move + 1]];

		// Testzug machen:
		//printDev("Betrachte Zug %i  von %i (%i) nach %i (%i).\n", move, moves[move], pieceFrom, moves[move+1], pieceTo);
		doMovePartial(&gs, moves[move], moves[move + 1]);

		if (isCheck(&gs)) { // Eigener Koenig unter Schach?
			//printDev("Ausortierter Zug %i von %i nach %i da Schachgebot von: %i\n", move, moves[move], moves[move+1], isCheck(&gs));
			// derzeit: nichts tun
		} else {
			found++;

			if (depth >= maxDepth) {
				eval = evaluateBoard(gs.board);
				evalAdding = 0;
				if (gs.turnNumber % 2 == 1) eval *= -1; // ist der Spieler, der am Zug ist, schwarz, die Bewertung umdrehen (da sie immer aus Sicht von Weiss erfolgt)
				//printDev("Betrachte Zug %i von %i nach %i. Bewertung: %i\n", move, moves[move], moves[move+1], eval);
			} else {
				gs.turnNumber++; // Zugnummer erhoehen damit der naechste Spieler am Zug ist
				char rfrom = 0; // der Wert wird nicht benoetigt
				char rto = 0;
				aiDeepSearch(&gs, &rfrom, &rto, &eval, &evalAdding, depth + 1);		

				eval *= -1; // NegaMax-Algorithmus: Die Bewertung umdrehen. Aus der Sicht des Spielers, der in dieser Funktion gerade berechnet wird, ist der andere Spieler der Gegner und ein gutes Ergebnis von ihm fuer ihn selber schlecht.
				evalAdding *= -1;
				gs.turnNumber--;

				if (debugMode && depth == 0) {
					if (pieceTo == 0) {
						printDev("[%3.0f%%] Deep Search: Zug mit %c von %i nach %i. Bewertung: %6i\n", ((double) move / (double) movesCounter) * 100, getPieceSymbolAsChar(pieceFrom), moves[move], moves[move+1], eval);
					} else {
						printDev("[%3.0f%%] Deep Search: Zug mit %c von %i nach %i und schlaegt %c. Bewertung: %6i\n", ((double) move / (double) movesCounter) * 100, getPieceSymbolAsChar(pieceFrom), moves[move], moves[move+1], getPieceSymbolAsChar(pieceTo), eval);
					}
				}
			}

			// Schlagzuege unterhalb der tiefsten Ebene (Evaluationsebene) werden minimal vorteilhaft bewertet.
			// Der Grund dafuer ist, dass es sonst zu Situationen kommen kann, in denen die KI zwar schlagen kann,
			// was auch vorteilhat waere im Sinne der Bewertung, es aber nicht tut.
			// Warum nicht? Weil in diesen Situationen die KI sieht, dass die eine Figur sicher schlagen kann, und zwar
			// innerhalb ihres ganzen Horizonts. Es ist dann aus Sicht der KI egal, ob sie die Figur diese oder naechste Runde schlaegt.
			// Aber es kann sein, dass die KI die Situation in der naechsten Runde wiede so bewertet und das Schlagen
			// immer auf einen zukuenftigen Zeitpunkt verschiebt. Deshalb wird das schlagen minimal positiv bewertet.
			if (pieceTo != 0) evalAdding += pow(2, maxDepth + 1 - depth);

			if (eval > *bestEval || (eval == *bestEval && evalAdding > *bestEvalAdding)) {
				if (debugMode && depth == 0) printDev("Dieser Zug hat nun die beste Bewertung.\n");
				*bestEval = eval;
				*bestEvalAdding = evalAdding;
				*from = moves[move];
				*to = moves[move + 1];
			}
		}

		// Zug rueckgaengig machen:
		gs.board[moves[move]] = pieceFrom;
		gs.board[moves[move + 1]] = pieceTo;

		//if (eval > valueCheckMate) {
		//	printDev("Checkmate-Cutoff!\n");
		//	break;
		//} 
	}

	// Wenn ich keine gueltigen Zuege habe, bin ich schachmatt oder patt.
	// Daher auf Schach pruefen: Besteht kein Schach, dann bin ich patt.
	// In diesem Fall gebe ich anstatt -valueCheckMate (=Schachmatt) als Bewertung 0 zurueck.
	// Damit weiss der Spieler, der vor mir am Zug ist, zwar nicht genau, dass es ein Patt ist.
	// Das ist aber irrelevant.
	if (found == 0) {
		if (isCheck(&gs) == 0) *bestEval = 0;
	}
}

/**
* Gibt 1 zurueck, wenn ein Zug ausgefuehrt wurde, sonst 0.
*/
char aiDeepMove(struct GameState *gameState) {
	char from 			= 0;
	char to 			= 0;
	char coordFrom[]	= "??";
	char coordTo[]		= "??";
	int eval 			= 0;
	int bestEvalAdding 	= 0;

	if (loadOpeningBookMove(gameState, coordFrom, coordTo)) {
		from = convertCoordToIndex(coordFrom);
		to = convertCoordToIndex(coordTo);
		printf("Eroeffnungsbuch: ");
	} else {
		timeStamp = time(NULL);
		aiDeepSearch(gameState, &from, &to, &eval, &bestEvalAdding, 0);
		if (debugMode) printDev("Calculation time: %i\n", (int) time(NULL) - timeStamp);
	}

	if (from != 0) {
		convertIndexToCoord(from, coordFrom); 
		convertIndexToCoord(to, coordTo);

		if ((*gameState).board[to] == 0) {
			printInfo("KI zieht mit %c von %s nach %s.\n", getPieceSymbolAsChar((*gameState).board[from]), coordFrom, coordTo);	
		} else {
			printInfo("KI zieht mit %c von %s nach %s und schlaegt %c.\n", getPieceSymbolAsChar((*gameState).board[from]), coordFrom, coordTo, getPieceSymbolAsChar((*gameState).board[to]));	
		}
		
		doMovePartial(gameState, from, to);
		doMoveFinal(gameState, from, to);

		return 1;
	} else {
		printf("Blah 3!\n"); exit(1);
		if (eval <= -valueCheckMate) {
			if (isCheck(gameState)) {
				printInfo("KI ist Schachmatt!\n");		
			} else {
				printInfo("KI gibt auf: Schachmatt in wenigen Zuegen!\n");	
			}
		} else {
			printError("Fehler: Keine Zuege gefunden, aber nicht Schachmatt!"); // DEBUG
		}
		
		return 0;
	}
}

/**
* Wrapper-Funktion: Ruft eine KI auf.
* Gibt 1 zurueck, wenn ein Zug ausgefuehrt wurde, sonst 0.
*/
char aiMove(struct GameState *gs, char ai) {
	int realMaxDef = maxDepth;
	int res;

	if (ai == 0) {
		if ((*gs).turnNumber % 2 == 0) { // Spieler 0 (weiss) ist am Zug
			ai = (*gs).ai0;
		} else {
			ai = (*gs).ai1;
		}
	}

	switch (ai) {
		case 1: // Fake-AI. Macht nichts. (Was regelwidrig ist - eigentlich muss ein Spieler ziehen.)
			(*gs).turnNumber++;
			return 1; // immer 1 zurueckgeben (semantisch gesehen ist das natuerlich widersinnig)
			break;
		case 2:
			return aiSimpleMove(gs);
			break;
		case 3:
			maxDepth = 3;
			res = aiDeepMove(gs);
			maxDepth = realMaxDef;
			return res;
			break;
		case 4:
			return aiDeepMove(gs);
			break;
		default:
			printError("Unbekannte AI-ID: %i\n", ai);
			return 0;
			break;
	}
}

/**
* Hauptmethode
*/
int main(int argc, char *argv[]) {
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	char o;
	char boardMode = 0; // 0/1
	char fullScreen = 1;
	char from[] = "??";
	char to[] = "??";
	char filename[255];
	char moves[28 * 16 * 2]; // keine Initialiserung!
	short movesCounter = 0;
	short board[120];
	int eval;
	int okay;
	int i;

	struct GameState gameState;
	struct GameState gameStateCopy;
	createStartBoard(board);
	setGameState(&gameState, board, 0, 3, 4);

	srand((unsigned int) time(NULL));  // Zufallsgenerator initialisieren

	if (fullScreen) system("cls");

	do {
		printf("\nSpieler: %i, Halbzug: %i\n", gameState.turnNumber % 2, gameState.turnNumber);
		printBoard(gameState.board, 0, boardMode);

		COLOR(128);
		printf("\n> Kommando:");
		COLOR(COLOR_DEFAULT);
		printf(" ");
        scanf("%c", &o); // Auf Char scannen
        fflush(stdin);

        if (fullScreen && o != 'm') system("cls");

        switch (o) {
        	case 'x':
        		// "magic" move - alles ist moeglich. Auch, Fehler zu produzieren.
        	case 'm':
        		printf("Zug von: ");
        		scanf("%s", from);
        		fflush(stdin);

        		printf("Zug nach: ");
        		scanf("%s", to);
        		fflush(stdin);

        		if (strlen(from) != 2 || strlen(to) != 2) {
        			printError("Ungueltige Koordinaten!\n");
        		} else {
        			autoSave(&gameState);
        			if (o == 'x') {
						doMovePartial(&gameState, convertCoordToIndex(from), convertCoordToIndex(to));
						doMoveFinal(&gameState, convertCoordToIndex(from), convertCoordToIndex(to));
        			} else {
        				if (doUserMove(&gameState, convertCoordToIndex(from), convertCoordToIndex(to))) system("cls");
        			}
        		}
        		break;
        	case 'n':
        		gameState.turnNumber--;
				printInfo("Zug zurueck.\n");
        		break;
        	case 'a':
        		do {
        			autoSave(&gameState);
        			okay = aiMove(&gameState, 0);
        		} while (autoMode && okay && ((gameState.turnNumber % 2 == 0 && gameState.ai0) || (gameState.turnNumber % 2 == 1 && gameState.ai1)));
        		break;
        	case 'c':
        		printInfo("Schach: %i\n", isCheck(&gameState));
        		break;
        	case 'h':
        		printHistory();
        		break;
        	case 'g':
        		generateMoves(&gameState, moves, &movesCounter);
        		printInfo("%i moegliche Zuege (ohne Beruecksichtigung von Schach).\n", movesCounter / 2);
				for (i = 0; i < movesCounter; i += 2) {
					printf("Zug mit %i von %i nach %i.\n", gameState.board[moves[i]], moves[i], moves[i + 1]);
				}
        		break;
        	case 'v':
        		eval = evaluateBoard(gameState.board);
        		printInfo("Evaluation (aus Sicht von weiss): %i\n", eval);
        		break;
        	case 't':
        		copyGameState(&gameState, &gameStateCopy);
    			okay = aiMove(&gameStateCopy, 3);
        		break;
        	case 'o':
        		okay = loadOpeningBookMove(&gameState, from, to);
        		if (okay) {
        			printInfo("Zugvorschlag aus dem Eroeffnungsbuch: mit %c von %s nach %s", getPieceSymbolAsChar(gameState.board[convertCoordToIndex(from)]), from, to);
        		} else {
        			printInfo("Das Eroeffnungsbuch enthaelt keinen passenden Zug!");
        		}
        		break;
        	case 's':
        		saveGame(&gameState, "quicksave", 1);
        		break;
        	case 'r':
				loadGame(&gameState, "quicksave");
        		break;
        	case 'l':
        		system("dir savegames\\*.sav /B");
				printf("\nLade Datei (Endung nicht angeben):\n");
				scanf("%s", filename);
				fflush(stdin);

				loadGame(&gameState, filename);
        		break;
        	case 'u':
        		loadAutoSave(&gameState);
        		break;
        	case 'b':
        		boardMode = (1 - boardMode);
        		printInfo("Brettdarstellung gewechselt auf: %i\n", boardMode);
        		break;
        	case 'd':
        		debugMode = (1 - debugMode);
        		printInfo("Debugmodus gewechselt auf: %i\n", debugMode);
        		break;
        	case '?':
        		printf("m (move)\tEinen Zug durchfuehren.\n");
        		printf("n (next)\tDen Spieler wechseln (ohne Zug, regelwidrig!)\n");
        		printf("a (ai)\t\tKI einen Zug durchfuehren lassen.\n");
        		printf("h (history)\tDen Spielverlauf anzeigen.\n");
        		printf("c (check)\tStellung auf Schach pruefen.\n");
        		printf("g (generate)\tMoegliche Zuege anzeigen lassen.\n");
        		printf("v (value)\tBewertung der Stellung anzeigen lassen.\n");
        		printf("t (tip)\t\tDie KI einen Zug-Tip anzeigen lassen.\n");
        		printf("s (save)\tQuicksave-Spielstand anlegen.\n");
        		printf("r (reload)\tQuicksave-Spielstand laden.\n");
        		printf("l (load)\tSpielstand laden (Dateiname angeben).\n");
        		printf("u (undo)\tLetzten Zug zuruecknehmen.\n");
        		printf("b (board)\tBrettdarstellung wechseln (fuer Debuggging).\n");
        		printf("d (open)\tDebugausgaben aktivieren/deaktivieren.\n");
        		printf("? (help)\tDiese Hilfe zu den Kommandos anzeigen lassen.\n");
        		printf("e (exit)\tDas Programm beenden.\n");
        		break;
        	case 'e':
        		// do nothing
        		break;
        	case '\n':
        		// do nothing
        		break;
        	default:
        		printError("Unbekannter Operator: %c\n", o);
        		break;
        }
        fflush(stdin);
	} while (o != 'e');

	return 0;
}
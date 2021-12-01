#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "Game.h"
#include "Utils.h"

void Reset(Game *g) {
	for (int i = 0; i < 26; i++)
		g->Position[i] = 0;
	g->WhiteHome = 0;
	g->BlackHome = 0;
	g->CurrentPlayer = Black;

	g->Dice[0] = 0;
	g->Dice[1] = 0;
}

int CountAllCheckers(PlayerSide side, Game* game) {
	int count = 0;
	for (int i = 0; i < 26; i++)
	{
		if (game->Position[i] & side)
			count += CheckerCount(game->Position[i]);
	}
	if (side == Black)
		count += game->BlackHome;
	if (side == White)
		count += game->WhiteHome;
	return count;
}

void StartPosition(Game *g) {
	srand(time(NULL));   // Initialization, should only be called once.

	Reset(g);

	g->Position[1] = 2 | Black;
	g->Position[6] = 5 | White;
	g->Position[8] = 3 | White;
	g->Position[12] = 5 | Black;
	g->Position[13] = 5 | White;
	g->Position[17] = 3 | Black;
	g->Position[19] = 5 | Black;
	g->Position[24] = 2 | White;
	SetPointsLeft(g);
}

void RollDice(Game* g) {
	g->Dice[0] = LlrandShift() % 6 + 1;
	g->Dice[1] = LlrandShift() % 6 + 1;
}

bool ToHome(Move move) {
	return move.color == Black && move.to == 25 || move.color == White && move.to == 0;
}

bool DoMove(Move move, Game *g) {
	ASSERT_DBG(move.from >= 0 && move.from <= 25 && move.to >= 0 && move.to <= 25 && (move.color == Black || move.color == White));
	/*char tmp[100];
	WriteGameString(tmp, g);*/

	ushort to = move.to;
	ushort from = move.from;
	bool toHome = ToHome(move);
	PlayerSide other = OtherColor(move.color);
	bool hit = CheckerCount(g->Position[to]) == 1 && (g->Position[to] & other)  && !toHome;
	if (hit)
		g->Position[to] = 0;

	if (toHome) {
		move.color == Black ? g->BlackHome++ : g->WhiteHome++;
		ASSERT_DBG(g->BlackHome <= 15 && g->WhiteHome <= 15);
	}
	else {
		g->Position[to]++;
		g->Position[to] |= move.color;
		ASSERT_DBG(CheckerCount(g->Position[to]) <= 15);
	}

	g->Position[from]--;
	if (CheckerCount(g->Position[from]) == 0)
		g->Position[from] = 0;

	if (move.color == Black)
	{
		g->BlackLeft -= (move.to - move.from);
		ASSERT_DBG(g->BlackLeft >= 0);
		if (hit)
		{
			g->WhiteLeft += (25 - move.to);
			ASSERT_DBG(g->WhiteLeft > 0);
			g->Position[25] ++;
			g->Position[25] |= White;
			ASSERT_DBG(CheckerCount(g->Position[25]) <= 15 && CheckerCount(g->Position[25]) >= 0);
		}
	}
	if (move.color == White)
	{
		g->WhiteLeft -= (move.from - move.to);
		ASSERT_DBG(g->WhiteLeft >= 0);
		if (hit)
		{
			g->BlackLeft += move.to; // += 25 - (25 - move.to);
			ASSERT_DBG(g->BlackLeft >= 0);
			g->Position[0] ++;
			g->Position[0] |= Black;
			ASSERT_DBG(CheckerCount(g->Position[0]) <= 15 && CheckerCount(g->Position[0]) >= 0);
		}
	}

	//ASSERT_DBG(CountAllCheckers(Black, g) == 15 && CountAllCheckers(White, g) == 15);

	return hit;
}

void UndoMove(Move move, bool hit, Game *g) {
	ASSERT_DBG(move.from >= 0 && move.from <= 25 && move.to >= 0 && move.to <= 25 && (move.color == Black || move.color == White));
	
	//use for bug tracking
	/*char* tmp[100];
	WriteGameString(tmp, g);*/

	ushort to = move.to;
	ushort from = move.from;
	bool fromHome = ToHome(move);

	g->Position[from]++;
	g->Position[from] |= move.color;

	if (fromHome) {
		move.color == Black ? g->BlackHome-- : g->WhiteHome--;
		ASSERT_DBG(g->BlackHome >= 0 && g->WhiteHome >= 0);
	}
	else {
		g->Position[to]--;
		if (CheckerCount(g->Position[to]) == 0)
			g->Position[to] = 0;
		ASSERT_DBG(g->Position[to] >= 0);

		if (hit)
		{
			g->Position[to] = 1 | OtherColor(move.color);
		}
	}

	if (move.color == Black)
	{
		g->BlackLeft += (move.to - move.from);
		ASSERT_DBG(g->BlackLeft >= 0);

		if (hit)
		{
			g->WhiteLeft -= (25 - move.to);
			ASSERT_DBG(g->WhiteLeft >= 0);
			g->Position[25]--;
			if (CheckerCount(g->Position[25]) == 0)
				g->Position[25] = 0;
			ASSERT_DBG(CheckerCount(g->Position[25]) >= 0);
		}
	}
	if (move.color == White)
	{
		g->WhiteLeft += (move.from - move.to);
		ASSERT_DBG(g->WhiteLeft >= 0);

		if (hit)
		{
			g->BlackLeft -= move.to; // -= 25 - (25 - move.to);
			ASSERT_DBG(g->BlackLeft >= 0);
			g->Position[0]--;
			if (CheckerCount(g->Position[0]) == 0)
				g->Position[0] = 0;
			ASSERT_DBG(CheckerCount(g->Position[0]) >= 0);
		}
	}
	// ASSERT_DBG(CountAllCheckers(Black, g) == 15 && CountAllCheckers(White, g) == 15);
}

bool IsBlockedFor(ushort pos, ushort color, Game *g) {
	if (pos > 25)
		return false;


	return (g->Position[pos] & OtherColor(color)) && (CheckerCount(g->Position[pos]) >= 2);
}

bool IsBlackBearingOff(ushort* lastCheckerPos, Game *g) {
	for (ushort i = 0; i <= 24; i++)
	{
		if ((g->Position[i] & Black) && CheckerCount(g->Position[i]) > 0)
		{
			*lastCheckerPos = i;
			return i >= 19;
		}
	}
	return true;
}

bool IsWhiteBearingOff(ushort* lastCheckerPos, Game *g) {
	for (ushort i = 25; i >= 1; i--)
	{
		if ((g->Position[i] & White) && CheckerCount(g->Position[i]) > 0)
		{
			*lastCheckerPos = i;
			return i <= 6;
		}
	}
	return true;
}

void CreateBlackMoveSets(int diceIdx, int diceCount, int* maxSetLength, Game *g) {
	ushort start = 0;
	ushort lastCheckerPos;
	bool bearingOff = IsBlackBearingOff(&lastCheckerPos, g);
	if (bearingOff)
		start = 19;

	ushort toIndex = CheckerCount(g->Position[0]) > 0 ? 1 : 25;

	for (ushort i = start; i < toIndex; i++)
	{
		if (!(g->Position[i] & Black))
			continue;
		int diceVal = diceIdx > 1 ? g->Dice[0] : g->Dice[diceIdx];
		int toPos = i + diceVal;

		// När man bär av, får man använda tärningar med för hög summa,
		// men bara på den checker längst från home.
		if (IsBlockedFor(toPos, Black, g))
			continue;

		if (bearingOff) {
			if (toPos > 25 && i != lastCheckerPos)
				continue;

			if (toPos > 25 && i == lastCheckerPos)
				toPos = 25;
		}
		else { //Not bearing off
			if (toPos > 24)
				continue;
		}

		// Atleast one move set is created.
		if (g->MoveSetsCount == 0)
			g->MoveSetsCount = 1;
		ushort seqIdx = g->MoveSetsCount - 1;
		Move* move = &g->PossibleMoveSets[seqIdx].Moves[diceIdx];
		if (move->color != 0) {
			// A move is already generated for this dice in this sequence. Branch off a new sequence.
			int copyCount = diceIdx;// SetLengths[seqIdx] - 1;
			g->SetLengths[seqIdx + 1] = copyCount;
			if (copyCount > 0)
				memcpy(&g->PossibleMoveSets[seqIdx + 1].Moves[0], &g->PossibleMoveSets[seqIdx].Moves[0], copyCount * sizeof(Move));
			move = &g->PossibleMoveSets[seqIdx + 1].Moves[diceIdx];
			g->MoveSetsCount++;
			ASSERT_DBG(g->MoveSetsCount < MAX_SETS_LENGTH);
			seqIdx++;
		}

		move->from = i;
		move->to = toPos;
		move->color = Black;

		g->SetLengths[seqIdx]++;
		*maxSetLength = max(*maxSetLength, g->SetLengths[seqIdx]);

		//TODO: Maybe omit identical sequences, hashing?
		if (diceIdx < diceCount - 1) {
			int hit = DoMove(*move, g);
			CreateBlackMoveSets(diceIdx + 1, diceCount, maxSetLength, g);
			UndoMove(*move, hit, g);
		}
	}
}

void CreateWhiteMoveSets(int diceIdx, int diceCount, int* maxSetLength, Game *g) {
	int start = 25;
	ushort lastCheckerPos;
	bool bearingOff = IsWhiteBearingOff(&lastCheckerPos, g);
	if (bearingOff)
		start = 6;

	int toIndex = CheckerCount(g->Position[25]) > 0 ? 25 : 0;

	for (int i = start; i >= toIndex; i--)
	{
		if (!(g->Position[i] & White))
			continue;
		int diceVal = diceIdx > 1 ? g->Dice[0] : g->Dice[diceIdx];
		int toPos = i - diceVal;

		// När man bär av, får man använda tärningar med för hög summa
		// Men bara på den checker längst från home.
		if (IsBlockedFor(toPos, White, g))
			continue;

		if (bearingOff) {
			if (toPos < 0 && i != lastCheckerPos)
				continue;

			if (toPos < 0 && i == lastCheckerPos)
				toPos = 0;
		}
		else { //Not bearing off
			if (toPos < 1)
				continue;
		}

		// Atleast one move set is created.
		if (g->MoveSetsCount == 0)
			g->MoveSetsCount = 1;
		ushort seqIdx = g->MoveSetsCount - 1;
		Move* move = &g->PossibleMoveSets[seqIdx].Moves[diceIdx];
		if (move->color != 0) {
			// A move is already generated for this dice in this sequence. Branch off a new sequence.
			int copyCount = diceIdx;
			g->SetLengths[seqIdx + 1] = copyCount;
			if (copyCount > 0)
				memcpy(&g->PossibleMoveSets[seqIdx + 1].Moves[0], &g->PossibleMoveSets[seqIdx].Moves[0], copyCount * sizeof(Move));
			move = &g->PossibleMoveSets[seqIdx + 1].Moves[diceIdx];
			g->MoveSetsCount++;
			ASSERT_DBG(g->MoveSetsCount < MAX_SETS_LENGTH);
			seqIdx++;
		}

		move->from = i;
		move->to = toPos;
		move->color = White;

		g->SetLengths[seqIdx]++;
		*maxSetLength = max(*maxSetLength, g->SetLengths[seqIdx]);

		//TODO: Maybe omit identical sequences, hashing?
		if (diceIdx < diceCount - 1) {
			int hit = DoMove(*move, g);
			CreateWhiteMoveSets(diceIdx + 1, diceCount, maxSetLength, g);
			UndoMove(*move, hit, g);
		}
	}
}

void ReverseDice(Game *g) {
	short temp = g->Dice[0];
	g->Dice[0] = g->Dice[1];
	g->Dice[1] = temp;
}

void RemoveShorterSets(int maxSetLength, Game *g) {
	bool modified = false;
	int realCount = g->MoveSetsCount;
	do
	{
		modified = false;
		for (int i = 0; i < realCount; i++)
		{
			if (g->SetLengths[i] < maxSetLength)
			{
				memcpy(&g->PossibleMoveSets[i], &g->PossibleMoveSets[i + 1], (MAX_SETS_LENGTH - i) * 4 * sizeof(Move));
				memcpy(&g->SetLengths[i], &g->SetLengths[i + 1], (MAX_SETS_LENGTH - i) * sizeof(ushort));
				modified = true;
				realCount--;
				break;
			}
		}
	} while (modified);
	g->MoveSetsCount = realCount;
}

void CreateMoves(Game* g) {
	for (int i = 0; i < MAX_SETS_LENGTH; i++)
	{
		g->SetLengths[i] = 0;
		for (int j = 0; j < 4; j++)
		{
			Move move;
			move.from = 0;
			move.to = 0;
			move.color = 0;
			g->PossibleMoveSets[i].Moves[j] = move;
		}
	}

	g->MoveSetsCount = 0;
	// Largest Dice first
	if (g->Dice[1] > g->Dice[0]) {
		ReverseDice(g);
	}

	int diceCount = g->Dice[0] == g->Dice[1] ? 4 : 2;
	int maxSetLength = 0;
	for (size_t i = 0; i < 2; i++)
	{
		maxSetLength = 0;
		if (g->CurrentPlayer & Black)
			CreateBlackMoveSets(0, diceCount, &maxSetLength, g);
		else
			CreateWhiteMoveSets(0, diceCount, &maxSetLength, g);

		//If no moves are found and dicecount == 2 reverse dice order and try again.
		if (g->MoveSetsCount == 0 && diceCount == 2) {
			ReverseDice(g);
		}
		else {
			break;
		}
	}

	RemoveShorterSets(maxSetLength, g);
}

void WriteGameString(char* s, Game *g) {

	//todo: handle two digit numbers.
	
	int idx = 0;
	for (size_t i = 0; i < 26; i++)
	{
		if (g->Position[i] > 0) {
			ushort black = g->Position[i] & Black;
			if (black) {
				s[idx++] = 'b';
			}
			else {
				s[idx++] = 'w';
			}
		}

		char *n = malloc(3);
		int length = sprintf_s(n, 4,"%d", CheckerCount(g->Position[i]));
		memcpy(&s[idx], n, length);
		idx += length;
		//s[idx++] = '0' + CheckerCount(g->Position[i]);
		s[idx++] = ' ';
	}
	s[idx++] = '0' + g->WhiteHome;
	s[idx++] = ' ';
	s[idx++] = '0' + g->BlackHome;
	s[idx] = '\0';
}

void SetPointsLeft(Game* g) {
	ushort blackLeft = 0;
	ushort whiteLeft = 0;
	for (int i = 0; i < 25; i++)
	{
		if (g->Position[i] & Black)
			blackLeft += (25 - i) * CheckerCount(g->Position[i]);
		if (g->Position[i] & White)
			whiteLeft += i * CheckerCount(g->Position[i]);
	}
	g->BlackLeft = blackLeft;
	g->WhiteLeft = whiteLeft;
}

void ReadGameString(char* s, Game *g) {
	Reset(g);
	// 28 tokens
	// BlackBar, pos 1 - 24, WhiteBar, BlackHome, WhiteHome
	// "0 b2 0 0 0 0 w5 0 w3 0 0 0 b5 w5 0 0 0 b3 0 b5 0 0 0 0 w2 0 0 0"
		
	size_t len = strlen(s);
	char copy[100];
	memcpy(copy, s, len + 1);

	char* context = NULL;
	char* token = strtok_s(copy, " ", &context);

	for (size_t i = 0; i < 26; i++)
	{
		char n[100];
		if (StartsWith(token, "b")) {
			SubString(token, n, 2, 3);
			g->Position[i] = atoi(n) | Black;
		}
		else if (StartsWith(token, "w")) {
			SubString(token, n, 2, 3);
			g->Position[i] = atoi(n) | White;
		}

		token = strtok_s(NULL, " ", &context);
		VERIFY(token != NULL);
	}
	g->WhiteHome = atoi(token);
	token = strtok_s(NULL, " ", &context);
	g->BlackHome = atoi(token);
	SetPointsLeft(&G);
}

void PrintGame(Game* g) {
	// ╔═══╤═══╤═══╤═══╤═══╤═══╤═══╤═══╗
	char top[] = { 201,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,203,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,209,205,205,205,187,0 };
	// ╟───┼───┼───┼───┼───┼───┼───┼───╢
	char rowLine[] = { 199,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,215,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,197,196,196,196,182,0 };
	// ╚═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╝
	char lastLine[] = { 200,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 202,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 207,205,205,205, 188, 0 };
	// ║

	// https://en.wikipedia.org/wiki/Code_page_437
	char vBorder = 186; //║
	char vLine = 179; // │
	char smile[] = { 32 , 2, 32, 0 }; // ☻
	system("cls");
	printf(" %s\n", top);
	for (int y = 0; y < 10; y++)
	{
		printf(" %c", vBorder);
		for (int x = 0; x < 12; x++)
		{
			PlayerSide color = g->Position[12 - x] & 48;
			int count = CheckerCount(g->Position[12 - x]);
			if (count > y)
				color == White ? PrintYellow(smile) : PrintBlue(smile);
			else
				printf("   ");
			if (x < 11)
				x == 5 ? printf("%c", vBorder) : printf("%c", vLine);
		}
		printf("%c", vBorder);
		printf("\n");
	}

	printf(" %s\n", rowLine);


	for (int y = 0; y < 10; y++)
	{
		printf(" %c", vBorder);
		for (int x = 13; x <= 24; x++)
		{
			PlayerSide color = g->Position[x] & 48;
			int count = CheckerCount(g->Position[x]);
			if (count >= 10 - y)
				color == White ? PrintYellow(smile) : PrintBlue(smile);
			else
				printf("   ");
			if (x < 24)
				x == 18 ? printf("%c", vBorder) : printf("%c", vLine);
		}
		printf("%c", vBorder);
		printf("\n");
	}
	printf(" %s\n", lastLine);

	printf("Black left: %d   Bar: %d   Home: %d\n", g->BlackLeft, CheckerCount(g->Position[0]), g->BlackHome);
	printf("White left: %d   Bar: %d   Home: %d\n", g->WhiteLeft, CheckerCount(g->Position[25]), g->WhiteHome);
	if (g->CurrentPlayer == Black)
		PrintBlue("Blacks turn. ");
	else if (g->CurrentPlayer == White)
		PrintYellow("Whites turn. ");
	printf("Dice: %d, %d\n", g->Dice[0], g->Dice[1]);
	char gs[500];
	WriteGameString(gs, g);
	printf("%s\n", gs);
}



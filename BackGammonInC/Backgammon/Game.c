#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "Game.h"
#include "Utils.h"
#include "Hash.h"

void Reset(int g) {
	for (int i = 0; i < 26; i++)
		Games[g].Position[i] = 0;
	Games[g].WhiteHome = 0;
	Games[g].BlackHome = 0;
	Games[g].CurrentPlayer = Black;

	Games[g].Dice[0] = 0;
	Games[g].Dice[1] = 0;
}

int CountAllCheckers(PlayerSide side, int g) {
	int count = 0;
	for (int i = 0; i < 26; i++)
	{
		if (Games[g].Position[i] & side)
			count += CheckerCount(Games[g].Position[i]);
	}
	if (side == Black)
		count += Games[g].BlackHome;
	if (side == White)
		count += Games[g].WhiteHome;
	return count;
}

void StartPosition(int g) {

	Reset(g);

	Games[g].Position[1] = 2 | Black;
	Games[g].Position[6] = 5 | White;
	Games[g].Position[8] = 3 | White;
	Games[g].Position[12] = 5 | Black;
	Games[g].Position[13] = 5 | White;
	Games[g].Position[17] = 3 | Black;
	Games[g].Position[19] = 5 | Black;
	Games[g].Position[24] = 2 | White;
	Games[g].BlackLeft = 167;
	Games[g].WhiteLeft = 167;
	
}

void RollDice(int g) {
	Games[g].Dice[1] = LlrandShift() % 6 + 1;
	Games[g].Dice[0] = LlrandShift() % 6 + 1;
}

bool ToHome(Move move) {
	return move.color == Black && move.to == 25 || move.color == White && move.to == 0;
}

bool DoMove(Move move, int g) {
	ASSERT_DBG(move.from >= 0 && move.from <= 25 && move.to >= 0 && move.to <= 25 && (move.color == Black || move.color == White));
	/*char tmp[100];
	WriteGameString(tmp, g);*/

	ushort to = move.to;
	ushort from = move.from;
	bool toHome = ToHome(move);
	PlayerSide other = OtherColor(move.color);
	bool hit = CheckerCount(Games[g].Position[to]) == 1 && (Games[g].Position[to] & other) && !toHome;
	if (hit)
		Games[g].Position[to] = 0;

	if (toHome) {
		move.color == Black ? Games[g].BlackHome++ : Games[g].WhiteHome++;
		ASSERT_DBG(Games[g].BlackHome <= 15 && Games[g].WhiteHome <= 15);
	}
	else {
		Games[g].Position[to]++;
		Games[g].Position[to] |= move.color;
		ASSERT_DBG(CheckerCount(Games[g].Position[to]) <= 15);
	}

	Games[g].Position[from]--;
	if (CheckerCount(Games[g].Position[from]) == 0)
		Games[g].Position[from] = 0;

	if (move.color == Black)
	{
		Games[g].BlackLeft -= (move.to - move.from);
		ASSERT_DBG(Games[g].BlackLeft >= 0);
		if (hit)
		{
			Games[g].WhiteLeft += (25 - move.to);
			ASSERT_DBG(Games[g].WhiteLeft > 0);
			Games[g].Position[25] ++;
			Games[g].Position[25] |= White;
			ASSERT_DBG(CheckerCount(Games[g].Position[25]) <= 15 && CheckerCount(Games[g].Position[25]) >= 0);
		}
	}
	if (move.color == White)
	{
		Games[g].WhiteLeft -= (move.from - move.to);
		ASSERT_DBG(Games[g].WhiteLeft >= 0);
		if (hit)
		{
			Games[g].BlackLeft += move.to; // += 25 - (25 - move.to);
			ASSERT_DBG(Games[g].BlackLeft >= 0);
			Games[g].Position[0] ++;
			Games[g].Position[0] |= Black;
			ASSERT_DBG(CheckerCount(Games[g].Position[0]) <= 15 && CheckerCount(Games[g].Position[0]) >= 0);
		}
	}

	ASSERT_DBG(CountAllCheckers(Black, g) == 15 && CountAllCheckers(White, g) == 15);

	return hit;
}

void UndoMove(Move move, bool hit, int g) {
	ASSERT_DBG(move.from >= 0 && move.from <= 25 && move.to >= 0 && move.to <= 25 && (move.color == Black || move.color == White));

	//use for bug tracking
	/*char* tmp[100];
	WriteGameString(tmp, g);*/

	ushort to = move.to;
	ushort from = move.from;
	bool fromHome = ToHome(move);

	Games[g].Position[from]++;
	Games[g].Position[from] |= move.color;

	if (fromHome) {
		move.color == Black ? Games[g].BlackHome-- : Games[g].WhiteHome--;
		ASSERT_DBG(Games[g].BlackHome >= 0 && Games[g].WhiteHome >= 0);
	}
	else {
		Games[g].Position[to]--;
		if (CheckerCount(Games[g].Position[to]) == 0)
			Games[g].Position[to] = 0;
		ASSERT_DBG(Games[g].Position[to] >= 0);

		if (hit)
		{
			Games[g].Position[to] = 1 | OtherColor(move.color);
		}
	}

	if (move.color == Black)
	{
		Games[g].BlackLeft += (move.to - move.from);
		ASSERT_DBG(Games[g].BlackLeft >= 0);

		if (hit)
		{
			Games[g].WhiteLeft -= (25 - move.to);
			ASSERT_DBG(Games[g].WhiteLeft >= 0);
			Games[g].Position[25]--;
			if (CheckerCount(Games[g].Position[25]) == 0)
				Games[g].Position[25] = 0;
			ASSERT_DBG(CheckerCount(Games[g].Position[25]) >= 0);
		}
	}
	if (move.color == White)
	{
		Games[g].WhiteLeft += (move.from - move.to);
		ASSERT_DBG(Games[g].WhiteLeft >= 0);

		if (hit)
		{
			Games[g].BlackLeft -= move.to; // -= 25 - (25 - move.to);
			ASSERT_DBG(Games[g].BlackLeft >= 0);
			Games[g].Position[0]--;
			if (CheckerCount(Games[g].Position[0]) == 0)
				Games[g].Position[0] = 0;
			ASSERT_DBG(CheckerCount(Games[g].Position[0]) >= 0);
		}
	}
	ASSERT_DBG(CountAllCheckers(Black, g) == 15 && CountAllCheckers(White, g) == 15);
}

bool IsBlockedFor(ushort pos, ushort color, int g) {
	if (pos > 25)
		return false;


	return (Games[g].Position[pos] & OtherColor(color)) && (CheckerCount(Games[g].Position[pos]) >= 2);
}

bool IsBlackBearingOff(ushort* lastCheckerPos, int g) {
	for (ushort i = 0; i <= 24; i++)
	{
		if ((Games[g].Position[i] & Black) && CheckerCount(Games[g].Position[i]) > 0)
		{
			*lastCheckerPos = i;
			return i >= 19;
		}
	}
	return true;
}

bool IsWhiteBearingOff(ushort* lastCheckerPos, int g) {
	for (ushort i = 25; i >= 1; i--)
	{
		if ((Games[g].Position[i] & White) && CheckerCount(Games[g].Position[i]) > 0)
		{
			*lastCheckerPos = i;
			return i <= 6;
		}
	}
	return true;
}
//
//bool HashSetExists(U64 hash, int g) {
//	// todo, try performance to loop in reverse.
//	// but dont compare last added hash
//	for (int i = 0; i < Games[g].MoveSetsCount - 1; i++)
//	{
//		if (Games[g].PossibleMoveSets[i].Hash == hash)
//			return true;
//	}
//	return false;
//}
//
//void CalcSetHash(MoveSet* moveSet) {
//	moveSet->Hash = MoveStartHash;
//	for (int i = 0; i < moveSet->Length; i++)
//	{
//		Move m = moveSet->Moves[i];
//		moveSet->Hash ^= PositionHash[m.from];
//		moveSet->Hash ^= PositionHash[m.to];
//	}
//}

void CreateBlackMoveSets(int fromPos, int diceIdx, int diceCount, int* maxSetLength, int g) {
	int start = fromPos;
	int nextStart = fromPos;
	bool checkerFound = false;
	ushort lastCheckerPos;
	bool bearingOff = IsBlackBearingOff(&lastCheckerPos, g);
	if (bearingOff)
		start = max(start, 19);

	ushort toIndex = CheckerCount(Games[g].Position[0]) > 0 ? 0 : 25;
	for (int i = start; i <= toIndex; i++)
	{
		if (!checkerFound)
			nextStart = i;

		if (!(Games[g].Position[i] & Black))
			continue;

		checkerFound = true;

		int diceVal = diceIdx > 1 ? Games[g].Dice[0] : Games[g].Dice[diceIdx];
		int toPos = i + diceVal;

		// När man bär av, får man använda tärningar med för högt värde,
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
		if (Games[g].MoveSetsCount == 0)
			Games[g].MoveSetsCount = 1;
		ushort setIdx = Games[g].MoveSetsCount - 1;
		MoveSet* moveSet = &Games[g].PossibleMoveSets[setIdx];
		Move* move = &moveSet->Moves[diceIdx];

		if (move->color != 0) {
			// A move is already generated for this dice in this sequence. Branch off a new set of moves.

			int copyCount = diceIdx;
			moveSet = &Games[g].PossibleMoveSets[setIdx + 1];
			moveSet->Length = copyCount;

			if (copyCount > 0)
				memcpy(&moveSet->Moves[0], &Games[g].PossibleMoveSets[setIdx].Moves[0], copyCount * sizeof(Move));

			move = &moveSet->Moves[diceIdx];
			Games[g].MoveSetsCount++;
			setIdx++;

			ASSERT_DBG(Games[g].MoveSetsCount < MAX_SETS_LENGTH);
		}

		move->from = i;
		move->to = toPos;
		move->color = Black;

		moveSet->Length++;
		*maxSetLength = max(*maxSetLength, moveSet->Length);

		if (diceIdx < diceCount - 1) {
			Move m = *move;
			int hit = DoMove(m, g);
			CreateBlackMoveSets(nextStart, diceIdx + 1, diceCount, maxSetLength, g);
			UndoMove(m, hit, g);
		}
		//else {
		//	// Last dice here
		//	// If it is the last dice. Check if there is a moveset generated for this hash.
		//	CalcSetHash(moveSet);
		//	if (HashSetExists(moveSet->Hash, g))
		//	{
		//		// Throw away this set.
		//		moveSet->Duplicate = true;
		//	}
		//}
	}
}


void CreateWhiteMoveSets(int fromPos, int diceIdx, int diceCount, int* maxSetLength, int g) {
	int start = fromPos;
	int nextStart = fromPos;
	bool checkerFound = false;
	ushort lastCheckerPos;
	bool bearingOff = IsWhiteBearingOff(&lastCheckerPos, g);
	if (bearingOff)
		start = min(start, 6);

	int toIndex = CheckerCount(Games[g].Position[25]) > 0 ? 25 : 0;

	for (int i = start; i >= toIndex; i--)
	{
		if (!checkerFound)
			nextStart = i;

		if (!(Games[g].Position[i] & White))
			continue;

		checkerFound = true;
		int diceVal = diceIdx > 1 ? Games[g].Dice[0] : Games[g].Dice[diceIdx];
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
		if (Games[g].MoveSetsCount == 0)
			Games[g].MoveSetsCount = 1;
		ushort setIdx = Games[g].MoveSetsCount - 1;
		MoveSet* moveSet = &Games[g].PossibleMoveSets[setIdx];
		Move* move = &moveSet->Moves[diceIdx];

		if (move->color != 0) {
			// A move is already generated for this dice in this sequence. Branch off a new sequence.
			int copyCount = diceIdx;
			moveSet = &Games[g].PossibleMoveSets[setIdx + 1];
			moveSet->Length = copyCount;

			if (copyCount > 0)
				memcpy(&moveSet->Moves[0], &Games[g].PossibleMoveSets[setIdx].Moves[0], copyCount * sizeof(Move));
			move = &moveSet->Moves[diceIdx];
			Games[g].MoveSetsCount++;
			setIdx++;
			ASSERT_DBG(Games[g].MoveSetsCount < MAX_SETS_LENGTH);
		}

		move->from = i;
		move->to = toPos;
		move->color = White;

		moveSet->Length++;
		*maxSetLength = max(*maxSetLength, moveSet->Length);

		if (diceIdx < diceCount - 1) {
			Move m = *move;
			int hit = DoMove(m, g);
			CreateWhiteMoveSets(nextStart, diceIdx + 1, diceCount, maxSetLength, g);
			UndoMove(m, hit, g);
		}
		//else {
		//	// Last dice here.
		//	// If it is the last dice. Check if there is a moveset generated for this hash.
		//	CalcSetHash(moveSet);
		//	if (HashSetExists(moveSet->Hash, g))
		//	{
		//		moveSet->Duplicate = true;
		//	}
		//}
	}

}

void ReverseDice(int g) {
	short temp = Games[g].Dice[0];
	Games[g].Dice[0] = Games[g].Dice[1];
	Games[g].Dice[1] = temp;
}

void RemoveShorterSets(int maxSetLength, int g) {
	bool modified = false;
	int realCount = Games[g].MoveSetsCount;
	do
	{
		modified = false;
		for (int i = 0; i < realCount; i++)
		{
			MoveSet* set = &Games[g].PossibleMoveSets[i];
			if (set->Length < maxSetLength)
			{
				memcpy(&Games[g].PossibleMoveSets[i], &Games[g].PossibleMoveSets[i + 1], (MAX_SETS_LENGTH - i) * 4 * sizeof(Move));
				modified = true;
				realCount--;
				break;
			}
		}
	} while (modified);
	Games[g].MoveSetsCount = realCount;
}

void CreateMoves(int g) {

	// 50% Better performance than for loop
	memset(&Games[g].PossibleMoveSets, 0, sizeof(Games[g].PossibleMoveSets));

	Games[g].MoveSetsCount = 0;
	// Largest Dice first
	if (Games[g].Dice[1] > Games[g].Dice[0]) {
		ReverseDice(g);
	}

	int diceCount = Games[g].Dice[0] == Games[g].Dice[1] ? 4 : 2;
	int maxSetLength = 0;
	for (size_t i = 0; i < 2; i++)
	{
		maxSetLength = 0;
		if (Games[g].CurrentPlayer & Black)
			CreateBlackMoveSets(0, 0, diceCount, &maxSetLength, g);
		else
			CreateWhiteMoveSets(25, 0, diceCount, &maxSetLength, g);

		//If no moves are found and dicecount == 2 reverse dice order and try again.
		if (Games[g].MoveSetsCount == 0 && diceCount == 2) {
			ReverseDice(g);
		}
		else {
			break;
		}
	}

	RemoveShorterSets(maxSetLength, g);
}

void WriteGameString(char* s, int g) {

	//todo: handle two digit numbers.

	int idx = 0;
	for (size_t i = 0; i < 26; i++)
	{
		if (Games[g].Position[i] > 0) {
			ushort black = Games[g].Position[i] & Black;
			if (black) {
				s[idx++] = 'b';
			}
			else {
				s[idx++] = 'w';
			}
		}

		char* n = malloc(3);
		int length = sprintf_s(n, 4, "%d", CheckerCount(Games[g].Position[i]));
		memcpy(&s[idx], n, length);
		idx += length;
		//s[idx++] = '0' + CheckerCount(Games[g].Position[i]);
		s[idx++] = ' ';
	}
	s[idx++] = '0' + Games[g].WhiteHome;
	s[idx++] = ' ';
	s[idx++] = '0' + Games[g].BlackHome;
	s[idx] = '\0';
}

void SetPointsLeft(int g) {
	ushort blackLeft = 0;
	ushort whiteLeft = 0;
	for (int i = 0; i < 25; i++)
	{
		if (Games[g].Position[i] & Black)
			blackLeft += (25 - i) * CheckerCount(Games[g].Position[i]);
		if (Games[g].Position[i] & White)
			whiteLeft += i * CheckerCount(Games[g].Position[i]);
	}
	Games[g].BlackLeft = blackLeft;
	Games[g].WhiteLeft = whiteLeft;
}

void ReadGameString(char* s, int g) {
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
			Games[g].Position[i] = atoi(n) | Black;
		}
		else if (StartsWith(token, "w")) {
			SubString(token, n, 2, 3);
			Games[g].Position[i] = atoi(n) | White;
		}

		token = strtok_s(NULL, " ", &context);
		VERIFY(token != NULL);
	}
	Games[g].WhiteHome = atoi(token);
	token = strtok_s(NULL, " ", &context);
	Games[g].BlackHome = atoi(token);
	SetPointsLeft(g);
}

void PrintGame(int g) {
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
			PlayerSide color = Games[g].Position[12 - x] & 48;
			int count = CheckerCount(Games[g].Position[12 - x]);
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
			PlayerSide color = Games[g].Position[x] & 48;
			int count = CheckerCount(Games[g].Position[x]);
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

	printf("Black left: %d   Bar: %d   Home: %d\n", Games[g].BlackLeft, CheckerCount(Games[g].Position[0]), Games[g].BlackHome);
	printf("White left: %d   Bar: %d   Home: %d\n", Games[g].WhiteLeft, CheckerCount(Games[g].Position[25]), Games[g].WhiteHome);
	if (Games[g].CurrentPlayer == Black)
		PrintBlue("Blacks turn. ");
	else if (Games[g].CurrentPlayer == White)
		PrintYellow("Whites turn. ");
	printf("Dice: %d, %d\n", Games[g].Dice[0], Games[g].Dice[1]);
	char gs[500];
	WriteGameString(gs, g);
	printf("%s\n", gs);
}



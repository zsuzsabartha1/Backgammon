
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include "Ai.h"
#include "Game.h"
#include "Utils.h"
//Negative score for leaving a blot on the board.
double BlotFactors[25];

//It is good to connect blocks since in the future the opponent might be blocked by them.
double ConnectedBlocksFactor[25];

void InitAi(bool constant) {
	for (int i = 0; i < 25; i++)
	{
		if (constant) {
			BlotFactors[i] = 1;
			ConnectedBlocksFactor[i] = 1;
		}
		else {
			BlotFactors[i] = RandomDouble(0, 1);
			ConnectedBlocksFactor[i] = RandomDouble(0, 1);
		}
	}
}

bool PlayersPassedEachOther(Game* g) {
	int minBlack = 25;
	int maxWhite = 0;
	for (int i = 0; i < 26; i++)
	{
		if (g->Position[i] & Black)
			minBlack = min(minBlack, i);

		if (g->Position[i] & White)
			maxWhite = max(maxWhite, i);
	}
	return minBlack > maxWhite;
}

double EvaluateCheckers(Game* g, char color) {
	double score = 0;
	int blockCount = 0;
	bool playersPassed = PlayersPassedEachOther(g);
	// TODO: Try calculate both colors in same loop. Better performance?
	for (int i = 1; i < 25; i++)
	{
		int p = color == White ? 25 - i : i;
		short v = g->Position[p];
		int checkCount = CheckerCount(v);
		if (checkCount > 1 && (v & color)) {
			blockCount++;
		}
		else {
			if (blockCount && !playersPassed) {
				score += pow((double)blockCount, ConnectedBlocksFactor[p]);
			}
			blockCount = 0;
		}

		if (checkCount == 1 && (v & color))
		{
			score -= (double)CheckerCount(v) * BlotFactors[p];
		}
	}
	return score;
}

double GetScore(Game* g) {
	if (g->CurrentPlayer == Black) {
		double bHome = 10000 * g->BlackHome;
		return bHome + EvaluateCheckers(g, Black) - EvaluateCheckers(g, White) - g->BlackLeft + g->WhiteLeft;
	}
	else {
		double wHome = 10000 * g->WhiteHome;
		return wHome + EvaluateCheckers(g, White) - EvaluateCheckers(g, Black) - g->WhiteLeft + g->BlackLeft;
	}
}

int FindBestMoveSet(Game* g) {
	int bestIdx = 0;
	double bestScore = -10000;
	CreateMoves(g);
	for (int i = 0; i < g->MoveSetsCount; i++)
	{
		Move moves[4];
		bool hits[4];
		for (int m = 0; m < g->SetLengths[i]; m++)
		{
			moves[m] = g->PossibleMoveSets[i].Moves[m];
			hits[m] = DoMove(moves[m], g);			
		}

		double score = GetScore(g);

		if (score > bestScore)
		{
			bestScore = score;
			bestIdx = i;
		}

		//Undoing in reverse
		for (int u = g->SetLengths[i] - 1; u >= 0; u--)
			UndoMove(moves[u], hits[u], g);
	}
	return bestIdx;
}

void PlayGame(Game* g) {
	StartPosition(g);

	RollDice(g);
	while (g->Dice[0] == g->Dice[1])
		RollDice(g);

	g->CurrentPlayer = g->Dice[0] > g->Dice[1] ? Black : White;
	while (g->BlackLeft > 0 && g->WhiteLeft > 0)
	{
		//char buf[BUF_SIZE];
		/*SetCursorPosition(0, 0);
		PrintGame(g);
		Sleep(100);*/
		//fgets(buf, 5000, stdin);

		int bestSetIdx = FindBestMoveSet(g);
		for (int i = 0; i < g->SetLengths[bestSetIdx]; i++)
		{
			DoMove(g->PossibleMoveSets[bestSetIdx].Moves[i], g);
			ASSERT_DBG(CountAllCheckers(Black, g) == 15 && CountAllCheckers(White, g) == 15);

			/*SetCursorPosition(0, 0);
			PrintGame(g);
			Sleep(100);*/
			//fgets(buf, 5000, stdin);
		}
		g->CurrentPlayer = OtherColor(g->CurrentPlayer);		
		RollDice(g);
	}
}

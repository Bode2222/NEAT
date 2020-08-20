#pragma once
#include "NEATGameInterface.h"
#include <iostream>
using namespace std;

class XORGame : public NEATGameInterface
{
private:
	vector<vector<float>> states = { {0, 0}, {0, 1}, {1, 0}, {1, 1} };
	vector<int> correctAction = { 0, 1, 1, 0 };
	//Holds the index for the current state at each instance of the game
	vector<int> instance;
	//Holds how far we have progressed in each instance
	vector<int> progressionCounter;
	//holds the sum of rewards in each instance
	vector<float> fitness;

	float successReward = 0.1, failureReward = -1, gameWonReward = 1;
	int maxProgress = 5;
public:
	XORGame(int numOfinstances) { instance = vector<int>(numOfinstances); progressionCounter = vector<int>(numOfinstances); fitness = vector<float>(numOfinstances); };
	tuple<vector<float>, float, bool> Step(int pipe, int action);
	tuple<vector<float>, float, bool> StepandDisplay(int pipe, int action);
	float GetMaxReward() { return 1.4; };
	int GetStateSize() { return 2; };
	int GetActionSize() { return 1; }
	int GetNumOfInstances() { return instance.size(); }
	void Reset();
	vector<float> GetStartState(int pipe) { return states[0]; };
	int Interprete(pair<vector<float>, int> NNetOutput);
};


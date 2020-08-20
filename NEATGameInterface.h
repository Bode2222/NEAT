#pragma once
#include <vector>
#include <tuple>
using namespace std;

//For neat games, every output should be an action, in flappy birds, to jump or not jump  will be 2 different neurons
class NEATGameInterface {
public:
	//Takes in what instance of the game we are playing and what action we took, returns the state we achieved, the reward we got, and Whether or not the game is over
	virtual std::tuple<vector<float>, float, bool> Step(int pipe, int action) =0;
	virtual std::tuple<vector<float>, float, bool> StepandDisplay(int pipe, int action) =0;
	virtual int GetStateSize() =0;
	virtual int GetActionSize() =0;
	virtual int GetNumOfInstances() = 0;
	virtual float GetMaxReward() = 0;
	virtual vector<float> GetStartState(int pipe) = 0;
	//Reset every instance of every game
	virtual void Reset() = 0;
	//Interprets the output if I need a list of continuous outputs, or if I need one discrete output to play the game
	virtual int Interprete(pair<vector<float>, int> NNetOutput) = 0;
};

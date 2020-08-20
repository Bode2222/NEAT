#pragma once
#include "NEATGameInterface.h"
#include "SFML/Graphics.hpp"
#include <chrono>
#include <thread>
#include <iostream>
using namespace std;

//Will assume we're working with a 100 by 100 grid
class FlappyGame : public NEATGameInterface
{
	//Display variables
	sf::RenderWindow *wd;
	const float screenWidth = 100;
	const float screenHeight = 100.f;
	const float frameRate = 30;
	const int xDim = 511, yDim = 511;
	bool windowClosed = false;

	//Drawn Variables
	sf::RectangleShape bird, gp1, gp1a, gp2, gp2a;
	
	//Player variables
	vector<float> y;
	vector<float> speed;
	vector<float> rewardList;
	vector<pair<float, float>> greenPipe;

	//Game variables:
	static float scrollSpeed;
	const float birdHeight = 3;
	const float birdWid = 3;
	const float deltaT = 0.1;
	const float gravity = 4.f;
	const float jumpForce = -3.5f;
	const float pipeWidth = 10.f;
	const float spaceBtwPipesY = 30.f;
	const float spaceBtwPipesX = screenWidth;

	//Game aux functions
	pair<float, float> GeneratePipe(pair<float, float>);
	void Display();

	//NEAT vars
	int instanceNum;
	float passReward = 0.1;
	float failReward = -1;
public:
	void Reset();
	int GetStateSize() { return 3; };
	int GetActionSize() { return 1; };
	vector<float> GetStartState(int pipe);
	float GetMaxReward() { return 40; }
	int GetNumOfInstances() { return instanceNum; };
	int Interprete(pair<vector<float>, int> NNetOutput);
	FlappyGame(int numOfInstances);
	std::tuple<vector<float>, float, bool> Step(int pipe, int action);
	std::tuple<vector<float>, float, bool> StepandDisplay(int pipe, int action);
	void UserPlay();
	~FlappyGame();
};


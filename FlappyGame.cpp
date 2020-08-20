#include "FlappyGame.h"

float FlappyGame::scrollSpeed = 1.5f;

FlappyGame::FlappyGame(int numOfInstances) {
	instanceNum = numOfInstances;
	y = vector<float>(numOfInstances, screenHeight/2.f);
	speed = vector<float>(numOfInstances, 0);
	rewardList = vector<float>(numOfInstances, 0);
	greenPipe = vector<pair<float, float>>(2);

	//Drawing vars instantiation
	int playerLen = birdWid / screenWidth * xDim;
	int playerHeight = birdHeight / screenHeight * yDim;
	int gpLen = pipeWidth / screenWidth * xDim;
	int gpHeight = screenHeight / screenHeight * yDim;
	wd =  new sf::RenderWindow(sf::VideoMode(xDim, yDim), "Flappy Bird");
	bird = sf::RectangleShape(sf::Vector2f(playerLen, playerHeight));
	bird.setFillColor(sf::Color::Red);
	bird.setOrigin(playerLen / screenWidth * xDim * 0.5f, playerHeight / screenHeight * yDim * 0.5f);

	gp1 = sf::RectangleShape(sf::Vector2f(gpLen, gpHeight));
	gp1.setOrigin(gpLen * 0.5, gpHeight * 0.5);
	gp1.setFillColor(sf::Color::Green);
	gp1a = sf::RectangleShape(sf::Vector2f(gpLen, gpHeight));
	gp1a.setOrigin(gpLen * 0.5, gpHeight * 0.5);
	gp1a.setFillColor(sf::Color::Green);
	gp2 = sf::RectangleShape(sf::Vector2f(gpLen, gpHeight));
	gp2.setOrigin(gpLen * 0.5, gpHeight * 0.5);
	gp2.setFillColor(sf::Color::Green);
	gp2a = sf::RectangleShape(sf::Vector2f(gpLen, gpHeight));
	gp2a.setOrigin(gpLen * 0.5, gpHeight * 0.5);
	gp2a.setFillColor(sf::Color::Green);
}

void FlappyGame::Reset() {
	for (int i = 0; i < instanceNum; i++) {
		y[i] = screenHeight / 2.f;
		speed[i] = 0;
		rewardList[i] = 0;
	}
	greenPipe[0] = GeneratePipe(make_pair(screenWidth * 0.5f, 0));
	greenPipe[1] = GeneratePipe(greenPipe[0]);
}

pair<float, float> FlappyGame::GeneratePipe(pair<float, float> exPipe) {
	float bottomBound = screenHeight - spaceBtwPipesY * 0.5f;
	float topBound = spaceBtwPipesY * 0.5f;
	float height = rand() % (int)(bottomBound - topBound);
	return make_pair(exPipe.first + spaceBtwPipesX, height + topBound);
}

vector<float> FlappyGame::GetStartState(int pipe) {
	return { 1.f * y[pipe] / yDim, (1.f * greenPipe[0].second - pipeWidth * 0.5f)/yDim, (greenPipe[0].second + pipeWidth * 0.5f)/yDim };
}

std::tuple<vector<float>, float, bool> FlappyGame::Step(int pipe, int action) {
	float reward = 0;
	bool gameOver = 0;

	//Bird motion
	if (action == 1) {
		speed[pipe] = jumpForce;
	}
	speed[pipe] += deltaT * gravity;
	y[pipe] += speed[pipe];

	//GreenPipe motion
	greenPipe[0].first -= scrollSpeed;
	greenPipe[1].first -= scrollSpeed;
	if (greenPipe[0].first < -pipeWidth * 0.5f) {
		greenPipe[0] = greenPipe[1];
		greenPipe[1] = GeneratePipe(greenPipe[0]);
	}

	//If bird is in the floor or ceiling
	if (y[pipe] + birdHeight * 0.5f >= screenHeight || y[pipe] - birdHeight * 0.5f <= 0) {
		//Game over
		reward = failReward; 
		gameOver = 1;
	}
	else {
		//If we the first pipe is in the middle of the screen
		if (greenPipe[0].first <= screenWidth * 0.5f + pipeWidth * 0.5f && greenPipe[0].first >= screenWidth * 0.5f - pipeWidth * 0.5f) {
			//If we're in bottom or top pipe
			if (y[pipe] + birdHeight * 0.5f >= greenPipe[0].second + spaceBtwPipesY * 0.5f || y[pipe] - birdHeight * 0.5f <= greenPipe[0].second - spaceBtwPipesY * 0.5f) {
				//Game over
				reward = failReward;
				gameOver = 1;
			}
			else if (greenPipe[0].first <= screenWidth * 0.5f + birdWid * 0.5f && greenPipe[0].first >= screenWidth * 0.5f - birdWid * 0.5f) {
				//Give us a point
				reward = passReward;
			}
		}
	}

	vector<float> locs = { 1.f * y[pipe] / yDim, (1.f * greenPipe[0].second - pipeWidth * 0.5f) / yDim, (greenPipe[0].second + pipeWidth * 0.5f) / yDim };;
	rewardList[pipe] += reward;

	return make_tuple(locs, rewardList[pipe], gameOver);
}

std::tuple<vector<float>, float, bool> FlappyGame::StepandDisplay(int pipe, int action) {
	auto temp = Step(pipe, action);
	Display();
	if (windowClosed) {
		return make_tuple(get<0>(temp), get<1>(temp), windowClosed);
	}
	return temp;
}

void FlappyGame::UserPlay() {
	Reset();
	float userFitness = 0;

	chrono::system_clock::time_point startTime = chrono::system_clock::now();
	chrono::system_clock::time_point lastFrameTime = chrono::system_clock::now();

	/*Convert Hz to milliseconds*/
	float timeInMilli = (1 / frameRate) * 1000.0f;
	bool gameOver = false;
	int action = 0;
	while (wd->isOpen() && !gameOver) {
		//Limit fps
		startTime = chrono::system_clock::now();
		chrono::duration<double, milli> work_time = startTime - lastFrameTime;
		if (work_time.count() < timeInMilli) {
			std::chrono::duration<double, std::milli> delta_ms(timeInMilli - work_time.count());
			auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
			this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
		}
		lastFrameTime = chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = lastFrameTime - startTime;

		sf::Event evnt;
		action = 0;
		while (wd->pollEvent(evnt)) {
			switch (evnt.type) {
			case sf::Event::KeyPressed:
				if (evnt.key.code == sf::Keyboard::Space) {
					action = 1;
				}
			}
		}
		auto temp = StepandDisplay(0, action);

		gameOver = get<2>(temp);
		userFitness = get<1>(temp);
		if (gameOver) wd->close();
	}
	cout << "Your score " << userFitness << endl;
}

int FlappyGame::Interprete(pair<vector<float>, int> NNetOutput) {
	if (GetActionSize() == 1) {
		return (NNetOutput.first[0] >= 0.5f);
	}
	else {
		return NNetOutput.second;
	}
}

void FlappyGame::Display() {
	int curY = y[0] / screenHeight * yDim;
	bird.setPosition(xDim / 2, curY);

	int gp1X = (1.f * (greenPipe[0].first / screenWidth) * xDim);
	int gp1Y = (1.f * greenPipe[0].second / screenHeight * yDim) - 0.5f * (spaceBtwPipesY/screenHeight * yDim) - (screenHeight/screenHeight * yDim) * 0.5f;
	gp1.setPosition(gp1X, gp1Y);
	int gp1aX = (greenPipe[0].first / screenWidth * xDim);
	int gp1aY = (greenPipe[0].second / screenHeight * yDim) + 0.5f * (spaceBtwPipesY / screenHeight * yDim) + (screenHeight / screenHeight * yDim) * 0.5f;
	gp1a.setPosition(gp1aX, gp1aY);
	int gp2X = (greenPipe[1].first / screenWidth * xDim);
	int gp2Y = (1.f * greenPipe[1].second / screenHeight * yDim) - 0.5f * (spaceBtwPipesY / screenHeight * yDim) - (screenHeight / screenHeight * yDim) * 0.5f;
	gp2.setPosition(gp2X, gp2Y);
	int gp2aX = (greenPipe[1].first / screenWidth * xDim);
	int gp2aY = (greenPipe[1].second / screenHeight * yDim) + 0.5f * (spaceBtwPipesY / screenHeight * yDim) + (screenHeight / screenHeight * yDim) * 0.5f;
	gp2a.setPosition(gp2aX, gp2aY);

	wd->requestFocus();
	sf::Event evnt;
	if (wd->pollEvent(evnt)) {
		switch(evnt.type) {
		case sf::Event::Closed:
			wd->close();
			windowClosed = true;
			break;
		}
	}

	wd->clear();
	wd->requestFocus();
	wd->draw(bird);
	wd->draw(gp1);
	wd->draw(gp1a);
	wd->draw(gp2);
	wd->draw(gp2a);
	wd->display();
}

FlappyGame::~FlappyGame() {
	delete(wd);
}

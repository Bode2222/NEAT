#include "XORGame.h"

tuple<vector<float>, float, bool> XORGame::Step(int pipe, int action) {
	if (correctAction[instance[pipe]] == action) {
		//Progress to the next stage
		instance[pipe]++;
		instance[pipe] %= states.size();
		//Increment progression counter in this pipe
		progressionCounter[pipe]++;
		if (progressionCounter[pipe] >= maxProgress) {
			fitness[pipe] += gameWonReward;
			return make_tuple(vector<float>({ -1.f, -1.f }), fitness[pipe], 1);
		}
		//Create and return tuple
		fitness[pipe] += successReward;
		return make_tuple(states[instance[pipe]], fitness[pipe], 0);
	}
	else {
		//set progression counter in this pipe to zero
		progressionCounter[pipe] = 0;
		//Game over
		fitness[pipe] += failureReward;
		return make_tuple(vector<float>({ -1.f, -1.f }), fitness[pipe], 1);
	}
}

tuple<vector<float>, float, bool> XORGame::StepandDisplay(int pipe, int action) {
	cout << "{ " << states[instance[pipe]][0] << ", " << states[instance[pipe]][1] << " }: " << action << endl;
	return Step(pipe, action);
}

void XORGame::Reset() {
	for (int i = 0; i < instance.size(); i++)
	{
		instance[i] = 0;
		fitness[i] = 0;
		progressionCounter[i] = 0;
	} 
}

int XORGame::Interprete(pair<vector<float>, int> NNetOutput) {
	if (GetActionSize() == 1) {
		return (NNetOutput.first[0] >= 0.5);
	}
	else {
		return NNetOutput.second;
	}
}

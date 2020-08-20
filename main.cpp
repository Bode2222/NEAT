#include <iostream>
#include "XORGame.h"
#include "FlappyGame.h"
#include "NEAT.h"

using namespace std;

//What if for the fitness of my xor game I used the distance to the correct ans instead of how many correct answers it got

//Log of changes I have made
//*Changed mutate so enabling and disabling also happens when weights are created but they already exist
//*Made a built in function in the game that gives the max score. If fitness passes this score, the game is over, so it wont play endless games forever
//*Made adding a new connection or neuron only happen if the agent gets mutated(80% probability)

//Why does the number of species increase then reduce to 1 only a generation apart?
//Like, why do they keep getting deleted immediately???
int main() {
	int numOfAgents = 150;
	//XORGame env(numOfAgents);
	FlappyGame env(numOfAgents);
	NEAT myAlgo(&env);
	myAlgo.Train(100);
	myAlgo.PlayGame();

	cout << "Program Terminated." << endl;
	cin.get();
	return 0;
}
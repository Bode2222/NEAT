#include "NEAT.h"

void Neuron::ActivateValue() {
	value = exp(-4.9*value);
	value = 1 / (1 + value);
}

float Neuron::calcValue() {
	if (checked) {
		return value;
	}
	else {
		checked = 1;
		value = 0;
		for (auto it : weights) {
			value += 1.f*it.first->calcValue() * it.second;
		}
		ActivateValue();
		return value;
	}
}

NEAT::NEAT(NEATGameInterface* e) {
	srand(time(0));
	env = e;
	numOfAgents = env->GetNumOfInstances();
}

std::unordered_map<int, Neuron> NEAT::InstantiateNetwork(const Gene& genome) {
	std::unordered_map<int, Neuron> result;
	for (int i = 0; i < genome.Connection.size(); i++) {
		if (genome.Connection[i].enabled) {
			result[genome.Connection[i].to].weights[&result[genome.Connection[i].from]] = genome.Connection[i].weight;
		}
	}
	return result;
}

pair<vector<float>, int> NEAT::FeedForward(std::unordered_map<int, Neuron>& neurons, const std::vector<float>& input) {
	int inputLayerSize = env->GetStateSize()+1;
	int outputLayerSize = env->GetActionSize();
	pair<vector<float>, int> result;
	if (input.size() != inputLayerSize-1) {
		cout << "Feed forward in neat class. Input doesnt match input layer size" << endl;
	}

	//Put input into input layer
	for (int i = 0; i < inputLayerSize; i++) {
		if (i == inputLayerSize - 1) {
			neurons[i].value = 1;
		}
		else {
			neurons[i].value = 1.f*input[i];

		}
		neurons[i].checked = 1;
	}

	//Using the output neurons, calc all values recursively
	float max = neurons[inputLayerSize].calcValue();
	result.first.push_back(max);
	result.second = 0;
	for (int i = 1; i < outputLayerSize; i++) {
		result.first.push_back(neurons[inputLayerSize + i].calcValue());
		if (result.first.back() > max) result.second = i;
	}

	//Set every neurons checked flag to 0
	for (auto it = neurons.begin(); it != neurons.end(); it++) {
		it->second.checked = 0;
	}
	return result;
}

float NEAT::CompareGene(Gene a, Gene b) {
	int E = 0, D = 0;
	float W = 0;
	int N = (a.Connection.size() > b.Connection.size()) ? a.Connection.size() : b.Connection.size();
	N = (N < 20) ? 1 : N;
	int numOfIntersects = 0;

	if (a.Connection.size() == 0 && b.Connection.size() == 0) {
		return 0;
	}
	else if (a.Connection.size() == 0 || b.Connection.size() == 0) {
		E = a.Connection.size() + b.Connection.size();
	}
	else {
		//run every connection in a and b through the innovation list to get their indexes
		map<int, float> occured;
		if (b.Connection.size() < a.Connection.size()) {
			int max = b.Connection[0].innovationIndex;
			for (int i = 0; i < b.Connection.size(); i++) {
				occured[b.Connection[i].innovationIndex] = b.Connection[i].weight;
			}
			for (int i = 0; i < a.Connection.size(); i++) {
				if (a.Connection[i].innovationIndex > max) {
					E++;
				}
				auto it = occured.find(a.Connection[i].innovationIndex);
				if (it != occured.end()) {
					numOfIntersects++;
					W += abs(it->second - a.Connection[i].weight);
					it->second = INT_MAX;
				}
				else {
					D++;
				}
			}
			W /= numOfIntersects;
			for (auto it : occured) {
				if (it.second != INT_MAX) {
					D++;
				}
			}
		}
		else {
			int max = b.Connection[0].innovationIndex;
			for (int i = 0; i < b.Connection.size(); i++) {
				occured[b.Connection[i].innovationIndex] = b.Connection[i].weight;
			}
			for (int i = 0; i < a.Connection.size(); i++) {
				if (a.Connection[i].innovationIndex > max) {
					E++;
				}
				auto it = occured.find(a.Connection[i].innovationIndex);
				if (it != occured.end()) {
					numOfIntersects++;
					W += abs(it->second - a.Connection[i].weight);
					it->second = INT_MAX;
				}
				else {
					D++;
				}
			}
			//W /= numOfIntersects;
			for (auto it : occured) {
				if (it.second != INT_MAX) {
					D++;
				}
			}
		}
	}
	
	float ans = (c1 * E / N) + (c2 * D / N) + c3 * W;
	return ans;
}

Gene NEAT::CrossGene(Gene a, Gene b) {
	map<int, Allele*> occured;
	Gene result(a.numOfNeurons);
	for (int i = 0; i < a.Connection.size(); i++) {
		occured[a.Connection[i].innovationIndex] = &a.Connection[i];
	}
	for (int i = 0; i < b.Connection.size(); i++) {
		auto it = occured.find(b.Connection[i].innovationIndex);
		if (it != occured.end()) {
			//pick a random one
			it->second = ((rand() % 10) / 10.f > 0.4f) ? it->second : &b.Connection[i];
			//if either of them are disabled
			if (!it->second->enabled || !b.Connection[i].enabled) {
				it->second->enabled = ((rand() % 100) / 100.f > disabledChance);
			}
		}
	}
	for (auto it : occured) {
		result.Connection.push_back(*it.second);
	}
	return result;
}

Gene NEAT::CrossGeneEqualFitness(Gene a, Gene b) {
	map<int, Allele*> occured;
	Gene result(a.numOfNeurons);
	for (int i = 0; i < a.Connection.size(); i++) {
		occured[a.Connection[i].innovationIndex] = &a.Connection[i];
	}
	for (int i = 0; i < b.Connection.size(); i++) {
		if (occured[b.Connection[i].innovationIndex] == 0) {
			occured[b.Connection[i].innovationIndex] = &b.Connection[i];
		}
		else{
			//pick a random one
			occured[b.Connection[i].innovationIndex] = ((rand() % 10) / 10.f > 0.4f) ? occured[b.Connection[i].innovationIndex] : &b.Connection[i];
			//if either of them are disabled
			if (!occured[b.Connection[i].innovationIndex]->enabled || !b.Connection[i].enabled) {
				occured[b.Connection[i].innovationIndex]->enabled = ((rand() % 100) / 100.f > disabledChance) ? 1 : 0;
			}
		}
	}
	for (auto it : occured) {
		result.Connection.push_back(*it.second);
	}
	return result;
}

void NEAT::Train(int generations) {
	float maxRewardAchievable = env->GetMaxReward();

	//Number of enabled parameters the champion has
	int s = 0;

	//Input layer size + bias + output layer size
	Gene temp(env->GetStateSize() + env->GetActionSize() + 1);
	vector<Gene> agent(numOfAgents, temp);

	//Keeps track of species, the genes in them and the fitness of each Gene in them
	vector<vector<pair<Gene*, float>>> species;
	//Keeps track of each species identifying gene
	vector<Gene*> speciesIDGene;
	//Keeps track of each species score
	vector<float> speciesScore;
	//Keeps track of the max score ever achieved of each species
	vector<float> maxSpeciesScorer;
	//Keeps track of how many generations it has been since each species last improved
	vector<int> maxSpeciesTimer;

	int maxGeneSpeciesID = 0;
	float maxFitness = INT_MIN;
	//In every generation
	for (int i = 0; i < generations; i++) {
		env->Reset();
		species = vector<vector<pair<Gene*, float>>>(speciesIDGene.size());
		speciesScore = vector<float>(speciesIDGene.size());
		maxSpeciesScorer = vector<float>(speciesIDGene.size());
		maxSpeciesTimer = vector<int>(speciesIDGene.size());

		//Selection: 
		//Speciate each agent and get its fitness
		for (int j = 0; j < numOfAgents; j++) {
			//Put into species
			int speciesIDNum = -1;
			bool foundSimilarSpecies = false;
				//check all available species for similarity
			for (int k = 0; k < speciesIDGene.size(); k++) {
				if (CompareGene(agent[j], *speciesIDGene[k]) <= sigmaT) {
					species[k].push_back(make_pair(&agent[j], 0));
					speciesIDNum = k;
					foundSimilarSpecies = true;
					break;
				}
			}
				//else, create a new species
			if (!foundSimilarSpecies) {
				speciesIDGene.push_back(&agent[j]);
				species.push_back(vector<pair<Gene*, float>>());
				species.back().push_back(make_pair(&agent[j], 0));
				maxSpeciesScorer.push_back(0);
				maxSpeciesTimer.push_back(0);
				speciesScore.push_back(0);
				speciesIDNum = species.size() - 1;
			}

			//convert agent('s genes) to network
			auto tempNet = InstantiateNetwork(agent[j]);
			vector<float> currentState = env->GetStartState(j);
			float fitness = 0;

			//Play Game
			for (int k = 0; k < maxStepsPerGame; k++) {
				int action = env->Interprete(FeedForward(tempNet, currentState));
				auto packet = env->Step(j, action);
				fitness = get<1>(packet);
				if (fitness > maxRewardAchievable) {
					i = generations;
					break;
				}
				if (get<2>(packet)) {
					break;
				}
				currentState = get<0>(packet);
			}
			
			//Update Champion
			if (fitness > maxFitness || (fitness >= maxFitness && agent[j].numOfNeurons < Champion.numOfNeurons)) {
				maxGeneSpeciesID = speciesIDNum; 
				maxFitness = fitness;
				Champion = agent[j];
				s = 0;
				for (int j = 0; j < Champion.Connection.size(); j++) {
					if (Champion.Connection[j].enabled) s++;
				}
			}

			//Add agents fitness to its own personal fitness, and to its species fitness
			if (speciesIDNum != -1) {
				species[speciesIDNum].back().second = fitness;
				speciesScore[speciesIDNum] += fitness;
			}
			else {
				cout << "This agent didn't get a species, how?" << endl;
			}
		}

		//Needed to calculate how many children each species is allowed to have
		float totalSpeciesScore = 0;
		//Needed to calculate children sharing
		float minSpeciesScore = 0;
		//Keeps track of how many agents performed well enough to go to the next generation without mutation
		int numOfChosen = 0;
		//Update speciesIDGene to a random gene in each species, calc the total score for each species, 
		//delete species that have exceeded given time without improving, sort species on performance
		for (int j = 0; j < species.size(); j++) {
			//If the species is not empty
			if (species[j].size() != 0 && maxSpeciesTimer[j] <= genB4Cull) {
				//Set its identifying gene as a random gene in it
				auto it = species[j].begin();
				int add = rand() % species[j].size();
				for (int k = 0; k < add; k++) it++;
				speciesIDGene[j] = it->first;

				//Divide each species total score by the number of agents in it to get its real score
				speciesScore[j] /= 1.f * species[j].size();
				totalSpeciesScore += speciesScore[j];
				//Update score of the species that performed the worst this generation
				if (speciesScore[j] < minSpeciesScore) {
					minSpeciesScore = speciesScore[j];
				}
				
				//Update the max score attained of each species or how long it has been since this species improved
				if (speciesScore[j] > maxSpeciesScorer[j]) {
					maxSpeciesTimer[j] = 0;
					maxSpeciesScorer[j] = speciesScore[j];
				}
				else {
					maxSpeciesTimer[j]++;
				}


				//Sort species based on performance
				sort(species[j].begin(), species[j].end(), [&](pair<Gene*, int> a, pair <Gene*, int> b) { return a.second > b.second; });

				//Push top agent in each species with greater than 5 agents into the next stage untouched
				if (species[j].size() > 5) {
					agent[numOfChosen] = *species[j][0].first;
					numOfChosen++;
				}
			}
			//If the species is empty delete it
			else {
				species.erase(species.begin() + j);
				speciesScore.erase(speciesScore.begin() + j);
				speciesIDGene.erase(speciesIDGene.begin() + j);
				maxSpeciesScorer.erase(maxSpeciesScorer.begin() + j);
				maxSpeciesTimer.erase(maxSpeciesTimer.begin() + j);
				j--;
			}
		}

		//Print to screen
		std::cout << "Generation " << i << " Size: " << s << " Score: " << maxFitness << " Species: " << species.size() << endl;

		//Keeps track of how many children each species gets to produce this generation
		vector<int> numOfChildrenPerSpecies(speciesScore.size());
		//Offset all scores by the score of the worst scoring species to give me all positive, non-zero numbers
		minSpeciesScore -= 1;
		float scoreLeft = totalSpeciesScore - minSpeciesScore*species.size();

		float childrenLeft = numOfAgents - numOfChosen;
		int distributedChildren = 0;
		//Assign the number of children each species has based on their fitness
		for (int j = 0; j < speciesScore.size(); j++) {
			float adjustedScore = (speciesScore[j] - minSpeciesScore);
			numOfChildrenPerSpecies[j] = 1.f * (adjustedScore / scoreLeft) * (childrenLeft);
			if (adjustedScore == scoreLeft) numOfChildrenPerSpecies[j] = childrenLeft;
			scoreLeft -= adjustedScore;
			childrenLeft -= numOfChildrenPerSpecies[j];
			distributedChildren += numOfChildrenPerSpecies[j];
		}
		
		//Crossover && Mutation:
		//only 75% of the next stage agents are crossed then mutated, the rest are just mutated from well performing agents
		for (int j = 0; j < species.size(); j++) {
			//Number of top performers to breed
			int chosenToBreed = round(species[j].size() * (1.f - cull));
			if (chosenToBreed == 0) break;
			//Number of agents produced via breeding agents
			int crossed = 0.75 * numOfChildrenPerSpecies[j];
			//Number of agents to be produced via mutation
			int mutated = numOfChildrenPerSpecies[j] - crossed;
			//Produce <crossed> agents
			for (int k = 0; k < crossed; k++) {
				//First decide whether its breeding intraspecies or interspecies
				float num = ((rand() % 1000) / 1000.f);
				//Interbreed
				if (num < interBreed) {
					int randSpecies = rand() % species.size();
					int chosenToBreedForRandSpecies = round(species[randSpecies].size() * (1 - cull));
					if (!chosenToBreedForRandSpecies) break;
					int randAgent = rand() % chosenToBreedForRandSpecies;
					auto agent1 = species[j][rand() % chosenToBreed];
					auto agent2 = species[randSpecies][randAgent];

					//Breed based on fitness
					if (agent1.second > agent2.second) {
						agent[numOfChosen] = CrossGene(*agent1.first, *agent2.first);
					}
					else if (agent1.second == agent2.second) {
						agent[numOfChosen] = CrossGeneEqualFitness(*agent1.first, *agent2.first);
					}
					else {
						agent[numOfChosen] = CrossGene(*agent1.first, *agent2.first);
					}
				}
				//Intrabreed
				else {
					auto agent1 = species[j][rand() % chosenToBreed];
					auto agent2 = species[j][rand() % chosenToBreed];

					//Breed based on fitness
					if (agent1.second > agent2.second) {
						agent[numOfChosen] = CrossGene(*agent1.first, *agent2.first);
					}
					else if (agent1.second == agent2.second) {
						agent[numOfChosen] = CrossGeneEqualFitness(*agent1.first, *agent2.first);
					}
					else {
						agent[numOfChosen] = CrossGene(*agent1.first, *agent2.first);
					}
				}
				//After Crossing, mutate
				agent[numOfChosen] = Mutate(*species[j][rand() % chosenToBreed].first);
				numOfChosen++;
			}
			//Produce <mutated> agents
			for (int k = 0; k < mutated; k++) {
				agent[numOfChosen] = Mutate(*species[j][rand() % chosenToBreed].first);
				numOfChosen++;
			}
		}
	}
}

Gene NEAT::Mutate(Gene x) {
	//Mutate existing weight
	float randNum = (rand() % 10) / 10.f;
	if (randNum < mutationChance) {
		//Change existing weights
		for (int i = 0; i < x.Connection.size(); i++) {
			randNum = (rand() % 10) / 10.f;
			if (randNum < perturbedChance) {
				int perturb = rand() % 3;
				if (perturb == 0)
					x.Connection[i].weight *= 0.5f + (rand() % 100) / 100.f;
				else if (perturb == 1)
					x.Connection[i].weight += (rand() % 200) / 100.f - 1;
				else
					x.Connection[i].weight = -x.Connection[i].weight;
			}
			else{
				x.Connection[i].weight = (rand() % 400) / 100.f - 2;
			}
		}

		//Add new weight
		randNum = (rand() % 10000) / 10000.f;
		if (randNum < addWeightChance) {
			int inputLayerSize = env->GetStateSize() + 1;
			int outputLayerSize = env->GetActionSize();

			//No weight comes from the output layer
			int from = rand() % (x.numOfNeurons - outputLayerSize);
			if (from >= inputLayerSize && from < inputLayerSize + outputLayerSize) {
				from += outputLayerSize;
				from %= x.numOfNeurons;
			}

			//No weight goes to the input layer
			int to = rand() % (x.numOfNeurons - inputLayerSize) + inputLayerSize;
			while (to == from) {
				to = rand() % (x.numOfNeurons - inputLayerSize) + inputLayerSize;
			}

			if (InnovationList[make_pair(from, to)] == 0) {
				InnovationList[make_pair(from, to)] = latestInovation;
				latestInovation++;
			}
			//If it already has this weight, toggle the weight
			bool alreadyHasThisWeight = false;
			int innovNum = InnovationList[make_pair(from, to)];
			for (int i = 0; i < x.Connection.size(); i++) {
				if (x.Connection[i].innovationIndex == innovNum) {
					alreadyHasThisWeight = true;
					//x.Connection[i].enabled != x.Connection[i].enabled;
					break;
				}
			}
			if (!alreadyHasThisWeight) {
				Allele newAllele;
				newAllele.enabled = 1;
				newAllele.from = from;
				newAllele.to = to;
				newAllele.innovationIndex = innovNum;
				newAllele.weight = ((rand() % 30000) / 10000.f) - 1.5f;
				x.Connection.push_back(newAllele);
			}
		}

		//Add new node
		randNum = (rand() % 10000) / 10000.f;
		if (randNum < addNodeChance && x.Connection.size() > 0) {
			x.numOfNeurons++;
			int rand1 = rand() % x.Connection.size();
			x.Connection[rand1].enabled = false;

			if (InnovationList[make_pair(x.Connection[rand1].from, x.numOfNeurons - 1)] == 0) {
				InnovationList[make_pair(x.Connection[rand1].from, x.numOfNeurons - 1)] = latestInovation;
				latestInovation++;
			}
			int toNewNodeInnovationNum = InnovationList[make_pair(x.Connection[rand1].from, x.numOfNeurons - 1)];
			Allele toNewNode;
			toNewNode.enabled = true;
			toNewNode.from = x.Connection[rand1].from;
			toNewNode.to = x.numOfNeurons - 1;
			toNewNode.weight = 1;
			toNewNode.innovationIndex = toNewNodeInnovationNum;
			x.Connection.push_back(toNewNode);

			if (InnovationList[make_pair(x.numOfNeurons - 1, x.Connection[rand1].to)] == 0) {
				InnovationList[make_pair(x.numOfNeurons - 1, x.Connection[rand1].to)] = latestInovation;
				latestInovation++;
			}
			int fromNewNodeInnovationNum = InnovationList[make_pair(x.numOfNeurons - 1, x.Connection[rand1].to)];
			Allele fromNewNode;
			fromNewNode.enabled = true;
			fromNewNode.from = x.numOfNeurons - 1;
			fromNewNode.to = x.Connection[rand1].to;
			fromNewNode.weight = x.Connection[rand1].weight;
			fromNewNode.innovationIndex = fromNewNodeInnovationNum;
			x.Connection.push_back(fromNewNode);
		}
				
	}
	return x;
}

void NEAT::PlayGame() {
	cout << "Champ Gene: " << endl;
	PrintGene(Champion);
	env->Reset();
	auto tempNet = InstantiateNetwork(Champion);
	vector<float> currentState = env->GetStartState(0);
	float fitness = 0;
	int k = 0;

	chrono::system_clock::time_point startTime = chrono::system_clock::now();
	chrono::system_clock::time_point lastFrameTime = chrono::system_clock::now();

	/*Convert Hz to milliseconds*/
	float timeInMilli = (1 / frameRate) * 1000.0f;
	bool gameOver = false;
	int action = 0;
	while (!gameOver && k < maxStepsPerGame) {
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
		
		int action = env->Interprete(FeedForward(tempNet, currentState));
		auto packet = env->StepandDisplay(0, action);
		fitness = get<1>(packet);
		if (get<2>(packet)) {
			break;
		}
		currentState = get<0>(packet);

		gameOver = get<2>(packet);
		if (gameOver) break;
	}

	cout << "Total fitness is: " << fitness << endl;
}

void NEAT::PrintGene(Gene x) {
	if (x.Connection.size() == 0) {
		cout << "No weights in this gene." << endl;
	}
	for (int i = 0; i < x.Connection.size(); i++) {
		if(x.Connection[i].enabled)
		cout << x.Connection[i].from << "->" << x.Connection[i].to << ": " << x.Connection[i].weight << endl;
	}
}

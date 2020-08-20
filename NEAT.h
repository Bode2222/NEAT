#pragma once
#include "NEATGameInterface.h"
#include <vector>
#include <utility> //pair
#include <unordered_map>
#include <map>//Unordered map cant use pairs for some reason. for details check https://www.geeksforgeeks.org/how-to-create-an-unordered_map-of-pairs-in-c/
#include <iostream>
#include <set>
#include <algorithm>//sort
#include <time.h>//For random
#include <cstdlib>//for random
#include <chrono>
#include <thread>
using namespace std;


struct Allele {
	int from, to, innovationIndex;
	float weight;
	bool enabled;
};

struct Gene {
	Gene() {};
	Gene(int x) { numOfNeurons = x; };
	//Starts from 1
	int numOfNeurons;
	vector<Allele> Connection;
};

struct Neuron {
	bool checked = false;
	float value = 0.f;
	unordered_map<Neuron*, float> weights;
	float calcValue();
private:
	void ActivateValue();
};

//Notes for converging as quickly as possible: 
//The more agents you use the better, If the answer config isnt too complicated it should be found within the first generationns,\
as generations go on more weights and neurons are added, so complex answers are found towards the end
//The rate at which neurons spawn affects the number of generations it takes to find a solution.\
If neurons spawn too quickly, it might not look through enough weights to see if that configuration is viable before scrapping that config and adding another neuron.\
Also If a solution requires a lot of nodes, and nodes dont spawn quickly enough, it'll take forever.\
//More agents in a generation allows you to explore more configurations with similar amounts of neurons/weights, while more generations\
Allows you to explore more complex configurations of neurons/weights
//The weights you give to c1, c2 and c3 (excess/disjoint neurons and difference of weights) affect speciation, and how agents are divided into species\
Affects their convergence speed. If you have some idea of what the final net should look like, make sure a species evolves in that direction by tweaking these\
values
//having Many species slow down the program
//Make of this what you will I wish you good luck!
class NEAT
{
	//Rate at which we play a game
	float frameRate = 30; //Hz

	//Mutates a gene
	Gene Mutate(Gene x);
	//cout's a gene to the console
	void PrintGene(Gene a);
	//Combines 2 agents. Most fit Agent first
	Gene CrossGene(Gene a, Gene b);
	//Compares 2 genes using c1, c2 and c3. returns a value representing how different they are
	float CompareGene(Gene a, Gene b);
	//Combines 2 agents of equal fitness
	Gene CrossGeneEqualFitness(Gene a, Gene b);
	//Reads a gene and produces a mapping from neuron index to neurons
	std::unordered_map<int, Neuron> InstantiateNetwork(const Gene& genome);
	//Given map of neurons and their values, and an input, produce the biggest output index
	pair<vector<float>, int> FeedForward(std::unordered_map<int, Neuron>& neurons, const std::vector<float>& input);

	const int maxStepsPerGame = INT_MAX;
	//How much disjoint alleles, excess alleles, and weight differences respectively affect speciation. Speciation affects convergence speed
	const float c1 = 1, c2 = 1, c3 = 0.4f;
	//DisabledChance: Chance that if either parents allele is disabled that it's child allele will be disabled.
	const float disabledChance = 0.75;
	//What % of the population will be killed due to bad performance. Lower values increase diversity in the population
	const float cull = 0.1;
	//Interspecies breed chance
	const float interBreed = 0.001;
	//Chance a new node is added
	const float addNodeChance = 0.0001;
	//Chance a new weight is added, might change for larger num of agents
	const float addWeightChance = 0.05;
	//Chance of Genome Mutation during breeding
	const float mutationChance = 0.8;
	//Chance of genome mutation being pertubation
	const float perturbedChance = 0.9;
	//Chance of genome mutation being randomly reassigned
	const float assignRandomChance = 0.1;
	//Similarity Threshold, change for larger num Of agents
	const float sigmaT = 3;
	//If a species doesnt improve for this amount of generations, it isnt allowed to reproduce
	const int genB4Cull = 15;

	Gene Champion;
	int numOfAgents = 0;
	NEATGameInterface* env;
	//Starts from 1
	int latestInovation = 1;
	map<std::pair<int, int>, int> InnovationList;
public:
	NEAT(NEATGameInterface* e);
	void Train(int NumOfGenerations);
	void PlayGame();
	~NEAT() {};
};


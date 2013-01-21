#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "gate.h"
#include "net.h"

class Circuit
{
public:
	int fanwide;
	int num_gates;
	int num_nets;
	int num_inputs;
	int num_outputs;
	int* outputs;
	int* inputs;
	Gate* gates;
	char* nets;
	int* fanouts;
	int* gatefanouts;


	Circuit(char * filename);
	~Circuit();
	void printNets();
	void printFanouts();
	void printGates();
};

#endif
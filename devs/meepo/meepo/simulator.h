#ifndef SIMULATOR_H
#define SIMULATOR_H
#include "circuit.h"
#include "event.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

class Simulator
{
public:
	Circuit* circ;
	std::vector<Event> eventQueue;
	std::vector<int> gateQueue;

	int timestep;
	char* prev_val_buffer;
	Simulator(char* circuitFile,char* runFile,char* outFile);

	void printEventQueue();
	void printGateQueue();
	void dumpNets(std::ofstream &offs);
};
#endif																								{

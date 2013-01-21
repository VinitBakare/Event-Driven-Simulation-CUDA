#ifndef CIRCUIT_H
#define CIRCUIT_H

enum { AND, OR, NOT, NAND, NOR, XNOR, FLIPFLOP };

struct Gate{
int type;
int in1;
int in2;
int out;
int delay;
};

class Circuit
{
public:
	int num_gates;
	int num_nets;
	int num_inputs;
	int num_outputs;
	struct Gate* gates;

};

#endif
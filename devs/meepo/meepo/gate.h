#ifndef GATE_H
#define GATE_H

enum { AND, OR, NOT, NAND, NOR, XNOR, FLIPFLOP };

class Gate
{
public:
	char type;
	int  in[2];
	int out;
	char val;
	char delay;


};

#endif
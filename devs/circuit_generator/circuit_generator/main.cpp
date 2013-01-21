#include <iostream>
#include <fstream>
#include <time.h>
#include "circuit.h"
#include <vector>
#include <algorithm>
using namespace std;

int main(int argc,char ** argv)
{
	cout << "Generating circuit.." << endl;
	srand(time(NULL));
	int num_types,max_delay,current_net_count;


	Circuit circ;

	num_types = 3;
	max_delay = 5;
	circ.num_gates=atoi(argv[1]);
	circ.num_nets=0;
	circ.num_inputs=atoi(argv[2]);
	circ.num_outputs = atoi(argv[3]);
	current_net_count = circ.num_inputs;
	circ.gates = new struct Gate[circ.num_gates];

	for(int i=0;i<circ.num_gates;i++)
	{
		circ.gates[i].type = rand() % num_types; 
		circ.gates[i].in1=-1;
		circ.gates[i].in2=-1;
		circ.gates[i].out=-1;
		circ.gates[i].delay= (rand() % (max_delay+1))+ 1;
	}
	int selection;
	for(int i=0;i<circ.num_inputs;i++)
	{
randomize:	selection = rand() % circ.num_gates;
		if(circ.gates[selection].in1 == -1 ) circ.gates[selection].in1 = i;
		else if(circ.gates[selection].in2 == -1 ) circ.gates[selection].in2 = i;
		else goto randomize;
	}

	for(int i=0;i<circ.num_gates;i++)
	{
		circ.gates[i].out = current_net_count;
		current_net_count++;

		if(circ.gates[i].type == NOT )
		{
			do{

				if(circ.gates[i].in1 != -1 && circ.gates[i].out != circ.gates[i].in1) circ.gates[i].in2 = circ.gates[i].in1;
				else if(circ.gates[i].in2 != -1 && circ.gates[i].out != circ.gates[i].in2) circ.gates[i].in1 = circ.gates[i].in2;
				else 
				{
					circ.gates[i].in1 = rand() % current_net_count;
					circ.gates[i].in2 = circ.gates[i].in1;
					//current_net_count++;
				}
			}while(circ.gates[i].in1 == circ.gates[i].out);
		}
		else
		{
			if(circ.gates[i].in1 ==-1) 
			{
				do
				{
					circ.gates[i].in1 =rand() % current_net_count;
				}while(  circ.gates[i].in1 == circ.gates[i].out);
				//current_net_count++;
			}
			if(circ.gates[i].in2 ==-1) 
			{
				do
				{
					circ.gates[i].in2 = rand() % current_net_count;
				}while(circ.gates[i].in2 == circ.gates[i].in1 || circ.gates[i].in2 == circ.gates[i].out);
				//current_net_count++;
			}
		}

	}

	/*for(int i=0;i<circ.num_gates;i++)
	{
		circ.gates[i].out = (rand() % (current_net_count-circ.num_inputs))+ circ.num_inputs;
	}*/

	circ.num_nets = current_net_count;
	ofstream outFile(argv[4]);
	outFile << circ.num_gates << " " << circ.num_nets << endl;
	outFile << circ.num_inputs << " " ;
	for(int i=0;i<circ.num_inputs;i++) outFile << i << " ";
	outFile << endl;
	outFile << circ.num_outputs << " ";

	vector<int> outputs;
	
	for(int i=0;i<circ.num_outputs;i++) outputs.push_back( (rand() % (circ.num_nets - circ.num_inputs))+ circ.num_inputs );
	sort(outputs.begin(),outputs.end());

	for(int i=0;i<circ.num_outputs;i++) outFile << outputs[i] << " ";
	outFile << endl;
	for(int i=0;i<circ.num_gates;i++)
	{
		outFile << circ.gates[i].type << " "; 
		outFile << circ.gates[i].in1  << " ";
		outFile << circ.gates[i].in2  << " ";
		outFile << circ.gates[i].out  << " ";
		outFile << circ.gates[i].delay << " ";
		outFile << endl;
	}
	cout << "Circuit generated!" << endl;
	cout << "Gates : " << circ.num_gates << endl;
	cout << "Inputs : " << circ.num_inputs << endl;
	cout << "Outputs : " << circ.num_outputs << endl;
}
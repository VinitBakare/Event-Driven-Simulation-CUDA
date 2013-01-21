#include <iostream>
#include <fstream>
#include "circuit.h"



using namespace std;



Circuit::Circuit(char* filename)
{
	//Process decleration
	cout << "Parsing circuit description from..." << endl;
	cout << "... " << filename << endl;

	//Initial read
	std::ifstream inFile(filename);
	inFile >> this->num_gates >> this->num_nets;
	cout << "Circuit contains " << this->num_gates << " gates."<< endl;
	
	this->gates = new Gate[num_gates];
	this->nets = new  char[num_nets];

	for(int i=0;i<num_nets;i++)
	{
		nets[i]=-1;
	}
	
	inFile >> this->num_inputs;
	cout << "Input vector size in each pass : " << this->num_inputs << endl;
	
	this->inputs = new int[num_inputs];

	
	
	fanwide = (100>(num_gates/10+5))?(num_gates/10+5):(100);
	fanouts = new int[num_nets*fanwide];
	for(int i=0;i< num_nets*fanwide;i++)
	{
		fanouts[i] = -1;
	}

	cout << "Reading input IDs" << endl;
	for(int i=0;i<num_inputs;i++)
	{
		inFile >> inputs[i];
	}
	cout << "Input IDs: " ;
	for(int i=0;i<num_inputs;i++)
	{
		cout << inputs[i] << " ";
	}
	cout << endl;

	inFile >> this->num_outputs;
	cout << "Output vector size in each pass : " << this->num_outputs << endl;

	this->outputs = new int[num_outputs];

	cout << "Reading output IDs" << endl;
	for(int i=0;i<num_outputs;i++)
	{
		inFile >> outputs[i];
	}
	cout << "Output IDs: " ;
	for(int i=0;i<num_outputs;i++)
	{
		cout << outputs[i] << " ";
	}
	cout << endl;

	cout << "Listing gates:"<< endl;
	int type,delay;
	for(int i=0;i<num_gates;i++)
	{
		inFile >> type >> gates[i].in[0] >> gates[i].in[1] >> gates[i].out >> delay;
		gates[i].type = type;
		gates[i].delay = delay;
		gates[i].val=-1;
		
		for(int j=0 ; j< fanwide ;j++)
		{
			if(fanouts[gates[i].in[0]*fanwide+j] == i) break;
			if(fanouts[gates[i].in[0]*fanwide+j] == -1 )
			{
					fanouts[gates[i].in[0]*fanwide+j] = i;
					break;
			}
			if(j == fanwide-1) cout << "Full, increase fanwide value." << endl;

		}

		for(int j=0 ; j< fanwide ;j++)
		{
			if(fanouts[gates[i].in[1]*fanwide+j] == i) break;
			if(fanouts[gates[i].in[1]*fanwide+j] == -1 )
			{
					fanouts[gates[i].in[1]*fanwide+j] = i;
					break;
			}
			if(j == fanwide-1) std::cout << "Full,increase fanwide value." << std::endl;

		}
		
	}
	printGates(); //DEBUG
	// printFanouts(); //DEBUG
	
	//gatefanouts = new int[num_gates*4];
	//std::cout << "gate fanouts" << std::endl;
	//for(int i=0;i<num_gates;i++)
	//{
	//	for(int j=0;j < fanwide;j++)
	//	{
	//		//std::cout << fanouts[gates[i].out*fanwide + j] << " ";
	//		gatefanouts[i*fanwide + j] = fanouts[gates[i].out*fanwide + j];
	//	}
	//	std::cout << std::endl;
	//}
	//for(int i=0;i<num_gates;i++)
	//{
	//	for(int j=0;j < fanwide;j++)
	//	{
	//		std::cout << gatefanouts[i*fanwide+j] << " ";
	//	}
	//	std::cout << std::endl;
	//}
	
	//std::cout << nets[6].fanout[0] << std::endl;


}

void Circuit::printNets()
{
	std::cout <<"nets: ";
	for(int i=0;i<num_nets;i++)
	{
		std::cout << (int)nets[i] << " ";
	}
	std::cout << std::endl;
}

void Circuit::printFanouts()
{
	cout << "Printing fanouts: " << endl;
	for(int i=0;i<num_nets;i++)
	{
		
		cout <<"net " << i <<" : " ;
		for(int j=0;j<fanwide;j++)
		{
			if(fanouts[i*fanwide + j]== -1 && j==0)
			{
				cout << "none";
			}
			if(fanouts[i*fanwide + j]== -1) break;
			cout << fanouts[i*fanwide + j] << " ";
		}
		cout << endl;

	}
}

void Circuit::printGates()
{
	for(int i=0;i<num_gates;i++)
	{
		switch (gates[i].type)
		{
		case AND:
			cout << "AND";
			break;
		case OR:
			cout << "OR";
			break;
		case NOT:
			cout << "NOT";
			break;
		default:
			cout << "indef";
			break;
		}
		cout << "(" << i << ")" ;
		cout << " connected to nets: " << gates[i].in[0] << " , "<< gates[i].in[1] << " ; outnet: " << gates[i].out << " val="<< (int)gates[i].val<< " delay:" << (int)gates[i].delay << endl;
	}
}
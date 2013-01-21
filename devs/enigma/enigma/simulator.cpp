#include "simulator.h"
#include <ctime>
#include <string>
#include <iomanip>

#define OUTPUT_ONLY 1

using namespace std;

std::ostream& field(std::ostream& o)
{
    // usually the console is 80-character wide.
    // divide the line into four fields.
    return o << setw(10) << std::right;
}

Simulator::Simulator(char* circuitFile,char* runFile,char* outFile)
{
	timestep = 0;
	circ = new Circuit(circuitFile);
	std::cout << "Circuit file is read successfully!" << std::endl;
	
	std::ifstream runs(runFile);
	std::ofstream resultdump(outFile);
	resultdump << "Results for " << circuitFile << " with the trace in " <<  runFile << endl;
	dumpNets(resultdump);
	prev_val_buffer = new char[circ->num_inputs];


	//Initial Schedule
	
	cout <<"Initial Scheduling..." << endl;
	int intval;
	for(int i=0 ;i<circ->num_inputs;i++)
	{
		runs >> intval;
		prev_val_buffer[i]=intval;
		eventQueue.push_back(Event(circ->inputs[i],prev_val_buffer[i],timestep));
	}

	printEventQueue();
	cout << "Initial Scheduling is done!" << endl;
	int current;
	clock_t begin,end;
	double elapsed_secs;

	cout << field << "Events "<< field << " Gates" <<field << "Time" << endl;
	while(1)
	{
		begin = clock();
		printEventQueue();// DEBUG
		cout << "\r" << field << eventQueue.size() << " ";
		processEvents();
		circ->printNets();// DEBUG
		printEventQueue();// DEBUG

		sort(gateQueue.begin(),gateQueue.end());
		cout << field << gateQueue.size() << " ";
		printGateQueue();// DEBUG
		processGates();
		dumpNets(resultdump);
		timestep++;
		for(int i=0;i< circ->num_inputs;i++)
		{
			runs >> current;
			if(runs.eof()) break;
			if(prev_val_buffer[i] == current) continue;
			prev_val_buffer[i] = current;
			eventQueue.push_back(Event(circ->inputs[i],prev_val_buffer[i],timestep));
		}
		if(runs.eof() && eventQueue.size()==0 && gateQueue.size()==0 ) break;
		end = clock();
		elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		cout << field << elapsed_secs; 
		system("PAUSE");
	}
	printEventQueue();// DEBUG
	cout << endl;
	
}

void Simulator::dumpNets(ofstream &offs)
{

	if(OUTPUT_ONLY)
	{
		for(int i=0;i<circ->num_outputs;i++)
		{
			if( circ->nets[circ->outputs[i]] == -1) offs << "U ";
			else offs << (int)(circ->nets[circ->outputs[i]]) << " ";
		}
		
	}
	else
	{
		for(int i=0;i<circ->num_nets;i++)
		{
			if( circ->nets[i] == -1) offs << "U ";
			else offs << (int)(circ->nets[i]) << " ";
		}
	}
	offs << endl;
}

void Simulator::printEventQueue()
{
	cout << "Printing EventQueue content:" << endl;
	for(int i=0;i<eventQueue.size();i++)
	{
		cout <<"Event for net " << eventQueue[i].netID << ", val=" << (int)eventQueue[i].value  << " for t=" << eventQueue[i].time << endl;
	}
	if(!eventQueue.size()) cout << "Empty event queue" << endl;
}

void Simulator::printGateQueue()
{
	cout << "Printing GateQueue content:" << endl;
	for(int i=0;i< gateQueue.size();i++)
	{
		cout <<"Gate " << gateQueue[i] << endl;
	}
	if(!gateQueue.size()) cout << "Empty gate queue" << endl;
}



void Simulator::processEvents()
{
	for(int i=0;i < eventQueue.size();i++)
	{
		if(eventQueue[i].time == timestep)
		{
			// set new value
			circ->nets[eventQueue[i].netID] = eventQueue[i].value;

			//schedule fanout gates
			for(int j=0;j < circ->fanwide ; j++)
			{
				if(circ->fanouts[eventQueue[i].netID*circ->fanwide + j] == -1) break;
				
				if( find(gateQueue.begin(),gateQueue.end(), circ->fanouts[eventQueue[i].netID*circ->fanwide + j]) == gateQueue.end() ) 
				{
					//cout << "Gate " << circ->fanouts[eventQueue[i].netID*circ->fanwide + j] << " is scheduled" << endl; //DEBUG
					gateQueue.push_back( circ->fanouts[eventQueue[i].netID*circ->fanwide + j]);
				}
				else
				{
					//cout << "!!!Reschedule ignored for Gate " << circ->fanouts[eventQueue[i].netID*circ->fanwide + j] <<  endl; //DEBUG
				}
			}
		}
	}
	//remove_if(eventQueue.begin(),eventQueue.end(),currentEvent);
	//removing proccessed events
	while(1)
	{
		int dex=0;
		for( ;dex<eventQueue.size();dex++)
		{
			if(eventQueue[dex].time==timestep)
			{
					eventQueue.erase(eventQueue.begin()+dex);
					break;
			}
		}
		if(dex==eventQueue.size()) break;
	}
}


int Simulator::simulateGate(int gateID)
{
	int newVal;
	switch(circ->gates[gateID].type)
	{
	case AND:
		newVal = circ->nets[circ->gates[gateID].in[0]] & circ->nets[circ->gates[gateID].in[1]];
		break;
	case OR:
		newVal = circ->nets[circ->gates[gateID].in[0]] | circ->nets[circ->gates[gateID].in[1]];
		break;
	case NOT:
		newVal = (circ->nets[circ->gates[gateID].in[0]])?(0):(1);
			break;
	default:
		cout << "WARNING! indef gate encountered in simulation" << endl;
		break;
	}
	if(newVal!=circ->gates[gateID].val)
	{
		circ->gates[gateID].val = newVal;
		return newVal;
	}
	else return -1;
}

void Simulator::processGates()
{
	int simRes;
	for(int i=0;i<gateQueue.size();i++)
	{
		//TODO simulate gate
		simRes = simulateGate(gateQueue[i]);
		if(simRes!=-1)
		{
			eventQueue.push_back(Event(circ->gates[gateQueue[i]].out,simRes,timestep+circ->gates[gateQueue[i]].delay));
		}
	}
	gateQueue.clear();
}

////Initial Schedule
	//for(int i=0; i< circ->num_inputs; i++)
	//{
	//	//TODO schedule for circ->inputs[i]
	//	runs >> prev_val_buffer[i];
	//	
	//	//TODO multidelay impl
	//	//circ->nets[circ->inputs[i]].val = prev_val_buffer[i];
	//	for(int k=0;k<circ->fanwide;k++)
	//	{
	//		if( circ->fanouts[circ->inputs[i]*circ->fanwide+ k] == -1) break;
	//		std::cout << "gate scheduled: " << circ->fanouts[circ->inputs[i]*circ->fanwide + k] << " with " << prev_val_buffer[i] << std::endl;
	//		if( std::find(scheduled.begin(),scheduled.end(), circ->fanouts[circ->inputs[i]*circ->fanwide + k])!= scheduled.end() ) 
	//		{
	//			std::cout <<" already scheduled for the gate, not pushing again!" <<std::endl;
	//		}
	//		else scheduled.push_back(circ->fanouts[circ->inputs[i]*circ->fanwide + k]);
	//	}
	//}

	////Changing inputs
	//int current;
	//while(1)
	//{
	//	
	//	for(int i=0; i< circ->num_inputs; i++)
	//	{
	//		runs >> current ;
	//		//TODO multidelay impl
	//		//circ->nets[circ->inputs[i]].val = current;
	//		if(runs.eof()) break;
	//		if(prev_val_buffer[i] != current) 
	//		{
	//			for(int k=0;k<circ->fanwide;k++)
	//			{
	//				if( circ->fanouts[circ->inputs[i]*circ->fanwide + k] == -1) break;
	//				std::cout << "gate scheduled: " << circ->fanouts[circ->inputs[i]*circ->fanwide+ k] << " with " << current << std::endl;
	//				if( std::find(scheduled.begin(),scheduled.end(), circ->fanouts[circ->inputs[i]*circ->fanwide + k])!= scheduled.end() ) 
	//				{
	//					std::cout <<" already scheduled for the gate, not pushing again!" <<std::endl;
	//				}
	//				else scheduled.push_back(circ->fanouts[circ->inputs[i]*circ->fanwide + k]);

	//			}
	//			prev_val_buffer[i] = current;
	//		}
	//	}
	//	if(runs.eof()) break;

	//	circ->writeNets();
	//}

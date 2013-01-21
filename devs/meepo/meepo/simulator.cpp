#include "simulator.h"
#include <ctime>
#include <string>
#include <iomanip>

#define OUTPUT_ONLY 1

using namespace std;

void initCUDA(Circuit *circ);
void cleanCUDA();
void processEventsCUDA(vector<Event> &eventQueue,vector<int> &gateQueue,Circuit * circ,int timestep);
void processGatesCUDA(vector<int> &gateQueue,vector<Event> &eventQueue,Circuit *circ,int timestep);

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
	cout << "Circuit file is read successfully!" << endl;
	
	initCUDA(circ);
	cout << "CUDA initialization is done!" << endl;

	std::ifstream runs(runFile);
	std::ofstream resultdump(outFile);
	resultdump << "Results for " << circuitFile << " with the trace in " <<  runFile << endl;
	dumpNets(resultdump);
	
	//gateTable = new int[circ->num_gates];
	//netTable = new int[circ->num_nets*2];
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

		//printEventQueue(); // DEBUG
		cout << "\r" << field << eventQueue.size() << " ";
		if(eventQueue.size())
			processEventsCUDA(eventQueue,gateQueue,circ,timestep);
		//circ->printNets(); // DEBUG
		//printEventQueue(); // DEBUG

		sort(gateQueue.begin(),gateQueue.end());
		cout << field <<  gateQueue.size() << " ";
		//printGateQueue(); // DEBUG

		if(gateQueue.size()) 
			processGatesCUDA(gateQueue,eventQueue,circ,timestep);
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
		//system("PAUSE"); //DEBUG
	}
	 //printEventQueue(); // DEBUG

	cout << endl;
	cleanCUDA();

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








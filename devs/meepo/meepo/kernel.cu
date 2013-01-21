
#include "cuda_runtime.h"

#include "device_launch_parameters.h"
#include "../cuda_be/book.h"
#include "circuit.h"
#include "event.h"
#include <vector>

#define THREADS (512)



#include <stdio.h>


 Gate *dev_gates;
 char *dev_nets;
 int *dev_fanouts;
 int *dev_inputs;
 char *dev_GateTable;
 int *dev_EventTable;
 Event *dev_eventsQ;
 int *dev_gatesQ;


 using namespace std;

void processEventsCUDA(vector<Event> &eventQueue,vector<int> &gateQueue,Circuit * circ,int timestep);
void processGatesCUDA(vector<int> &gateQueue,vector<Event> &eventQueue,Circuit * circ,int timestep);
__global__ void eventKernel(Event* dev_events,char* dev_nets,int *dev_fanouts,char* dev_GateTable,int currentTime,int num_gates,int fanwide,int num_events);
__global__ void cleanGateTable(char * dev_GateTable,int num_gates);
__global__ void simulateGatesKernel(int* gatesQ,Gate* dev_gates,char* dev_nets,char* simRes,int Qsize);

void processGatesCUDA(vector<int> &gateQueue,vector<Event> &eventQueue,Circuit *circ, int timestep)
{
	//cout <<"Hendek "<< endl;
	char* simRes;
	char* dev_simRes;
	simRes = new char[gateQueue.size()];
	for(int i=0;i<gateQueue.size();i++) simRes[i]=-1;
	
	HANDLE_ERROR(
		cudaMalloc((void**)&dev_simRes,gateQueue.size()*sizeof(char)));
	HANDLE_ERROR(
		cudaMemcpy(dev_simRes,simRes,gateQueue.size()*sizeof(char),cudaMemcpyHostToDevice));
	HANDLE_ERROR(
		cudaMalloc((void**)&dev_gatesQ,gateQueue.size()*sizeof(int)));
	HANDLE_ERROR(
		cudaMemcpy(dev_gatesQ,&gateQueue[0],gateQueue.size()*sizeof(int),cudaMemcpyHostToDevice));




	////TODO simulate gate
	simulateGatesKernel<<<gateQueue.size()/THREADS+1,THREADS>>>(dev_gatesQ,dev_gates,dev_nets,dev_simRes,gateQueue.size());

	HANDLE_ERROR(
		cudaMemcpy(simRes,dev_simRes,gateQueue.size()*sizeof(char),cudaMemcpyDeviceToHost));

	for(int i=0;i<gateQueue.size();i++)
	{
		//TODO simulate gate
		//simRes = simulateGate(gateQueue[i]);
		if(simRes[i]!=-1)
		{
			eventQueue.push_back(Event(circ->gates[gateQueue[i]].out,simRes[i],timestep+circ->gates[gateQueue[i]].delay));
		}
	}
	gateQueue.clear();
	cudaFree(dev_simRes);
	cudaFree(dev_gatesQ);
}

__global__ void simulateGatesKernel(int* dev_gatesQ,Gate* dev_gates,char* dev_nets,char *dev_simRes,int Qsize)
{
	char newVal;
	int gateID;
	int i = threadIdx.x + blockIdx.x * blockDim.x;
	if(i<Qsize)
	{
		gateID = dev_gatesQ[i];
		switch(dev_gates[gateID].type)
		{
		case AND:
			newVal = dev_nets[dev_gates[gateID].in[0]] & dev_nets[dev_gates[gateID].in[1]];
			break;
		case OR:
			newVal = dev_nets[dev_gates[gateID].in[0]] | dev_nets[dev_gates[gateID].in[1]];
			break;
		case NOT:
			newVal = (dev_nets[dev_gates[gateID].in[0]])?(0):(1);
			break;
		default:
			newVal=-2; // ERROR INDEF
			break;
		}
		if(newVal!=dev_gates[gateID].val)
		{
			dev_gates[gateID].val = newVal;
			dev_simRes[i] = newVal;
		}
		else dev_simRes[i] = -1;
	}
}

 void processEventsCUDA(vector<Event> &eventQueue,vector<int> &gateQueue,Circuit * circ,int timestep)
{
	// cout << "CUDA event processing..." << endl; //DEBUG
	char* gatesToBeScheduled =new char[circ->num_gates];

	cleanGateTable<<<((circ->num_gates)/THREADS)+1,THREADS>>>(dev_GateTable,circ->num_gates);
	
	cudaFree(dev_eventsQ);

	HANDLE_ERROR(
		cudaMalloc((void**)&dev_eventsQ,eventQueue.size()*sizeof(Event)));
	
	HANDLE_ERROR(
		cudaMemcpy(dev_eventsQ,&eventQueue[0],eventQueue.size()*sizeof(Event),cudaMemcpyHostToDevice));


	eventKernel<<<(eventQueue.size()/32)+1,32>>>(dev_eventsQ,dev_nets,dev_fanouts,dev_GateTable,
											 timestep,circ->num_gates,circ->fanwide,eventQueue.size());

	
	HANDLE_ERROR(
		cudaMemcpy(gatesToBeScheduled,dev_GateTable,(circ->num_gates)*sizeof(char),cudaMemcpyDeviceToHost));

	HANDLE_ERROR(
		cudaMemcpy(circ->nets,dev_nets,(circ->num_nets)*sizeof(char),cudaMemcpyDeviceToHost));

	
	//TODO pushback to GATEqueue
	for(int i=0;i<circ->num_gates;i++)
	{
		if(gatesToBeScheduled[i]==1)
			gateQueue.push_back(i);
	}

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

	
	//cudaFree(dev_events);
	//cudaDeviceSynchronize();
}

__global__ void cleanGateTable(char *gateTable,int num_gates)
{
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	if(x<num_gates)
		gateTable[x]=0;

}

__global__ void eventKernel(Event* dev_events,char* dev_nets,int *dev_fanouts,char* dev_GateTable,int currentTime,int num_gates,int fanwide,int num_events)
{
	int i = threadIdx.x + blockIdx.x * blockDim.x;
	if(i<num_events)
	{
		if(dev_events[i].time == currentTime)
		{
			dev_nets[dev_events[i].netID] = dev_events[i].value;

			for(int k=0;k<fanwide;k++)
			{
				if( dev_fanouts[(dev_events[i].netID)*fanwide+k ] ==-1) break;
				//TODO solve it!
				dev_GateTable[dev_fanouts[(dev_events[i].netID)*fanwide+k]] = 1;
				//dev_GateTable[0]=1;
			}
		}
	}
}

__host__ void initCUDA(Circuit *circ)
{
	HANDLE_ERROR(
		cudaMalloc((void**)&dev_gates,sizeof(Gate)*circ->num_gates));
	HANDLE_ERROR(
		cudaMemcpy(dev_gates,circ->gates,sizeof(Gate)*circ->num_gates,cudaMemcpyHostToDevice));

	HANDLE_ERROR(
		cudaMalloc((void**)&dev_nets,sizeof(int)*circ->num_nets));
	HANDLE_ERROR(
		cudaMemcpy(dev_nets,circ->nets,circ->num_nets*sizeof(int),cudaMemcpyHostToDevice));

	HANDLE_ERROR(
		cudaMalloc((void**)&dev_fanouts,sizeof(int)*(circ->num_nets)*(circ->fanwide)));
	HANDLE_ERROR(
		cudaMemcpy(dev_fanouts,circ->fanouts,sizeof(int)*(circ->num_nets)*(circ->fanwide),cudaMemcpyHostToDevice));

	HANDLE_ERROR(
		cudaMalloc((void**)&dev_GateTable,(circ->num_gates)*sizeof(char)));


	//HANDLE_ERROR(
	//	cudaMalloc((void**)&dev_events,10*sizeof(Event)));
	//HANDLE_ERROR(
	//	cudaMalloc((void**)&dev_gatesQ,sizeof(int)*circ->num_gates));
	
}



__host__ void cleanCUDA()
{
	cudaFree(dev_gates);
	cudaFree(dev_eventsQ);
	cudaFree(dev_nets);
	cudaFree(dev_fanouts);
	cudaFree(dev_GateTable);
	cudaFree(dev_gatesQ);
}



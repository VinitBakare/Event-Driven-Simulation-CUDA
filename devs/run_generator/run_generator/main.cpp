#include <iostream>
#include <fstream>
#include <time.h>

using namespace std;

int main(int argc,char** argv)
{
	int num_gates,num_nets,num_inputs,num_runs;
	ifstream inFile(argv[1]);
	ofstream outFile(argv[3]);
	num_runs= atoi(argv[2]);
	srand(time(NULL));

	inFile >> num_gates >> num_nets >> num_inputs;

	cout << "Gates : " << num_gates << " Nets : " << num_nets << " Inputs : " << num_inputs << endl;
	cout << "Generating simulation run..." << endl;

	for(int i=0 ;i<num_runs;i++)
	{
		for(int j=0;j<num_inputs;j++)
		{
			outFile << (rand() % 2) << " ";
		}
		outFile << endl;
	}
	cout << argv[3] << " run file generated successfully" << endl;
	return 0;
	
}
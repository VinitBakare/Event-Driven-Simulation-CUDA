#include <iostream>
#include "simulator.h"
#include <ctime>

using namespace std;

int main()
{
	 std::cout << "Hello World!" << std::endl;
	 //Circuit* c1 = new Circuit("demo1.cir");

	 clock_t begin = clock();
	 Simulator* sim = new Simulator("./../../inputs_outputs/demo1.cir","./../../inputs_outputs/demo1_d.run","./../../inputs_outputs/demo1.out");
	 clock_t end = clock();

	 double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

	 cout << "Simulation time : " << elapsed_secs << endl;
	 cout << "Simluation steps: " << sim->timestep << endl;
	 cout << "Simulation time per timestep :" << elapsed_secs/(sim->timestep) << endl;
	 return 0;
}
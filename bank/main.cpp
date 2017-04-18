#include <iostream>
#include <limits>
#include "queue_system.h"

using namespace std;
using namespace bank;


int main()
{
	duration total_service_time(240);
	size_t window_num = 4;
	size_t simulate_num = 1000000;

	bank::queue_system qs(total_service_time, window_num);
	qs.simulate(simulate_num);

	cout.precision(numeric_limits<double>::max_digits10);

	cout << "Average time of customers stayed in bank: " << fixed << qs.get_average_stay_time() << endl;
	cout << "Average customer arrived bank per minute: " << fixed << qs.get_average_customers() << endl;

	getchar();
}

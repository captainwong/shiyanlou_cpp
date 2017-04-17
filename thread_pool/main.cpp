#include "thread_pool.h"
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>

using namespace std;

int main()
{
	{
		thread_pool pool(4);
		vector<future<string>> results;

		for (int i = 0; i < 8; i++) {
			results.emplace_back(pool.enqueue([i]() {
				stringstream ss;
				ss << this_thread::get_id() << " hello " << i << endl;
				cout << ss.str();
				ss.str(""); ss.clear();

				this_thread::sleep_for(chrono::seconds(1));

				ss << this_thread::get_id() << " world " << i << endl;
				cout << ss.str();
				ss.str(""); ss.clear();

				ss << this_thread::get_id() << " finished " << i << endl;;
				return ss.str();
			}));
		}

		for (auto&& result : results) {
			cout << result.get() << endl;
		}

		/*for (auto iter = results.begin(); iter != results.end(); ) {
			if (future_status::ready == iter->wait_for(chrono::milliseconds(500))) {
				cout << iter->get();
				iter = results.erase(iter);
				continue;
			}

			this_thread::sleep_for(chrono::milliseconds(100));
		}*/

	}

	system("pause");
}
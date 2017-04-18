#pragma once
#include <list>
#include <vector>
#include <queue>
#include <random>
#include "bank_global.h"
#include "bank_event.h"
#include "service_window.h"

namespace bank {

class queue_system
{
private:
	time_point service_stop_time_ = {};
	duration total_service_time_ = {};
	duration total_customer_stay_time_ = {};
	double average_customers_ = 0.0;
	double average_stay_time_ = 0.0;
	size_t total_customer_num_ = 0;

	std::vector<service_window_ptr> windows_ = {};
	std::queue<customer_ptr> customer_queue_ = {};
	std::list<event_ptr> event_list_ = {};
	event_ptr cur_event_ = {};

	std::mt19937 random_engine_;

public:

	explicit queue_system(const duration& total_service_time, size_t window_num);
	~queue_system() = default;

	void simulate(size_t simulate_num);

	double get_average_customers() const { return average_customers_; }
	double get_average_stay_time() const { return average_stay_time_; }

protected:

	duration run();

	void init();

	void end();

	service_window_ptr get_idle_service_window();

	void customer_arrive();

	void customer_departure();



};

};

#include "queue_system.h"
#include <random>
#include <cassert>

namespace bank {

auto event_list_sort_func = [](const event_ptr& e1, const event_ptr& e2) {
	return e1->occur_time_ < e2->occur_time_;
};

queue_system::queue_system(const duration& total_service_time, size_t window_num)
	: total_service_time_(total_service_time)
	, random_engine_()
{
	for (size_t i = 0; i < window_num; i++) {
		windows_.emplace_back(std::make_shared<service_window>(i));
	}
}

void queue_system::simulate(size_t simulate_num)
{
	service_stop_time_ = clock::now() + total_service_time_;

	duration dr = {};
	for (size_t i = 0; i < simulate_num; i++) {
		dr += run();
	}

	average_stay_time_ = dr.count() / simulate_num;
	average_customers_ = total_customer_num_ * 1.0 / (total_service_time_.count() * simulate_num);
}

duration queue_system::run()
{
	init();

	while (cur_event_) {
		switch (cur_event_->event_type_) {
		case event_type::arrive:
			customer_arrive();
			break;

		case event_type::departure:
			customer_departure();
			break;

		default:
			assert(0);
			break;
		}

		if (event_list_.empty()) {
			cur_event_.reset();
		} else {
			cur_event_ = event_list_.front();
			event_list_.pop_front();
		}
	}

	end();

	return total_customer_stay_time_ / total_customer_num_;
}

void queue_system::init()
{
	std::uniform_int_distribution<size_t> ud(0, 99);
	auto tp = clock::now() + duration(ud(random_engine_));
	cur_event_ = std::make_shared<bank_event>(tp, arrive, 0);
}

void queue_system::end()
{
	for (auto& window : windows_) {
		window->set_idle();
	}

	//assert(customer_queue_.empty());
	while (!customer_queue_.empty()) {
		customer_queue_.pop();
	}

	assert(event_list_.empty());
}

service_window_ptr queue_system::get_idle_service_window()
{
	for (auto& window : windows_) {
		if (window->is_idle()) {
			return window;
		}
	}

	return service_window_ptr();
}

void queue_system::customer_arrive()
{
	total_customer_num_++;

	std::uniform_int_distribution<size_t> ud(0, 99);

	auto timepoint = cur_event_->occur_time_ + duration(ud(random_engine_));

	if (timepoint < service_stop_time_) {
		event_list_.emplace_back(std::make_shared<bank_event>(timepoint, event_type::arrive, 0));
	}

	auto _customer = std::make_shared<customer>(cur_event_->occur_time_, duration(ud(random_engine_)));
	customer_queue_.emplace(_customer);

	auto window = get_idle_service_window();
	if (window) {
		_customer = customer_queue_.front();
		customer_queue_.pop();

		window->serve(_customer);
		window->set_busy();

		// departure event
		auto customer_departure_time = cur_event_->occur_time_ + _customer->duration_;
		event_list_.emplace_back(std::make_shared<bank_event>(customer_departure_time, event_type::departure, window->get_id()));
		event_list_.sort(event_list_sort_func);
	}
}

void queue_system::customer_departure()
{
	if (cur_event_->occur_time_ < service_stop_time_) {
		assert(cur_event_->window_id_ < windows_.size());
		auto customer_arrive_time = windows_[cur_event_->window_id_]->get_customer_arrive_time();
		auto customer_stay_time = cur_event_->occur_time_ - customer_arrive_time;
		total_customer_stay_time_ += std::chrono::duration_cast<duration>(customer_stay_time);

		if (!customer_queue_.empty()) {
			auto _customer = customer_queue_.front();
			customer_queue_.pop();
			windows_[cur_event_->window_id_]->serve(_customer);

			// departure event
			auto customer_departure_time = cur_event_->occur_time_ + _customer->duration_;
			event_list_.emplace_back(std::make_shared<bank_event>(customer_departure_time, event_type::departure, cur_event_->window_id_));
			event_list_.sort(event_list_sort_func);
		} else {
			windows_[cur_event_->window_id_]->set_idle();
		}
	}
}

};

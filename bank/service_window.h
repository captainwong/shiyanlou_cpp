#pragma once
#include "bank_global.h"
#include "custumer.h"

namespace bank {

class service_window
{
public:
	enum status {
		busy,
		idle,
	};

private:
	status status_ = idle;

	customer_ptr customer_ = {};

	size_t id_ = 0;

public:

	explicit service_window(size_t id) : id_(id) {}

	bool is_idle() const { return status_ == idle; }

	size_t get_id() const { return id_; }

	void serve(const customer_ptr& customer) {
		customer_ = customer;
	}

	void set_busy() { status_ = busy; }

	void set_idle() { status_ = idle; }

	time_point get_customer_arrive_time() const {
		return customer_->arrive_time_;
	}

	duration get_customer_duratoin() const {
		return customer_->duration_;
	}

};

using service_window_ptr = std::shared_ptr<service_window>;

};

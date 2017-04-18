#pragma once
#include "bank_global.h"

namespace bank {

class customer
{
public:

	//protected:
	time_point arrive_time_ = {};
	duration duration_ = {};

public:
	explicit customer(const time_point& arrive_time,
					  const duration& duration)
		: arrive_time_(arrive_time)
		, duration_(duration)
	{}
};

using customer_ptr = std::shared_ptr<customer>;

};

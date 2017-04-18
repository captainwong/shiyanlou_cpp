#pragma once
#include "bank_global.h"

namespace bank {

enum event_type {
	arrive,
	departure,
};

struct bank_event
{
	time_point occur_time_ = {};
	event_type event_type_ = arrive;
	size_t window_id_ = 0;

	explicit bank_event(const time_point& occur_time, event_type et, size_t window_id)
		: occur_time_(occur_time)
		, event_type_(et)
		, window_id_(window_id)
	{}
};

using event_ptr = std::shared_ptr<bank_event>;

};

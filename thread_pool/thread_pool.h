#pragma once
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <queue>
#include <future>
#include <memory>
#include <stdexcept>
#include <functional>
#include <utility>
#include <condition_variable>


class thread_pool
{
public:
	thread_pool(size_t thread_num);

	template<typename F, typename... Args>
	auto enqueue(F&& f, Args&&... args)
		->std::future<typename std::result_of<F(Args...)>::type>;

	~thread_pool();

private:
	std::vector<std::thread> workers_ = {};
	std::queue<std::function<void()>> tasks_ = {};
	std::mutex mutex_ = {};
	std::condition_variable cv_ = {};
	bool running_ = true;

};

inline thread_pool::thread_pool(size_t thread_num)
{
	for (size_t i = 0; i < thread_num; i++) {
		workers_.emplace_back([this]() {
			while (true) {
				std::function<void()> task;

				{
					std::unique_lock<std::mutex> ul(this->mutex_);
					this->cv_.wait(ul, [this]() {
						return !this->running_ || !this->tasks_.empty();
					});

					if (!this->running_ && this->tasks_.empty()) {
						return;
					}

					task = std::move(this->tasks_.front());
					this->tasks_.pop();
				}

				task();
			}
		});
	}
}

inline thread_pool::~thread_pool()
{
	{
		std::unique_lock<std::mutex> ul(mutex_);
		running_ = false;
	}

	cv_.notify_all();

	for (auto& worker : workers_) {
		worker.join();
	}
}

template<class F, class... Args>
auto thread_pool::enqueue(F&& f, Args&&... args)
	->std::future<typename std::result_of<F(Args...)>::type>
{
	using return_type = typename std::result_of<F(Args...)>::type;

	auto task = std::make_shared<std::packaged_task<return_type()>>(
		std::bind(std::forward<F>(f),
				  std::forward<Args>(args)...)
		);

	auto result = task->get_future();

	{
		std::unique_lock<std::mutex> ul(mutex_);

		if (!running_) {
			throw std::runtime_error("enqueue on stopped thread_pool");
		}

		tasks_.emplace([task]() {(*task)(); });
	}

	cv_.notify_one();

	return result;
}

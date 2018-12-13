#ifndef THREAD_SAFE_HPP
#define THREAD_SAFE_HPP

#include <mutex>
#include <string>
#include "expression.hpp"
#include <condition_variable>

class inputCommunication {
public:
	void store_input(std::string);
	std::string get_input();
	bool empty();
private:
	mutable std::mutex input_mutex;
	std::string input_string;
	std::condition_variable input_ready;
	bool input_is_ready = false;
};


class outputCommunication {
public:
	void store_output(std::pair<std::string, Expression>);
	std::pair<std::string, Expression> get_output();
	std::pair<std::string, Expression> try_get_output();
	bool empty();
private:
	mutable std::mutex output_mutex;
	std::condition_variable output_ready;
	std::pair<std::string, Expression> output;
	bool output_is_ready = false;
};


class env_mqueue {
public:
	void store_interrupt(bool value);
	bool interrupt_value();
private:
	mutable std::mutex env_mut;
	bool interrupt = false;
};
#endif // !THREAD_SAFE_HPP

#include "thread_safe.hpp"


std::string inputCommunication::get_input() {
	std::unique_lock<std::mutex> lock(input_mutex);
	while(!input_is_ready){
		input_ready.wait(lock);
	}
	input_is_ready = false;
	return input_string;
}

bool inputCommunication::empty()
{
	std::lock_guard<std::mutex> lock(input_mutex);
	return input_string.empty();
}

void inputCommunication::store_input(std::string in) {
	std::unique_lock<std::mutex> lock(input_mutex);
	input_string = in;
	input_is_ready = true;
	input_ready.notify_one();
}

std::pair<std::string, Expression> outputCommunication::get_output() {
	std::unique_lock<std::mutex> lock(output_mutex);
	while (!output_is_ready) {
		output_ready.wait(lock);
	}
	output_is_ready = false;
	return output;
}

std::pair<std::string, Expression> outputCommunication::try_get_output()
{
	std::unique_lock<std::mutex> lock(output_mutex);
	if (!output_is_ready) {
		return std::pair<std::string, Expression>("", Expression());
	}
	output_is_ready = false;
	return output;
}

bool outputCommunication::empty()
{
	std::lock_guard<std::mutex> lock(output_mutex);
	return (output.first.empty() || output.second.head().isNone());
}

void outputCommunication::store_output(std::pair<std::string, Expression> out) {
	std::unique_lock<std::mutex> lock(output_mutex);
	output = out;
	output_is_ready = true;
	output_ready.notify_one();
}

void env_mqueue::store_interrupt(bool value) {
	std::lock_guard<std::mutex> lock(env_mut);
	interrupt = value;
}

bool env_mqueue::interrupt_value() {
	std::lock_guard<std::mutex> lock(env_mut);
	return interrupt;
}
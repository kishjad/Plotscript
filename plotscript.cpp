#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>

#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "startup_config.hpp"
#include "thread_safe.hpp"

#include <csignal>
#include <cstdlib>
#include <atomic>
static std::atomic_bool interrupt(false);

volatile sig_atomic_t global_status_flag = 0;
// *****************************************************************************
// install a signal handler for Cntl-C on Windows
// *****************************************************************************
#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>

// this function is called when a signal is sent to the process
BOOL WINAPI interrupt_handler(DWORD fdwCtrlType) {

	switch (fdwCtrlType) {
	case CTRL_C_EVENT: // handle Cnrtl-C
										 // if not reset since last call, exit
		if (global_status_flag > 0) {
			exit(EXIT_FAILURE);
		}
		++global_status_flag;
		return TRUE;

	default:
		return FALSE;
	}
}

// install the signal handler
inline void install_handler() { SetConsoleCtrlHandler(interrupt_handler, TRUE); }
// *****************************************************************************

// *****************************************************************************
// install a signal handler for Cntl-C on Unix/Posix
// *****************************************************************************
#elif defined(__APPLE__) || defined(__linux) || defined(__unix) ||             \
    defined(__posix)
#include <unistd.h>

// this function is called when a signal is sent to the process
void interrupt_handler(int signal_num) {

	if (signal_num == SIGINT) { // handle Cnrtl-C
															// if not reset since last call, exit
		if (global_status_flag > 0) {
			exit(EXIT_FAILURE);
		}
		++global_status_flag;
	}
}

// install the signal handler
inline void install_handler() {

	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = interrupt_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);
}
#endif

void error(const std::string & err_str) {
	std::cerr << "Error: " << err_str << std::endl;
}

void interpretation(inputCommunication & ins, outputCommunication &out) {
	
	Interpreter interp;
	/*env_mqueue new_queue;
	interp.send_signal(&new_queue);*/
	std::ifstream ifs(STARTUP_FILE);
	if (!ifs) {
		error("Could not open file for reading.");
	}
	if (!interp.parseStream(ifs)) {
		error("Error: Invalid Expression. Could not parse.");
	}
	else {
		try {
			Expression exp = interp.evaluate();
		}
		catch (const SemanticError & ex) {
			std::cerr << ex.what() << std::endl;
		}
	}
	
	while (1) {
		auto user_input = ins.get_input();
		if (user_input == "%stop") {
			break;
		}
		else {
			std::istringstream expression(user_input);
			if (!interp.parseStream(expression)) {
				//error("Invalid Expression. Could not parse.");
				out.store_output(std::pair<std::string, Expression>("Error: Invalid Expression. Could not parse.", Expression()));
			}
			else {
				try {
					Expression exp = interp.evaluate();
					out.store_output(std::pair<std::string, Expression>("", exp));
				}
				catch (const SemanticError & ex) {
					out.store_output(std::pair<std::string, Expression>(ex.what(), Expression()));
				}
			}
		}
	}
	
}

void prompt(){
  std::cout << "\nplotscript> ";
}

std::string readline(){
  std::string line;
  std::getline(std::cin, line);

  return line;
}

void info(const std::string & err_str){
  std::cout << "Info: " << err_str << std::endl;
}

int eval_from_stream(std::istream & stream){

  Interpreter interp;
  
  if(!interp.parseStream(stream)){
    error("Error: Invalid Program. Could not parse.");
    return EXIT_FAILURE;
  }
  else{
    try{
      Expression exp = interp.evaluate();
      std::cout << exp << std::endl;
    }
    catch(const SemanticError & ex){
      std::cerr << ex.what() << std::endl;
      return EXIT_FAILURE;
    }	
  }

  return EXIT_SUCCESS;
}

int eval_from_file(std::string filename){
      
  std::ifstream ifs(filename);
  
  if(!ifs){
    error("Could not open file for reading.");
    return EXIT_FAILURE;
  }
  
  return eval_from_stream(ifs);
}

int eval_from_command(std::string argexp){

  std::istringstream expression(argexp);

  return eval_from_stream(expression);
}


// A REPL is a repeated read-eval-print loop
void repl(inputCommunication &ins, outputCommunication& out) {
	install_handler();
	
	std::thread t1;
	t1 = std::thread(interpretation, std::ref(ins), std::ref(out));
	
	//User inputs
  while(!std::cin.eof()){
		interrupt.store(false);
    prompt();
    std::string line = readline();
		global_status_flag = 0;
    if(line.empty()) continue;
			//%stop the thread by saving %stop to exit out of the while loop
			//and then joining the thread
			if (line == "%stop") {
				if (t1.joinable()) {
					ins.store_input(line);
					t1.join();
				}
			}
			//%start the thread by checking if the thread is running and then
			//starting it
			else if (line == "%start") {
				if (!t1.joinable()) {
					t1 = std::thread(interpretation,std::ref(ins), std::ref(out));
				}
			}
			else if (line == "%reset") {
				if (t1.joinable()) {
					ins.store_input("%stop");
					t1.join();
					t1 = std::thread(interpretation, std::ref(ins), std::ref(out));
				}
				else {
					t1 = std::thread(interpretation,  std::ref(ins), std::ref(out));
				}
			}
			else if (line == "%exit") {
				if (t1.joinable()) {
					ins.store_input("%stop");
					t1.join();
				}
				exit(EXIT_SUCCESS);
			}

			else if (t1.joinable()) {
				ins.store_input(line);
				std::pair<std::string, Expression> eval_output;

				while (true) {
					
					auto output = out.try_get_output();

					if (global_status_flag > 0) {
						std::cout << "Error: interpreter kernel interrupted\n";
						interrupt.store(true);
						break;
					}

					if (output.first.empty() && output.second.head().isNone()) {
						continue;
					}
					else {
						eval_output = output;
						if (eval_output.first.empty()) {
							std::cout << eval_output.second << std::endl;
						}
						else {
							std::cout << eval_output.first << std::endl;
						}
						break;
					}
				}
			}
			else {
				std::cout << "Error: interpreter kernel not running" << std::endl;
			}
		}
	t1.join();
}

int main(int argc, char *argv[])
{
	inputCommunication ins;
	outputCommunication out;
	
  if(argc == 2){
    return eval_from_file(argv[1]);
  }
  else if(argc == 3){
    if(std::string(argv[1]) == "-e"){
      return eval_from_command(argv[2]);
    }
    else{
      error("Incorrect number of command line arguments.");
    }
  }
  else{
    repl(std::ref(ins),std::ref(out));
  }
}

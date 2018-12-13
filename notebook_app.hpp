#ifndef NOTEBOOK_APP_HPP
#define NOTEBOOK_APP_HPP

#include "output_widget.hpp"
#include "input_widget.hpp"
#include "semantic_error.hpp"
#include "interpreter.hpp"
#include "startup_config.hpp"
#include "expression.hpp"
#include "thread_safe.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <QLayout>
#include <thread>
#include <QPushButton>
#include <QTimer>
#include <atomic>


class NotebookApp : public QWidget
{
	Q_OBJECT
	public:
	NotebookApp(QWidget * parent = nullptr);
	~NotebookApp();

public slots:
	/*receives data from input.hpp with the suer entered string 
	and runs it through interpreter to evaluate it*/
	void evaluate(std::string);
	void start_thread();
	void stop_thread();
	void reset_thread();
	void output_is_ready();
	void interrupted();
	//void timer_event();

signals:
	void sendExpression(std::string);
signals:
	void sendCircle(double, double, double);
signals:
	void sendLine(double, double, double, double, double);
signals:
	void sendText(std::string, double, double, double, double);
signals:
	void sendClear();
signals:
	void sendDiscrete(std::vector<double>, std::vector<double>,std::map<std::string, std::string>);
signals:
	void sendOutput();

protected:
	void timerEvent(QTimerEvent *event);
private:
	/*loadStartupFile loads up the startup file which
	contains the lambda fucntions for make-point, 
	make-line and make-text*/
	void loadStartupFile(Interpreter &interp);
	/*parse expression recursively parses the input
	from the user and calls sendSignal when it reaches
	the lowest level*/
	void parseExpression(Expression &exp);

	/*Send signal parses the input expression to check
	whether it is a line, point, string or other and then
	emits signals accordingly.*/
	void sendSignal(Expression &exp);
	

	inputCommunication input_comms; 

	outputCommunication output_comms;

	/*env_mqueue environment_queue;*/

	Interpreter interp;

	std::thread eval_thread;

	void interpretation(Interpreter &interp);

	int timerID;

  std::thread spawn(Interpreter &interp) {
		return std::thread(&NotebookApp::interpretation, this,std::ref(interp));
	}
};

#endif
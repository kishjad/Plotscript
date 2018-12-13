#include "notebook_app.hpp"

NotebookApp::NotebookApp(QWidget * parent) : QWidget(parent) {
	
	this->setObjectName(QString("NotebookApp"));
	InputWidget* input = new InputWidget(this);
	OutputWidget* output = new OutputWidget(this);
	QPushButton *start = new QPushButton("Start Kernel", this);
	start->setObjectName("start");
	QPushButton *stop = new QPushButton("Stop Kernel", this);
	stop->setObjectName("stop");
	QPushButton *reset = new QPushButton("Reset Kernel", this);
	reset->setObjectName("reset");
	QPushButton *interrupt = new QPushButton("Interrupt", this);
	interrupt->setObjectName("interrupt");
	//Setting Layout
	auto layout = new QVBoxLayout();
	auto buttons = new QHBoxLayout();
	buttons->addWidget(start);
	buttons->addWidget(stop);
	buttons->addWidget(reset);
	buttons->addWidget(interrupt);
	layout->addLayout(buttons);
	layout->addWidget(input);
	layout->addWidget(output);
	setLayout(layout);

	//Signal and slot connection
	connect(input, SIGNAL(generatesignal(std::string)), this, SLOT(evaluate(std::string)));
	connect(this, SIGNAL(sendExpression(std::string)), output, SLOT(showExpression(std::string)));
	connect(this, SIGNAL(sendCircle(double, double, double)), output, SLOT(showCircle(double, double, double)));
	connect(this, SIGNAL(sendLine(double, double, double, double, double)), output, SLOT(showLine(double, double, double, double, double)));
	connect(this, SIGNAL(sendClear()), output, SLOT(showClear()));
	connect(this, SIGNAL(sendText(std::string, double, double, double, double)), output, SLOT(showText(std::string, double, double, double, double)));
	connect(start, SIGNAL(clicked()), this, SLOT(start_thread()));
	connect(stop, SIGNAL(clicked()), this, SLOT(stop_thread()));
	connect(reset, SIGNAL(clicked()), this, SLOT(reset_thread()));
	connect(interrupt, SIGNAL(clicked()), this, SLOT(interrupted()));

	loadStartupFile(interp);
	eval_thread = spawn(interp);
	timerID = QObject::startTimer(10);
}

NotebookApp::~NotebookApp()
{
	if (eval_thread.joinable()) {
		input_comms.store_input("%stop");
		eval_thread.join();
	}
	QObject::killTimer(timerID);
}

void NotebookApp::start_thread() {
	if (!eval_thread.joinable()) {
		interp = Interpreter();
		loadStartupFile(interp);
		eval_thread = spawn(interp);
	}
}

void NotebookApp::stop_thread() {
	if (eval_thread.joinable()) {
		input_comms.store_input("%stop");
		eval_thread.join();
	}
}

void NotebookApp::interrupted() {
	emit sendClear();
	emit sendExpression("Error: interpreter kernel interrupted");
}

void NotebookApp::timerEvent(QTimerEvent *event)
{
	auto eval_output = output_comms.try_get_output();
	if (!eval_output.first.empty() || !eval_output.second.head().isNone()) {
		if (eval_output.first.empty()) {
			emit sendClear();
			parseExpression(eval_output.second);

		}
		else {
			emit sendClear();
			emit sendExpression(eval_output.first);
		}
	}
	else {
		QObject::timerEvent(event);
	}
}

void NotebookApp::reset_thread() {
	if (eval_thread.joinable()) {
		input_comms.store_input("%stop");
		eval_thread.join();
		interp = Interpreter();
		loadStartupFile(interp);
		eval_thread = spawn(interp);
	}
	else {
		interp = Interpreter();
		loadStartupFile(interp);
		eval_thread = spawn(interp);
	}
}

void NotebookApp::output_is_ready()
{
	auto output = output_comms.get_output();
	if (output.first.empty()) {
		emit sendClear();
		parseExpression(output.second);
	}
	else {
		emit sendClear();
		emit sendExpression(output.first);
	}
}


void NotebookApp::evaluate(std::string input)
{
	if (eval_thread.joinable()) {
		input_comms.store_input(input);
	}
	
	else {
		emit sendClear();
		emit sendExpression("Error: interpreter kernel not running");
	}
}

void NotebookApp::loadStartupFile(Interpreter & interp)
{
	std::ifstream ifs(STARTUP_FILE);
	if (!ifs) {
		emit sendClear();
		emit sendExpression("Could not open file for reading.");
	}
	if (!interp.parseStream(ifs)) {
		emit sendClear();
		emit sendExpression("Error: Invalid Program. Could not parse.");
	}
	else {
		try {
			Expression exp = interp.evaluate();
			//emit sendClear();
			parseExpression(exp);
		}
		catch (const SemanticError & ex) {
			emit sendClear();
			emit sendExpression(ex.what());
		}
	}
}

void NotebookApp::interpretation(Interpreter&interp)
{
	//REPL
	while (1) {
		auto user_input = input_comms.get_input();
		if (user_input == "%stop") {
			break;
		}

		std::istringstream expression(user_input);
		if (!interp.parseStream(expression)) {
			output_comms.store_output(std::pair<std::string, Expression>("Error: Invalid Program. Could not parse.", Expression()));
		}
		else {
			try {
				Expression exp = interp.evaluate();
				output_comms.store_output(std::pair<std::string, Expression>("", exp));
				
			}
			catch (const SemanticError & ex) {
				output_comms.store_output(std::pair<std::string, Expression>(ex.what(), Expression()));
				
			}
		}

	}
}



void NotebookApp::parseExpression(Expression &exp) {
	std::ostringstream stream;
	stream << exp;
	if (exp.isHeadComplex() || exp.isHeadNumber()) {
		sendExpression(stream.str());
	}
	else if (exp.isHeadSymbol()) {

		if ((exp.head().asSymbol() == "list") && (exp.prop().empty()))
		{
			for (auto it = exp.tailBegin(); it != exp.tailEnd(); ++it) {
				parseExpression(*it);
			}
		}

		else if (!exp.prop().empty()) {
			sendSignal(exp);
		}


		else if (exp.head().asSymbol() != "lambda") {
			sendExpression(stream.str());
		}
	}
	else {
		sendExpression("Error In Evaluation");
	}
}

void NotebookApp::sendSignal(Expression &exp)
{
	auto property = exp.prop();

	std::string names[3] = { "\"point\"","\"line\"","\"text\"" };

	if (property["\"object-name\""].head().asSymbol() == names[0]) {
		auto tail = exp.tailConstBegin();
		auto x = tail->head().asNumber();
		tail++;
		double y = tail->head().asNumber();
		double dia = property["\"size\""].head().asNumber();
		emit sendCircle(x, y, dia);
	}

	else if (property["\"object-name\""].head().asSymbol() == names[1]) {
		auto tail = exp.tailConstBegin();

		auto subTail0 = tail->tailConstBegin();
		auto x1 = subTail0->head().asNumber();
		subTail0++;
		auto y1 = subTail0->head().asNumber();

		tail++;

		auto subTail1 = tail->tailConstBegin();
		auto x2 = subTail1->head().asNumber();
		subTail1++;
		auto y2 = subTail1->head().asNumber();

		double thickness = property["\"thickness\""].head().asNumber();

		emit sendLine(x1, y1, x2, y2, thickness);
	}

	else if (property["\"object-name\""].head().asSymbol() == names[2]) {
		std::string str = exp.head().asSymbol();
		auto t = exp.prop()["\"position\""].tailConstBegin();
		double x = t->head().asNumber();
		t++;
		double y = t->head().asNumber();
		double rotate = property["\"text-rotation\""].head().asNumber();
		double size = property["\"text-scale\""].head().asNumber();
		emit sendText(str, x, y, rotate, size);
	}

}


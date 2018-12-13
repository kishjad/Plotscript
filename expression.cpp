#include "expression.hpp"

#include <sstream>
#include "environment.hpp"
#include "semantic_error.hpp"
#include <iomanip>
#include <algorithm>
#include <math.h>
#include <string>
const double BOUNDING_SIZE = 20;
const int D = 2;
const int C = 2;
const int A = 3;
const int B = 3;
const float NUM_OF_ITERATIONS = 50;
const int MAX_ITER = 10;


Expression::Expression(){}

Expression::Expression(const Atom & a){

  m_head = a;
}

// recursive copy
Expression::Expression(const Expression & a){

  m_head = a.m_head;
  for(auto e : a.m_tail){
    m_tail.push_back(e);
  }
	for (auto e : a.property) {
		property.emplace(e);
	}
}

Expression & Expression::operator=(const Expression & a){

  // prevent self-assignment
  if(this != &a){
    m_head = a.m_head;
    m_tail.clear();
    for(auto e : a.m_tail){
      m_tail.push_back(e);
    } 
		property.clear();
		for (auto e : a.property) {
			property.emplace(e);
		}
  }
  
  return *this;
}


Atom & Expression::head(){
  return m_head;
}

const Atom & Expression::head() const{
  return m_head;
}

std::map<std::string, Expression>& Expression::prop() {
	return property;
}

const std::map<std::string, Expression>& Expression::prop() const {
	return property;
}

bool Expression::isHeadNumber() const noexcept{
  return m_head.isNumber();
}

bool Expression::isHeadSymbol() const noexcept{
  return m_head.isSymbol();
}  

bool Expression::isHeadComplex() const noexcept{
  return m_head.isComplex();
}

void Expression::append(const Atom & a){
  m_tail.emplace_back(a);
}

void Expression::append(const Expression & E) {
	m_tail.emplace_back(E);
}


Expression * Expression::tail(){
  Expression * ptr = nullptr;
  
  if(m_tail.size() > 0){
    ptr = &m_tail.back();
  }

  return ptr;
}

void Expression::reset() {
	*this = Expression();
}

Expression::ConstIteratorType Expression::tailConstBegin() const noexcept{
  return m_tail.cbegin();
}

Expression::ConstIteratorType Expression::tailConstEnd() const noexcept{
  return m_tail.cend();
}

std::vector<Expression>::iterator Expression::tailBegin() noexcept
{
	return m_tail.begin();
}

std::vector<Expression>::iterator Expression::tailEnd() noexcept
{
	return  m_tail.end();
}

Expression apply(const Atom & op, const std::vector<Expression> & args, const Environment & env){

  // head must be a symbol
  if(!op.isSymbol()){
    throw SemanticError("Error during evaluation: procedure name not symbol");
  }


  // must map to a proc
  if(!env.is_proc(op)){
    throw SemanticError("Error during evaluation: symbol does not name a procedure");
  }
  
  // map from symbol to proc
  Procedure proc = env.get_proc(op);
  
  // call proc with args
  return proc(args);
}

Expression Expression::handle_lookup(const Atom & head, const Environment & env){
    if(head.isSymbol()){ // if symbol is in env return value
      if(env.is_exp(head)){
				Expression temp = env.get_exp(head);
				return temp;
      }
			else if (head.isSymbol() && head.asSymbol()[0] == '"') {
				return Expression(Atom(head.asSymbol()));
			}
      else{
				throw SemanticError("Error during evaluation: unknown symbol");
      }
    }
    else if(head.isNumber()){
			return Expression(head);
    }
	else if (head.isComplex()) {
		return Expression(head);
	}
    else{
      throw SemanticError("Error during evaluation: Invalid type in terminal expression");
    }
}	

Expression Expression::handle_define(Environment & env) {

	// tail must have size 3 or error
	if (m_tail.size() != 2) {
		throw SemanticError("Error during evaluation: invalid number of arguments to define");
	}

	// tail[0] must be symbol
	if (!m_tail[0].isHeadSymbol()) {
		throw SemanticError("Error during evaluation: first argument to define not symbol");
	}

	// but tail[0] must not be a special-form or procedure
	std::string s = m_tail[0].head().asSymbol();
	if ((s == "define") || (s == "begin")) {
		throw SemanticError("Error during evaluation: attempt to redefine a special-form");
	}

	if (env.is_proc(m_tail[0].head().asSymbol())) {
		throw SemanticError("Error during evaluation: attempt to redefine a built-in procedure");
	}

	// eval tail[1]
	Expression result = m_tail[1].eval(env);

	//and add to env
	env.add_exp(m_tail[0].head(), result);

	return result;
}

Expression Expression::handle_begin(Environment & env){
  
  if(m_tail.size() == 0){
    throw SemanticError("Error during evaluation: zero arguments to begin");
  }

  // evaluate each arg from tail, return the last
  Expression result;
  for(Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it){
    result = it->eval(env);
  }
  
  return result;
}

Expression Expression::handle_lambda(Environment & env) {
	if (m_tail.size() != 2) {
		throw SemanticError("Error during lambda: invalid number of arguments to define");
	}

	if (env.is_exp(m_head)) {
		throw SemanticError("Error during evaluation: attempt to redefine a previously defined symbol");
	}

	Expression result{ m_head };

	//Atom head;
	Expression inputargs;
	inputargs.append(m_tail[0].head());
	for (auto e = m_tail[0].tailConstBegin(); e != m_tail[0].tailConstEnd(); ++e)
	{
		inputargs.append(*e);
	}
	result.append(inputargs);

	result.append(m_tail[1]);
	return result;
}

Expression Expression::lambdaEval(const Atom&sym, Environment & env, std::vector<Expression>m_tail) {
	Expression exp = env.get_exp(sym);
	unsigned int counter = 0;
	for (auto vars = exp.m_tail[0].tailConstBegin(); vars != exp.m_tail[0].tailConstEnd(); ++vars) {
		//std::cout << (*vars);
		++counter;
	}

	if (counter != m_tail.size())
	{
		throw SemanticError("Error: invalid arguments to the lambda function");
	}
	Environment env2(env);

	//making the defines for the vars
	counter = 0;
	for (auto vars = exp.m_tail[0].tailConstBegin(); vars != exp.m_tail[0].tailConstEnd(); ++vars)
	{
		Expression temp(Atom("define"));
		temp.append(Expression(*vars));
		temp.append(Expression(m_tail[counter]));
		++counter;
		temp.eval(env2);
	}
	Expression t = exp.m_tail[1].eval(env2);
	return t;
}

Expression Expression::handle_map(Environment &env)
{

	if (m_tail[0].tail() != nullptr)
	{
		throw SemanticError("Error: first argument to map not a procedure");
	}

	else if ((env.is_proc(m_tail[0].head())) || (env.isLambda(m_tail[0].head().asSymbol()))) {
		//ADDED STATEMENT
		if ( env.is_proc(m_tail[1].head().asSymbol()) ){
			m_tail[1] = m_tail[1].eval(env);
		}
		

		if (m_tail[1].head().asSymbol() == "list" ){

			Expression result(Atom("list"));

			Expression ret = m_tail[1].eval(env);
			for (auto a = ret.tailConstBegin(); a != ret.tailConstEnd(); ++a) {
				Expression toSend(Atom("apply"));
				Expression t(Atom("list"));
				t.append(*a);
				toSend.append(m_tail[0].head());
				toSend.append(t);
				result.append(toSend.eval(env));
			}
			return result;
			
		}

		else {
			throw SemanticError("Error: second argument not a list");
		}
	}
	else {
		throw SemanticError("Error: first argument to apply not a procedure");
	}
	return Expression();
}

Expression Expression::handle_apply(Environment &env) 
{
	if (m_tail[0].tail() != nullptr)
	{
		throw SemanticError("Error: first argument to apply not a procedure");
	}
	
	if ( (env.is_proc(m_tail[0].head())) || (env.isLambda(m_tail[0].head().asSymbol())) ) {
		if (m_tail[1].head().asSymbol() == "list") {
			Expression result(Atom("list"));

			//handling non-lambda procedures
			if (env.is_proc(m_tail[0].head())) {
				std::vector<Expression> t;
				Expression ret = m_tail[1].eval(env);

				for (auto a = ret.tailConstBegin(); a != ret.tailConstEnd(); ++a) {
					t.push_back(*a);
				}
				return apply(m_tail[0].head(), t, env);
			}

			//handling lambda
			if (env.isLambda(m_tail[0].head().asSymbol())) {
				std::vector<Expression> t;
				Expression ret = m_tail[1].eval(env);

				for (auto a = ret.tailConstBegin(); a != ret.tailConstEnd(); ++a) {
					t.push_back(*a);
				}

				return lambdaEval(m_tail[0].head().asSymbol(), env, t);
			}
		}
		else {
			throw SemanticError("Error: second argument not a list");
		}
	}
	else {
		throw SemanticError("Error: first argument to apply not a procedure");
	}
	return Expression();
}

Expression Expression::handle_set_property(Environment &env)
{
	if (m_tail.size() != 3) {
		throw SemanticError("Error: invalid number of arguments to set-property");
	}

	// tail[0] must be string 
	if (!m_tail[0].isHeadSymbol() || m_tail[0].m_head.asSymbol()[0] != '"') {
		throw SemanticError("Error: first argument to set-property not a string");
	}

	// but tail[0] must not be a special-form or procedure
	std::string s = m_tail[0].head().asSymbol();
	if ((s == "define") || (s == "begin")) {
		throw SemanticError("Error: attempt to set-property of a special-form");
	}

	Expression result = m_tail[2].eval(env);
	std::string key = m_tail[0].head().asSymbol();
	Expression value = m_tail[1].eval(env);
	result.property[key]=value;
	Expression t = result;
	return t;
}

Expression Expression::handle_get_property(Environment &env) {
	
	if (m_tail.size() != 2) {
		throw SemanticError("Error: invalid number of arguments to get-property");
	}

	// tail[0] must be string
	if (!m_tail[0].isHeadSymbol() || m_tail[0].m_head.asSymbol()[0] != '"') {
		throw SemanticError("Error: first argument to get-property not a string");
	}

	// but tail[0] must not be a special-form or procedure
	std::string s = m_tail[0].head().asSymbol();
	if ((s == "define") || (s == "begin")) {
		throw SemanticError("Error: attempt to get-property of a special-form");
	}

	// eval tail[1]
	Expression t = env.get_exp(m_tail[1].head());

	auto it = t.property.find(m_tail[0].head().asSymbol());
	if (it == t.property.end()) {
		Expression result(Atom("NONE"));
		return result;
	}
	else {
		Expression result = t.property[m_tail[0].head().asSymbol()];
		return result;
	}
	
}

Expression Expression::construct_line(Expression&p1, Expression&p2, Environment &env) {
	Expression result(Atom("make-line"));
	result.append(p1);
	result.append(p2);
	Expression temp = result.eval(env);
	temp.property["\"thickness\""] = Expression(0);
	return temp;
}

Expression Expression::construct_point(double x, double y, Environment &env) {
	Expression result(Atom("make-point"));
	result.append(x);
	result.append(y);
	return result.eval(env);
}

std::map<std::string,double> Expression::scaling_factor_and_bounds(const Expression&data) {
	std::map<std::string, double> result;
	double x_max = -100000000;
	double y_max = -100000000;
	double y_min = 100000000;
	double x_min = 100000000;
	for (auto &t : data.m_tail) {
		x_min = ((t.m_tail[0].head().asNumber() < x_min) ? t.m_tail[0].head().asNumber() : x_min);
		y_min = ((t.m_tail[1].head().asNumber() < y_min) ? t.m_tail[1].head().asNumber() : y_min);
		x_max = ((t.m_tail[0].head().asNumber() > x_max) ? t.m_tail[0].head().asNumber() : x_max);
		y_max = ((t.m_tail[1].head().asNumber() > y_max) ? t.m_tail[1].head().asNumber() : y_max);
	}
	result["x_max"] = x_max;
	result["y_max"] = y_max;
	result["y_min"] = y_min;
	result["x_min"] = x_min;
	result["x_scale"] = (BOUNDING_SIZE)/(x_max - x_min);
	result["y_scale"] = (BOUNDING_SIZE)/(y_max - y_min);
	return result;
}

Expression Expression::add_axes(Environment &env, std::map<std::string, double> &values) {
	Expression result(Atom("list"));
	if ((values["x_min"] <= 0) && (values["x_max"] >= 0)) {
		Expression lower_p = construct_point(0, values["y_smin"], env);
		Expression upper_p = construct_point(0, values["y_smax"], env);
		Expression line_y = construct_line(lower_p, upper_p, env);
		result.append(line_y);
	}
	if ((values["y_min"] <= 0) && (values["y_max"] >= 0)) {
		Expression lower_p = construct_point(values["x_smin"], 0, env);
		Expression upper_p = construct_point(values["x_smax"], 0, env);
		Expression line_x = construct_line(lower_p, upper_p, env);
		result.append(line_x);
	}
	return result;
}

Expression Expression::add_boundaries(Environment &env, std::map<std::string, double> &values) {
	Expression result(Atom("list"));

	Expression TL = construct_point(values["x_smin"], values["y_smax"], env);
	Expression TR = construct_point(values["x_smax"], values["y_smax"], env);
	Expression BL = construct_point(values["x_smin"], values["y_smin"], env);
	Expression BR = construct_point(values["x_smax"], values["y_smin"], env);

	Expression top = construct_line(TL, TR, env);
	Expression bottom = construct_line(BL, BR, env);
	Expression right = construct_line(TR, BR, env);
	Expression left = construct_line(TL, BL, env);

	result.append(top);
	result.append(bottom);
	result.append(left);
	result.append(right);

	return result;
}

Expression Expression::add_options(Environment &env, std::map<std::string, double> &values) {

	Expression result(Atom(this->m_tail[1].head().asSymbol()));

	result.property["\"object-name\""] = Expression(Atom("\"text\""));
	result.property["\"text-scale\""] = Expression(values["text-scale"]);

	if (this->m_tail[0].head().asSymbol() == "\"title\"") {
		result.property["\"text-rotation\""] = Expression(0);
		Expression pos = construct_point((values["x_smax"] + values["x_smin"]) / 2, values["y_smax"] - A, env);
		result.property["\"position\""] = pos;
	}

	else if (this->m_tail[0].head().asSymbol() == "\"ordinate-label\"") {
		result.property["\"text-rotation\""] = Expression(-(std::atan2(0, -1) / 2));
		Expression pos = construct_point(values["x_smin"] - A, (values["y_smax"] + values["y_smin"]) / 2, env);
		result.property["\"position\""] = pos;
	}

	else if (this->m_tail[0].head().asSymbol() == "\"abscissa-label\"") {
		result.property["\"text-rotation\""] = Expression(0);
		Expression pos = construct_point((values["x_smax"] + values["x_smin"]) / 2, values["y_smin"] + A, env);
		result.property["\"position\""] = pos;
	}
	return result;
}

Expression Expression::add_labels(Environment &env, std::map<std::string, double> &values)
{
	Expression result(Atom("list"));
	Expression position;
	std::ostringstream output;
	output.precision(2);
	output << values["x_max"];
	Expression AU(Atom("\"" + output.str() + "\""));
	output.str("");
	AU.property["\"object-name\""] = Expression(Atom("\"text\""));
	AU.property["\"text-rotation\""] = Expression(0);
	AU.property["\"text-scale\""] = Expression(values["text-scale"]);
	position = construct_point(values["x_smax"] , values["y_smin"] + C, env);
	AU.property["\"position\""] = position;
	result.append(AU);

	output << values["y_max"];
	Expression OU(Atom("\"" + output.str() + "\""));
	output.str("");
	OU.property["\"object-name\""] = Expression(Atom("\"text\""));
	OU.property["\"text-rotation\""] = Expression(0);
	OU.property["\"text-scale\""] = Expression(values["text-scale"]);
	position = construct_point(values["x_smin"] - D, values["y_smax"], env);
	OU.property["\"position\""] = position;
	result.append(OU);

	output << values["x_min"];
	Expression AL(Atom("\"" + output.str() + "\""));
	output.str("");
	AL.property["\"object-name\""] = Expression(Atom("\"text\""));
	AL.property["\"text-rotation\""] = Expression(0);
	AL.property["\"text-scale\""] = Expression(values["text-scale"]);
	position = construct_point(values["x_smin"], values["y_smin"] + C, env);
	AL.property["\"position\""] = position;
	result.append(AL);

	output << values["y_min"];
	Expression OL(Atom("\"" + output.str() + "\""));
	output.str("");
	OL.property["\"object-name\""] = Expression(Atom("\"text\""));
	OL.property["\"text-rotation\""] = Expression(0);
	OL.property["\"text-scale\""] = Expression(values["text-scale"]);
	position = construct_point(values["x_smin"] - D, values["y_smin"], env);
	OL.property["\"position\""] = position;
	result.append(OL);

	return result;
}

void Expression::sample_points(Environment & env, Expression& newdata, const Atom & sym)
{
	double sampling = (this->m_tail[1].head().asNumber() - this->m_tail[0].head().asNumber()) / NUM_OF_ITERATIONS;
	double x = m_tail[0].head().asNumber();
	double y = 0;
	for (double i = 0; i <= NUM_OF_ITERATIONS; i++) {
		std::vector<Expression> temp;
		Expression t(x);
		temp.push_back(t);
		y = lambdaEval(sym, env, temp).head().asNumber();
		Expression a = construct_point(x, y, env);
		newdata.append(a);
		x += sampling;
	}
}

Expression Expression::draw_discrete(Environment &env, std::map<std::string, double> &value) {
	Expression result(Atom("list"));
 //Rescale all data to N*N

	Expression rescaled_data(Atom("list"));
	for (auto &tail : this->m_tail) {
		double rescaled_x = tail.m_tail[0].head().asNumber() * value["x_scale"];
		double rescaled_y = tail.m_tail[1].head().asNumber() * value["y_scale"] * -1;
		

		Expression point = construct_point(rescaled_x, rescaled_y, env);
		point.property["\"size\""] = Expression(0.5);
		result.append(point);
		Expression point2;
		if (value["y_min"] > 0) {
			point2 = construct_point(rescaled_x, value["y_smin"], env);
		}
		else {
			point2 = construct_point(rescaled_x, 0, env);
		}
		Expression line = construct_line(point,point2,env);
		result.append(line);
	}

	return result;
}

bool Expression::implement_iteration(Environment &env, Expression& new_points, Expression& current_points, const Atom & func) {
	bool ret_value = false;
	new_points.append(current_points.m_tail[0]);
	for (unsigned int iter=0;iter <= current_points.m_tail.size() - 2; iter+=2) {
		
		double angle = angle_between(current_points.m_tail[iter], current_points.m_tail[iter + 1], current_points.m_tail[iter + 2]);
		if (angle < 175.0) {
			Expression x1 = Expression((current_points.m_tail[iter].m_tail[0].head().asNumber() + current_points.m_tail[iter + 1].m_tail[0].head().asNumber()) / 2);
			Expression x2 = Expression((current_points.m_tail[iter+1].m_tail[0].head().asNumber() + current_points.m_tail[iter + 2].m_tail[0].head().asNumber()) / 2);
			std::vector<Expression> temp;
			temp.push_back(x1);
			Expression y1 = lambdaEval(func, env, temp);
			temp.clear();
			temp.push_back(x2);
			Expression y2 = lambdaEval(func, env, temp);
			new_points.append(construct_point(x1.head().asNumber(), y1.head().asNumber(), env));
			new_points.append(current_points.m_tail[iter + 1]);
			new_points.append(construct_point(x2.head().asNumber(), y2.head().asNumber(), env));
			ret_value = true;
		}
		else {
			new_points.append(current_points.m_tail[iter+1]);
		}
		new_points.append(current_points.m_tail[iter + 2]);
	}
	return ret_value;
}

Expression Expression::draw_continuous(Environment &env, std::map<std::string, double> &value, const Atom & func) {
	Expression data_points(Atom("list"));
	Expression new_data_points(Atom("list"));

	for (auto &tail : this->m_tail) {
		double rescaled_x = tail.m_tail[0].head().asNumber();
		double rescaled_y = tail.m_tail[1].head().asNumber();
		data_points.append(construct_point(rescaled_x, rescaled_y, env));
	}

	//Implement iteration with the points
	bool angles_less_than_175 = false;
	for (unsigned int i = 0; i < MAX_ITER; ++i) {
		
		//calling the function 10 times
		angles_less_than_175 = implement_iteration(env,new_data_points,data_points,func);

		if (!angles_less_than_175) {
			break;
		}
		else {
			data_points = new_data_points;
			new_data_points.m_tail.clear();
		}
	}

	std::map<std::string, double> result = scaling_factor_and_bounds(new_data_points);
	value["x_scale"] = result["x_scale"];
	value["y_scale"] = result["y_scale"];
	value["x_max"] = result["x_max"];
	value["x_min"] = result["x_min"];
	value["y_max"] = result["y_max"];
	value["y_min"] = result["y_min"];

	data_points.m_tail.clear();
	for (auto &tail : new_data_points.m_tail) {
		double rescaled_x = tail.m_tail[0].head().asNumber()* value["x_scale"];
		double rescaled_y = tail.m_tail[1].head().asNumber() *  value["y_scale"] * -1;
		data_points.append(construct_point(rescaled_x, rescaled_y, env));
	}



	Expression data_lines(Atom("list"));
	for (unsigned int i = 0; i < data_points.m_tail.size() - 1; ++i) {
		data_lines.append(construct_line(data_points.m_tail[i], data_points.m_tail[i + 1], env));
	}

	return data_lines;
}

Expression Expression::handle_discrete_plot(Environment &env)
{
	Procedure proc = env.get_proc(m_head);
	std::vector<Expression> results;
	Expression data;
	Expression options;

	if (m_tail.size() == 2) {
		data = m_tail[0].eval(env);
		options = m_tail[1].eval(env);
		if ((options.head().asSymbol() != "list") || (data.head().asSymbol() != "list")) {
			throw SemanticError("Error in call to discrete plot: Arguments must be of type list");
		}
	}
	else if (m_tail.size() == 1) {
		data = m_tail[0].eval(env);
		if (data.head().asSymbol() != "list") {
			throw SemanticError("Error in call to discrete plot: Arguments must be of type list");
		}
	}
	else {
		throw SemanticError("Error in call to discrete plot: Invalid number of arguments");
	}
	
	
	double textScale = 1;
	//Finding the text-scale
	for (auto & a : options.m_tail) {
		if (a.m_tail[0].head().asSymbol() == "\"text-scale\"") {
			textScale = a.m_tail[1].head().asNumber();
		}
	}
	//Get max, min and scale
	std::map<std::string,double> values = scaling_factor_and_bounds(data);
	values["text-scale"] = textScale;
	values["x_smax"] = values["x_scale"] * values["x_max"];
	values["x_smin"] = values["x_scale"] * values["x_min"];
	values["y_smax"] = values["y_scale"] * values["y_max"] * -1;
	values["y_smin"] = values["y_scale"] * values["y_min"] * -1;

	//Adding the bounding lines
	Expression bound = add_boundaries(env,values);
	for (auto & a : bound.m_tail) {
		results.push_back(a);
	}

	//adding title, absicca, ordinate names
	if (!options.m_tail.empty()) {
		for (auto & a : options.m_tail) {
			if (a.m_tail[0].head().asSymbol() != "\"text-scale\"") {
				auto e = a.add_options(env, values);
				results.push_back(e);
			}
		}
	}

	//Adding axes labels
	Expression labels = add_labels(env, values);
	for (auto & a : labels.m_tail) {
		results.push_back(a);
	}
	//Adding axes if required
	Expression axes = add_axes(env, values);
	for (auto & a : axes.m_tail) {
		results.push_back(a);
	}

	//Adding points and lines
	Expression t = data.draw_discrete(env,values);
	for (auto & a : t.m_tail) {
		results.push_back(a);
	}

	return proc(results);
}

double Expression::angle_between(Expression p1, Expression p2, Expression p3) {
	double x1 = p1.m_tail[0].head().asNumber();
	double y1 = p1.m_tail[1].head().asNumber();
	double x2 = p2.m_tail[0].head().asNumber();
	double y2 = p2.m_tail[1].head().asNumber();
	double x3 = p3.m_tail[0].head().asNumber();
	double y3 = p3.m_tail[1].head().asNumber();
	double v1_x = (x1 - x2);
	double v1_y = (y1 - y2);
	double v2_x = (x3 - x2);
	double v2_y = (y3 - y2);

	double v1_v2 = v1_x * v2_x + v1_y * v2_y;
	double mod_v1 = sqrt(pow(v1_x, 2) + pow(v1_y, 2));
	double mod_v2 = sqrt(pow(v2_x, 2) + pow(v2_y, 2));

 	auto ang = acos((float)( v1_v2 / (mod_v1*mod_v2))) * 180 / (std::atan2(0, -1));
	return (abs(ang));
}


Expression Expression::handle_continuous_plot(Environment &env) {
	Procedure proc = env.get_proc(m_head);
	std::vector<Expression> results;

	Expression func;
	Expression bounds;
	Expression options;

	if (m_tail.size() == 3) {
		func = m_tail[0];
		bounds = m_tail[1].eval(env);
		options = m_tail[2].eval(env);
		if ((options.head().asSymbol() != "list") || (bounds.head().asSymbol() != "list")) {
			throw SemanticError("Error in call to continuous plot: Arguments must be of type list");
		}
		
	}
	else if (m_tail.size() == 2) {
		func = m_tail[0];
		bounds = m_tail[1].eval(env);
		
		if (bounds.head().asSymbol() != "list") {
			throw SemanticError("Error in call to continuous plot: Arguments must be of type list");
		}
	}
	else {
		throw SemanticError("Error in call to continuous plot: Invalid number of arguments");
	}


	if (!env.isLambda(func.head())) {
		throw SemanticError("Error in call to continuous plot: first arguments must be of type lambda");
	}

	double textScale = 1;
	///Finding the text-scale
	for (auto & a : options.m_tail) {
		if (a.m_tail[0].head().asSymbol() == "\"text-scale\"") {
			textScale = a.m_tail[1].head().asNumber();
		}
	}
	Expression data(Atom("list"));
	//Sampling equally spaced points and save it in Expression data
	bounds.sample_points(env, data, func.head());
	

	//Get max, min and scale
	std::map<std::string, double> values = scaling_factor_and_bounds(data);
	values["text-scale"] = textScale;
	values["x_smax"] = values["x_scale"] * values["x_max"];
	values["x_smin"] = values["x_scale"] * values["x_min"];
	values["y_smax"] = values["y_scale"] * values["y_max"] * -1;
	values["y_smin"] = values["y_scale"] * values["y_min"] * -1;

	Expression t = data.draw_continuous(env, values, func.head());
	for (auto & a : t.m_tail) {
		results.push_back(a);
	}

	//Adding the bounding lines
	Expression bound = add_boundaries(env, values);
	for (auto & a : bound.m_tail) {
		results.push_back(a);
	}

	//Adding properties to title,absicca,ordinate
	if (!options.m_tail.empty()) {
		for (auto & a : options.m_tail) {
			if (a.m_tail[0].head().asSymbol() != "\"text-scale\"") {
				auto e = a.add_options(env, values);
				results.push_back(e);
			}
		}
	}



	//Adding axes labels
	Expression labels = add_labels(env, values);
	for (auto & a : labels.m_tail) {
		results.push_back(a);
	}
	//Adding axes if required
	Expression axes = add_axes(env, values);
	for (auto & a : axes.m_tail) {
		results.push_back(a);
	}

	

	return proc(results);
}

// this is a simple recursive version. the iterative version is more
// difficult with the ast data structure used (no parent pointer).
// this limits the practical depth of our AST
Expression Expression::eval(Environment & env){
 
  if(m_tail.empty() && (m_head.asSymbol()!="list")){
    return handle_lookup(m_head, env);
  }

  // handle begin special-form
  else if(m_head.isSymbol() && m_head.asSymbol() == "begin"){
    return handle_begin(env);
  }
  // handle define special-form
  else if(m_head.isSymbol() && m_head.asSymbol() == "define"){
    return handle_define(env);
  }

  // handle lambda special-forms
  else if (((m_head.isSymbol()) && (m_head.asSymbol() == "lambda"))) {
	  return handle_lambda(env);
  }

  else if (m_head.isSymbol() && m_head.asSymbol() == "apply") {
	  return handle_apply(env);
  }

  else if (m_head.isSymbol() && m_head.asSymbol() == "map") {
	  return handle_map(env);
  }
	
	else if (m_head.isSymbol() && m_head.asSymbol() == "set-property") {
		return handle_set_property(env);
	}

	else if (m_head.isSymbol() && m_head.asSymbol() == "get-property") {
		return handle_get_property(env);
	}

	else if (m_head.isSymbol() && m_head.asSymbol() == "discrete-plot") {
		return handle_discrete_plot(env);
	}

	else if (m_head.isSymbol() && m_head.asSymbol() == "continuous-plot") {
		return handle_continuous_plot(env);
	}

  else if (env.isLambda(m_head)) {

	  return lambdaEval(m_head,env,m_tail);
  }
  
  else{ 
    std::vector<Expression> results;
    for(Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it){
      results.push_back(it->eval(env));
    }
    return apply(m_head, results, env);

  }
}


std::ostream & operator<<(std::ostream & out, const Expression & exp){
  //Check if complex, and if it is then don't print the brackets becasue 
  //std::cout or complex contain ()
  if(exp.head().isComplex())
  {
    out << exp.head();
    for(auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e){
    out << *e;
    }
  }



  else if (exp.head().asSymbol() == "list"){
	  out << "(";
	  /*out << exp.head();*/

	  for(auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e){
		out << *e;
		if (e != exp.tailConstEnd() - 1)
		{
			out << " ";
		}
	  }

	  out << ")";
  }
  

  else if (exp.head().asSymbol() == "lambda") {
	  out << "(";
	  
	  for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
			  out << "";
			  out << *e;

		  if (e != exp.tailConstEnd() - 1)
		  {
			  out << " ";
		  }
	  }
	  out << ")";
  }

	else if (exp.head().asSymbol() == "NONE") {
		out << exp.head();
	}

  else if (exp.head().isSymbol()) {
	  out << "(";
	  out << exp.head();

	  for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
		  out <<" "<<*e;
	  }

	  out << ")";
  }

  else{
	  out << "(";
	  out << exp.head();

	  for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
		  out << *e;
		  if (e != exp.tailConstEnd() - 1)
		  {
			  out << " ";
		  }
	  }

	  out << ")";
  }

  return out;
}

bool Expression::operator==(const Expression & exp) const noexcept{

  bool result = (m_head == exp.m_head);

  result = result && (m_tail.size() == exp.m_tail.size()) && (property.size() == exp.property.size());

  if(result){
    for(auto lefte = m_tail.begin(), righte = exp.m_tail.begin();
	(lefte != m_tail.end()) && (righte != exp.m_tail.end());
	++lefte, ++righte){
      result = result && (*lefte == *righte);
    }
  }

	if (result) {
		for (auto lefte = property.begin(), righte = exp.property.begin();
			(lefte != property.end()) && (righte != exp.property.end());
			++lefte, ++righte) {
			result = result && (*lefte == *righte);
		}
	}

  return result;
}

bool operator!=(const Expression & left, const Expression & right) noexcept{

  return !(left == right);
}

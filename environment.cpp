#include "environment.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include "environment.hpp"
#include "semantic_error.hpp"

/*********************************************************************** 
Helper Functions
**********************************************************************/

// predicate, the number of args is nargs
bool nargs_equal(const std::vector<Expression> & args, unsigned nargs){
  return args.size() == nargs;
}

/*********************************************************************** 
Each of the functions below have the signature that corresponds to the
typedef'd Procedure function pointer.
**********************************************************************/

// the default procedure always returns an expresison of type None
Expression default_proc(const std::vector<Expression> & args){
  args.size(); // make compiler happy we used this parameter
  return Expression();
};

//This function rounds up the values of the complex number because it didn't show (0,0).

void round(std::complex<double>& input)
{
  std::complex<double>temp1(0,0);
  if(input.imag() != 0)
  {
    if( ((input.real() - 0) <= std::numeric_limits<double>::epsilon()) && ((input.imag() - 0) <= std::numeric_limits<double>::epsilon()) )
    {
      input = std::complex<double>(0,0);
    }
    else if((input.imag() - 0) <= std::numeric_limits<double>::epsilon())
    {
      input = std::complex<double>(input.real(),0);
    }
    else if((input.real() - 0) <= std::numeric_limits<double>::epsilon())
    {
      input = std::complex<double>(0,input.imag());
    }
  }
}

//List is an expression of expressions
Expression list(const std::vector<Expression> & args){
	Atom Head("list");
	Expression result{ Head };
	/*Expression result{};*/
  for(auto &a: args){
    result.append(a);
  }
  return result;
};

Expression first(const std::vector<Expression> &args) {
	if (nargs_equal(args, 1)) {
		if (args[0].head().asSymbol() == "list") {
			if (args[0].tailConstBegin() == args[0].tailConstEnd())
			{
				throw SemanticError("Error: argument to first is an empty list");
			}
			else
			{
				auto e = args[0].tailConstBegin();
				return Expression(*e);
			}
			
		}
		else {
			throw SemanticError("Error: argument to first is not a list");
		}
	}
	else {
		throw SemanticError("Error: more than one argument in call to first");
	}
}

Expression rest(const std::vector<Expression> &args) {
	if (nargs_equal(args, 1)) {
		if (args[0].head().asSymbol() == "list") {
			if (args[0].tailConstBegin() == args[0].tailConstEnd())
			{
				throw SemanticError("Error: argument to first is an empty list");
			}
			else
			{
				Atom Head("list");
				Expression result{ Head };
				for (auto e = ++(args[0].tailConstBegin()); e != args[0].tailConstEnd(); ++e) {
					result.append(*e);
				}
				return result;
			}

		}
		else {
			throw SemanticError("Error: argument to rest is not a list");
		}
	}
	else {
		throw SemanticError("Error: more than one argument in call to rest");
	}
}

Expression join(const std::vector<Expression> &args) {
	if (nargs_equal(args, 2)) {
		if ((args[0].head().asSymbol() == "list")&&(args[1].head().asSymbol() == "list")) {
			Atom Head("list");
			Expression result{ Head };
			for (auto e = (args[0].tailConstBegin()); e != args[0].tailConstEnd(); ++e) {
				result.append(*e);
			}
			for (auto e = (args[1].tailConstBegin()); e != args[1].tailConstEnd(); ++e) {
				result.append(*e);
			}
			return result;
		}
		else {
			throw SemanticError("Error: argument to join not a list");
		}
	}
	else {
		throw SemanticError("Error in call to join: invalid number of arguments");
	}
}

Expression range(const std::vector<Expression> &args) {
	if (nargs_equal(args, 3)) {
		if ( (args[0].head().isNumber()) && (args[1].head().isNumber()) && (args[2].head().isNumber()) ) {
			if ((args[2].head().asNumber() > 0) && (args[1].head().asNumber() < 0) && (args[0].head().asNumber() > 0)) {
				throw SemanticError("Error: begin greater than end in range");
			}
			else if ((args[2].head().asNumber() <= 0) )
			{
				throw SemanticError("Error: negative or zero increment in range");
			}
			else{
				Atom Head("list");
				Expression result{ Head };
				for (auto e = args[0].head().asNumber(); e <= args[1].head().asNumber(); e += args[2].head().asNumber())
				{
					result.append(Expression(e));
				}
				return result;
			}
			
		}
		else {
			throw SemanticError("Error: argument to range not numbers");
		}
	}
	else {
		throw SemanticError("Error in call to range: invalid number of arguments");
	}
}

Expression length(const std::vector<Expression> &args) {
	
	if (nargs_equal(args, 1)) {
		if (args[0].head().asSymbol() == "list") {
			int leng = 0;
			for (auto e = (args[0].tailConstBegin()); e != args[0].tailConstEnd(); ++e) {
				leng++;
			}
			return Expression(leng);
		}
		else {
			throw SemanticError("Error: argument to length is not a list");
		}
	}
	else {
		throw SemanticError("Error: more than one argument in call to length");
	}
}

Expression append(const std::vector<Expression> &args) {

	if (nargs_equal(args, 2)) {
		if (args[0].head().asSymbol() == "list") {
			Atom Head("list");
			Expression result{ Head };
			for (auto e = (args[0].tailConstBegin()); e != args[0].tailConstEnd(); ++e) {
				result.append(*e);
			}
			result.append(args[1]);
			return result;
		}
		else {
			throw SemanticError("Error: first argument to append not a list");
		}
	}
	else {
		throw SemanticError("Error: more than two argument in call to append");
	}
}

Expression add(const std::vector<Expression> & args){
  bool areArgumentsComplex = false;
  // check all aruments are numbers, while adding
  std::complex<double>result(0,0);
  for( auto & a :args){
    if(a.isHeadComplex()){
      areArgumentsComplex = true;
      result += a.head().asComplex();
    }
    else if(a.isHeadNumber()){
      result += a.head().asNumber();      
    }
    else{
      throw SemanticError("Error in call to add, argument not a number");
    }
  }
  
  if(result.imag() == 0 && areArgumentsComplex == false){
    return Expression(result.real());
  }
  else{
    return Expression(result);
  }
  
};

Expression mul(const std::vector<Expression> & args){
  bool areArgumentsComplex = false;
  // check all aruments are numbers, while multiplying
  
  std::complex<double>result(0,0);
  if(args[0].isHeadComplex())
  {
    areArgumentsComplex = true;
    result = args[0].head().asComplex();
  }
  else if(args[0].isHeadNumber())
  {
    result = std::complex<double>(args[0].head().asNumber(),0);
  }
  else 
  {
    throw SemanticError("Error in call to mul, argument not a number");
  }
  bool first = true;
  for( auto & a :args){
    if (first)
    { first = false; continue; }

    if(a.isHeadComplex()){
      areArgumentsComplex = true;
      result *= a.head().asComplex();
    }
    else if(a.isHeadNumber()){
      result *= a.head().asNumber();      
    }
    else{
      throw SemanticError("Error in call to mul, argument not a number");
    }
  }

 if(areArgumentsComplex == false){
    return Expression(result.real());
  }
  else{
    return Expression(result);
  }
};

Expression subneg(const std::vector<Expression> & args){

  std::complex<double>result(0,0);
  bool areArgumentsComplex = false;
  // preconditions
  if(nargs_equal(args,1)){
    if(args[0].isHeadNumber()){
      result = -args[0].head().asNumber();
    }
    else if(args[0].isHeadComplex()){
      result = -args[0].head().asComplex();
    }
    else{
      throw SemanticError("Error in call to negate: invalid argument.");
    }
  }
  else if(nargs_equal(args,2)){
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){
      result = args[0].head().asNumber() - args[1].head().asNumber();
    }
    else if( (args[0].isHeadComplex()) && (args[1].isHeadComplex()) ){
      areArgumentsComplex = true;
      result = args[0].head().asComplex() - args[1].head().asComplex();
    }
    else if( (args[0].isHeadComplex()) && (args[1].isHeadNumber()) ){
      areArgumentsComplex = true;
      result = args[0].head().asComplex() - args[1].head().asNumber();
    }
    else if( (args[0].isHeadNumber()) && (args[1].isHeadComplex()) ){
      areArgumentsComplex = true;
      result = args[0].head().asNumber() - args[1].head().asComplex();
    }
    else{      
      throw SemanticError("Error in call to subtraction: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to subtraction or negation: invalid number of arguments.");
  }

  if(result.imag() == 0 && areArgumentsComplex == false){
    return Expression(result.real());
  }
  else{
    return Expression(result);
  }

};

Expression div(const std::vector<Expression> & args){

  std::complex<double>result(0,0);
  bool areArgumentsComplex = false;

  if(nargs_equal(args,2)){
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){
      result = args[0].head().asNumber() / args[1].head().asNumber();
    }
    else if( (args[0].isHeadComplex()) && (args[1].isHeadComplex()) ){
      areArgumentsComplex = true;
      result = args[0].head().asComplex() / args[1].head().asComplex();
    }
    else if( (args[0].isHeadComplex()) && (args[1].isHeadNumber()) ){
      areArgumentsComplex = true;
      result = args[0].head().asComplex() / args[1].head().asNumber();
    }
    else if( (args[0].isHeadNumber()) && (args[1].isHeadComplex()) ){
      areArgumentsComplex = true;
      result = args[0].head().asNumber() / args[1].head().asComplex();
    }
    else{      
      throw SemanticError("Error in call to division: invalid argument.");
    }
  }
  else if (nargs_equal(args, 1)) {
	  if (args[0].isHeadNumber()) {
		  result = 1.0 / args[0].head().asNumber();
	  }
	  else if (args[0].isHeadComplex()) {
		  areArgumentsComplex = true;
		  result = std::complex<double>(1.0) / args[0].head().asComplex();
	  }
	  else {
		  throw SemanticError("Error in call to division: invalid argument.");
	  }
  }
  else{
    throw SemanticError("Error in call to division: invalid number of arguments.");
  }
  
  if(areArgumentsComplex == false){
    return Expression(result.real());
  }
  else{
    return Expression(result);
  }

};
//Results for all calculationa are assumed to be complex 
//and if only real numbers are to be displayed then the 
//real part of the complex number is outputed. 

//Added fucntions:
Expression power(const std::vector<Expression> & args){
  bool areArgumentsComplex = false;
  // check all aruments are numbers, while multiplying
  std::complex<double>result(1,0);
  if(nargs_equal(args,2)){
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){
      result = std::pow(args[0].head().asNumber(),args[1].head().asNumber());
    }
    else if( args[0].isHeadComplex() && args[1].isHeadComplex() ){
      areArgumentsComplex = true;
      result = std::pow(args[0].head().asComplex(),args[1].head().asComplex());
    }
    else if( args[0].isHeadNumber() && args[1].isHeadComplex() ){
      areArgumentsComplex = true;
      result = std::pow(args[0].head().asNumber(),args[1].head().asComplex());
    }
    else if( args[0].isHeadComplex() && args[1].isHeadNumber() ){
      areArgumentsComplex = true;
      result = std::pow(args[0].head().asComplex(),args[1].head().asNumber());
    }
    else{      
      throw SemanticError("Error in call to power: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to power: invalid number of arguments.");
  }

  if(areArgumentsComplex == false){
    return Expression(result.real());
  }
  else{
    return Expression(result);
  }
};

Expression sqrt(const std::vector<Expression> & args){
 
  // check all aruments are numbers, while multiplying
  std::complex<double>result(0,0);
  if(nargs_equal(args,1)){
    if( (args[0].isHeadNumber()) )
    {
      //Check if its a positive number
      if(args[0].head().asNumber() >= 0)
      {
        result = std::pow(args[0].head().asNumber(),0.5);
      }
      else if(args[0].head().asNumber() < 0)
      {
        std::complex<double> temp = args[0].head().asNumber();
        result = std::pow(temp,0.5);
      }
      // else{
      //   throw SemanticError("Error in call to sqrt: negative number.");
      // }
    }

    else if( (args[0].isHeadComplex()) )
    {
      result = std::pow(args[0].head().asComplex(),0.5);
    }

    else{      
      throw SemanticError("Error in call to sqrt: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to sqrt: invalid number of arguments.");
  }
  round(result);
  if(result.imag() == 0){
    return Expression(result.real());
  }
  else{
    return Expression(result);
  }
};

Expression ln(const std::vector<Expression> & args){
  // check all aruments are numbers, while multiplying
  double result = 1;
  if(nargs_equal(args,1)){
    if( (args[0].isHeadNumber()) )
    {
      if(args[0].head().asNumber() > 0)
      {
        result = std::log(args[0].head().asNumber());
      }
      else{
        throw SemanticError("Error in call to ln: negative number.");
      }
    }
    else{      
      throw SemanticError("Error in call to ln: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to ln: invalid number of arguments.");
  }
  //round(result);
  return Expression(result);
};

Expression cos(const std::vector<Expression> & args){
  // check all aruments are numbers, while multiplying
  double result = 1;
  if(nargs_equal(args,1)){
    if( (args[0].isHeadNumber()) )
    {
      result = std::cos(args[0].head().asNumber());
    }
    else{      
      throw SemanticError("Error in call to cos: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to cos: invalid number of arguments.");
  }
  return Expression(result);
};

Expression sin(const std::vector<Expression> & args){
  // check all aruments are numbers, while multiplying
  double result = 1;
  if(nargs_equal(args,1)){
    if( (args[0].isHeadNumber()) )
    {
      result = std::sin(args[0].head().asNumber());
    }
    else{      
      throw SemanticError("Error in call to sin: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to sin: invalid number of arguments.");
  }
  return Expression(result);
};

Expression tan(const std::vector<Expression> & args){
  // check all aruments are numbers, while multiplying
  double result = 1;
  if(nargs_equal(args,1)){
    if( (args[0].isHeadNumber()) )
    {
      result = std::tan(args[0].head().asNumber());
    }
    else{      
      throw SemanticError("Error in call to tan: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to tan: invalid number of arguments.");
  }
  return Expression(result);
};

Expression real(const std::vector<Expression> & args){
  // check all aruments are numbers, while multiplying
  double result = 0;
  if(nargs_equal(args,1)){
    if( (args[0].isHeadComplex()) )
    {
      result = args[0].head().asComplex().real();
    }
    else{      
      throw SemanticError("Error in call to real: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to real: invalid number of arguments.");
  }
  return Expression(result);
};

Expression imag(const std::vector<Expression> & args){
  // check all aruments are numbers, while multiplying
  double result = 0;
  if(nargs_equal(args,1)){
    if( (args[0].isHeadComplex()) )
    {
      result = args[0].head().asComplex().imag();
    }
    else{      
      throw SemanticError("Error in call to imag: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to imag: invalid number of arguments.");
  }
  return Expression(result);
};

Expression mag(const std::vector<Expression> & args){
  // check all aruments are numbers, while multiplying
  double result = 0;
  if(nargs_equal(args,1)){
    if( (args[0].isHeadComplex()) )
    {
      result = abs(args[0].head().asComplex());
    }
    else{      
      throw SemanticError("Error in call to mag: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to mag: invalid number of arguments.");
  }
  return Expression(result);
};

Expression arg(const std::vector<Expression> & args){
  // check all aruments are numbers, while multiplying
  double result = 0;
  if(nargs_equal(args,1)){
    if( (args[0].isHeadComplex()) )
    {
      result = arg(args[0].head().asComplex());
    }
    else{      
      throw SemanticError("Error in call to arg: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to arg: invalid number of arguments.");
  }
  return Expression(result);
};

Expression conj(const std::vector<Expression> & args){
  // check all aruments are numbers, while multiplying
  std::complex<double>result(0,0);
  if(nargs_equal(args,1)){
    if( (args[0].isHeadComplex()) )
    {
      result = conj(args[0].head().asComplex());
    }
    else{      
      throw SemanticError("Error in call to conj: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to conj: invalid number of arguments.");
  }
  return Expression(result);
};


Expression discreteplot(const std::vector<Expression> & args) {

	Expression result(Atom("list"));

		for (auto &a : args)
		{
			result.append(a);
		}

	return result;
}


Expression continuousplot(const std::vector<Expression> & args) {

	Expression result(Atom("list"));

	for (auto &a : args)
	{
		result.append(a);
	}

	return result;
}

const double PI = std::atan2(0, -1);
const double EXP = std::exp(1);
const double e = std::exp(1);
const std::complex<double> I(0,1);

Environment::Environment(){

  reset();
}

bool Environment::is_known(const Atom & sym) const{
  if(!sym.isSymbol()) return false;
  
  return ((envmap.find(sym.asSymbol()) != envmap.end()) );
}


bool Environment::is_exp(const Atom & sym) const{
  if(!sym.isSymbol()) return false;
  
  auto envresult = envmap.find(sym.asSymbol());

  return (envresult != envmap.end() );
}

Expression Environment::get_exp(const Atom & sym) const{

  Expression exp;
  
  if(sym.isSymbol()){
    auto result = envmap.find(sym.asSymbol());
    if((result != envmap.end()) && (result->second.type == ExpressionType)){
      exp = result->second.exp;
    }
  }
  return exp;
}

void Environment::add_exp(const Atom & sym, const Expression & exp){

  if(!sym.isSymbol()){
    throw SemanticError("Attempt to add non-symbol to environment");
  }

  // error if overwriting symbol map
  if (envmap.find(sym.asSymbol()) != envmap.end()){
	  envmap.at(sym.asSymbol()) = EnvResult(ExpressionType, exp);
  }
  else {
	  envmap.emplace(sym.asSymbol(), EnvResult(ExpressionType, exp));
  }
}

bool Environment::is_proc(const Atom & sym) const{
  if(!sym.isSymbol()) return false;
  
  auto result = envmap.find(sym.asSymbol());
  return (result != envmap.end()) && (result->second.type == ProcedureType);
}

Procedure Environment::get_proc(const Atom & sym) const{

  //Procedure proc = default_proc;

  if(sym.isSymbol()){
    auto result = envmap.find(sym.asSymbol());
    if((result != envmap.end()) && (result->second.type == ProcedureType)){
      return result->second.proc;
    }
  }
  return default_proc;
}

bool Environment::isLambda(const Atom&sym) const {
	if (sym.isSymbol()) {
		auto result  = envmap.find(sym.asSymbol());
		if (result != envmap.end())
		{
			if (result->second.exp.head().asSymbol() == "lambda") {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
		
	}
	else {
		return false;
	}
}

/*
  Reset the environment to the default state. First remove all entries and
  then re-add the default ones.
 */
void Environment::reset(){

  envmap.clear();
  
  // Built-In value of pi
  envmap.emplace("pi", EnvResult(ExpressionType, Expression(PI)));
  
  // Build-In value of e
  envmap.emplace("e", EnvResult(ExpressionType, Expression(e)));

  // Build-In value of I
  envmap.emplace("I", EnvResult(ExpressionType, Expression(I)));

  // Procedure: add;
  envmap.emplace("+", EnvResult(ProcedureType, add)); 

  // Procedure: subneg;
  envmap.emplace("-", EnvResult(ProcedureType, subneg)); 

  // Procedure: mul;
  envmap.emplace("*", EnvResult(ProcedureType, mul)); 

  // Procedure: div;
  envmap.emplace("/", EnvResult(ProcedureType, div)); 
 
  // Procedure: ^;
  envmap.emplace("^", EnvResult(ProcedureType, power)); 

  // Procedure: sqrt
  envmap.emplace("sqrt", EnvResult(ProcedureType, sqrt));

  // Procedure: ln
  envmap.emplace("ln", EnvResult(ProcedureType, ln));

  // Procedure: sin
  envmap.emplace("sin", EnvResult(ProcedureType, sin));

  // Procedure: cos
  envmap.emplace("cos", EnvResult(ProcedureType, cos));
  
  // Procedure: tan
  envmap.emplace("tan", EnvResult(ProcedureType, tan));

  // Procedure: real
  envmap.emplace("real", EnvResult(ProcedureType, real));

  // Procedure: imag
  envmap.emplace("imag", EnvResult(ProcedureType, imag));

  // Procedure: mag
  envmap.emplace("mag", EnvResult(ProcedureType, mag));
  
  // Procedure: arg
  envmap.emplace("arg", EnvResult(ProcedureType, arg));

  // Procedure: conj
  envmap.emplace("conj", EnvResult(ProcedureType , conj));

  // Procedure: list
  envmap.emplace("list", EnvResult(ProcedureType, list));

  //Procedure: first
  envmap.emplace("first", EnvResult(ProcedureType, first));

  //Procedure: rest
  envmap.emplace("rest", EnvResult(ProcedureType, rest));

  //Procedure: length
  envmap.emplace("length", EnvResult(ProcedureType, length));

  //Procedure: append
  envmap.emplace("append", EnvResult(ProcedureType, append));

  //Procedure: join
  envmap.emplace("join", EnvResult(ProcedureType, join));

  //Procedure: range
  envmap.emplace("range", EnvResult(ProcedureType, range));

	//Procedure: discrete-plot
	envmap.emplace("discrete-plot", EnvResult(ProcedureType, discreteplot));

	//Procedure: continuous-plot
	envmap.emplace("continuous-plot", EnvResult(ProcedureType, continuousplot));
}


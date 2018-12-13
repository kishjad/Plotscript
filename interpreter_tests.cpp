#include "catch.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "semantic_error.hpp"
#include "interpreter.hpp"
#include "expression.hpp"

Expression run(const std::string & program){
  
  std::istringstream iss(program);
    
  Interpreter interp;
    
  bool ok = interp.parseStream(iss);
  if(!ok){
    std::cerr << "Failed to parse: " << program << std::endl; 
  }
  REQUIRE(ok == true);

  Expression result;
  REQUIRE_NOTHROW(result = interp.evaluate());

  return result;
}
Expression runplot(const std::string & program) {
	Interpreter interp;

	std::string lambda = "(define point (lambda (x y) (list x y)))";
	std::istringstream po(lambda);
	interp.parseStream(po);
	interp.evaluate();

	lambda = "(define make-point (lambda (a b) (set-property \"size\" (0) (set-property \"object-name\" \"point\" (point a b)))))";
	std::istringstream po1(lambda);
	interp.parseStream(po1);
	interp.evaluate();

	lambda = "(define make-point (lambda (a b) (set-property \"size\" (0) (set-property \"object-name\" \"point\" (point a b)))))";
	std::istringstream po2(lambda);
	interp.parseStream(po2);
	interp.evaluate();

	lambda = "(define make-line (lambda (a b) (set-property \"thickness\" (1) (set-property \"object-name\" \"line\" (point a b)))))";
	std::istringstream po3(lambda);
	interp.parseStream(po3);
	interp.evaluate();

	lambda = "(define make-text (lambda (a) (set-property \"text-rotation\" (0) (set-property \"text-scale\" (1) (set-property \"position\" (make-point 0 0) (set-property \"object-name\" \"text\" (a))))) ) )";
	std::istringstream po4(lambda);
	interp.parseStream(po4);
	interp.evaluate();

	std::istringstream iss(program);

	

	bool ok = interp.parseStream(iss);
	if (!ok) {
		std::cerr << "Failed to parse: " << program << std::endl;
	}
	REQUIRE(ok == true);

	Expression result;
	REQUIRE_NOTHROW(result = interp.evaluate());

	return result;
}

TEST_CASE( "Test Interpreter parser with expected input", "[interpreter]" ) {

  std::string program = "(begin (define r 10) (* pi (* r r)))";

  std::istringstream iss(program);
 
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == true);
}

TEST_CASE( "Test Interpreter parser with numerical literals", "[interpreter]" ) {

  std::vector<std::string> programs = {"(1)", "(+1)", "(+1e+0)", "(1e-0)"};
  
  for(auto program : programs){
    std::istringstream iss(program);
 
    Interpreter interp;

    bool ok = interp.parseStream(iss);

    REQUIRE(ok == true);
  }

  {
    std::istringstream iss("(define x 1abc)");
    
    Interpreter interp;

    bool ok = interp.parseStream(iss);

    REQUIRE(ok == false);
  }
}

TEST_CASE( "Test Interpreter parser with truncated input", "[interpreter]" ) {

  {
    std::string program = "(f";
    std::istringstream iss(program);
  
    Interpreter interp;
    bool ok = interp.parseStream(iss);
    REQUIRE(ok == false);
  }
  
  {
    std::string program = "(begin (define r 10) (* pi (* r r";
    std::istringstream iss(program);

    Interpreter interp;
    bool ok = interp.parseStream(iss);
    REQUIRE(ok == false);
  }
}

TEST_CASE( "Test Interpreter parser with extra input", "[interpreter]" ) {

  std::string program = "(begin (define r 10) (* pi (* r r))) )";
  std::istringstream iss(program);

  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with single non-keyword", "[interpreter]" ) {

  std::string program = "hello";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with empty input", "[interpreter]" ) {

  std::string program;
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with empty expression", "[interpreter]" ) {

  std::string program = "( )";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with bad number string", "[interpreter]" ) {

  std::string program = "(1abc)";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with incorrect input. Regression Test", "[interpreter]" ) {

  std::string program = "(+ 1 2) (+ 3 4)";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter result with literal expressions", "[interpreter]" ) {
  
  { // Number
    std::string program = "(4)";
    Expression result = run(program);
    REQUIRE(result == Expression(4.));
  }

  { // Symbol
    std::string program = "(pi)";
    Expression result = run(program);
    REQUIRE(result == Expression(atan2(0, -1)));
  }

}

TEST_CASE( "Test Interpreter result with simple procedures (add)", "[interpreter]" ) {

  { // add, binary case
    std::string program = "(+ 1 2)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(3.));
  }
  
  { // add, 3-ary case
    std::string program = "(+ 1 2 3)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(6.));
  }

  { // add, 6-ary case
    std::string program = "(+ 1 2 3 4 5 6)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(21.));
  }
}
  
TEST_CASE( "Test Interpreter special forms: begin and define", "[interpreter]" ) {

  {
    std::string program = "(define answer 42)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(42.));
  }

  {
    std::string program = "(begin (define answer 42)\n(answer))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(42.));
  }
  
  {
    std::string program = "(begin (define answer (+ 9 11)) (answer))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(20.));
  }

  {
    std::string program = "(begin (define a 1) (define b 1) (+ a b))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(2.));
  }
}

TEST_CASE( "Test a medium-sized expression", "[interpreter]" ) {

  {
    std::string program = "(+ (+ 10 1) (+ 30 (+ 1 1)))";
    Expression result = run(program);
    REQUIRE(result == Expression(43.));
  }
}

TEST_CASE( "Test arithmetic procedures", "[interpreter]" ) {

  {
    std::vector<std::string> programs = {"(+ 1 -2)",
					 "(+ -3 1 1)",
					 "(- 1)",
					 "(- 1 2)",
					 "(* 1 -1)",
					 "(* 1 1 -1)",
					 "(/ -1 1)",
					 "(/ 1 -1)"};

    for(auto s : programs){
      Expression result = run(s);
      REQUIRE(result == Expression(-1.));
    }
  }
}


TEST_CASE( "Test some semantically invalid expresions", "[interpreter]" ) {
  
  std::vector<std::string> programs = {"(@ none)", // so such procedure
				       "(- 1 1 2)", // too many arguments
				       "(define begin 1)", // redefine special form
				       "(define pi 3.14)","(define str \"text\")"}; // redefine builtin symbol
    for(auto s : programs){
      Interpreter interp;

      std::istringstream iss(s);
      
      bool ok = interp.parseStream(iss);
      REQUIRE(ok == true);
      
      //REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE( "Test for exceptions from semantically incorrect input", "[interpreter]" ) {

  std::string input = R"(
(+ 1 a)
)";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "Test malformed define", "[interpreter]" ) {

    std::string input = R"(
(define a 1 2)
)";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "Test using number as procedure", "[interpreter]" ) {
    std::string input = R"(
(1 2 3)
)";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE("Test simple list using an input", "[interpreter]") {
	std::string input = "(list 1 2 3)";
	Expression result = run(input);
	Expression comp(Atom("list"));
	comp.append(1);
	comp.append(2);
	comp.append(3);

	REQUIRE(result == comp);
}

TEST_CASE("Test simple lambda using an input", "[interpreter]") {
	std::string input = "(lambda (x) (+ x 1))";
	Expression result = run(input);
	
	Expression tail1(Atom("+"));
	tail1.append(Atom("x"));
	tail1.append(1);

	Expression comp(Atom("lambda"));
	comp.append(Atom("x"));
	comp.append(tail1);

	REQUIRE(comp != result);
	/*tail1.append()*/
}

TEST_CASE("Testing first in list", "[interpreter]") {
	SECTION("Testing simple list") {
		std::string input = "(first (list 2 3 4))";
		Expression result = run(input);
		Expression comp(2);
		REQUIRE(comp == result);
	}
	SECTION("Testing invalid calls") {
		std::string input = "(first (list))";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
	SECTION("Testing 2 arguments") {
		std::string input = "(first (list 1 2) (list 3 4))";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
}

TEST_CASE("Testing rest in list", "[interpreter]") {
	SECTION("Testing simple list") {
		std::string input = "(rest (list 2 3))";
		Expression result = run(input);
		Expression comp(Atom("list"));
		comp.append(3);
		REQUIRE(comp == result);
	}
	SECTION("Testing simple list with 1 member") {
		std::string input = "(rest (list 2))";
		Expression result = run(input);
		Expression comp(Atom("list"));
		REQUIRE(comp == result);
	}
	SECTION("Testing invalid calls") {
		std::string input = "(rest (list))";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
	SECTION("Testing 2 arguments") {
		std::string input = "(rest (list 1 2) (list 3 4))";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
}

TEST_CASE("Testing length in list", "[interpreter]") {
	SECTION("Testing simple list") {
		std::string input = "(length (list 2 3))";
		Expression result = run(input);
		Expression comp(2);
		REQUIRE(comp == result);
	}
	SECTION("Testing invalid calls") {
		std::string input = "(length 1)";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
	SECTION("Testing 2 arguments") {
		std::string input = "(length (list 1 2) (list 3 4))";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
}

TEST_CASE("Testing join in list", "[interpreter]") {
	SECTION("Testing 2 simple list") {
		std::string input = "(join (list 2 3) (list))";
		Expression result = run(input);
		Expression comp(Atom("list"));
		comp.append(2);
		comp.append(3);
		REQUIRE(comp == result);
	}
	SECTION("Testing invalid calls") {
		std::string input = "(join 1)";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
	SECTION("Testing 2 list arguments") {
		std::string input = "(join (list 1 2) (list 3 4))";
		Expression result = run(input);
		Expression comp(Atom("list"));
		comp.append(1);
		comp.append(2);
		comp.append(3);
		comp.append(4);
		REQUIRE(comp == result);
	}
}

TEST_CASE("Testing range in list", "[interpreter]") {
	SECTION("Testing a simple list creation") {
		std::string input = "(range 0 1 1)";
		Expression result = run(input);
		Expression comp(Atom("list"));
		comp.append(0);
		comp.append(1);
		REQUIRE(comp == result);
	}
	SECTION("Testing invalid argument number") {
		std::string input = "(range 1)";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
}

TEST_CASE("Testing lambda", "[interpreter]") {
	SECTION("Testing a simple list creation") {
		std::string input = "(begin (define x (lambda (y) (+ 1 y))) (x 3))";
		Expression result = run(input);
		Expression comp(4);
		REQUIRE(comp == result);
	}

	SECTION("Testing a simple list creation") {
		std::string input = "(begin (define x (lambda (y) (+ 1 y))) (x 3))";
		Expression result = run(input);
		Expression comp(4);
		REQUIRE(comp == result);
	}
}

TEST_CASE("Testing math functions", "[interpreter]") {
	SECTION("Testing sqrt") {
		std::string input = "(- I (sqrt -1))";
		Expression result = run(input);
		Expression comp(std::complex<double>(0,0));
		REQUIRE(comp == result);
	}
	SECTION("Testing sqrt") {
		std::string input = "(sqrt I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0.70710678118654757, 0.70710678118654757));
		REQUIRE(comp == result);
	}
	SECTION("Testing arg") {
		std::string input = "(arg I)";
		Expression result = run(input);
		Expression comp(1.5707963267948966);
		REQUIRE(comp == result);
	}
	SECTION("Testing conj") {
		std::string input = "(conj I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0, -1));
		REQUIRE(comp == result);
	}
	SECTION("Testing mag") {
		std::string input = "(mag I)";
		Expression result = run(input);
		Expression comp(1);
		REQUIRE(comp == result);
	}
	SECTION("Testing ^") {
		std::string input = "(^ 2 3)";
		Expression result = run(input);
		Expression comp(8);
		REQUIRE(comp == result);
	}
	SECTION("Testing cos") {
		std::string input = "(cos 0)";
		Expression result = run(input);
		Expression comp(1);
		REQUIRE(comp == result);
	}
	SECTION("Testing sin") {
		std::string input = "(sin 0)";
		Expression result = run(input);
		Expression comp(0);
		REQUIRE(comp == result);
	}
	SECTION("Testing tan") {
		std::string input = "(tan 0)";
		Expression result = run(input);
		Expression comp(0);
		REQUIRE(comp == result);
	}
	SECTION("Testing ln") {
		std::string input = "(ln e)";
		Expression result = run(input);
		Expression comp(1);
		REQUIRE(comp == result);
	}
	SECTION("Testing imag") {
		std::string input = "(imag I)";
		Expression result = run(input);
		Expression comp(1);
		REQUIRE(comp == result);
	}
	SECTION("Testing functions that throw error for complex") {
		std::vector<std::string>input = { "(tan I)","(cos I)","(sin I)","(ln I)","(imag 3)",
										   "(imag 3 2)", "(real 23 3)", "(tan I 3)", "(cos 2 3)",
										  "(sin 3 4)", "(ln 4 3)", "(first 3)", "(rest 3)",
										  "(join 2 3)", "(range 3 -1 1)","(range 0 5 -1)",\
										  "(range 3 + 1)","(+ 1 +)","(* + 3)","(* 3 +)","(- y)",
										  "(- 4 +)","(- 43 4 4)","(/ 72 2 2)","(mag 3)","(mag 3 2)",
										  "(arg 3)", "(arg 3 2)", "(conj 3)", "(conj 3 2)","(^ 4 3 3)","(sqrt 23 3)","(begin)","(map)","(map a)",
											"(apply)"};
		for (auto a : input) {
			std::istringstream iss(a);
			Interpreter interp;
			bool ok = interp.parseStream(iss);
			REQUIRE(ok == true);
			REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
		}
	}
	SECTION("Testing + with complex-complex") {
		std::string input = "(+ I I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0,2));
		REQUIRE(comp == result);
	}
	SECTION("Testing - with complex-complex") {
		std::string input = "(- I I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0, 0));
		REQUIRE(comp == result);
	}
	SECTION("Testing * with complex-complex") {
		std::string input = "(* I I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(-1, 0));
		REQUIRE(comp == result);
	}
	SECTION("Testing / with complex-complex") {
		std::string input = "(/ I I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(1, 0));
		REQUIRE(comp == result);
	}
	SECTION("Testing + with complex-real") {
		std::string input = "(+ I 2)";
		Expression result = run(input);
		Expression comp(std::complex<double>(2,1));
		REQUIRE(comp == result);
	}
	SECTION("Testing + with real-complex") {
		std::string input = "(+ 2 I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(2, 1));
		REQUIRE(comp == result);
	}
	SECTION("Testing - with complex-real") {
		std::string input = "(- I 4)";
		Expression result = run(input);
		Expression comp(std::complex<double>(-4,1));
		REQUIRE(comp == result);
	}
	SECTION("Testing - with real-complex") {
		std::string input = "(- 4 I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(4, -1));
		REQUIRE(comp == result);
	}
	SECTION("Testing - with single complex") {
		std::string input = "(- I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0, -1));
		REQUIRE(comp == result);
	}
	SECTION("Testing * with complex-real") {
		std::string input = "(* I 2)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0, 2));
		REQUIRE(comp == result);
	}
	SECTION("Testing * with real-complex") {
		std::string input = "(* 2 I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0, 2));
		REQUIRE(comp == result);
	}
	SECTION("Testing / with complex-real") {
		std::string input = "(/ I 1)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0,1));
		REQUIRE(comp == result);
	}
	SECTION("Testing / complex") {
		std::string input = "(/ I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0, -1));
		REQUIRE(comp == result);
	}
	SECTION("Testing / with real-complex") {
		std::string input = "(/ 1 I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0, -1));
		REQUIRE(comp == result);
	}
	SECTION("Testing ^ with real-complex") {
		std::string input = "(^ 1 I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(1,0));
		REQUIRE(comp == result);
	}
	SECTION("Testing ^ with complex-complex") {
		std::string input = "(^ I I)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0.20787957635076193,0));
		REQUIRE(comp == result);
	}
	SECTION("Testing ^ with real-real") {
		std::string input = "(^ 2 3)";
		Expression result = run(input);
		Expression comp(8);
		REQUIRE(comp == result);
	}
	SECTION("Testing ^ with complex-real") {
		std::string input = "(^ I 1)";
		Expression result = run(input);
		Expression comp(std::complex<double>(0, 1));
		REQUIRE(comp == result);
	}
}

TEST_CASE("Testing append list", "[interpreter]") {
	SECTION("append a simple list") {
		std::string input = "(begin (define x (list)) (append x 1))";
		Expression result = run(input);
		Expression comp(Atom("list"));
		comp.append(1);
		REQUIRE(comp == result);
	}
	SECTION("append throws exception for no list type") {
		std::string input = "(begin (define x (1)) (append x 1))";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
	SECTION("append throws exception number of arguments") {
		std::string input = "(begin (define x (list)) (append 1))";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
}

TEST_CASE("Testing apply", "[interpreter]") {
	SECTION("apply list to a lambda function") {
		std::string input = "(begin (define complexAsList (lambda (x) (list (real x)))) (apply complexAsList (list (+ 1 (* 3 I)))))";
		Expression result = run(input);
		Expression comp(Atom("list"));
		comp.append(1);
		REQUIRE(comp == result);
	}

	SECTION("apply list to a procedure") {
		std::string input = "(apply + (list 1 2))";
		Expression result = run(input);
		Expression comp(3);
		REQUIRE(comp == result);
	}
	SECTION("Invalid first argument") {
		std::string input = "(apply (+ 1) (list 1 2))";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
	SECTION("INvalid second argument") {
		std::string input = "(apply + 3)";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
}

TEST_CASE("Testing map", "[interpreter]") {
	SECTION("map list to a lambda function") {
		std::string input = "(begin (define f (lambda (x) (ln x))) (map f (list e (^ e 2))))";
		Expression result = run(input);
		Expression comp(Atom("list"));
		comp.append(1);
		comp.append(2);
		REQUIRE(comp == result);
	}

	SECTION("map list to a procedure") {
		std::string input = "(map / (list 1 2))";
		Expression result = run(input);
		Expression comp(Atom("list"));
		comp.append(1);
		comp.append(0.5);
		REQUIRE(comp == result);
	}
	SECTION("Invalid first argument") {
		std::string input = "(map (+ 1) (list 1 2))";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
	SECTION("INvalid second argument") {
		std::string input = "(map + (+ 3 2))";
		std::istringstream iss(input);
		Interpreter interp;
		bool ok = interp.parseStream(iss);
		REQUIRE(ok == true);
		REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
	}
}

TEST_CASE("invalid expressions", "[interpreter]") {
	SECTION("invalid define calls") {
		std::vector<std::string>input = { "(define 1 32)","(define define 3)","(define ^ 3)" };
		for (auto a : input) {
			std::istringstream iss(a);
			Interpreter interp;
			bool ok = interp.parseStream(iss);
			REQUIRE(ok == true);
			REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
		}
	
	}
	SECTION("invalid lambda calls") {
		std::vector<std::string>input = {"(lambda (c) (s) (s))","(begin (define x (lambda (d) (+ 1 d))) (x 3 2))"};
		for (auto a : input) {
			std::istringstream iss(a);
			Interpreter interp;
			bool ok = interp.parseStream(iss);
			REQUIRE(ok == true);
			REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
		}

	}
	SECTION("invalid begin calls") {
		std::vector<std::string>input = { "(begin)" };
		for (auto a : input) {
			std::istringstream iss(a);
			Interpreter interp;
			bool ok = interp.parseStream(iss);
			REQUIRE(ok == true);
			REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
		}

	}
}
TEST_CASE("set-property", "[interpreter]") {
	SECTION("invalid set-property calls") {
		std::vector<std::string>input = { "(set-property define define 2)","(set-property a)","(set-property a z)","(set-property (+ 1 2) \"number\" \"three\")" };
		for (auto a : input) {
			std::istringstream iss(a);
			Interpreter interp;
			bool ok = interp.parseStream(iss);
			REQUIRE(ok == true);
			REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
		}
	}
	SECTION("valid set-property calls") {
		std::string input = "(set-property \"number\" (3) \"three\")";
		Expression result = run(input);
		Expression result2(Atom("\"three\""));
		REQUIRE(result2.head().asSymbol() == result.head().asSymbol());
	}
}
//(define b(set - property "note" "a complex number" a))

TEST_CASE("get-property", "[interpreter]") {
	SECTION("invalid get-property calls") {
		std::vector<std::string>input = { "(get-property \"name\")","(get-property define)","(get-property a z)","(get-property (+ 1 2) \"number\" \"three\")","(get-property a)" };
		for (auto a : input) {
			std::istringstream iss(a);
			Interpreter interp;
			bool ok = interp.parseStream(iss);
			REQUIRE(ok == true);
			REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
		}
	}

	SECTION("valid get-property NONE calls") {
		std::string input = "(begin (define b (10)) (get-property \"foo\" b))";
		Expression result = run(input);
		Expression result2(Atom("NONE"));
		REQUIRE(result2 == result);
	}
}

TEST_CASE("get tail", "[interpreter]") {

	SECTION("check empty tail") {
		Expression result(Atom("A"));
		auto begin = result.tailBegin();
		auto end = result.tailEnd();
		REQUIRE(begin == end);
	}
	SECTION("check const empty tail") {
		Expression result(Atom("A"));
		const auto begin = result.tailBegin();
		const auto end = result.tailEnd();
		REQUIRE(begin == end);
	}

	SECTION("check non empty tail") {
		Expression result(Atom("A"));
		Expression tail(Atom(1));
		result.append(tail);
		auto begin = result.tailBegin();
		auto end = result.tailEnd();
		REQUIRE(begin != end);
	}
}

TEST_CASE("discrete plot", "[interpreter]") {

	SECTION("check errors") {
		std::vector<std::string>input = { "(discrete-plot (-1 1))","(discrete-plot dog)" };
		for (auto a : input) {
			std::istringstream iss(a);
			Interpreter interp;
			bool ok = interp.parseStream(iss);
			REQUIRE(ok == true);
			REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
		}
	}
	SECTION("check discretr plot without properties") {
		std::string input = "(discrete-plot (list (list -1 -1) (list 1 1)))";
		Expression result = runplot(input);
		REQUIRE((result.tailBegin() + 14) == result.tailEnd());
	}

	SECTION("check non empty tail") {
		std::string input = "(begin (define f (lambda (x) (list x (+ (* 2 x) 1)))) (discrete-plot (map f (range -2 2 0.5)) (list (list \"title\" \"The Data\") (list \"abscissa-label\" \"X Label\") (list \"ordinate-label\" \"Y Label\") (list \"text-scale\" 1))))";
		Expression result = runplot(input);
		REQUIRE((result.tailBegin() + 31) == result.tailEnd());
	}
}

TEST_CASE("continuous plot", "[interpreter]") {

	SECTION("check errors") {
		std::vector<std::string>input = {"(continuous-plot (-1 1))","(continuous-plot dog)","(continuous-plot (5) (list -2 2))" };
		for (auto a : input) {
			std::istringstream iss(a);
			Interpreter interp;
			bool ok = interp.parseStream(iss);
			REQUIRE(ok == true);
			REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
		}
	}
	SECTION("check continuous plot without properties") {
		std::string input = "(begin (define f (lambda(x) (/ 1 (+ 1 (^ e (- (* 20 x))))))) (continuous-plot f (list -1 1)))";
		Expression result = runplot(input);
		REQUIRE(result.tailBegin() != result.tailEnd());

	}

}

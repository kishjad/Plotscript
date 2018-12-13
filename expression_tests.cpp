#include "catch.hpp"

#include "expression.hpp"
#include "ostream"

TEST_CASE( "Test default expression", "[expression]" ) {

  Expression exp;

  REQUIRE(!exp.isHeadNumber());
  REQUIRE(!exp.isHeadSymbol());
  REQUIRE(!exp.isHeadComplex());
}

TEST_CASE( "Test double expression", "[expression]" ) {

  Expression exp(6.023);

  REQUIRE(exp.isHeadNumber());
  REQUIRE(!exp.isHeadSymbol());
  REQUIRE(!exp.isHeadComplex());
}


TEST_CASE( "Test symbol expression", "[expression]" ) {

  Expression exp(Atom("asymbol"));

  REQUIRE(!exp.isHeadNumber());
  REQUIRE(exp.isHeadSymbol());
  REQUIRE(!exp.isHeadComplex());
}

TEST_CASE( "Test complex expression", "[expression]" ) {

  Expression exp(std::complex<double>(12,3));

  REQUIRE(!exp.isHeadNumber());
  REQUIRE(!exp.isHeadSymbol());
  REQUIRE(exp.isHeadComplex());
}

TEST_CASE(" Test append function", "[expression]") {
	Expression exp;
	Expression app(Atom("temp"));
	
	exp.append(app);

	REQUIRE(exp.tail() != nullptr);
}
TEST_CASE(" Test prop function", "[expression]") {
	Expression exp;
	Expression app(Atom("temp"));

	exp.append(app);
	auto pro = exp.prop();
	REQUIRE(pro.size() == 0);
}
TEST_CASE(" Test const prop function", "[expression]") {
	const Expression exp;
	const auto pro = exp.prop();
	REQUIRE(pro.size() == 0);
}

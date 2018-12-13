/*! \file expression.hpp
Defines the Expression type and assiciated functions.
 */
#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <string>
#include <vector>
#include <map>
#include "token.hpp"
#include "atom.hpp"


//static auto ptr = &INTERRUPT_IS_SET;
// forward declare Environment
class Environment;

/*! \class Expression
\brief An expression is a tree of Atoms.

An expression is an atom called the head followed by a (possibly empty) 
list of expressions called the tail.
 */
class Expression {
public:

  typedef std::vector<Expression>::const_iterator ConstIteratorType;
	//typedef std::vector<Expression>::iterator IteratorType;

  /// Default construct and Expression, whose type in NoneType
  Expression();
	
  /*! Construct an Expression with given Atom as head an empty tail
    \param atom the atom to make the head
  */
  Expression(const Atom & a);

  /// deep-copy construct an expression (recursive)
  Expression(const Expression & a);

  /// deep-copy assign an expression  (recursive)
  Expression & operator=(const Expression & a);

  /// return a reference to the head Atom
  Atom & head();

	/// return a reference to the property map
	std::map<std::string, Expression>& prop();
	 
	///returns a const refernce to prop map
	const std::map<std::string, Expression>& prop() const;

  /// return a const-reference to the head Atom
  const Atom & head() const;

  /// append Atom to tail of the expression
  void append(const Atom & a);

  /// return a pointer to the last expression in the tail, or nullptr
  Expression * tail();

  /// return a const-iterator to the beginning of tail
  ConstIteratorType tailConstBegin() const noexcept;

  /// return a const-iterator to the tail end
  ConstIteratorType tailConstEnd() const noexcept;

	///return a iterator to the beginning of tail
	std::vector<Expression>::iterator tailBegin() noexcept;

	/// return a const-iterator to the tail end
	std::vector<Expression>::iterator tailEnd() noexcept;

  /// convienience member to determine if head atom is a number
  bool isHeadNumber() const noexcept;

  /// convienience member to determine if head atom is a symbol
  bool isHeadSymbol() const noexcept;

  /// conviniennce member to determine if head atom is a complex
  bool isHeadComplex() const noexcept;

  /// Evaluate expression using a post-order traversal (recursive)
  Expression eval(Environment & env);

  /// Evaluate lambda expression using a post-order traversal (recursive)
  Expression lambdaEval(const Atom & sym, Environment & env, std::vector<Expression>m_tail);

  /// equality comparison for two expressions (recursive)
  bool operator==(const Expression & exp) const noexcept;
  
  /// Appending expressions into expressions
  void append(const Expression &E);

	void reset();
  
private:

  // the head of the expression
  Atom m_head;

  // the tail list is expressed as a vector for access efficiency
  // and cache coherence, at the cost of wasted memory.
  std::vector<Expression> m_tail;

	std::map<std::string,Expression> property;

  // convenience typedef
  typedef std::vector<Expression>::iterator IteratorType;
  
  // internal helper methods
  Expression handle_lookup(const Atom & head, const Environment & env);
  Expression handle_define(Environment & env);
  Expression handle_begin(Environment & env);
  Expression handle_lambda(Environment & env);
  Expression handle_apply(Environment &env);
  Expression handle_map(Environment &env);
	Expression handle_set_property(Environment &env);
	Expression handle_get_property(Environment &env);
	Expression handle_discrete_plot(Environment &env);
	Expression handle_continuous_plot(Environment &env);

	Expression construct_line(Expression&p1, Expression&p2, Environment &env);

	Expression construct_point(double x, double y, Environment &env);

	std::map<std::string, double> scaling_factor_and_bounds(const Expression&data);

	Expression add_options(Environment &env, std::map<std::string, double> &values);

	Expression add_boundaries(Environment &env, std::map<std::string, double> &value);

	Expression add_axes(Environment &env, std::map<std::string, double> &value);

	Expression draw_discrete(Environment &env, std::map<std::string, double> &value);
	Expression draw_continuous(Environment &env, std::map<std::string, double> &value, const Atom & sym);

	Expression add_labels(Environment &env, std::map<std::string, double> &values);

	void sample_points(Environment &env, Expression& newdata, const Atom & sym);

	bool implement_iteration(Environment &env, Expression& new_points,Expression& current_points,const Atom & sym);

	double angle_between(Expression p1, Expression p2, Expression p3);

	
};

/// Render expression to output stream
std::ostream & operator<<(std::ostream & out, const Expression & exp);

/// inequality comparison for two expressions (recursive)
bool operator!=(const Expression & left, const Expression & right) noexcept;
  
#endif

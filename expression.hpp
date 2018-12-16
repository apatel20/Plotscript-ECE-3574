/*! \file expression.hpp
Defines the Expression type and assiciated functions.
 */
#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <string>
#include <vector>
#include <utility>
#include <map>
#include <csignal>
#include <cstdlib>

#include "token.hpp"
#include "atom.hpp"

// forward declare Environment
class Environment;

extern volatile sig_atomic_t global_status_flag;

/*! \class Expression
\brief An expression is a tree of Atoms.

An expression is an atom called the head followed by a (possibly empty)
list of expressions called the tail.
 */
class Expression {
public:

  typedef std::vector<Expression>::const_iterator ConstIteratorType;

  /// Default construct and Expression, whose type in NoneType
  Expression();

  /*! Construct an Expression with given Atom as head an empty tail
    \param atom the atom to make the head
  */
  Expression(const Atom & a);

  /// deep-copy construct an expression (recursive)
  Expression(const Expression & a);

  Expression(const std::vector<Expression> & a);

  /// deep-copy assign an expression  (recursive)
  Expression & operator=(const Expression & a);

  /// return a reference to the head Atom
  Atom & head();

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

  /// convienience member to determine if head atom is a number
  bool isHeadNumber() const noexcept;

  bool isHeadComplex() const noexcept;

  /// convienience member to determine if head atom is a symbol
  bool isHeadSymbol() const noexcept;

  bool isHeadList() const noexcept;

  bool isHeadLambda() const noexcept;

  bool isHeadNone() const noexcept;

  Expression handleMakePoint() const noexcept;

  Expression handleMakeLine() const noexcept;

  Expression handleMakeText() const noexcept;

  Expression handleRotation() const noexcept;

  Expression handleScale() const noexcept;

  Expression searchMap() const noexcept;

  /// Evaluate expression using a post-order traversal (recursive)
  Expression eval(Environment & env);

  /// equality comparison for two expressions (recursive)
  bool operator==(const Expression & exp) const noexcept;

  int tailSize() const noexcept;

  int getPropSize() const noexcept;

  Expression getExpressionFirst() const noexcept;

  Expression getTail(int location) const noexcept;

  void generateBoundingBoxLines(Expression & plotResult, double & scaledAU, double & scaledAL, double & scaledOU, double & scaledOL);

  void generateTextLabels(Expression & plotResult, Expression & evalText, double & scaledAU, double & scaledAL, double & scaledOL, double & scaledOU);

  void createStrings(double & au, double & al, double & ol, double & ou, double & scaledAU, double & scaledAL, double & scaledOL, double & scaledOU, Expression & plotResult);

  void scalePointsLines(Expression & plotResult, Expression & evalText, Expression & evalPoints, double & au, double & al, double & ol, double & ou);

  void continuousBoundingBox(Expression & conPlotResult, double & scaledOL2, double & scaledOU2, double & scaledAL2, double & scaledAU2);

  void continuousTextLabels(Expression & conPlotResult, Expression & evalText, double & scaledOL2, double & scaledOU2, double & scaledAL2, double & scaledAU2);

  void continuousCreateStrings(Expression & conPlotResult, double & au2, double & al2, double & ol2, double & ou2, double & scaledAU2, double & scaledAL2, double & scaledOL2, double & scaledOU2);

  void continuousAngleCalc(Expression & conPlotResult, std::vector<Expression> points, double & xscale, double & yscale, Environment & env);

  std::vector<Expression> splitLine(Expression point1, Expression point2, Expression point3, double & xscale, double & yscale, Environment & env);
private:

  // the head of the expression
  Atom m_head;

  // the tail list is expressed as a vector for access efficiency
  // and cache coherence, at the cost of wasted memory.
  std::vector<Expression> m_tail;

  // convenience typedef
  typedef std::vector<Expression>::iterator IteratorType;

  // internal helper methods
  Expression handle_lookup(const Atom & head, const Environment & env);
  Expression handle_define(Environment & env);
  Expression handle_begin(Environment & env);
  Expression handle_lambda(Environment & env);
  Expression handle_apply(Environment & env);
  Expression handle_map(Environment & env);
  Expression handle_setProp(Environment & env);
  Expression handle_getProp(Environment & env);
  Expression handle_discretePlot(Environment & env);
  Expression handle_continuousPlot(Environment & env);

  void handleApplyLambda(const std::vector<Expression> & arg, const std::vector<Expression> & input, Environment & env, const Expression & proc);

  std::string round(double num);
  bool checkAngle175(Expression point1, Expression point2, Expression point3);

  std::map<std::string, Expression> prop;
};

/// Render expression to output stream
std::ostream & operator<<(std::ostream & out, const Expression & exp);

/// inequality comparison for two expressions (recursive)
bool operator!=(const Expression & left, const Expression & right) noexcept;

#endif

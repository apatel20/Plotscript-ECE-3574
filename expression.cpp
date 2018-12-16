#include "expression.hpp"
#include "consumer.hpp"

#include <sstream>
#include <list>
#include <complex>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <math.h>
#include <atomic>


using std::endl;
using std::cout;

#include "environment.hpp"
#include "semantic_error.hpp"

volatile sig_atomic_t global_status_flag = 0;

Expression::Expression(){}

Expression::Expression(const Atom & a){
  m_head = a;
}

int Expression::tailSize() const noexcept
{
  return m_tail.size();//helper to get tail size
}

Expression Expression::getExpressionFirst() const noexcept //helper to get the frist eleemnt of m_tail
{
  return m_tail[0];
}

Expression Expression::getTail(int location) const noexcept
{
  return m_tail[location];
}

Expression::Expression(const std::vector<Expression> & a)//create a vector of expressions
{
  m_head.setList();//set the listkind
  for(auto & e : a)
  {
    m_tail.push_back(e);//push the elements back
  }
}

// recursive copy
Expression::Expression(const Expression & a){
  m_head = a.m_head;
  prop = a.prop;
  for(auto e : a.m_tail){
    m_tail.push_back(e);
  }
}

Expression & Expression::operator=(const Expression & a){

  // prevent self-assignment
  if(this != &a){
    m_head = a.m_head;
    prop = a.prop;
    m_tail.clear();
    for(auto e : a.m_tail){
      m_tail.push_back(e);
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

bool Expression::isHeadNumber() const noexcept{
  return m_head.isNumber();
}

bool Expression::isHeadSymbol() const noexcept{
  return m_head.isSymbol();
}

bool Expression::isHeadComplex() const noexcept{
  return m_head.isComplex();//check if complex
}

bool Expression::isHeadList() const noexcept{
  return m_head.isList();//check if list
}

bool Expression::isHeadNone() const noexcept
{
  return m_head.isNone();
}

void Expression::append(const Atom & a){
  m_tail.emplace_back(a);
}


Expression * Expression::tail(){
  Expression * ptr = nullptr;

  if(m_tail.size() > 0){
    ptr = &m_tail.back();
  }

  return ptr;
}

Expression::ConstIteratorType Expression::tailConstBegin() const noexcept{
  return m_tail.cbegin();
}

Expression::ConstIteratorType Expression::tailConstEnd() const noexcept{
  return m_tail.cend();
}

void handleApplyLambda(const std::vector<Expression> & arg, const std::vector<Expression> & input, Environment & env)
{
  //check if variables have already been defined
  for (auto it = input.begin(), it2 = arg.begin(); it != input.end(), it2 != arg.end(); ++it, ++it2)
  {
    env.add_replace((*it).head(), (*it2));//ceheck to see if in env
  }
}

Expression apply(const Atom & op, const std::vector<Expression> & args, const Environment & env){

  if (env.is_exp(op))
  {
    Environment env2(env);
    Expression expLambda = env2.get_exp(op);
    Expression procedure;
    std::vector<Expression> arguments;

    for (auto it = expLambda.tailConstBegin(); it != expLambda.tailConstEnd(); ++it)
    {
      if ((*it).isHeadList())
      {
        for (auto it2 = (*it).tailConstBegin(); it2 != (*it).tailConstEnd(); ++it2)
        {
          arguments.push_back(*it2);
        }
        if (args.size() != arguments.size())
        {
          throw SemanticError("Error during evaluation: arguments do not match up with args");
        }
      }
      else
      {
        procedure = (*it);//get the procedure and evaluate after putting expression in environment
      }
    }
    handleApplyLambda(args, arguments, env2);

    return procedure.eval(env2);
  }

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
      if(env.is_exp(head) || env.is_proc(head)){
	       return env.get_exp(head);
      }
      else if (head.asSymbol()[0] == 34 && head.asSymbol()[head.asSymbol().length()-1] == 34)
      {
        return Expression(head);
      }
      else{
	       throw SemanticError("Error during evaluation: unknown symbol");
      }
    }
    else if(head.isNumber()){
      return Expression(head);
    }
    else{
      throw SemanticError("Error during evaluation: Invalid type in terminal expression");
    }
}

Expression Expression::handle_apply(Environment & env)
{
  if (m_tail.size() != 2)
  {
    throw SemanticError("Error: not enough arguments in apply");//error checking
  }
  if (m_tail[1].head().asSymbol() != "list")
  {
    throw SemanticError("Error: second argument to apply not a list");
  }
  Expression removeSymbols = m_tail[1].eval(env);//eval the proc to get the expression

  if (env.is_exp(m_tail[0].head()))
  {
    std::vector<Expression> tempExpression;
    for (auto it = removeSymbols.tailConstBegin(); it != removeSymbols.tailConstEnd(); ++it)//iterate through
    {
      tempExpression.push_back(*it);
    }
    return apply(m_tail[0].head(), tempExpression, env);//apply the expression of arguments to the lambda
  }

  if (m_tail[0].tailSize() != 0 || !env.is_proc(m_tail[0].head()))
  {
    throw SemanticError("Error: first argument to apply not a procedure");
  }

  Expression newExp(m_tail[0].head());
  for (auto it = m_tail[1].tailConstBegin(); it != m_tail[1].tailConstEnd(); ++it)
  {
    newExp.append((*it).head());//if calling apply directly without lambda
  }
  return newExp.eval(env);
}

Expression Expression::handle_lambda(Environment & env)
{
    if(m_tail.size() != 2){
      throw SemanticError("Error during evaluation: not enough or too much arguments to lambda");
    }

    if(!m_tail[0].isHeadSymbol()){
      throw SemanticError("Error during evaluation: first argument to begin not symbol");
    }

    std::vector<Expression> expressions;

    Expression temp;
    temp.m_tail.push_back(m_tail[0].head());
    temp.head().setList();

    for (auto it = m_tail[0].tailConstBegin(); it != m_tail[0].tailConstEnd(); ++it)
    {
      temp.m_tail.push_back(*it);
    }

    expressions.push_back(Expression(temp));
    expressions.push_back(Expression(m_tail[1]));

    return Expression(expressions);
}

Expression Expression::handle_map(Environment & env)
{//very similar to apply, difference is that you iterate through the args to map to lambda
  if (m_tail.size() != 2)
  {
    throw SemanticError("Error: not enough arguments in map");
  }

  Expression resultList = m_tail[1].eval(env);

  if (m_tail[1].head().asSymbol() != "list" && !resultList.isHeadList())
  {
    throw SemanticError("Error: second argument to map not a list");
  }
  Expression removeSymbols = m_tail[1].eval(env);
  if (env.is_exp(m_tail[0].head()))
  {
    std::vector<Expression> tempExpression;
    std::vector<Expression> finalResult;
    for (auto it = removeSymbols.tailConstBegin(); it != removeSymbols.tailConstEnd(); ++it)
    {
      tempExpression.push_back(*it);
      Expression returned = apply(m_tail[0].head(),tempExpression, env);
      finalResult.push_back(returned);
      tempExpression.clear();
    }
    return Expression(finalResult);
  }

  if (m_tail[0].tailSize() != 0 || !env.is_proc(m_tail[0].head()))
  {
    throw SemanticError("Error: first argument to map not a procedure");
  }

  std::vector<Expression> tempExpression;
  std::vector<Expression> finalResult;
  for (auto it = removeSymbols.tailConstBegin(); it != removeSymbols.tailConstEnd(); ++it)
  {
    tempExpression.push_back(*it);
    Expression returned = apply(m_tail[0].head(),tempExpression, env);
    finalResult.push_back(returned);
    tempExpression.clear();
  }
  return Expression(finalResult);
}

Expression Expression::handle_begin(Environment & env){

  if(m_tail.size() == 0){
    throw SemanticError("Error during evaluation: zero arguments to begin");
  }

  if(!m_tail[0].isHeadSymbol()){
    throw SemanticError("Error during evaluation: first argument to begin not symbol");
  }

  // evaluate each arg from tail, return the last
  Expression result;
  for(Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it){
    result = it->eval(env);
  }

  return result;
}

Expression Expression::handle_setProp(Environment & env){

  if(m_tail.size() != 3)
  {
    throw SemanticError("Error during evaluation: not enough arguments to set property");
  }

  if (m_tail[0].head().asSymbol()[0] != 34 || m_tail[0].head().asSymbol()[m_tail[0].head().asSymbol().length()-1] != 34)
  {
    throw SemanticError("Error: first argument to set-property not a string");
  }

  Expression third = m_tail[2].eval(env);
  m_tail[1].eval(env);

  third.prop[m_tail[0].head().asSymbol()] = m_tail[1].eval(env);

  return third;
}

Expression Expression::handle_getProp(Environment & env)
{
  if (m_tail.size() != 2)
  {
    throw SemanticError("Error during evaluation: not enough arguments to get property");
  }

  if (m_tail[0].head().asSymbol()[0] != 34 || m_tail[0].head().asSymbol()[m_tail[0].head().asSymbol().length()-1] != 34)
  {
    throw SemanticError("Error: first argument to get-property not a string");
  }

  return (m_tail[1].eval(env).prop[m_tail[0].head().asSymbol()]);
}


Expression Expression::handle_define(Environment & env){

  // tail must have size 3 or error
  if(m_tail.size() != 2){
    throw SemanticError("Error during evaluation: invalid number of arguments to define");
  }

  // tail[0] must be symbol
  if(!m_tail[0].isHeadSymbol()){
    throw SemanticError("Error during evaluation: first argument to define not symbol");
  }

  // but tail[0] must not be a special-form or procedure
  std::string s = m_tail[0].head().asSymbol();//stores the first argument after define
  if((s == "define") || (s == "begin") || (s == "apply")){
    throw SemanticError("Error during evaluation: attempt to redefine a special-form");
  }

  // eval tail[1]
  Expression result = m_tail[1].eval(env);

  //and add to env
  env.add_exp(m_tail[0].head(), result);

  return result;
}


void Expression::generateBoundingBoxLines(Expression & plotResult, double & scaledAU, double & scaledAL, double & scaledOU, double & scaledOL)
{
  std::vector<Expression> resultLines;

  Expression pointBottomLeft;
  pointBottomLeft.append(Atom(scaledAL));
  pointBottomLeft.append(Atom(-scaledOL));
  pointBottomLeft.head().setList();

  Expression pointTopLeft;
  pointTopLeft.append(Atom(scaledAL));
  pointTopLeft.append(Atom(-scaledOU));
  pointTopLeft.head().setList();

  Expression pointBottomRight;
  pointBottomRight.append(Atom(scaledAU));
  pointBottomRight.append(Atom(-scaledOL));
  pointBottomRight.head().setList();


  Expression pointTopRight;
  pointTopRight.append(Atom(scaledAU));
  pointTopRight.append(Atom(-scaledOU));
  pointTopRight.head().setList();

  double xmiddle = (scaledAU+scaledAL)/(scaledAU*20-scaledAL*20);
  double ymiddle = (scaledOU+scaledOL)/(scaledOU*20-scaledOL*20);

  Expression pointMiddleTop;
  pointMiddleTop.append(Atom(xmiddle));
  pointMiddleTop.append(Atom(-scaledOU));
  pointMiddleTop.head().setList();

  Expression pointRightMiddle;
  pointRightMiddle.append(Atom(scaledAU));
  pointRightMiddle.append(Atom(-ymiddle));
  pointRightMiddle.head().setList();

  Expression pointBottomMiddle;
  pointBottomMiddle.append(Atom(xmiddle));
  pointBottomMiddle.append(Atom(-scaledOL));
  pointBottomMiddle.head().setList();

  Expression pointLeftMiddle;
  pointLeftMiddle.append(Atom(scaledAL));
  pointLeftMiddle.append(Atom(-ymiddle));
  pointLeftMiddle.head().setList();


  //now to create the 6 lines using the points

  Expression linePoints;
  linePoints.m_tail.push_back(pointTopLeft);
  linePoints.m_tail.push_back(pointTopRight);
  linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
  linePoints.prop["\"thickness\""] = Expression(0);
  linePoints.head().setList();
  plotResult.m_tail.push_back(linePoints);
  linePoints.m_tail.clear();

  linePoints.m_tail.push_back(pointTopRight);
  linePoints.m_tail.push_back(pointBottomRight);
  linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
  linePoints.prop["\"thickness\""] = Expression(0);
  plotResult.m_tail.push_back(linePoints);
  linePoints.m_tail.clear();

  linePoints.m_tail.push_back(pointBottomRight);
  linePoints.m_tail.push_back(pointBottomLeft);
  linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
  linePoints.prop["\"thickness\""] = Expression(0);
  plotResult.m_tail.push_back(linePoints);
  linePoints.m_tail.clear();

  linePoints.m_tail.push_back(pointBottomLeft);
  linePoints.m_tail.push_back(pointTopLeft);
  linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
  linePoints.prop["\"thickness\""] = Expression(0);
  plotResult.m_tail.push_back(linePoints);
  linePoints.m_tail.clear();

  linePoints.m_tail.push_back(pointMiddleTop);
  linePoints.m_tail.push_back(pointBottomMiddle);
  linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
  linePoints.prop["\"thickness\""] = Expression(0);
  plotResult.m_tail.push_back(linePoints);
  linePoints.m_tail.clear();

  if (scaledOL <= 0)
  {
    linePoints.m_tail.push_back(pointLeftMiddle);
    linePoints.m_tail.push_back(pointRightMiddle);
    linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
    linePoints.prop["\"thickness\""] = Expression(0);
    plotResult.m_tail.push_back(linePoints);
    linePoints.m_tail.clear();
  }

}

void Expression::createStrings(double & au, double & al, double & ol, double & ou, double & scaledAU, double & scaledAL, double & scaledOL, double & scaledOU, Expression & plotResult)
{
  Expression au1 = Expression("\"" + std::to_string(int(au)) + "\"");
  au1.prop["\"object-name\""] = Expression(Atom("\"text\""));
  Expression x;
  x.append(Atom(scaledAU));
  x.append(Atom(-(scaledOL-2)));
  x.head().setList();
  au1.prop["\"position\""] = x;
  au1.prop["\"scale\""] = Expression(1);
  au1.prop["\"rotation\""] = Expression(0);
  plotResult.m_tail.push_back(au1);

  Expression al1 = Expression("\"" + std::to_string(int(al)) + "\"");
  al1.prop["\"object-name\""] = Expression(Atom("\"text\""));
  Expression y;
  y.append(Atom(scaledAL));
  y.append(Atom(-(scaledOL-2)));
  y.head().setList();
  al1.prop["\"position\""] = y;
  al1.prop["\"scale\""] = Expression(1);
  al1.prop["\"rotation\""] = Expression(0);
  plotResult.m_tail.push_back(al1);

  Expression ol1 = Expression("\"" + std::to_string(int(ol)) + "\"");
  ol1.prop["\"object-name\""] = Expression(Atom("\"text\""));
  Expression a;
  a.append(Atom(scaledAL-2));
  a.append(Atom(-scaledOL));
  a.head().setList();
  ol1.prop["\"position\""] = a;
  ol1.prop["\"scale\""] = Expression(1);
  ol1.prop["\"rotation\""] = Expression(0);
  plotResult.m_tail.push_back(ol1);

  Expression ou1 = Expression("\"" + std::to_string(int(ou)) + "\"");
  ou1.prop["\"object-name\""] = Expression(Atom("\"text\""));
  Expression z;
  z.append(Atom(scaledAL-2));
  z.append(Atom(-scaledOU));
  z.head().setList();
  ou1.prop["\"position\""] = z;
  ou1.prop["\"scale\""] = Expression(1);
  ou1.prop["\"rotation\""] = Expression(0);
  plotResult.m_tail.push_back(ou1);
}

void Expression::scalePointsLines(Expression & plotResult, Expression & evalText, Expression & evalPoints, double & au, double & al, double & ol, double & ou)
{
  //points contains the flipped points
  //lines contains the lines from flipped points to the complimentary

  //AL is xmin
  //AU is xmax
  //OL is ymin
  //OU is ymax
  double xscale = 20.0/(au-al);
  double yscale = 20.0/(ou-ol);
  double scaledAU = au * xscale;
  double scaledAL = al * xscale;
  double scaledOU = ou * yscale;
  double scaledOL = ol * yscale;

  for (auto it = evalPoints.m_tail.begin(); it != evalPoints.m_tail.end(); ++it)
  {
    Expression flippedPoint;

    //creating points

    flippedPoint.append((*it).m_tail[0].head().asNumber() * xscale);
    flippedPoint.append((*it).m_tail[1].head().asNumber() * -1 * yscale);
    flippedPoint.prop["\"object-name\""] = Expression(Atom("\"point\""));
    flippedPoint.prop["\"size\""] = Expression(0.5);
    flippedPoint.head().setList();

    plotResult.m_tail.push_back(flippedPoint);

    std::vector<Expression> linePoints;
    Expression line;

    Expression x;//used to create complimetary point

    x.append(((*it).m_tail[0].head().asNumber() * xscale));

    if (scaledOL > 0)
    {
      x.append(Atom(-scaledOL));
    }
    else
    {
      x.append(Atom(0));
    }

    x.head().setList();
    linePoints.push_back(x);
    linePoints.push_back(flippedPoint);

    line = Expression(linePoints);

    line.prop["\"object-name\""] = Expression(Atom("\"line\""));
    line.prop["\"thickness\""] = Expression(0);
    plotResult.m_tail.push_back(line);

    linePoints.clear();
    line.m_tail.clear();
  }
  createStrings(au, al, ol, ou, scaledAU, scaledAL, scaledOL, scaledOU, plotResult);
  generateBoundingBoxLines(plotResult, scaledAU, scaledAL, scaledOU, scaledOL);
  generateTextLabels(plotResult, evalText, scaledAU, scaledAL, scaledOL, scaledOU);
}

Expression Expression::handle_discretePlot(Environment &env)
{
  Expression plotResult;//big list

  double greatestX = -999999999;
  double greatestY = -9999999;
  double leastX = 99999999;
  double leastY = 999999;
  double au = 0.0;
  double al = 0.0;
  double ou = 0.0;
  double ol = 0.0;

  //initialize variables for the different parameters for the plot

  Expression evalPoints = m_tail[0].eval(env);

  for (auto it = evalPoints.m_tail.begin(); it != evalPoints.m_tail.end(); ++it)//evalPoints has the points to plot
  {
    if (greatestX < (*it).m_tail[0].head().asNumber())
    {
      greatestX = (*it).m_tail[0].head().asNumber();
    }
    if (greatestY < (*it).m_tail[1].head().asNumber())
    {
      greatestY = (*it).m_tail[1].head().asNumber();
    }
    if (leastX > (*it).m_tail[0].head().asNumber())
    {
      leastX = (*it).m_tail[0].head().asNumber();
    }
    if (leastY > (*it).m_tail[1].head().asNumber())
    {
      leastY = (*it).m_tail[1].head().asNumber();
    }
  }

  au = greatestX;
  al = leastX;
  ol = leastY;
  ou = greatestY;

  Expression evalText = m_tail[1].eval(env);
  scalePointsLines(plotResult, evalText, evalPoints, au, al, ol, ou);

  plotResult.m_head.setList();
  return plotResult;
}

void Expression::generateTextLabels(Expression & plotResult, Expression & evalText, double & scaledAU, double & scaledAL, double & scaledOL, double & scaledOU)
{
  Expression scale = Expression(1);

  double xmiddle = (scaledAU+scaledAL)/2;
  double ymiddle = (scaledOU+scaledOL)/2;

  for (auto it = evalText.m_tail.begin(); it != evalText.m_tail.end(); ++it)//evalText has the text labels
  {
    if ((*it).m_tail[0].head().asSymbol() == "\"title\"")
    {
      Expression title = (*it).m_tail[1];
      title.prop["\"object-name\""] = Expression(Atom("\"text\""));
      Expression x;
      x.append(Atom(xmiddle));
      x.append(Atom(-(scaledOU+3)));
      x.head().setList();
      title.prop["\"position\""] = x;
      title.prop["\"scale\""] = scale;
      title.prop["\"rotation\""] = Expression(0);
      plotResult.m_tail.push_back(title);
      x.m_tail.clear();
    }
    else if ((*it).m_tail[0].head().asSymbol() == "\"abscissa-label\"")
    {
      Expression absLabel = (*it).m_tail[1];
      absLabel.prop["\"object-name\""] = Expression(Atom("\"text\""));
      Expression x;
      x.append(Atom(xmiddle));
      x.append(Atom(-(scaledOL-3)));
      x.head().setList();
      absLabel.prop["\"position\""] = x;
      absLabel.prop["\"scale\""] = scale;
      absLabel.prop["\"rotation\""] = Expression(0);
      plotResult.m_tail.push_back(absLabel);
      x.m_tail.clear();
    }
    else if ((*it).m_tail[0].head().asSymbol() == "\"ordinate-label\"")
    {
      Expression ordLabel = (*it).m_tail[1];
      ordLabel.prop["\"object-name\""] = Expression(Atom("\"text\""));
      Expression x;
      x.append(Atom(scaledAL-3));
      x.append(Atom(-ymiddle));
      x.head().setList();
      ordLabel.prop["\"position\""] = x;
      ordLabel.prop["\"scale\""] = scale;
      ordLabel.prop["\"rotation\""] = Expression(-90);
      plotResult.m_tail.push_back(ordLabel);
      x.m_tail.clear();
    }
    else if ((*it).m_tail[0].head().asSymbol() == "\"text-scale\"")
    {
      Expression scale = (*it).m_tail[1];
    }
  }
}

Expression Expression::handle_continuousPlot(Environment &env)
{
  Expression conPlotResult;//big list

  double greatestX = -999999999;
  double greatestY = -9999999;
  double leastX = 99999999;
  double leastY = 999999;
  double au2 = 0.0;
  double al2 = 0.0;
  double ou2 = 0.0;
  double ol2 = 0.0;

  //initialize variables for the different parameters for the plot

  Expression evalBounds = m_tail[1].eval(env);

  al2 = evalBounds.m_tail[0].head().asNumber();
  au2 = evalBounds.m_tail[1].head().asNumber();

  double pointWidth = (au2-al2) / 50;//scaled pointWidth

  std::vector<Expression> points;

  for (auto start = al2; start <= au2+pointWidth; start += pointWidth)//create 50 points for the sample size
  {
    Expression point;
    std::vector<Expression> tempX;
    point.append(Atom(start));
    tempX.push_back(Expression(start));

    Expression yCord = apply(m_tail[0].head(), tempX, env);
    double rawYCord = yCord.head().asNumber();

    if (greatestX < start)
    {
      greatestX = start;//find the largest for the labels
    }
    if (leastX > start)
    {
      leastX = start;
    }
    if (greatestY < rawYCord)
    {
      greatestY = rawYCord;
    }
    if (leastY > rawYCord)
    {
      leastY = rawYCord;
    }
    point.append(Atom(rawYCord));

    point.prop["\"object-name\""] = Expression(Atom("\"point\""));
    point.prop["\"size\""] = Expression(0.5);//give the properties
    point.head().setList();
    points.push_back(point);

    tempX.clear();
    point.m_tail.clear();
  }

  ol2 = leastY;
  ou2 = greatestY;

  double xscale = 20.0/(au2-al2);
  double yscale = 20.0/(ou2-ol2);

  double scaledAU2 = au2 * xscale;
  double scaledAL2 = al2 * xscale;
  double scaledOU2 = ou2 * yscale;
  double scaledOL2 = ol2 * yscale;

  for (auto it = points.begin(); it != points.end(); ++it)
  {
    (*it).m_tail[0] = Expression((*it).m_tail[0].head().asNumber() * xscale);
    (*it).m_tail[1] = Expression((*it).m_tail[1].head().asNumber() * yscale * -1);//scale the points
  }

  continuousBoundingBox(conPlotResult, scaledOL2, scaledOU2, scaledAL2, scaledAU2);

  if (m_tail.size() == 3)//that means that there is no text labels
  {
    Expression evalText = m_tail[2].eval(env);
    continuousTextLabels(conPlotResult, evalText, scaledOL2, scaledOU2, scaledAL2, scaledAU2);
  }
  continuousCreateStrings(conPlotResult, au2, al2, ol2, ou2, scaledAU2, scaledAL2, scaledOL2, scaledOU2);//crwates the labels but not title
  continuousAngleCalc(conPlotResult, points, xscale, yscale, env);
  conPlotResult.head().setList();
  return conPlotResult;
}

void Expression::continuousAngleCalc(Expression & conPlotResult, std::vector<Expression> points, double & xscale, double & yscale, Environment & env)
{
  int counter = 0;
  bool changed = false;
  //this is the implementation of the algorithm
  while (counter < 10 || !changed)
  {

    for (auto it = points.begin(), it2 = points.begin()+1, it3 = points.begin()+2; ;) //go through the list of points twice
    {
      if (checkAngle175(*it, *it2, *it3))
      {
        changed = true;
        std::vector<Expression> points2add = splitLine(*it, *it2, *it3, xscale, yscale, env);//get the midpoint points
        it = it3;
      }
      else
      {
        it = it3;
      }

      if (it == points.end()-3)
      {
        break;
      }
      else
      {
        it2 = it3+1;
        it3 += 2;
      }
    }
    counter++;
    if (!changed || counter == 1)//break if counter is greater than 1
    {
      break;
    }
    changed = false;
  }

  for (auto it = points.begin(), it2 = points.begin()+1; it != points.end()-1, it2 != points.end(); ++it, ++it2)//add the points to the final list
  {
    Expression line;

    line.m_tail.push_back(*it);
    line.m_tail.push_back(*it2);

    line.head().setList();
    line.prop["\"object-name\""] = Expression(Atom("\"line\""));
    line.prop["\"thickness\""] = Expression(0);
    conPlotResult.m_tail.push_back(line);

    line.m_tail.clear();
  }
}


std::vector<Expression> Expression::splitLine(Expression point1, Expression point2, Expression point3, double & xscale, double & yscale, Environment & env)
{
  double midx21 = (point1.m_tail[0].head().asNumber() + point2.m_tail[0].head().asNumber()) / 2;//midpoint x line 1
  double midx31 = (point3.m_tail[0].head().asNumber() + point2.m_tail[0].head().asNumber()) / 2;//midpoint x line 2

  Expression midpoint1;
  Expression midpoint2;//two points
  midpoint1.prop["\"object-name\""] = Expression(Atom("\"point\""));
  midpoint2.prop["\"size\""] = Expression(0.5);
  midpoint1.head().setList();
  midpoint2.head().setList();
  std::vector<Expression> mid1;
  std::vector<Expression> mid2;
  midpoint1.append(Atom(midx21));
  midpoint2.append(Atom(midx31));

  mid1.push_back(Expression(midx21/xscale));
  mid2.push_back(Expression(midx31/xscale));

  Expression midy21 = apply(m_tail[0].head(), mid1, env);
  Expression midy31 = apply(m_tail[0].head(), mid2, env);
  double midy21Raw = midy21.head().asNumber() * -1;
  double midy31Raw = midy31.head().asNumber() * -1;

  double scaledMid21 = midy21Raw * yscale;
  double scaledMid31 = midy31Raw * yscale;
  midpoint1.append(Atom(scaledMid21));
  midpoint2.append(Atom(scaledMid31));

  std::vector<Expression> newPoints;
  newPoints.push_back(point1);
  newPoints.push_back(midpoint1);
  newPoints.push_back(point2);
  newPoints.push_back(midpoint2);
  newPoints.push_back(point3);

  return newPoints;

}

bool Expression::checkAngle175(Expression point1, Expression point2, Expression point3)
{
  double dx21 = point2.m_tail[0].head().asNumber() - point1.m_tail[0].head().asNumber();//vector i1
  double dy21 = point2.m_tail[1].head().asNumber() - point1.m_tail[1].head().asNumber();//vector j1
  double dx31 = point2.m_tail[0].head().asNumber() - point3.m_tail[0].head().asNumber();//vector i2
  double dy31 = point2.m_tail[1].head().asNumber() - point3.m_tail[1].head().asNumber();//vector j2

  double m12 = sqrt( dx21*dx21 + dy21*dy21 );//magnitude of the vecttor
  double m13 = sqrt( dx31*dx31 + dy31*dy31 );

  //time to calculate the dot product
  double dotProduct = dx21 * dx31 + dy21 * dy31;

  double magnitude = m12 * m13;

  double result = (dx21*dx31 + dy21*dy31) / (m12 * m13);

  double theta = acos(result);

  if (theta < (175 * atan2(0,-1) / 180))
  {
    return true;
  }
  else
  {
    return false;
  }
}

void Expression::continuousCreateStrings(Expression & conPlotResult, double & au2, double & al2, double & ol2, double & ou2, double & scaledAU2, double & scaledAL2, double & scaledOL2, double & scaledOU2)
{
  Expression au1 = Expression(round(au2));
  au1.prop["\"object-name\""] = Expression(Atom("\"text\""));
  Expression x;
  x.append(Atom(scaledAU2));
  x.append(Atom(-(scaledOL2-2)));
  x.head().setList();
  au1.prop["\"position\""] = x;
  au1.prop["\"scale\""] = Expression(1);
  au1.prop["\"rotation\""] = Expression(0);
  conPlotResult.m_tail.push_back(au1);

  Expression al1 = Expression(round(al2));
  al1.prop["\"object-name\""] = Expression(Atom("\"text\""));
  Expression y;
  y.append(Atom(scaledAL2));
  y.append(Atom(-(scaledOL2-2)));
  y.head().setList();
  al1.prop["\"position\""] = y;
  al1.prop["\"scale\""] = Expression(1);
  al1.prop["\"rotation\""] = Expression(0);
  conPlotResult.m_tail.push_back(al1);

  Expression ol1 = Expression(round(ol2));
  ol1.prop["\"object-name\""] = Expression(Atom("\"text\""));
  Expression a;
  a.append(Atom(scaledAL2-2));
  a.append(Atom(-scaledOL2));
  a.head().setList();
  ol1.prop["\"position\""] = a;
  ol1.prop["\"scale\""] = Expression(1);
  ol1.prop["\"rotation\""] = Expression(0);
  conPlotResult.m_tail.push_back(ol1);

  Expression ou1 = Expression(round(ou2));
  ou1.prop["\"object-name\""] = Expression(Atom("\"text\""));
  Expression z;
  z.append(Atom(scaledAL2-2));
  z.append(Atom(-scaledOU2));
  z.head().setList();
  ou1.prop["\"position\""] = z;
  ou1.prop["\"scale\""] = Expression(1);
  ou1.prop["\"rotation\""] = Expression(0);
  conPlotResult.m_tail.push_back(ou1);
}

void Expression::continuousTextLabels(Expression & conPlotResult, Expression & evalText, double & scaledOL2, double & scaledOU2, double & scaledAL2, double & scaledAU2)
{
  Expression scale = Expression(1);

  double xmiddle = (scaledAU2+scaledAL2)/2;
  double ymiddle = (scaledOU2+scaledOL2)/2;

  for (auto it = evalText.m_tail.begin(); it != evalText.m_tail.end(); ++it)//evalText has the text labels
  {
    if ((*it).m_tail[0].head().asSymbol() == "\"title\"")
    {
      Expression title = (*it).m_tail[1];
      title.prop["\"object-name\""] = Expression(Atom("\"text\""));
      Expression x;
      x.append(Atom(xmiddle));
      x.append(Atom(-(scaledOU2+3)));
      x.head().setList();
      title.prop["\"position\""] = x;
      title.prop["\"scale\""] = scale;
      title.prop["\"rotation\""] = Expression(0);
      conPlotResult.m_tail.push_back(title);
      x.m_tail.clear();
    }
    else if ((*it).m_tail[0].head().asSymbol() == "\"abscissa-label\"")
    {
      Expression absLabel = (*it).m_tail[1];
      absLabel.prop["\"object-name\""] = Expression(Atom("\"text\""));
      Expression x;
      x.append(Atom(xmiddle));
      x.append(Atom(-(scaledOL2-3)));
      x.head().setList();
      absLabel.prop["\"position\""] = x;
      absLabel.prop["\"scale\""] = scale;
      absLabel.prop["\"rotation\""] = Expression(0);
      conPlotResult.m_tail.push_back(absLabel);
      x.m_tail.clear();
    }
    else if ((*it).m_tail[0].head().asSymbol() == "\"ordinate-label\"")
    {
      Expression ordLabel = (*it).m_tail[1];
      ordLabel.prop["\"object-name\""] = Expression(Atom("\"text\""));
      Expression x;
      x.append(Atom(scaledAL2-3));
      x.append(Atom(-ymiddle));
      x.head().setList();
      ordLabel.prop["\"position\""] = x;
      ordLabel.prop["\"scale\""] = scale;
      ordLabel.prop["\"rotation\""] = Expression(-90);
      conPlotResult.m_tail.push_back(ordLabel);
      x.m_tail.clear();
    }
    else if ((*it).m_tail[0].head().asSymbol() == "\"text-scale\"")
    {
      Expression scale = (*it).m_tail[1];
    }
  }
}

std::string Expression::round(double num)
{
    std::ostringstream os;
    os << std::setprecision(2);
    os << num;

    std::string strObj = os.str();

    return ("\"" + strObj + "\"");
}


void Expression::continuousBoundingBox(Expression & conPlotResult, double & scaledOL2, double & scaledOU2, double & scaledAL2, double & scaledAU2)
{
  Expression pointBottomLeft;
  pointBottomLeft.append(Atom(scaledAL2));
  pointBottomLeft.append(Atom(-scaledOL2));
  pointBottomLeft.head().setList();

  Expression pointTopLeft;
  pointTopLeft.append(Atom(scaledAL2));
  pointTopLeft.append(Atom(-scaledOU2));
  pointTopLeft.head().setList();

  Expression pointBottomRight;
  pointBottomRight.append(Atom(scaledAU2));
  pointBottomRight.append(Atom(-scaledOL2));
  pointBottomRight.head().setList();

  Expression pointTopRight;
  pointTopRight.append(Atom(scaledAU2));
  pointTopRight.append(Atom(-scaledOU2));
  pointTopRight.head().setList();

  double xmiddle = (scaledAU2+scaledAL2)/(scaledAU2*20-scaledAL2*20);
  double ymiddle = (scaledOU2+scaledOL2)/(scaledOU2*20-scaledOL2*20);;

  Expression pointMiddleTop;
  pointMiddleTop.append(Atom(xmiddle));
  pointMiddleTop.append(Atom(-scaledOU2));
  pointMiddleTop.head().setList();

  Expression pointRightMiddle;
  pointRightMiddle.append(Atom(scaledAU2));
  pointRightMiddle.append(Atom(-ymiddle));
  pointRightMiddle.head().setList();

  Expression pointBottomMiddle;
  pointBottomMiddle.append(Atom(xmiddle));
  pointBottomMiddle.append(Atom(-scaledOL2));
  pointBottomMiddle.head().setList();

  Expression pointLeftMiddle;
  pointLeftMiddle.append(Atom(scaledAL2));
  pointLeftMiddle.append(Atom(-ymiddle));
  pointLeftMiddle.head().setList();

  //now to create the 6 lines using the points

  Expression linePoints;
  linePoints.m_tail.push_back(pointTopLeft);
  linePoints.m_tail.push_back(pointTopRight);
  linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
  linePoints.prop["\"thickness\""] = Expression(0);
  linePoints.head().setList();
  conPlotResult.m_tail.push_back(linePoints);
  linePoints.m_tail.clear();

  linePoints.m_tail.push_back(pointTopRight);
  linePoints.m_tail.push_back(pointBottomRight);
  linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
  linePoints.prop["\"thickness\""] = Expression(0);
  conPlotResult.m_tail.push_back(linePoints);
  linePoints.m_tail.clear();

  linePoints.m_tail.push_back(pointBottomRight);
  linePoints.m_tail.push_back(pointBottomLeft);
  linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
  linePoints.prop["\"thickness\""] = Expression(0);
  conPlotResult.m_tail.push_back(linePoints);
  linePoints.m_tail.clear();

  linePoints.m_tail.push_back(pointBottomLeft);
  linePoints.m_tail.push_back(pointTopLeft);
  linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
  linePoints.prop["\"thickness\""] = Expression(0);
  conPlotResult.m_tail.push_back(linePoints);
  linePoints.m_tail.clear();

  linePoints.m_tail.push_back(pointMiddleTop);
  linePoints.m_tail.push_back(pointBottomMiddle);
  linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
  linePoints.prop["\"thickness\""] = Expression(0);
  conPlotResult.m_tail.push_back(linePoints);
  linePoints.m_tail.clear();

  if (scaledOL2 <= 0)
  {
    linePoints.m_tail.push_back(pointLeftMiddle);
    linePoints.m_tail.push_back(pointRightMiddle);
    linePoints.prop["\"object-name\""] = Expression(Atom("\"line\""));
    linePoints.prop["\"thickness\""] = Expression(0);
    conPlotResult.m_tail.push_back(linePoints);
    linePoints.m_tail.clear();
  }
}

Expression Expression::handleMakePoint() const noexcept
{
  return prop.at("\"size\"");
}

Expression Expression::handleMakeLine() const noexcept
{
  return prop.at("\"thickness\"");
}

Expression Expression::handleMakeText() const noexcept
{
  return prop.at("\"position\"");
}

Expression Expression::handleRotation() const noexcept
{
  return prop.at("\"rotation\"");
}

Expression Expression::handleScale() const noexcept
{
  return prop.at("\"scale\"");
}

Expression Expression::searchMap() const noexcept
{
  return prop.at("\"object-name\"");
}


int Expression::getPropSize() const noexcept
{
  return prop.size();
}

// this is a simple recursive version. the iterative version is more
// difficult with the ast data structure used (no parent pointer).
// this limits the practical depth of our AST
Expression Expression::eval(Environment & env){

  if (global_status_flag > 0)
  {
    throw SemanticError("Error: interpreter kernel interrupted");
    //return Expression(Atom("Error: interpreter kernel interrupted"));
  }

  if(m_tail.empty()){
    if (m_head.isSymbol() && m_head.asSymbol() == "list")//check case for empty list
    {
      return Expression(m_tail);
    }
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
  else if (m_head.isSymbol() && m_head.asSymbol() == "lambda")
  {
    return handle_lambda(env);
  }
  else if (m_head.isSymbol() && m_head.asSymbol() == "apply")
  {
    return handle_apply(env);
  }
  else if (m_head.isSymbol() && m_head.asSymbol() == "map")
  {
    return handle_map(env);
  }
  else if (m_head.isSymbol() && m_head.asSymbol() == "set-property")
  {
    return handle_setProp(env);
  }
  else if (m_head.isSymbol() && m_head.asSymbol() == "get-property")
  {
    return handle_getProp(env);
  }
  else if (m_head.isSymbol() && m_head.asSymbol() == "discrete-plot")
  {
    return handle_discretePlot(env);
  }
  else if (m_head.isSymbol() && m_head.asSymbol() == "continuous-plot")
  {
    return handle_continuousPlot(env);
  }
  // else attempt to treat as procedure
  else{
    std::vector<Expression> results;
    for(Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it){
      results.push_back(it->eval(env));
    }
    return apply(m_head, results, env);
  }
}


std::ostream & operator<<(std::ostream & out, const Expression & exp){
    if (exp.isHeadList())
    {
      out << "(";

      out << exp.head();
      int counter = exp.tailSize()-1;

      for(auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e){
        out << *e;
        if (counter != 0)
        {
          out << " ";
          counter--;
        }
      }

      out << ")";
    }
    else if (exp.isHeadNone())
    {
      out << "NONE";
    }
    else
    {
      if (!exp.isHeadComplex())
      {
        out << "(";
      }
      out << exp.head();

      for(auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e){
        out << " ";
        out << *e;
      }

      if (!exp.isHeadComplex())
      {
        out << ")";
      }
    }
    return out;
}

bool Expression::operator==(const Expression & exp) const noexcept{

  bool result = (m_head == exp.m_head);

  result = result && (m_tail.size() == exp.m_tail.size());

  if(result){
    for(auto lefte = m_tail.begin(), righte = exp.m_tail.begin();
	(lefte != m_tail.end()) && (righte != exp.m_tail.end());
	++lefte, ++righte){
      result = result && (*lefte == *righte);
    }
  }

  return result;
}

bool operator!=(const Expression & left, const Expression & right) noexcept{

  return !(left == right);
}

#include "environment.hpp"

#include <cassert>
#include <cmath>
#include <complex>
#include <iostream>

using std::endl;
using std::cout;

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

Environment::Environment(const Environment & env)
{
  envmap = env.envmap;//copy over the current environment
}

const std::vector<Expression> LIST = {};//empty list case for expression

//begin join
Expression join(const std::vector<Expression> & args)
{
  if (!nargs_equal(args, 2))//check for correct arugmentes #
  {
    throw SemanticError("Error: two arguments for join not given");
  }
  else
  {
    if (args[0].isHeadList() && args[1].isHeadList())//both have to be list
    {
      std::vector<Expression> result;
      for(auto e = args[0].tailConstBegin(); e != args[0].tailConstEnd(); ++e)
      {
        result.push_back(*e);//push
      }

      for(auto e = args[1].tailConstBegin(); e != args[1].tailConstEnd(); ++e)
      {
        result.push_back(*e);
      }
      return Expression(result);
    }
    else
    {
      throw SemanticError("Error: argument to join not a list");//error
    }
  }
  return Expression();
}

Expression append(const std::vector<Expression> & args)
{
  if (!nargs_equal(args,2))//check for number of arguments
  {
    throw SemanticError("Error: two arguments for append not given");
  }
  else
  {
    if (!args[0].isHeadList())//is head not list
    {
      throw SemanticError("Error: first argument to append not a list");
    }
    else
    {
      std::vector<Expression> result;
      for(auto e = args[0].tailConstBegin(); e != args[0].tailConstEnd(); ++e)
      {
        result.push_back(*e);//push back
      }
      if (args[1].isHeadList())
      {
        for(auto e = args[1].tailConstBegin(); e != args[1].tailConstEnd(); ++e)
        {
          result.push_back(*e);
        }
      }
      else
      {
        result.push_back(args[1]);
      }
      return Expression(result);
    }
  }
  return Expression();
}

//range procedures
Expression range(const std::vector<Expression> & args)
{
  if (nargs_equal(args,3))
  {
    if (args[0].head().isNumber() && args[1].head().isNumber() && args[2].head().isNumber())//check for all args
    {
      if (args[2].head().asNumber() <= 0)
      {
        throw SemanticError("Error: negative or zero increment in range");//error
      }

      if (args[1].head().asNumber() < args[0].head().asNumber())
      {
        throw SemanticError("Error: begin greater than end in range");
      }
      double beginValue = args[0].head().asNumber();//get the variables to make the list
      double endValue = args[1].head().asNumber();
      double incrementValue = args[2].head().asNumber();
      std::vector<Expression> result;
      for (double i = beginValue; i <= endValue; i += incrementValue)
      {
        result.push_back(Expression(i));
      }
      return (Expression(result));
    }
    else
    {
      throw SemanticError("Error: one of the arguments is not a number");
    }
  }
  else
  {
    throw SemanticError("Error: did not give 3 arguments");
  }
    return Expression();
}

//length Procedure
Expression length(const std::vector<Expression> & args)
{
  if (nargs_equal(args,1))//lenghth takea lsit
  {
    if (!args[0].isHeadList())
    {
      throw SemanticError("Error: argument to length is not a list");
    }
    else
    {
      int counter = 0;
      for (auto it = args[0].tailConstBegin(); it != args[0].tailConstEnd(); ++it)
      {
        counter++;
      }
      return Expression(counter);//return count
    }
  }
  else
  {
    throw SemanticError("Error: not enough or too many arguments in call to length");
  }
}

//first procedure
Expression first(const std::vector<Expression> & args)
{
  if (nargs_equal(args,1))
  {
    if (args[0].head().isNone())
    {
      throw SemanticError("Error: argument to first is an empty list");
    }
    if (!args[0].isHeadList())
    {
      throw SemanticError("Error: argument to first is not a list");
    }
    else
    {
      if (args[0].tailSize() == 0)
      {
        throw SemanticError("Error: argument to first is an empty list");
      }
      return Expression(args[0].getExpressionFirst());//get first element of list
    }
  }
  else if (args.size() > 1)
  {
    throw SemanticError("Error: more than one argument in call to first");
  }
}

//rest procedure
Expression rest(const std::vector<Expression> & args)
{
  std::vector<Expression> result;
  if (nargs_equal(args,1))
  {
    if (args[0].head().isNone())
    {
      throw SemanticError("Error: argument to rest is an empty list");
    }
    if (!args[0].isHeadList())
    {
      throw SemanticError("Error: argument to rest is not a list");
    }
    else
    {
      if (args[0].tailSize() == 0)//return if tail is zero
      {
        throw SemanticError("Error: argument to rest is an empty list");
      }
      for(auto e = args[0].tailConstBegin()+1; e != args[0].tailConstEnd(); ++e)
      {
        result.push_back(*e);
      }
      return Expression(result);//return expression
    }
  }
  else if (args.size() > 1)
  {
    throw SemanticError("Error: more than one argument in call to rest");
  }
}

//added list function
Expression list(const std::vector<Expression> & args)
{
  std::vector<Expression> result;//empty vectro

  for (auto & a : args)
  {
    result.push_back(a);
  }
  return Expression(result);//return after putting arguments
};

//added sqrt function
Expression squareroot(const std::vector<Expression> & args)
{
  double result = 0;

  if (nargs_equal(args,1))//if # or args is 1
  {
    for (auto & a : args)
    {
      if (a.isHeadNumber())//is it a number?
      {
        if (a.head().asNumber() < 0)//is negative
        {
          std::complex<double> complexResult = std::sqrt(std::complex<double>(a.head().asNumber(),0));//squareroot
          return Expression(complexResult);
        }
        else
        {
            result = std::sqrt(a.head().asNumber());//calculate squareroot
        }
      }
      else if (a.isHeadComplex())//is complex
      {
        std::complex<double> complexsqrt = std::sqrt(a.head().asComplex());
        return Expression(complexsqrt);
      }
    }
  }
  else
  {
    throw SemanticError("Error in call to exponential: invalid number of arguments.");
  }
  return Expression(result);
};
//end of sqrt function

//added exponential function
Expression exponential(const std::vector<Expression> & args)
{
  double result = 0;

  if(nargs_equal(args,2))//if # or args is 2
  {
    if ((args[0].isHeadComplex()) || (args[1].isHeadComplex()))//find which arg is complex
    {
      std::complex<double> complexResult;
      if (args[0].isHeadComplex() && args[1].isHeadNumber())//which arg is complex
      {
        complexResult = std::pow(args[0].head().asComplex(), args[1].head().asNumber());
      }
      else if (args[1].isHeadComplex() && args[0].isHeadNumber())
      {
        complexResult = std::pow(args[0].head().asNumber(), args[1].head().asComplex());
      }
      else
      {
        complexResult = std::pow(args[0].head().asComplex(), args[1].head().asComplex());
      }
      return Expression(complexResult);
    }
    else if((args[0].isHeadNumber()) && (args[1].isHeadNumber()))//is both numbers
    {
      result = std::pow(args[0].head().asNumber(),args[1].head().asNumber());
    }
  }
  else
  {
    throw SemanticError("Error in call to exponential: invalid number of arguments.");
  }
  return Expression(result);
};
//end of exponential function

//added natural log function
Expression naturalLog(const std::vector<Expression> & args)
{
  double result = 0;

  for (auto & a : args)
  {
    if (a.isHeadNumber())//is number
    {
      if (a.head().asNumber() < 0)//if number is negative
      {
        throw SemanticError("Error in call to naturalLog, argument is negative");
      }
      if(!nargs_equal(args,1))//if # or args is 1
      {
        throw SemanticError("Error in call to naturalLog, too many arguments");
      }
      result = std::log(a.head().asNumber());//take log
    }
    else
    {
      throw SemanticError("Error in call to naturalLog, argument not a number");
    }
  }
  return Expression(result);
};
//end of natural log function

//added sin function
Expression sine(const std::vector<Expression> & args)
{
  double result = 0;

  for (auto & a : args)
  {
    if (a.isHeadNumber())
    {
      if(!nargs_equal(args,1))//is number of args not 1
      {
        throw SemanticError("Error in call to sine, too many arguments");
      }
      result = std::sin(a.head().asNumber());//take sine
    }
    else
    {
      throw SemanticError("Error in call to sine, argument not a number");
    }
  }
  return Expression(result);
};
//end of sin function

//added cos function
Expression cosine(const std::vector<Expression> & args)
{
  double result = 0;

  for (auto & a : args)
  {
    if (a.isHeadNumber())
    {
      if(!nargs_equal(args,1))
      {
        throw SemanticError("Error in call to cosine, too many arguments");
      }
      result = std::cos(a.head().asNumber());//take cos
    }
    else
    {
      throw SemanticError("Error in call to cosine, argument not a number");
    }
  }
  return Expression(result);
};
//end of cos function

//added tan function
Expression tangent(const std::vector<Expression> & args)
{
  double result = 0;

  for (auto & a : args)
  {
    if (a.isHeadNumber())//is number
    {
      if(!nargs_equal(args,1))//is number of args not 1
      {
        throw SemanticError("Error in call to tangent, too many arguments");
      }
      result = std::tan(a.head().asNumber());//take tan
    }
    else
    {
      throw SemanticError("Error in call to tangent, argument not a number");
    }
  }
  return Expression(result);
};
//end of tan function

Expression add(const std::vector<Expression> & args){

  // check all aruments are numbers, while adding
  double result = 0;
  std::complex<double> complexResult;
  bool complex = false;
  bool addedReal = false;

  for (auto & a : args)
  {
    if (a.isHeadComplex())//is complex
    {
      complex = true;
      if (!addedReal)//have you added reals already?
      {
        addedReal = true;
        complexResult += result;
      }
      complexResult += a.head().asComplex();//add to complex result
    }
    else if(a.isHeadNumber()){
      if (complex)
      {
        complexResult += a.head().asNumber();
      }
      else
      {
        result += a.head().asNumber();
      }
    }
  }

  if (complex)
  {
    return Expression(complexResult);
  }
  return Expression(result);
};

Expression mul(const std::vector<Expression> & args){

  // check all aruments are numbers, while multiplying
  double result = 1;
  std::complex<double> complexResult(1.0,0.0);
  bool complex = false;
  bool multipliedReal = false;

  for (auto & a : args)
  {
    if (a.isHeadComplex())//is complex
    {
      complex = true;
      if (!multipliedReal)//have you multiplied reals already?
      {
        multipliedReal = true;
        complexResult *= result;
      }
      complexResult *= a.head().asComplex();
    }
    else if(a.isHeadNumber()){//is number?
      if (complex)
      {
        complexResult *= a.head().asNumber();
      }
      else
      {
        result *= a.head().asNumber();
      }
    }
  }

  if (complex)
  {
    return Expression(complexResult);
  }
  return Expression(result);
};

Expression subneg(const std::vector<Expression> & args){

  double result = 0;

  // preconditions
  if(nargs_equal(args,1)){//check number of args
    if(args[0].isHeadNumber()){
      result = -args[0].head().asNumber();
    }
    else if (args[0].isHeadComplex())//is complex??
    {
      std::complex<double> complexResult = -args[0].head().asComplex();//take negative
      return Expression(complexResult);
    }
    else{
      throw SemanticError("Error in call to negate: invalid argument.");
    }
  }
  else if(nargs_equal(args,2)){//2 arguments
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){//both numbers?
      result = args[0].head().asNumber() - args[1].head().asNumber();//subtract
    }
    else if ((args[0].isHeadComplex()) || (args[1].isHeadComplex()))
    {
      std::complex<double> complexResult;
      if (args[0].isHeadComplex() && args[1].isHeadNumber())
      {
        complexResult = args[0].head().asComplex() - args[1].head().asNumber();
      }
      else if (args[1].isHeadComplex() && args[0].isHeadNumber())
      {
        complexResult = args[0].head().asNumber() - args[1].head().asComplex();
      }
      else
      {
        complexResult = args[0].head().asComplex() - args[1].head().asComplex();
      }
      return Expression(complexResult);
    }
    else{
      throw SemanticError("Error in call to subtraction: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to subtraction or negation: invalid number of arguments.");
  }

  return Expression(result);
};

Expression div(const std::vector<Expression> & args){

  double result = 0;

  if(nargs_equal(args,2)){//how many args
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){
      result = args[0].head().asNumber() / args[1].head().asNumber();
    }
    else if ((args[0].isHeadComplex()) || (args[1].isHeadComplex()))//either one complex?
    {
      std::complex<double> complexResult;
      if (args[0].isHeadComplex() && args[1].isHeadNumber())
      {
        complexResult = args[0].head().asComplex() / args[1].head().asNumber();
      }
      else if (args[1].isHeadComplex() && args[0].isHeadNumber())
      {
        complexResult = args[0].head().asNumber() / args[1].head().asComplex();
      }
      else
      {
        complexResult = args[0].head().asComplex() / args[1].head().asComplex();
      }
      return Expression(complexResult);
    }
    else{
      throw SemanticError("Error in call to division: invalid argument.");
    }
  }
  else if (nargs_equal(args,1))
  {
    if (args[0].isHeadComplex())
    {
      std::complex<double> complexResult;
      complexResult = 1.0 / args[0].head().asComplex();
      return Expression(complexResult);
    }
    else if (args[0].isHeadNumber())
    {
      double result = 1.0 / args[0].head().asNumber();
      return Expression(result);
    }
    else
    {
      throw SemanticError("Error in call to division: value not number or complex.");
    }
  }
  else{
    throw SemanticError("Error in call to division: invalid number of arguments.");
  }
  return Expression(result);
};

//functon to output real part of complex
Expression realComplex(const std::vector<Expression> & args)
{
  if (nargs_equal(args,1))
  {
    if (!args[0].isHeadComplex())//error checking
    {
      throw SemanticError("Error in call to real, argument not complex");
    }
  }
  else
  {
    throw SemanticError("Error in call to real: invalid number of arguments.");
  }
  return Expression(args[0].head().asComplex().real());//get real
};
//end of realComplex function

//functon to output imag part of complex
Expression imagComplex(const std::vector<Expression> & args)
{
  if (nargs_equal(args,1))
  {
    if (!args[0].isHeadComplex())
    {
      throw SemanticError("Error in call to imaginary, argument not complex");
    }
  }
  else
  {
    throw SemanticError("Error in call to imaginary: invalid number of arguments.");
  }
  return Expression(args[0].head().asComplex().imag());//get imaginary
};
//end of imagComplex function

//functon to output mag part of complex
Expression magComplex(const std::vector<Expression> & args)
{
  if (nargs_equal(args,1))
  {
    if (!args[0].isHeadComplex())
    {
      throw SemanticError("Error in call to mag, argument not complex");
    }
  }
  else
  {
    throw SemanticError("Error in call to mag: invalid number of arguments.");
  }
  return Expression(std::abs(args[0].head().asComplex()));//get magnitude
};
//end of magComplex function

//functon to output arg part of complex
Expression argComplex(const std::vector<Expression> & args)
{
  if (nargs_equal(args,1))
  {
    if (!args[0].isHeadComplex())
    {
      throw SemanticError("Error in call to arg, argument not complex");
    }
  }
  else
  {
    throw SemanticError("Error in call to arg: invalid number of arguments.");
  }
  return Expression(std::arg(args[0].head().asComplex()));//getp hase
};
//end of argComplex function

//functon to output conj part of complex
Expression conjComplex(const std::vector<Expression> & args)
{
  if (nargs_equal(args,1))// only 1 arg
  {
    if (!args[0].isHeadComplex())//if not complex
    {
      throw SemanticError("Error in call to conj, argument not complex");
    }
  }
  else
  {
    throw SemanticError("Error in call to conj: invalid number of arguments.");
  }
  return Expression(std::conj(args[0].head().asComplex()));//return conjugate
};
//end of conjComplex function

//CONSTANTS defined

const double PI = std::atan2(0, -1);
const double EXP = std::exp(1);
const std::complex<double> I(0.0,1.0);

Environment::Environment(){
  reset();
}

bool Environment::is_known(const Atom & sym) const{
  if(!sym.isSymbol()) return false;

  return envmap.find(sym.asSymbol()) != envmap.end();
}

bool Environment::is_exp(const Atom & sym) const{
  if(!sym.isSymbol())
  {
    return false;
  }

  auto result = envmap.find(sym.asSymbol());

  return (result != envmap.end()) && (result->second.type == ExpressionType);
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
  /*if(envmap.find(sym.asSymbol()) != envmap.end()){
    throw SemanticError("Attempt to overwrite symbol in environemnt");
  }*/
  std::map<std::string, EnvResult>::iterator it;
  it = envmap.find(sym.asSymbol());

  if (it != envmap.end())
  {
    envmap.erase(it);
  }
  envmap.emplace(sym.asSymbol(), EnvResult(ExpressionType, exp));
}

bool Environment::is_proc(const Atom & sym) const{
  if(!sym.isSymbol())
  {
    return false;
  }
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

void Environment::add_replace(const Atom & sym, const Expression & exp)
{
  if (envmap.find(sym.asSymbol()) != envmap.end())
  {
    envmap.erase(sym.asSymbol());
  }
  add_exp(sym, exp);
}

/*
Reset the environment to the default state. First remove all entries and
then re-add the default ones.
 */
void Environment::reset(){

  envmap.clear();

  // Built-In value of pi
  envmap.emplace("pi", EnvResult(ExpressionType, Expression(PI)));

  // Procedure: add;
  envmap.emplace("+", EnvResult(ProcedureType, add));

  // Procedure: subneg;
  envmap.emplace("-", EnvResult(ProcedureType, subneg));

  // Procedure: mul;
  envmap.emplace("*", EnvResult(ProcedureType, mul));

  // Procedure: div;
  envmap.emplace("/", EnvResult(ProcedureType, div));

  //Procedure: sqrt;
  envmap.emplace("sqrt", EnvResult(ProcedureType, squareroot));

  //Procedure: exp;
  envmap.emplace("^", EnvResult(ProcedureType, exponential));

  //Procedure: ln;
  envmap.emplace("ln", EnvResult(ProcedureType, naturalLog));

  //Procedure: sine;
  envmap.emplace("sin", EnvResult(ProcedureType, sine));

  //Procedure: cosine;
  envmap.emplace("cos", EnvResult(ProcedureType, cosine));

  //Procedure: tangent;
  envmap.emplace("tan", EnvResult(ProcedureType, tangent));

  //Built in value of e;
  envmap.emplace("e", EnvResult(ExpressionType, Expression(EXP)));

  //Built in value of I
  envmap.emplace("I", EnvResult(ExpressionType, Expression(I)));

  //Procedure: real;
  envmap.emplace("real", EnvResult(ProcedureType, realComplex));

  //Procedure: imag;
  envmap.emplace("imag", EnvResult(ProcedureType, imagComplex));

  //Procedure: mag;
  envmap.emplace("mag", EnvResult(ProcedureType, magComplex));

  //Procedure: arg;
  envmap.emplace("arg", EnvResult(ProcedureType, argComplex));

  //Procedure: conj;
  envmap.emplace("conj", EnvResult(ProcedureType, conjComplex));

  //Procedure: list;
  envmap.emplace("list", EnvResult(ProcedureType, list));

  //Expression list;
  envmap.emplace("list", EnvResult(ExpressionType, Expression(LIST)));

  //Procedure: first;
  envmap.emplace("first", EnvResult(ProcedureType, first));

  //Procedure: range;
  envmap.emplace("range", EnvResult(ProcedureType, range));//working

  //Procedure: rest;
  envmap.emplace("rest", EnvResult(ProcedureType, rest));

  //Procedure: length;
  envmap.emplace("length", EnvResult(ProcedureType, length));

  //Procedure: append
  envmap.emplace("append", EnvResult(ProcedureType, append));//working

  //Procedure: join
  envmap.emplace("join", EnvResult(ProcedureType, join));//working
}

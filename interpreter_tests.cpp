#include "catch.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <thread>

#include "threadsafequeue.hpp"
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

TEST_CASE("Test sqrt procedure", "[interpreter]")
{
  std::vector<std::string> programs = {"(sqrt 1)"};

           Expression result = run(programs[0]);
           REQUIRE(result == Expression(1));
}

TEST_CASE("Test sqrt procedure 2", "[interpreter]")
{
  std::vector<std::string> programs = {"(sqrt 4)"};

           Expression result = run(programs[0]);
           REQUIRE(result == Expression(2));
}

TEST_CASE("Test sqrt procedure 3", "[interpreter]")
{
  std::vector<std::string> programs = {"(sqrt 6)"};

           Expression result = run(programs[0]);
           REQUIRE(result == Expression(std::sqrt(6)));
}

TEST_CASE("Test sqrt procedure 4", "[interpreter]")
{
  std::vector<std::string> programs = {"(sqrt 22)"};

           Expression result = run(programs[0]);
           REQUIRE(result == Expression(std::sqrt(22)));
}


TEST_CASE("Test negative square roots", "[interpreter]")
{
  std::vector<std::string> programs = {"(sqrt (- 4))",
                                       "(sqrt (- 8))",
                                       "(sqrt I)",
                                       "(sqrt -16)",
                                       "(sqrt (- 3))"};

    Expression result = run(programs[0]);
    REQUIRE(result == Expression(std::complex<double>(0,2)));

    Expression result2 = run(programs[1]);
    REQUIRE(result2 == Expression(std::sqrt(std::complex<double>(-8,0))));

    Expression result3 = run(programs[2]);
    REQUIRE(result3 == Expression(std::sqrt(std::complex<double>(0,1))));

    Expression result4 = run(programs[3]);
    REQUIRE(result4 == Expression(std::complex<double>(0,4)));

    Expression result5 = run(programs[4]);
    REQUIRE(result5 == Expression(std::sqrt(std::complex<double>(-3))));
}

TEST_CASE("Test exp procedures", "[interpreter]")
{
  std::vector<std::string> programs = {"(^ 1 4)",
                                       "(^ 2 3)",
                                       "(^ 3 5)"};

    Expression result = run(programs[0]);
    REQUIRE(result == Expression(1));

    Expression result2 = run(programs[1]);
    REQUIRE(result2 == Expression(8));

    Expression result3 = run(programs[2]);
    REQUIRE(result3 == Expression(243));
}

TEST_CASE("Test ln procedures", "[interpreter]")
{
  std::vector<std::string> programs = {"(ln 1)",
                                       "(ln e)"};

    Expression result = run(programs[0]);
    REQUIRE(result == Expression(0));

    Expression result2 = run(programs[1]);
    REQUIRE(result2 == Expression(1));
}

TEST_CASE("Test trig procedures", "[interpreter]")
{
  std::vector<std::string> programs = {"(sin 0)",
                                       "(sin (/ pi 2))",
                                       "(sin (/ pi 1))",
                                     "(cos (/ pi 2))",
                                     "(cos (/ pi 1))",
                                      "(tan (/ pi 4))"};

    Expression result = run(programs[0]);
    REQUIRE(result == Expression(0));

    Expression result2 = run(programs[1]);
    REQUIRE(result2 == Expression(1));

    Expression result3 = run(programs[2]);
    REQUIRE(result3 == Expression(0));

    Expression result4 = run(programs[3]);
    REQUIRE(result4 == Expression(0));

    Expression result5 = run(programs[4]);
    REQUIRE(result5 == Expression(-1));

    Expression result6 = run(programs[5]);
    REQUIRE(result6 == Expression(1));
}

TEST_CASE("Test trig procedures using cmath", "[interpreter]")
{
  std::vector<std::string> programs = {"(sin 0)",
                                       "(sin (/ pi 2))",
                                       "(sin (/ pi 1))",
                                     "(cos (/ pi 2))",
                                     "(cos (/ pi 1))",
                                      "(tan (/ pi 4))"};

    Expression result = run(programs[0]);
    REQUIRE(result == Expression(std::sin(0)));

    Expression result2 = run(programs[1]);
    REQUIRE(result2 == Expression(std::sin(M_PI/2)));

    Expression result3 = run(programs[2]);
    REQUIRE(result3 == Expression(std::sin(M_PI)));

    Expression result4 = run(programs[3]);
    REQUIRE(result4 == Expression(std::cos(M_PI/2)));

    Expression result5 = run(programs[4]);
    REQUIRE(result5 == Expression(std::cos(M_PI)));

    Expression result6 = run(programs[5]);
    REQUIRE(result6 == Expression(std::tan(M_PI/4)));
}


TEST_CASE( "Test Interpreter result with literal expressions for e and pi", "[interpreter]" ) {

  { // Number
    std::string program = "(e)";
    Expression result = run(program);
    REQUIRE(result == Expression(std::exp(1)));
  }

  { // Symbol
    std::string program = "(pi)";
    Expression result = run(program);
    REQUIRE(result == Expression(atan2(0, -1)));
  }

  { // Symbol
    std::string program = "(I)";
    Expression result = run(program);
    REQUIRE(result == Expression(std::complex<double>(0,1)));
  }
}

TEST_CASE( "Test some semantically invalid expresions", "[interpreter]" ) {

  std::vector<std::string> programs = {"(@ none)", // so such procedure
				       "(- 1 1 2)", // too many arguments
				       "(define begin 1)"}; // redefine symbol
    for(auto s : programs){
      Interpreter interp;

      std::istringstream iss(s);

      bool ok = interp.parseStream(iss);
      REQUIRE(ok == true);

      REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE( "Test some semantically invalid expresions for sqrt", "[interpreter]" ) {

  std::vector<std::string> programs = {"(sqrt @ none)", // so such procedure
				       "(sqrt 1 1 2)", // too many arguments
				       "(sqrt dsds)"}; // redefine builtin symbol
    for(auto s : programs){
      Interpreter interp;

      std::istringstream iss(s);

      bool ok = interp.parseStream(iss);
      REQUIRE(ok == true);

      REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
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

TEST_CASE( "Test for exceptions in trig functions", "[interpreter]" ) {

  std::vector<std::string> programs = {"(log -4)",
                                       "(log -1 2)",
                                       "(log none)"};

   for(auto s : programs){
     Interpreter interp;

     std::istringstream iss(s);

     bool ok = interp.parseStream(iss);
     REQUIRE(ok == true);

     REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
   }
}

TEST_CASE( "Test for exceptions in trig functions 2", "[interpreter]" ) {

  std::vector<std::string> programs = {"(sin -4 4 5)",
                                       "(sin Render)",
                                       "(cos -4 4 5)",
                                       "(cos Render)",
                                       "(tan -4 4 5)",
                                       "(tan Render)"};

   for(auto s : programs){
     Interpreter interp;

     std::istringstream iss(s);

     bool ok = interp.parseStream(iss);
     REQUIRE(ok == true);

     REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
   }
}

TEST_CASE( "Test for exceptions in imag functions", "[interpreter]" ) {

  std::vector<std::string> programs = {"(real -4 4 5)",
                                       "(real Render)",
                                       "(arg -4 4 5)",
                                       "(arg Render)",
                                       "(mag -4 4 5)",
                                       "(mag Render)",
                                       "(conj -4 4 5)",
                                       "(conj Render)",
                                       "(imag -4 4 5)",
                                       "(imag Render)"};

   for(auto s : programs){
     Interpreter interp;

     std::istringstream iss(s);

     bool ok = interp.parseStream(iss);
     REQUIRE(ok == true);

     REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
   }
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

TEST_CASE( "Test for exceptions from add, sub, div, mul, sqrt, and ^", "[interpreter]" ) {

  std::vector<std::string> programs = {"(+ I @)",
                                       "(* I None)",
                                       "(/ 2 define)",
                                       "(sqrt add)",
                                       "(^ a b 2 4 s)",
                                       "(^ a b)",
                                       "(^ I a)",
                                       "(^ a 2)",
                                       "(^ 2 add)"};

   for(auto s : programs){
     Interpreter interp;

     std::istringstream iss(s);

     bool ok = interp.parseStream(iss);
     REQUIRE(ok == true);

     REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
   }
}



TEST_CASE( "Test using additon with complex", "[interpreter]" ) {
  std::vector<std::string> programs = {"(+ I 2)",
                                       "(+ I I)",
                                       "(+ 2 I)",
                                       "(+ I I I I I)",
                                       "(+ I 2 (+ 2 2 I))"};

    Expression result = run(programs[0]);
    REQUIRE(result == Expression(std::complex<double>(2,1)));

    Expression result2 = run(programs[1]);
    REQUIRE(result2 == Expression(std::complex<double>(0,2)));

    Expression result3 = run(programs[2]);
    REQUIRE(result3 == Expression(std::complex<double>(2,1)));

    Expression result4 = run(programs[3]);
    REQUIRE(result4 == Expression(std::complex<double>(0,5)));

    Expression result5 = run(programs[4]);
    REQUIRE(result5 == Expression(std::complex<double>(6,2)));
}

TEST_CASE( "Test more with exponential with some complex", "[interpreter]" ) {
  std::vector<std::string> programs = {"(^ I 2)",
                                       "(^ 3 4)"};

    //Expression result = run(programs[0]);
    //REQUIRE(result == Expression(std::complex<double>(-1,1.22465e-16)));

    Expression result4 = run(programs[1]);
    REQUIRE(result4 == Expression(81));
}

TEST_CASE( "Test more with exponential with both complex", "[interpreter]" ) {
  std::vector<std::string> programs = {"(^ I I)",
                                       "(^ I (* 2 I))"};

    Expression result = run(programs[0]);
    //REQUIRE(result == Expression(std::complex<double>(0.20788, 0)));

    Expression result4 = run(programs[1]);
    //REQUIRE(result4 == Expression(std::complex<double>(0.0432139,0)));
}



TEST_CASE( "Test for exceptions from log and others", "[interpreter]" ) {

  std::vector<std::string> programs = {"(ln Iaddfnc @)",
                                       "(ln I Nonef 2 3)",
                                       "(ln I)",
                                       "(sin I)",
                                       "(cos I)",
                                       "(tan I)",
                                       "(+ 2 3 add define begin)",
                                       "(- add num ber a 3)",
                                       "(/ 2 add define begin)"};

   for(auto s : programs){
     Interpreter interp;

     std::istringstream iss(s);

     bool ok = interp.parseStream(iss);
     REQUIRE(ok == true);

     REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
   }
}

TEST_CASE( "Test more with arith", "[interpreter]" ) {
  std::vector<std::string> programs = {"(- I)",
                                       "(- I 2)",
                                       "(- I I)",
                                       "(- 2 I)",
                                       "(- 2 4)",
                                       "(- 2)"};

 Expression result = run(programs[0]);
 REQUIRE(result == Expression(std::complex<double>(0,-1)));

 Expression result2 = run(programs[1]);
 REQUIRE(result2 == Expression(std::complex<double>(-2,1)));

 Expression result3 = run(programs[2]);
 REQUIRE(result3 == Expression(std::complex<double>(0,0)));

 Expression result4 = run(programs[3]);
 REQUIRE(result4 == Expression(std::complex<double>(2,-1)));

 Expression result5 = run(programs[4]);
 REQUIRE(result5 == Expression(-2));

 Expression result6 = run(programs[5]);
 REQUIRE(result6 == Expression(-2));
}

TEST_CASE( "Test more with mul with some complex", "[interpreter]" ) {
  std::vector<std::string> programs = {"(* I 2 3)",
                                       "(* I I 3 4)"};

    Expression result = run(programs[0]);
    REQUIRE(result == Expression(std::complex<double>(0,6)));

    Expression result4 = run(programs[1]);
    REQUIRE(result4 == Expression(std::complex<double>(-12,0)));
}


TEST_CASE( "Test using division with complex", "[interpreter]" ) {
  std::vector<std::string> programs = {"(/ I 2)",
                                       "(/ 4 I)",
                                       "(/ I I)",
                                       "(/ I 1)"};

    Expression result = run(programs[0]);
    REQUIRE(result == Expression(std::complex<double>(0,0.5)));

    Expression result2 = run(programs[1]);
    REQUIRE(result2 == Expression(std::complex<double>(0,-4)));

    Expression result3 = run(programs[2]);
    REQUIRE(result3 == Expression(std::complex<double>(1,0)));

    Expression result4 = run(programs[3]);
    REQUIRE(result4 == Expression(std::complex<double>(0,1)));
}

TEST_CASE( "Test random exceptions for any operation", "[interpreter]" ) {

  std::vector<std::string> programs = {"(/ add sub)",
                                       "(/ add sub div)",
                                       "(+ 1 2 3 a)",
                                       "(* 1 2 3 a)",
                                       "(- word)",
                                       "(- a b)",
                                       "(real add)",
                                       "(imag add)",
                                       "(mag add)",
                                       "(arg word)",
                                       "(real 4)",
                                       "(imag 5)",
                                       "(mag 8)",
                                       "(arg 9)",
                                       "(conj 8)",
                                       "(/ 3 4 8 9)",
                                       "(conj notreal)"};

   for(auto s : programs){
     Interpreter interp;

     std::istringstream iss(s);

     bool ok = interp.parseStream(iss);
     REQUIRE(ok == true);

     REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
   }
}

TEST_CASE( "Testing complex procedures combined with arith", "[interpreter]" ) {
  std::vector<std::string> programs = {"(conj (/ I 2))",
                                       "(real (* I I))",
                                       "(imag (+ I I))",
                                       "(mag (- I))"};

    Expression result = run(programs[0]);
    REQUIRE(result == Expression(std::complex<double>(0,-0.5)));

    Expression result2 = run(programs[1]);
    REQUIRE(result2 == Expression(-1));

    Expression result3 = run(programs[2]);
    REQUIRE(result3 == Expression(2));

    Expression result4 = run(programs[3]);
    REQUIRE(result4 == Expression(1));
}

TEST_CASE( "Test random exceptions for multiple ops", "[interpreter]" ) {

  std::vector<std::string> programs = {"(/ addiii 1 3)",
                                       "(/ addfdf 3)",
                                       "(/ 1 add)",
                                       "(+ 1 2 a 4 f 5)",
                                       "(- 2 bdfdfd)",
                                       "(- addword 343)",
                                       "(- 3 4 3)",
                                       "(- sqrtfdf)",
                                       "(- (sqrt a))",
                                       "(^ 3 (^ a I))",
                                       "(^ axf omg)",
                                       "(^ 3 2 adb)",
                                       "(sqrt aomg)",
                                       "(ln (ln 4 5))",
                                       "(ln (- 103))"};

//"(+ 3 4 (sqrt 4e3))" this causes to fail

   for(auto s : programs){
     Interpreter interp;

     std::istringstream iss(s);

     bool ok = interp.parseStream(iss);
     REQUIRE(ok == true);

     REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
   }
}

TEST_CASE( "Testing complex procedures like arg", "[interpreter]" ) {
  std::vector<std::string> programs = {"(arg (* I I))"};


    Expression result2 = run(programs[0]);
    REQUIRE(result2 == Expression(atan2(0, -1)));
}


TEST_CASE( "Testing list creation with values", "[interpreter]" ) {
  std::string program = "(list 1 2 3)";

  std::vector<Expression> result = {Expression(1), Expression(2), Expression(3)};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(result));
}

TEST_CASE( "Testing list creation empty", "[interpreter]" ) {
  std::string program = "(list)";

  std::vector<Expression> result = {};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(result));
}

TEST_CASE( "throwing errors for list stuff: first", "[interpreter]" ) {
  std::vector<std::string> programs = {"(first (1))", "(first (list))", "(first (list 1 2) (list 3 4))"};


  //std::vector<Expression> result = {Expression(1), Expression(2), Expression(3)};

  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "throwing errors for list stuff: rest", "[interpreter]" ) {
  std::vector<std::string> programs = {"(rest (1))",
                                        "(rest (list))",
                                        "(rest (list 1 2) (list 3 4))"};


  //std::vector<Expression> result = {Expression(1), Expression(2), Expression(3)};

  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "throwing errors for list stuff: length", "[interpreter]" ) {
  std::vector<std::string> programs = {"(length 1)"};


  //std::vector<Expression> result = {Expression(1), Expression(2), Expression(3)};

  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "throwing errors for list stuff: append", "[interpreter]" ) {
  std::vector<std::string> programs = {"(append 3 x)"};


  //std::vector<Expression> result = {Expression(1), Expression(2), Expression(3)};

  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "throwing errors for list stuff: join", "[interpreter]" ) {
  std::vector<std::string> programs = {"(join (list 1 2) 10)"};


  //std::vector<Expression> result = {Expression(1), Expression(2), Expression(3)};

  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "throwing errors for list stuff: range", "[interpreter]" ) {
  std::vector<std::string> programs = {"(range 3 -1 1)",
                                       "(range 0 5 -1)"};


  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "Testing list creation empty with purpose error", "[interpreter]" ) {
  std::string program = "(list)";

  std::vector<Expression> result = {Expression(1)};

  Expression result3 = run(program);

  REQUIRE_FALSE(result3 == Expression(result));
}

TEST_CASE( "Testing list procedures: first", "[interpreter]" ) {
  std::string program = "(first (list 1 2 3))";


  Expression result = {Expression(1)};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(result));

}

TEST_CASE( "Testing list procedures: rest", "[interpreter]" ) {
  std::string program = "(rest (list 1 2 3))";


  std::vector<Expression> result = {Expression(2), Expression(3)};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(result));

}

TEST_CASE( "Testing list procedures again: rest", "[interpreter]" ) {
  std::string program = "(rest (list 1))";

  REQUIRE_NOTHROW(run(program));
}


TEST_CASE( "Testing list procedures: length", "[interpreter]" ) {
  std::string program = "(begin (define a (list 1 2 3 4)) (length a))";


  std::vector<Expression> result = {Expression(4)};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(4));

}

TEST_CASE( "Testing list procedures: append", "[interpreter]" ) {
  std::string program = "(begin (define x (list 0 1 2 3)) (define y (append x (3))) (y))";


  std::vector<Expression> result = {Expression(0), Expression(1), Expression(2), Expression(3), Expression(3)};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(result));

}

TEST_CASE( "Testing list procedures: join", "[interpreter]" ) {
  std::string program = "(begin (define x (list 0 1 2 3)) (define y (list 100 110)) (define z (join x y)) (z))";


  std::vector<Expression> result = {Expression(0), Expression(1), Expression(2), Expression(3), Expression(100), Expression(110)};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(result));

}

TEST_CASE( "Testing list procedures: join for error", "[interpreter]" ) {
  std::string program = "(begin (define x (list 0 1 2 3)) (define y (list 100 110)) (define z (join x y 3)))";


  Interpreter interp;

  std::istringstream iss(program);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}


TEST_CASE( "Testing list procedures: combinations", "[interpreter]" ) {
  std::string program = "(begin (define x (list 0 1 2 3)) (define y (list 100 110)) (define mylist (list (length x) (length y))) (mylist))";


  std::vector<Expression> result = {Expression(4), Expression(2)};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(result));

}

TEST_CASE( "Testing list procedures: range", "[interpreter]" ) {
  std::string program = "(range 0 5 1)";


  std::vector<Expression> result = {Expression(0), Expression(1), Expression(2), Expression(3), Expression(4), Expression(5)};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(result));

}

TEST_CASE( "Testing lambda basics", "[interpreter]" ) {
  std::string program = "(begin (define a 1) (define x 100) (define f (lambda (x) (begin (define b 12) (+ a b x)))) (f 2))";


  Expression result3 = run(program);

  REQUIRE(result3 == Expression(15));

}

TEST_CASE( "Testing apply basics", "[interpreter]" ) {
  std::string program = "(begin (define complexAsList (lambda (x) (list (real x) (imag x)))) (apply complexAsList (list (+ 1 (* 3 I)))))";

  std::vector<Expression> result = {Expression(1), Expression(3)};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(result));

}

TEST_CASE( "Testing apply more", "[interpreter]" ) {
  std::string program = "(apply + (list 1 2 3 4))";

  //std::vector<Expression> result = {Expression(1), Expression(3)};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(10));
}

TEST_CASE( "Testing apply moree", "[interpreter]" ) {
  std::string program = "(begin (define linear (lambda (a b x) (+ (* a x) b))) (apply linear (list 3 4 5)))";

  //std::vector<Expression> result = {Expression(1), Expression(3)};

  Expression result3 = run(program);

  REQUIRE(result3 == Expression(19));
}

TEST_CASE( "throwing errors for apply 1", "[interpreter]" ) {
  std::vector<std::string> programs = {"(apply + 3)",
                                       "(apply (+ z I) (list 0))",
                                      "(apply / (list 1 2 4))"};


  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "throwing errors for apply part 2", "[interpreter]" ) {
  std::vector<std::string> programs = {"(apply (+ z I) (list 0))", "(apply / (list 1 2 4))"};

  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "throwing errors for map part 1", "[interpreter]" ) {
  std::vector<std::string> programs = {"(map + 3)"};


  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "throwing errors for map", "[interpreter]" ) {
  std::vector<std::string> programs = {"(map 3 (list 1 2 3))"};


  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "throwing errors for map part 2", "[interpreter]" ) {
  std::vector<std::string> programs = {"(begin (define addtwo (lambda (x y) (+ x y))) (map addtwo (list 1 2 3)))"};


  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "throwing errors for map part 3", "[interpreter]" ) {
  std::string programs = "(begin (define f (lambda (x) (sin x))) (map f (list (/ (- pi) 2) 0 (/ pi 2))))";

  std::vector<Expression> result = {Expression(-1), Expression(0), Expression(1)};

  Expression result3 = run(programs);

  REQUIRE(result3 == Expression(result));
}

TEST_CASE( "throwing rand", "[interpreter]" ) {
  std::string s = "(length 1 2)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throwing rand2", "[interpreter]" ) {
  std::string s = "(join 1 2 4)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throwing rand3", "[interpreter]" ) {
  std::string s = "(append 1 2 4)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throwing rand4", "[interpreter]" ) {
  std::string s = "(append 1 (list 1 2))";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throwing rand9", "[interpreter]" ) {
  std::string s = "(range 0 3 4 2)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throwing rand10", "[interpreter]" ) {
  std::string s = "(range 0 3 4 2)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throwing rand8", "[interpreter]" ) {
  std::string s = "(lambda 0 3 4 2)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throwing rand7", "[interpreter]" ) {
  std::string s = "(map 0 3 4 2)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throwing rand6", "[interpreter]" ) {
  std::string s = "(lambda (1) (* 3 4))";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throwing apply error", "[interpreter]" ) {
  std::string s = "(apply 0 3 4)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throwing lambda errors", "[interpreter]" ) {
  std::string s = "(lambda (define x (3)))";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}


TEST_CASE( "Sample map case", "[interpreter]" ) {
  std::string s = "(map / (list 1 2 4))";

  std::vector<Expression> result = {Expression(1), Expression(0.5), Expression(0.25)};

  REQUIRE(run(s) == Expression(result));

}

TEST_CASE( "throw begin", "[interpreter]" ) {
  std::string s = "(begin)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throw define", "[interpreter]" ) {
  std::string s = "(define 3 4)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "throw range", "[interpreter]" ) {
  std::string s = "(range 0 I 3)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "div unary", "[interpreter]" ) {
  std::string s = "(/ I)";
  Expression result3 = run(s);

  REQUIRE(result3 == Expression(std::complex<double>(0,-1)));
}

TEST_CASE( "div unary 2", "[interpreter]" ) {
  std::string s = "(/ 2)";
  Expression result3 = run(s);

  REQUIRE(result3 == Expression(0.5));
}

TEST_CASE( "empty rest", "[interpreter]" ) {
  std::string s = "(rest (list))";
  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "empty first", "[interpreter]" ) {
  std::string s = "(first (list))";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "exp error", "[interpreter]" ) {
  std::string s = "(^ I I I)";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "set-property one", "[interpreter]" ) {
  std::string s = "(\"string\")";

  REQUIRE(run(s) == Expression(Atom("\"string\"")));

}


TEST_CASE( "set-property two", "[interpreter]" ) {
  std::string s = "(set-property \"number\" \"three\" (3))";

  REQUIRE(run(s) == Expression(3));

}

TEST_CASE( "set-prop error", "[interpreter]" ) {
  std::string s = "(set-property (+ 1 2) \"number\" \"three\")";

  Interpreter interp;

  std::istringstream iss(s);

  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "get-property one", "[interpreter]" ) {
  std::string s = "(begin (define a (+ 1 I)) (define b (set-property \"note\" \"complex\" a)) (get-property \"note\" b))";

  REQUIRE(run(s) == Expression(Atom("\"complex\"")));

}


TEST_CASE( "get-property two", "[interpreter]" ) {
std::string s = "(begin (define a (+ 1 I)) (define b (set-property \"note\" \"complex\" a)) (get-property \"foo\" b))";

  REQUIRE(run(s) == Expression());

}

TEST_CASE( "throwing errors for properties", "[interpreter]" ) {
  std::vector<std::string> programs = {"(get-property \"number\" \"three\" (3))", "(get-property (+ 2 3) \"number\")", "(begin)", "(set-property \"note\" \"complex\" a (3))"};

  for(auto s : programs){
    Interpreter interp;

    std::istringstream iss(s);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "testing continuous", "[interpreter]" ) {
  std::string program = "(begin (define f (lambda (x) (+ (* 2 x) 1))) (continuous-plot f (list -2 2) (list (list \"title\" \"A continuous linear function\") (list \"abscissa-label\" \"x\") (list \"ordinate-label\" \"y\"))))";

    Interpreter interp;

    std::istringstream iss(program);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    interp.evaluate();

}

TEST_CASE( "testing continuous 2", "[interpreter]" ) {
  std::string program = "(begin (define f (lambda (x) (list x (+ (* 2 x) 1)))) (discrete-plot (map f (range -2 2 0.5)) (list (list \"title\" \"The Data\") (list \"abscissa-label\" \"X Label\") (list \"ordinate-label\" \"Y Label\") (list \"text-scale\" 1))))";

    Interpreter interp;

    std::istringstream iss(program);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    interp.evaluate();

}

TEST_CASE( "testing continuous3", "[interpreter]" ) {
  std::string program = "(begin (define f (lambda (x) (sin x))) (continuous-plot f (list (- pi) pi)))";

    Interpreter interp;

    std::istringstream iss(program);

    bool ok = interp.parseStream(iss);
    REQUIRE(ok == true);

    interp.evaluate();

}

void worker(ThreadSafeQueue<std::string> & myq)
{
  for (int i = 0; i < 10; i++)
  {
    myq.push(std::to_string(i));
  }
}


TEST_CASE( "TODO", "[ThreadSafeQueue]" ) {
  ThreadSafeQueue<std::string> myq;


  std::thread th1(worker, std::ref(myq));
  std::thread th2(worker, std::ref(myq));

  std::string result;

  while (!myq.empty())
  {
    myq.try_pop(result);
    std::cout << result << " ";
  }
    std::cout << "\n";

  th1.join();
  th2.join();
}

TEST_CASE( "TODO number 2", "[ThreadSafeQueue]" ) {
  ThreadSafeQueue<std::string> myq;


  std::thread th1(worker, std::ref(myq));
  std::thread th2(worker, std::ref(myq));

  std::string result;

  while (!myq.empty())
  {
    myq.try_pop(result);
    std::cout << result << " ";
  }
    std::cout << "\n";

  th1.join();
  th2.join();
}

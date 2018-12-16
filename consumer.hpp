#include <thread>
#include <iostream>
#include <fstream>
#include <atomic>

#include "threadsafequeue.hpp"
#include "semantic_error.hpp"
#include "expression.hpp"
#include "interpreter.hpp"
#include "startup_config.hpp"

typedef std::string Input;
typedef ThreadSafeQueue<Input> Mq1;
typedef std::pair<Expression,std::string> Output;
typedef ThreadSafeQueue<Output> Mq2;

using std::endl;

class Consumer
{
public:

  Consumer(Mq1 * input, Mq2 * output)
  {
    inq = input;
    outq = output;
  }

  void operator()() const
  {
    std::ifstream ifs(STARTUP_FILE);
    Interpreter interp;
    interp.parseStream(ifs);
    interp.evaluate();

    while (true)
    {
      Input stringIn;
      inq->wait_pop(stringIn);

      if (stringIn == "")
      {
        break;
      }

      std::istringstream expression(stringIn);

      if (!interp.parseStream(expression))
      {
        std::string error = "Error: Invalid Expression. Could not parse.";
        outq->push(std::make_pair(Expression(),error));
      }
      else
      {
        try
        {
          Expression exp = interp.evaluate();
          outq->push(std::make_pair(exp,"NONE"));
        }
        catch(const SemanticError & ex)
        {
          std::string error = ex.what();
          outq->push(std::make_pair(Expression(),error));
        }
      }
    }
  }

private:
  Mq1 * inq;
  Mq2  * outq;
};

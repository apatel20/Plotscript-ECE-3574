#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <atomic>
#include <csignal>
#include <cstdlib>

#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "startup_config.hpp"
#include "consumer.hpp"
#include "expression.hpp"

using std::endl;
using std::cout;

typedef std::string Input;
typedef ThreadSafeQueue<Input> Mq1;
typedef std::pair<Expression,std::string> Output;
typedef ThreadSafeQueue<Output> Mq2;

// This global is needed for communication between the signal handler
// and the rest of the code. This atomic integer counts the number of times
// Cntl-C has been pressed by not reset by the REPL code.
//volatile sig_atomic_t global_status_flag = 0

// *****************************************************************************
// install a signal handler for Cntl-C on Windows
// *****************************************************************************
#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>

// this function is called when a signal is sent to the process
BOOL WINAPI interrupt_handler(DWORD fdwCtrlType) {

  switch (fdwCtrlType) {
  case CTRL_C_EVENT: // handle Cnrtl-C
    // if not reset since last call, exit
    if (global_status_flag > 0) {
      exit(EXIT_FAILURE);
    }
    ++global_status_flag;
    return TRUE;

  default:
    return FALSE;
  }
}

// install the signal handler
inline void install_handler() { SetConsoleCtrlHandler(interrupt_handler, TRUE); }
// *****************************************************************************

// *****************************************************************************
// install a signal handler for Cntl-C on Unix/Posix
// *****************************************************************************
#elif defined(__APPLE__) || defined(__linux) || defined(__unix) ||             \
    defined(__posix)
#include <unistd.h>

// this function is called when a signal is sent to the process
void interrupt_handler(int signal_num) {

  if(signal_num == SIGINT){ // handle Cnrtl-C
    // if not reset since last call, exit
    if (global_status_flag > 0) {
      exit(EXIT_FAILURE);
    }
    ++global_status_flag;
  }
}

// install the signal handler
inline void install_handler() {

  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = interrupt_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);
}
#endif


void prompt(){
  std::cout << "\nplotscript> ";
}

std::string readline(){
  std::string line;
  std::getline(std::cin, line);
  if (std::cin.fail() || std::cin.eof()) {
     std::cin.clear(); // reset cin state
     line.clear(); //clear input string
     std::cout << "Interrupted stdin.\n";
   }

  return line;
}

void error(const std::string & err_str){
  std::cerr << "Error: " << err_str << std::endl;
}

void info(const std::string & err_str){
  std::cout << "Info: " << err_str << std::endl;
}

int eval_from_stream(std::istream & stream){

  Interpreter interp;

  if(!interp.parseStream(stream)){
    error("Invalid Program. Could not parse.");
    return EXIT_FAILURE;
  }
  else{
    try{
      Expression exp = interp.evaluate();
      std::cout << exp << std::endl;
    }
    catch(const SemanticError & ex){
      std::cerr << ex.what() << std::endl;
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

int eval_from_file(std::string filename){

  std::ifstream ifs(filename);

  if(!ifs){
    error("Could not open file for reading.");
    return EXIT_FAILURE;
  }

  return eval_from_stream(ifs);
}

int eval_from_command(std::string argexp){

  std::istringstream expression(argexp);

  return eval_from_stream(expression);
}

std::thread startupThread(Mq1 &inq, Mq2 &outq)
{
  Consumer c1(&inq, &outq);
  std::thread thread1(c1);
  return thread1;
}

// A REPL is a repeated read-eval-print loop
void repl(Mq1 & inq, Mq2 &outq, std::thread & interpreter){

  bool execute = true;
  bool prevStop = false;

  while(!std::cin.eof()){
    global_status_flag = 0;

    prompt();
    std::string line = readline();

    if(line.empty()) continue;


    if (global_status_flag > 0)
    {

      std::cout << "Error: interpreter kernel interrupted" << endl;
      break;
    }

    if (line == "%start")
    {
      if (!interpreter.joinable())
      {
        interpreter = startupThread(inq, outq);
      }
      execute = false;
    }

    if (line == "%stop")
    {
      if (interpreter.joinable())
      {
        inq.push("");
        interpreter.join();
        prevStop = true;
      }
      execute = false;
    }

    if (line == "%reset")
    {
      if (interpreter.joinable())
      {
        inq.push("");
        interpreter.join();
        interpreter = startupThread(inq, outq);
      }
      else
      {
        interpreter = startupThread(inq, outq);
      }
      execute = false;
    }

    if (line == "%exit")
    {
      inq.push("");
      break;
    }

    if (execute && !prevStop)
    {
      inq.push(line);

      //waiting for queue B
      std::pair<Expression,std::string> result;
      outq.wait_pop(result);

      if (result.second == "NONE")//is a valid plotscript evaluated
      {
        std::cout << result.first << std::endl;
      }
      else
      {
        std::cerr << result.second << std::endl;
      }
    }
    else if (execute && prevStop)
    {
      std::cout << "Error: interpreter kernel not running" << endl;
      prevStop = false;
    }
    execute = true;
  }
}

int main(int argc, char *argv[])
{
  Mq1 inq;
  Mq2 outq;
  install_handler();
  std::thread interpreter = startupThread(inq, outq);

  if(argc == 2){
    return eval_from_file(argv[1]);
  }
  else if(argc == 3){
    if(std::string(argv[1]) == "-e"){
      return eval_from_command(argv[2]);
    }
    else{
      error("Incorrect number of command line arguments.");
    }
  }
  else{
    repl(inq, outq, interpreter);
  }

  interpreter.join();

  return EXIT_SUCCESS;
}

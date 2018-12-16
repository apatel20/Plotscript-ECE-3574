#include "notebook_app.hpp"
#include "startup_config.hpp"

#include <QDebug>

using std::cout;
using std::endl;

Mq1 inq;
Mq2 outq;

NotebookApp::NotebookApp()
{
  startupThread();

  input = new InputWidget;
  output = new OutputWidget;

  start = new QPushButton("Start Kernel");
  stop = new QPushButton("Stop Kernel");
  reset = new QPushButton("Reset Kernel");
  interrupt = new QPushButton("Interrupt");

  start->setObjectName("start");
  stop->setObjectName("stop");
  reset->setObjectName("reset");
  interrupt->setObjectName("interrupt");

  auto top = new QHBoxLayout;
  top->addWidget(start);
  top->addWidget(stop);
  top->addWidget(reset);
  top->addWidget(interrupt);

  QObject::connect(input, SIGNAL(keyPressed()), this, SLOT(onKeyPressed()));
  QObject::connect(this, SIGNAL(toOutput(Interpreter &, std::string, bool &)), output, SLOT(onOutput(Interpreter&, std::string, bool &)));
  QObject::connect(start, SIGNAL(clicked()), this, SLOT(onStart()));
  QObject::connect(stop, SIGNAL(clicked()), this, SLOT(onStop()));
  QObject::connect(reset, SIGNAL(clicked()), this, SLOT(onReset()));
  QObject::connect(interrupt, SIGNAL(clicked()), this, SLOT(onInterrupt()));

  auto layout = new QVBoxLayout;
  layout->addLayout(top);
  layout->addWidget(input);
  layout->addWidget(output);

  setLayout(layout);
}

void NotebookApp::startupThread()
{
  Consumer c2(&inq, &outq);
  gui_thread = std::thread(c2);
}

NotebookApp::~NotebookApp()
{
  inq.push("");
  gui_thread.join();
}

void NotebookApp::onStart()
{
  //qDebug("START PRESSED");
  if (!gui_thread.joinable())
  {
    startupThread();
  }
}

void NotebookApp::onStop()
{
  //qDebug("STOP PRESSED");
  if (gui_thread.joinable())
  {
    inq.push("");
    gui_thread.join();
    //prevStop = true;
  }
}

void NotebookApp::onReset()
{
  //qDebug("RESET PRESSED");
  if (gui_thread.joinable())
  {
    inq.push("");
    gui_thread.join();
    startupThread();
  }
  else
  {
    startupThread();
  }
}

void NotebookApp::onInterrupt()
{
  ++global_status_flag;
}

void NotebookApp::onKeyPressed()
{
  global_status_flag = 0;
  Interpreter interp = startup();

  std::string expression2parse = getQPlainTextString();

  if (!gui_thread.joinable())//if there is not a second thread
  {
    displayError = true;
  }

  emit toOutput(interp, expression2parse, displayError);
}

std::string NotebookApp::getQPlainTextString()
{
  return input->toPlainText().toStdString();
}

Interpreter NotebookApp::startup()
{
  std::ifstream ifs(STARTUP_FILE);

  if(!ifs){

  }

  Interpreter interp;

  if(!interp.parseStream(ifs)){

  }

  interp.evaluate();

  return interp;
}

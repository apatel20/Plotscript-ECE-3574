#ifndef NOTEBOOK_APP_HPP
#define NOTEBOOK_APP_HPP

#include "input_widget.hpp"
#include "output_widget.hpp"
#include "interpreter.hpp"
#include "expression.hpp"
#include "semantic_error.hpp"
#include "consumer.hpp"
#include "threadsafequeue.hpp"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLayout>
#include <string>
#include <fstream>

extern Mq1 inq;
extern Mq2 outq;

class NotebookApp : public QWidget
{
  Q_OBJECT
public:
  NotebookApp();
  ~NotebookApp();

  InputWidget *input;
  OutputWidget *output;

  friend class OutputWidget;

  std::string getQPlainTextString();

  Interpreter startup();

private:
  QString expression;
  bool displayError = false;

  void startupThread();

  QPushButton *start;
  QPushButton *stop;
  QPushButton *reset;
  QPushButton *interrupt;


  std::thread gui_thread;

public slots:
  void onKeyPressed();
  void onStart();
  void onStop();
  void onReset();
  void onInterrupt();

signals:
  void toOutput(Interpreter &, std::string expression, bool & displayError);

};

#endif

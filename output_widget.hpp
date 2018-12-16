#ifndef OUTPUT_WIDGET_HPP
#define OUTPUT_WIDGET_HPP

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLayout>
#include <QTimer>

#include "expression.hpp"
#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "threadsafequeue.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

using std::string;
using std::cout;
using std::endl;

typedef std::string Input;
typedef ThreadSafeQueue<Input> Mq1;
typedef std::pair<Expression,std::string> Output;
typedef ThreadSafeQueue<Output> Mq2;


class OutputWidget : public QWidget
{
  Q_OBJECT
public:
  OutputWidget(QWidget *parent = nullptr);
  friend class NotebookTest;

private:
  QGraphicsView *graphic;
  QGraphicsScene *scene;

  QTimer *timer;

  bool plot = false;
  bool lambda = false;

public slots:
  void onOutput(Interpreter &interp, std::string expression, bool & displayError);
  void onExpiration();

};

#endif

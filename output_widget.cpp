#include "output_widget.hpp"
#include "notebook_app.hpp"
#include "startup_config.hpp"

#include <QDebug>
#include <QGraphicsTextItem>
#include <QtMath>

OutputWidget::OutputWidget(QWidget *parent) : QWidget(parent)
{
  setObjectName("output");
  QLayout *layout = new QVBoxLayout();

  timer = new QTimer(this);

  scene = new QGraphicsScene();
  graphic = new QGraphicsView(scene);

  layout->addWidget(graphic);
  setLayout(layout);

  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onExpiration()));

  graphic->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
	graphic->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

  graphic->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void OutputWidget::onExpiration()
{
  std::pair<Expression,std::string> result;
  if (outq.try_pop(result))
  {
    if (result.second == "NONE")//is a valid plotscript evaluated
    {
      //std::cout << result.first << std::endl;
      //instead of cout, grab the expression to check
      Expression exp = result.first;

      if (exp.getPropSize() != 0)//is a point, line, or text
      {
        Expression property = exp.searchMap();
        if (property.head().asSymbol() == "\"point\"")//its point
        {
          Expression size = exp.handleMakePoint();
          scene->addEllipse(exp.getTail(0).head().asNumber()-(size.head().asNumber()/2),
                            exp.getTail(1).head().asNumber()-(size.head().asNumber()/2),
                            size.head().asNumber(),size.head().asNumber(),
                            QPen(),
                            QBrush(Qt::SolidPattern));
        }
        else if (property.head().asSymbol() == "\"line\"")//its line
        {
          Expression thickness = exp.handleMakeLine();
          QPen line;
          line.setWidth(thickness.head().asNumber());
          scene->addLine(exp.getTail(0).getTail(0).head().asNumber(), exp.getTail(0).getTail(1).head().asNumber(), exp.getTail(1).getTail(0).head().asNumber(), exp.getTail(1).getTail(1).head().asNumber(), line);
        }
        else //its text
        {
          Expression position = exp.handleMakeText();
          Expression scale = exp.handleScale();
          Expression rotation = exp.handleRotation();
          string withoutQuotes = exp.head().asSymbol();
          withoutQuotes.erase(0,1);
          withoutQuotes.erase(withoutQuotes.size()-1);

          auto font = QFont("Monospace");
          font.setStyleHint(QFont::TypeWriter);
          font.setPointSize(1);

          QFontMetrics fontm(font);
          QGraphicsTextItem *text = scene->addText(QString::fromStdString(withoutQuotes), font);
          int stringWidth = text->boundingRect().width();
          int stringHeight = text->boundingRect().height();
          text->setPos(position.getTail(0).head().asNumber()-stringWidth/2, position.getTail(1).head().asNumber()-stringHeight/2);
          text->setRotation(qRadiansToDegrees(-rotation.head().asNumber()));
          text->setScale(scale.head().asNumber());
        }
      }
      else //is a normal evaluation plotscript code
      {
        if ((exp.tailSize() != 0 && !lambda) || (plot && exp.isHeadList()))//is a list kind
        {
          for (auto it = exp.tailConstBegin(); it != exp.tailConstEnd(); ++it)
          {
            Expression property = (*it).searchMap();
            if (property.head().asSymbol() == "\"point\"")//its point
            {
              Expression size = (*it).handleMakePoint();
              scene->addEllipse((*it).getTail(0).head().asNumber()-(size.head().asNumber()/2),
                                (*it).getTail(1).head().asNumber()-(size.head().asNumber()/2),
                                size.head().asNumber(),size.head().asNumber(),
                                QPen(Qt::NoPen),
                                QBrush(Qt::SolidPattern));
            }
            else if (property.head().asSymbol() == "\"line\"")//its line
            {
              Expression thickness = (*it).handleMakeLine();
              QPen line;
              line.setWidth(thickness.head().asNumber());
              scene->addLine((*it).getTail(0).getTail(0).head().asNumber(), (*it).getTail(0).getTail(1).head().asNumber(), (*it).getTail(1).getTail(0).head().asNumber(), (*it).getTail(1).getTail(1).head().asNumber(), line);
            }
            else if (property.head().asSymbol() == "\"text\"") //its text
            {
              Expression position = (*it).handleMakeText();
              Expression scale = (*it).handleScale();
              Expression rotation = (*it).handleRotation();
              string withoutQuotes = (*it).head().asSymbol();
              withoutQuotes.erase(0,1);
              withoutQuotes.erase(withoutQuotes.size()-1);

              auto font = QFont("Monospace");
              font.setStyleHint(QFont::TypeWriter);
              font.setPointSize(1);

              QGraphicsTextItem *text = scene->addText(QString::fromStdString(withoutQuotes), font);
              double stringWidth = text->boundingRect().width();
              double stringHeight = text->boundingRect().height();
              text->setPos(position.getTail(0).head().asNumber()-stringWidth/2, position.getTail(1).head().asNumber()-stringHeight/2);
              text->setTransformOriginPoint(stringWidth/2, stringHeight/2);
              if (rotation.head().asNumber() == -90)
              {
                text->setRotation(-90);
              }
              else
              {
                text->setRotation(qRadiansToDegrees(-rotation.head().asNumber()));
              }
              text->setScale(scale.head().asNumber());
            }
            else
            {
              std::ostringstream out;
              out << exp;
              QString result = QString::fromStdString(out.str());
              scene->addText(result);
              graphic->setScene(scene);
            }
            graphic->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
          }
        }
        else if (!lambda)
        {
          std::ostringstream out;
          out << exp;
          QString result = QString::fromStdString(out.str());
          scene->addText(result);
          graphic->setScene(scene);
        }
      }
    }
    else // that means there is an error of some kind
    {
      QString semanticError = QString::fromStdString(result.second);
      scene->addText(semanticError);
    }
  }
  else
  {
    timer->start(100);
  }
}

void OutputWidget::onOutput(Interpreter &interp, std::string expression, bool & displayError)
{
  lambda = false;
  plot = false;
  string substring = "lambda";
  string plotString = "plot";
  if (expression.find(substring) != std::string::npos)
  { 
    lambda = true;
  }

  if (expression.find(plotString) != std::string::npos)
  {
    plot = true;
  }

  scene->clear();

  if (!displayError)
  {

    inq.push(expression);

    //std::istringstream parse(expression);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::pair<Expression,std::string> result;
    if (outq.try_pop(result))
    {
      if (result.second == "NONE")//is a valid plotscript evaluated
      {
        //std::cout << result.first << std::endl;
        //instead of cout, grab the expression to check
        Expression exp = result.first;

        if (exp.getPropSize() != 0)//is a point, line, or text
        {
          Expression property = exp.searchMap();
          if (property.head().asSymbol() == "\"point\"")//its point
          {
            Expression size = exp.handleMakePoint();
            scene->addEllipse(exp.getTail(0).head().asNumber()-(size.head().asNumber()/2),
                              exp.getTail(1).head().asNumber()-(size.head().asNumber()/2),
                              size.head().asNumber(),size.head().asNumber(),
                              QPen(),
                              QBrush(Qt::SolidPattern));
          }
          else if (property.head().asSymbol() == "\"line\"")//its line
          {
            Expression thickness = exp.handleMakeLine();
            QPen line;
            line.setWidth(thickness.head().asNumber());
            scene->addLine(exp.getTail(0).getTail(0).head().asNumber(), exp.getTail(0).getTail(1).head().asNumber(), exp.getTail(1).getTail(0).head().asNumber(), exp.getTail(1).getTail(1).head().asNumber(), line);
          }
          else //its text
          {
            Expression position = exp.handleMakeText();
            Expression scale = exp.handleScale();
            Expression rotation = exp.handleRotation();
            string withoutQuotes = exp.head().asSymbol();
            withoutQuotes.erase(0,1);
            withoutQuotes.erase(withoutQuotes.size()-1);

            auto font = QFont("Monospace");
            font.setStyleHint(QFont::TypeWriter);
            font.setPointSize(1);

            QFontMetrics fontm(font);
            QGraphicsTextItem *text = scene->addText(QString::fromStdString(withoutQuotes), font);
            int stringWidth = text->boundingRect().width();
            int stringHeight = text->boundingRect().height();
            text->setPos(position.getTail(0).head().asNumber()-stringWidth/2, position.getTail(1).head().asNumber()-stringHeight/2);
            text->setRotation(qRadiansToDegrees(-rotation.head().asNumber()));
            text->setScale(scale.head().asNumber());
          }
        }
        else //is a normal evaluation plotscript code
        {
          if ((exp.tailSize() != 0 && !lambda) || (plot && exp.isHeadList()))//is a list kind
          {
            for (auto it = exp.tailConstBegin(); it != exp.tailConstEnd(); ++it)
            {
              Expression property = (*it).searchMap();
              if (property.head().asSymbol() == "\"point\"")//its point
              {
                Expression size = (*it).handleMakePoint();
                scene->addEllipse((*it).getTail(0).head().asNumber()-(size.head().asNumber()/2),
                                  (*it).getTail(1).head().asNumber()-(size.head().asNumber()/2),
                                  size.head().asNumber(),size.head().asNumber(),
                                  QPen(Qt::NoPen),
                                  QBrush(Qt::SolidPattern));
              }
              else if (property.head().asSymbol() == "\"line\"")//its line
              {
                Expression thickness = (*it).handleMakeLine();
                QPen line;
                line.setWidth(thickness.head().asNumber());
                scene->addLine((*it).getTail(0).getTail(0).head().asNumber(), (*it).getTail(0).getTail(1).head().asNumber(), (*it).getTail(1).getTail(0).head().asNumber(), (*it).getTail(1).getTail(1).head().asNumber(), line);
              }
              else if (property.head().asSymbol() == "\"text\"") //its text
              {
                Expression position = (*it).handleMakeText();
                Expression scale = (*it).handleScale();
                Expression rotation = (*it).handleRotation();
                string withoutQuotes = (*it).head().asSymbol();
                withoutQuotes.erase(0,1);
                withoutQuotes.erase(withoutQuotes.size()-1);

                auto font = QFont("Monospace");
                font.setStyleHint(QFont::TypeWriter);
                font.setPointSize(1);

                QGraphicsTextItem *text = scene->addText(QString::fromStdString(withoutQuotes), font);
                double stringWidth = text->boundingRect().width();
                double stringHeight = text->boundingRect().height();
                text->setPos(position.getTail(0).head().asNumber()-stringWidth/2, position.getTail(1).head().asNumber()-stringHeight/2);
                text->setTransformOriginPoint(stringWidth/2, stringHeight/2);
                if (rotation.head().asNumber() == -90)
                {
                  text->setRotation(-90);
                }
                else
                {
                  text->setRotation(qRadiansToDegrees(-rotation.head().asNumber()));
                }
                text->setScale(scale.head().asNumber());
              }
              else
              {
                std::ostringstream out;
                out << exp;
                QString result = QString::fromStdString(out.str());
                scene->addText(result);
                graphic->setScene(scene);
              }
              graphic->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
            }
          }
          else if (!lambda)
          {
            std::ostringstream out;
            out << exp;
            QString result = QString::fromStdString(out.str());
            scene->addText(result);
            graphic->setScene(scene);
          }
        }
      }
      else // that means there is an error of some kind
      {
        QString semanticError = QString::fromStdString(result.second);
        scene->addText(semanticError);
      }
    }
    else
    {
      timer->start(100);
    }
  }
  else
  {
    QString kernelError = QString::fromStdString("Error: interpreter kernel not running");
    scene->addText(kernelError);
    displayError = false;
  }
}

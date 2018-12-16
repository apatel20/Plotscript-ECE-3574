#ifndef INPUT_WIDGET_HPP
#define INPUT_WIDGET_HPP

#include <QPlainTextEdit>
#include <QWidget>
#include <QString>

class InputWidget : public QPlainTextEdit
{
  Q_OBJECT
public:
  InputWidget(QWidget *parent = nullptr);

protected:
  void keyPressEvent(QKeyEvent * event);

signals:
  void keyPressed();
};

#endif

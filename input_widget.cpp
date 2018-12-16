#include "input_widget.hpp"

#include <QDebug>

InputWidget::InputWidget(QWidget *parent): QPlainTextEdit(parent)
{
  setObjectName("input");
}

void InputWidget::keyPressEvent(QKeyEvent * event)
{
  if (event->key() == Qt::Key_Return && event->modifiers() == Qt::ShiftModifier)
  {
    emit keyPressed();
  }
  QPlainTextEdit::keyPressEvent(event);
}

#include "input_widget.hpp"
#include "output_widget.hpp"
#include "notebook_app.hpp"

#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  NotebookApp notebook;

  notebook.show();

  return app.exec();
}

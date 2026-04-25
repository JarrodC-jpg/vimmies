#include "MainWindow.h"
#include <QApplication>
#include <QWidget>
#include <qdebug.h>
#include <qlogging.h>

int main(int argc, char *argv[]) {
  // Create the qt application object - this handles events,windows, etc.
  QApplication app(argc, argv);

  // Create a basic window (QWidget is the base class for all windows/widgets)
  MainWindow window;
  window.show();

  return app.exec();
}

#include "mainWindow.h"
#include "scene3d.h"
#include <stdlib.h>
#include <QtWidgets/QApplication>

// Entry point of application
int main(int argc, char** argv)
{
    QCoreApplication::addLibraryPath("./");
    QApplication app(argc, argv);

    // Create MainWindow object
    MainWindow window;

    // Set title and size of new window
    window.setWindowTitle("Mesh Viewer");
    window.resize(800, 600);

    // Show and activate window
    window.show();
    window.activateWindow();
    window.raise();

    // Return error if arise
    return app.exec();
}

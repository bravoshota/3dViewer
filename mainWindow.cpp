#include "common.h"
#include "scene3d.h"
#include "mainWindow.h"
#include "functions.h"
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QtWidgets>

// The MainWindow class to visualization of 3D-objects and their processing control
MainWindow::MainWindow()
{
    // create the window controls
    // define the action object to menu items creation
    QAction * action;

    // create the 'File' menu which will provide the file loading/saving
    QMenu * menu = menuBar()->addMenu(tr("&File"));
    // create 'New Model' item
    menu->addAction(tr("New Model"), this, &MainWindow::openModel);
    menu->addSeparator();
    // create 'Quit' item
    menu->addAction(tr("&Quit"), this, &QWidget::close);

    // create the 'Process' menu which will provide the start of different calculations
    menuActions = menuBar()->addMenu(tr("&Process"));
    // start poligonization
    action = menuActions->addAction(tr("Poligonize"), this, &MainWindow::poligonize);
    action->setEnabled(false);

    // create widget (Scene3D object) to show the 3D-objects
    widget = new Scene3D(this);
    // Set it as central widget of window
    setCentralWidget(widget);

    // create the 'Elements' menu which will provide the control of visibility of elements
    menuOptions = menuBar()->addMenu(tr("Elements"));
    // create the checker for axis
    action = menuOptions->addAction(tr("Axis"));
    action->setCheckable(true);
    action->setChecked(true);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);
    // create the checker for solid's wireframe
    action = menuOptions->addAction(tr("Wireframe"));
    action->setCheckable(true);
    action->setChecked(false);
    action->setEnabled(false);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);
    // create the checker for solid's triangles
    action = menuOptions->addAction(tr("Triangles"));
    action->setCheckable(true);
    action->setChecked(false);
    action->setEnabled(false);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    m_lastOpenedDir = QDir::currentPath();
}

// Open the STL file
void MainWindow::openModel()
{
    // get the filename using the standard Qt file dialog
    QString fileName = QFileDialog::getOpenFileName(this, "Load",
                                                    m_lastOpenedDir.absolutePath(),
                                                    "Models (*.stl)");
    if (fileName.isEmpty())
        return;

    QFileInfo fInfo(fileName);
    m_lastOpenedDir = fInfo.absoluteDir();

    // extract the file extension
    QString ext = fileName.right(3);

    // if the file is STL
    if (QString::compare(ext, "stl", Qt::CaseInsensitive) == 0)
    {
        std::vector<common::Vertex> vertices;
        std::vector<common::Triangle> faces;

        // check the type of STL (ascii or binary)
        int type = getStlFileFormat(fileName);
        if (type == STL_BINARY) {
            if (!openStlBin(fileName.toLatin1().data(), vertices, faces)) {
                QMessageBox::warning(nullptr, "ERROR!", "this STL file has incorrect format");
                return;
            }
        } else
        if (type == STL_ASCII) {
            if (!openStlAsc(fileName.toLatin1().data(), vertices, faces)) {
                QMessageBox::warning(nullptr, "ERROR!", "this STL file has incorrect format");
                return;
            }
        } else {
            QMessageBox msgBox;
            msgBox.setText("This file corrupt and cannot be loaded");
            msgBox.setInformativeText("STL Load");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
            menuActions->actions()[0]->setEnabled(false);
            return;
        }
        widget->setData(std::move(vertices), std::move(faces));
        if (!widget->load()) {
            menuActions->actions()[0]->setEnabled(false);
            QMessageBox::warning(nullptr, "ERROR!", "this STL file is corrupt");
            return;
        }
    }

    menuActions->actions()[0]->setEnabled(true);

    // enable and set on 'Axis' checker
    menuOptions->actions()[0]->setChecked(true);
    menuOptions->actions()[0]->setEnabled(true);
    // enable and set on 'Wireframe' checker
    menuOptions->actions()[1]->setChecked(true);
    menuOptions->actions()[1]->setEnabled(true);
    // enable and set on 'Triangles' checker
    menuOptions->actions()[2]->setChecked(true);
    menuOptions->actions()[2]->setEnabled(true);

    // refresh the 'elements visibility' variable
    setDockOptions();
}

// Create the 'elements visibility' variable according to checkers of 'Elements' menu
void MainWindow::setDockOptions()
{
    // use the 'Elements' menu
    QList<QAction*> actions = menuOptions->actions();
    // initiate variable with zero
    widget->m_showMask = 0;

    // set the mask of 'Axis' item
    if (actions[0]->isChecked())
        widget->m_showMask |= shAxis;
    // set the mask of 'Wireframe' item
    if (actions[1]->isChecked())
        widget->m_showMask |= shWireframe;
    // set the mask of 'Triangles' item
    if (actions[2]->isChecked())
        widget->m_showMask |= shTriangles;

    // update the showed elements
    widget->update();
}

void MainWindow::poligonize()
{
    widget->poligonize();
}

void MainWindow::keyPressEvent(QKeyEvent *pe)
{
    switch (pe->key())
    {
    case Qt::Key_N: openModel(); break;
    default: widget->keyPressEvent(pe); break;
    }
}

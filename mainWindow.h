#pragma once

#include <QMainWindow>

// The masks of 'elements visibility' variable 
#define shAxis      0x01
#define shWireframe 0x02
#define shFacets    0x04

class Scene3D;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
    Scene3D *widget;    // Qt widget to show the 3D objects
    QMenu *menuActions; // 'Process' menu
    QMenu *menuOptions; // 'Elements' menu

private slots:
	void openModel();
    void setFigureOn();
	void setDockOptions();
    void keyPressEvent(QKeyEvent* pe);
};

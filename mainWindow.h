#pragma once

#include <QMainWindow>
#include <QDir>

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
    QDir m_lastOpenedDir;

private slots:
	void openModel();
	void setDockOptions();
    void startPoligonization();
    void keyPressEvent(QKeyEvent* pe);
};

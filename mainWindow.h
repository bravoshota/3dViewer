#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QDir>

class Scene3D;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
    Scene3D *widget;    // Qt widget to show the 3D objects
    QMenu *m_menuActions; // 'Process' menu
    QMenu *m_menuOptions; // 'Elements' menu
    QDir m_lastOpenedDir;
    QLabel m_statusLabel;

private:
    QString generateGroundString() const;

private slots:
	void openModel();
	void setDockOptions();
    void changeOrientation();
    void poligonize();
    void detectSupportedTriangles();
    void editGroundHeight();
    void keyPressEvent(QKeyEvent* pe);
};

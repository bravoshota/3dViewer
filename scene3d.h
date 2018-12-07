#pragma once

#include "common.h"
#include <vector>
#include <QtGui> 
#include <QtWidgets>
#include <QtOpenGL/QGLWidget>

// Scene3D class to 3D objects visualization using Qt
class Scene3D : public QGLWidget
{
private:
    std::vector<Vertex> m_vertices;
    std::vector<Triangle> m_faces;
    std::vector<uint8_t> m_color;

    GLdouble xRot;				// the rotation angle of X axis
    GLdouble yRot;				// the rotation angle of Y axis
    GLdouble zRot;				// the rotation angle of Z axis
    GLdouble zTra;				// translation by Z axis
    GLdouble nSca;				// scale of image

    GLdouble asp;				// aspect ratio to set the maximum distance from mesh surface to origin equal to one
	// shifts to set the center of mesh on the point of origin
    GLdouble cx;					// shift by X axis
    GLdouble cy;					// shifh by Y axis
    GLdouble cz;					// shift by Z axis
	QPoint ptrMousePosition;	// the last saved mouse position

	void scale_plus();
	void scale_minus();
	void rotate_up();
	void rotate_down();
	void rotate_left();
	void rotate_right();
	void translate_down();
	void translate_up();
	void defaultScene();

	void drawAxis();
    void drawWireframe();
    void drawFacets();

protected:
    void initializeGL() override;
    void resizeGL(int nWidth, int nHeight) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* pe) override;
    void mouseMoveEvent(QMouseEvent* pe) override;
    void mouseReleaseEvent(QMouseEvent* pe) override;
    void wheelEvent(QWheelEvent* pe) override;

public:
    Scene3D(QWidget *parent = 0);
    void setData(std::vector<Vertex> &&vertices, std::vector<Triangle> &&faces);
    void load();
    void keyPressEvent(QKeyEvent* pe) override;

    int showElem;
};

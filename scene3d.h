#pragma once

#include "common.h"
#include <vector>
#include <QtOpenGL/QGLWidget>

// Scene3D class to 3D objects visualization using Qt
class Scene3D : public QGLWidget
{
private:
    std::vector<common::Vertex> m_vertices;
    std::vector<common::Triangle> m_faces;
    std::vector<common::Vector> m_faceNormals;
    std::vector<common::Edge> m_edges;
    std::vector<uint8_t> m_color;
    std::vector<uint32_t> m_triangleEdges;
    std::vector<std::vector<uint32_t>> m_edgeTriangles;

    GLdouble m_rotateX;				// the rotation angle of X axis
    GLdouble m_rotateY;				// the rotation angle of Y axis
    GLdouble m_rotateZ;				// the rotation angle of Z axis
    GLdouble m_translX;				// translation by Z axis
    GLdouble m_translZ;				// translation by Z axis
    GLdouble m_scale;				// scale of image
    GLdouble m_scaleDefault;        // default scale for the model

	// shifts to set the center of mesh on the point of origin
	QPoint ptrMousePosition;	// the last saved mouse position

    void scaleUp();
    void scaleDown();
    void rotateUpX();
    void rotateDownX();
    void rotateUpY();
    void rotateDownY();
    void rotateUpZ();
    void rotateDownZ();
    void translateDown();
    void translateUp();
    void translateLeft();
    void translateRight();
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
    Scene3D(QWidget *parent = nullptr);
    void setData(std::vector<common::Vertex> &&vertices,
                 std::vector<common::Triangle> &&faces);
    bool load();
    void keyPressEvent(QKeyEvent* pe) override;

    int m_showMask;
};

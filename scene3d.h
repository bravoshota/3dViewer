#pragma once

#include "common.h"
#include <vector>
#include <unordered_set>
#include <QtOpenGL/QGLWidget>

// The masks of 'elements visibility' variable
#define shAxis      0x01
#define shWireframe 0x02
#define shTriangles 0x04
#define shNormals   0x08

// Scene3D class to 3D objects visualization using Qt
class Scene3D : public QGLWidget
{
private:
    // general data
    std::vector<common::Vertex>        m_vertices;
    std::vector<common::Triangle>      m_triangles;
    std::vector<common::Vector>        m_normals;
    std::vector<common::Vertex>        m_normalVertices;
    std::vector<uint32_t>              m_normalIndices;
    std::vector<common::Edge>          m_edges;
    std::vector<uint32_t>              m_triangleEdges;
    std::vector<std::vector<uint32_t>> m_edgeTriangles;
    std::vector<uint32_t>              m_triangleFaces;
    std::vector<std::vector<uint32_t>> m_faces;
    // drawing helpers
    std::vector<common::Vertex>   m_drawVertices;
    std::vector<common::Triangle> m_drawTriangles;
    std::vector<uint8_t>          m_drawColor;

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
    void drawTriangles();
    void drawNormals();

    bool fixTrianglesOrientation(const common::Triangle &tria1, common::Triangle &tria2,
                                 const common::Edge &edge) const;
    void fixTrianglesOrientation();

protected:
    void initializeGL() override;
    void resizeGL(int nWidth, int nHeight) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* pe) override;
    void mouseMoveEvent(QMouseEvent* pe) override;
    void mouseReleaseEvent(QMouseEvent* pe) override;
    void wheelEvent(QWheelEvent* pe) override;
    void updateForDraw();

public:
    Scene3D(QWidget *parent = nullptr);
    bool setModel(std::vector<common::Vertex> &&vertices,
                  std::vector<common::Triangle> &&faces);
    bool fitModel();
    bool updateAll();
    void changeOrientation();
    bool poligonize(double angleInRadians = 0.9);
    void keyPressEvent(QKeyEvent* pe) override;

    int m_showMask;
};

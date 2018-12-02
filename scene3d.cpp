#include "scene3d.h"
#include "functions.h"
#include "mainWindow.h"
#include <fstream>
#include <vector>
#include <QtGui> 
#include <QtWidgets>
#include <float.h>

// Initiation of Scene3D object
Scene3D::Scene3D(QWidget* parent) : QGLWidget(parent)
{
    // default values of parameters
    xRot = -90;
    yRot = 0;
    zRot = 0;
    zTra = 0;
    nSca = 1;
}

// Initiation of OpenGL
void Scene3D::initializeGL()
{
    // set the background color
    qglClearColor(Qt::white);
    // to hide the covered deeper objects
    glEnable(GL_DEPTH_TEST);
    // disable the shade
    glShadeModel(GL_FLAT);
    // to use the blended colors
    glEnable(GL_BLEND);
    // enable the transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // calculate the aspect ratio
    load();
    // to use the arrays of vertices for drawing
    glEnableClientState(GL_VERTEX_ARRAY);
    // to use the arrays of colors for drawing
    glEnableClientState(GL_COLOR_ARRAY);
    // to use to draw triangles
    glEnableClientState(GL_INDEX_ARRAY);
}

// Recalculate the scene parameters after the window will resized
void Scene3D::resizeGL(int nWidth, int nHeight)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // choose the most problem direction
    GLfloat ratio = (GLfloat)nHeight / (GLfloat)nWidth;
    if (nWidth >= nHeight)
        // change the matrix using horizontal ratio
        glOrtho(-1.0 / ratio, 1.0 / ratio, -1.0, 1.0, -10.0, 1.0);
    else
        // change the matrix using vertical ratio
        glOrtho(-1.0, 1.0, -1.0*ratio, 1.0*ratio, -10.0, 1.0);
    // transform to window coordinates
    glViewport(0, 0, (GLint)nWidth, (GLint)nHeight);
}

// Draw the scene
void Scene3D::paintGL()
{
    // set the initial parameters
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    // reset the transformation matrix to identity
    glLoadIdentity();
    // apply the scale
    glScalef(nSca, nSca, nSca);
    // apply the translation
    glTranslatef(0.0f, zTra, 0.0f);
    // apply the rotations
    glRotatef(xRot, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot, 0.0f, 1.0f, 0.0f);
    glRotatef(zRot, 0.0f, 0.0f, 1.0f);

    // draw the elements using the 'elements visibility' variable
    if (showElem & shAxis) drawAxis();
    if (showElem & shWireframe) drawWireframe();
    if (showElem & shFacets) drawFacets();
}

// Set the initial position of actions doing by mouse
void Scene3D::mousePressEvent(QMouseEvent* pe)
{
    // save the mouse position
    ptrMousePosition = pe->pos();
}

// Event don't used; it was created for consistent purpose
void Scene3D::mouseReleaseEvent(QMouseEvent* /*pe*/)
{
}

// Process rotation by mouse
void Scene3D::mouseMoveEvent(QMouseEvent* pe)
{
    // calculate rotation by X axis
    xRot += 180 / nSca * (GLfloat)(pe->y() - ptrMousePosition.y()) / height();
    // calculate rotation by Z axis
    zRot += 180 / nSca * (GLfloat)(pe->x() - ptrMousePosition.x()) / width();
    // save the mouse position
    ptrMousePosition = pe->pos();
    // draw the scene
    updateGL();
}

// Process zoom by mouse
void Scene3D::wheelEvent(QWheelEvent* pe)
{
    // choose the scale method
    if ((pe->delta()) > 0)
        scale_plus();
    else if ((pe->delta()) < 0)
        scale_minus();

    // draw the scene
    updateGL();
}

// The controls actions of the scene
// zoom
void Scene3D::setData(std::vector<Vertex> &&vertices, std::vector<Triangle> &&faces)
{
    std::swap(m_vertices, vertices);
    std::swap(m_faces, faces);
}

void Scene3D::scale_plus()
{
    nSca = nSca * 1.1;
}

void Scene3D::scale_minus()
{
    nSca = nSca/1.1;
}

// rotation
void Scene3D::rotate_up()
{
    xRot += 1.0f;
}

void Scene3D::rotate_down()
{
    xRot -= 1.0f;
}

void Scene3D::rotate_left()
{
    zRot += 1.0f;
}

void Scene3D::rotate_right()
{
    zRot -= 1.0f;
}

// translation
void Scene3D::translate_down()
{
    zTra -= 0.05f;
}

void Scene3D::translate_up()
{
    zTra += 0.05f;
}

// reset to default
void Scene3D::defaultScene()
{
    xRot=-90;
    yRot=0;
    zRot=0;
    zTra=0;
    nSca=1;
}

// Draw the axis
void Scene3D::drawAxis() 
{
    // set the line width
    glLineWidth(3.0f);
    // set the mode to 'lines'
    glBegin(GL_LINES);

    // the X axis
    glColor4f(1.00f, 0.00f, 0.00f, 1.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    // the Y axis
    glColor4f(0.00f, 1.00f, 0.00f, 1.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -1.0f, 0.0f);
    // the Z axis
    glColor4f(0.00f, 0.00f, 1.00f, 1.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -1.0f);

    // end of the 'lines' mode
    glEnd();
}

// Calculate the aspect ratio of given mesh
void Scene3D::load()
{
    // initiate the ratio and center point
    asp = 1;
    cx = 0;
    cy = 0;
    cz = 0;

    // if we have no vertices return
    if (m_vertices.empty())
        return;

    // initiate the total number of vertices of all parts
    m_color.resize(4 * m_vertices.size());

    // initiate the maximum and minimum values of X, Y and Z
    float mx = FLT_MAX;
    float my = FLT_MAX;
    float mz = FLT_MAX;
    float Mx =-FLT_MAX;
    float My =-FLT_MAX;
    float Mz =-FLT_MAX;
    // loop over the parts
    for (size_t i = 0; i < m_vertices.size(); ++i)
    {
        const Vertex &p = m_vertices[i];
        // check and replace the maximum and minimum values of X, Y and Z
        if (mx > p.x)
            mx = p.x;
        if (Mx < p.x)
            Mx = p.x;
        if (my > p.y)
            my = p.y;
        if (My < p.y)
            My = p.y;
        if (mz > p.z)
            mz = p.z;
        if (Mz < p.z)
            Mz = p.z;

        m_color[4*i + 0] = 0.5f;
        m_color[4*i + 1] = 0.7f;
        m_color[4*i + 2] = 0.5f;
        m_color[4*i + 3] = 0.3f;
    }

    // define the middle point
    cx = (mx + Mx) / 2;
    cy = (my + My) / 2;
    cz = (mz + Mz) / 2;

    // calculate the lowest ratio
    if (Mx != mx) asp = 1 / (Mx - mx);
    if (My != my) { if (1 / (My - my) < asp) asp = 1 / (My - my); }
    if (Mz != mz) { if (1 / (Mz - mz) < asp) asp = 1 / (Mz - mz); }
}

void Scene3D::keyPressEvent(QKeyEvent *pe)
{
    // set the actions of keyboard keys
    switch (pe->key())
    {
    case Qt::Key_Plus: scale_plus(); break;
    case Qt::Key_Equal: scale_plus(); break;
    case Qt::Key_Minus: scale_minus(); break;
    case Qt::Key_Up: rotate_up(); break;
    case Qt::Key_Down: rotate_down(); break;
    case Qt::Key_Left: rotate_left(); break;
    case Qt::Key_Right: rotate_right(); break;
    case Qt::Key_Z: translate_down(); break;
    case Qt::Key_X: translate_up(); break;
    case Qt::Key_Space: defaultScene(); break;
    case Qt::Key_Escape: this->close(); break;
    }
    updateGL();
}

// Draw the facets of mesh
void Scene3D::drawFacets()
{
    // check do the mesh exist
    if (m_vertices.empty())
        return;
    // set the vertices
    glVertexPointer(3, GL_FLOAT, 0, m_vertices.data());
    // set the colors
    glColorPointer(4, GL_FLOAT, 0, m_color.data());
    // set the facets
    glDrawElements(GL_TRIANGLES, 3*m_faces.size(), GL_UNSIGNED_INT, m_faces.data());
}

// Draw the wireframe of mesh
void Scene3D::drawWireframe()
{
    // check do the mesh exist
    if (m_vertices.empty())
        return;
    /*
    // set the line width
    glLineWidth(2.0f);
    // set the vertices
    glVertexPointer(3, GL_FLOAT, 0, m_vertices.data());
    // set the colors
    glColorPointer(4, GL_FLOAT, 0, EdgeColor);
    // set the edges
    glDrawElements(GL_LINES, 6 * fmesh, GL_UNSIGNED_INT, EdgeIndex);
    */
}

#include "scene3d.h"
#include "functions.h"
#include "mainWindow.h"
#include <QMouseEvent>
#include <unordered_set>
#include <fstream>
#include <float.h>

// Initiation of Scene3D object
Scene3D::Scene3D(QWidget* parent) : QGLWidget(parent)
{
    m_scaleDefault = 1;
    defaultScene();
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
    // to use to draw triangles
    glEnableClientState(GL_INDEX_ARRAY);
}

// Recalculate the scene parameters after the window will resized
void Scene3D::resizeGL(int nWidth, int nHeight)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // choose the most problem direction
    GLdouble ratio = static_cast<GLdouble>(nHeight)/nWidth;
    if (nWidth >= nHeight)
        // change the matrix using horizontal ratio
        glOrtho(-1.0 / ratio, 1.0 / ratio, -1.0, 1.0, -10.0, 1.0);
    else
        // change the matrix using vertical ratio
        glOrtho(-1.0, 1.0, -1.0*ratio, 1.0*ratio, -10.0, 1.0);
    // transform to window coordinates
    glViewport(0, 0, nWidth, nHeight);
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
    glScaled(m_scale, m_scale, m_scale);
    // apply the translation
    glTranslated(m_translX, 0.0, 0.0);
    glTranslated(0.0, m_translZ, 0.0);
    // apply the rotations
    glRotated(m_rotateX, 1.0, 0.0, 0.0);
    glRotated(m_rotateY, 0.0, 1.0, 0.0);
    glRotated(m_rotateZ, 0.0, 0.0, 1.0);

    // draw the elements using the 'elements visibility' variable
    drawAxis();
    drawWireframe();
    drawFacets();
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
    m_rotateX += 180.0 * static_cast<GLdouble>(pe->y() - ptrMousePosition.y()) / height();
    // calculate rotation by Z axis
    m_rotateZ += 180.0 * static_cast<GLdouble>(pe->x() - ptrMousePosition.x()) / width();
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
        scaleUp();
    else if ((pe->delta()) < 0)
        scaleDown();

    // draw the scene
    updateGL();
}

void Scene3D::scaleUp()
{
    m_scale = m_scale * 1.1;
    if (m_scale > 1e+20)
        m_scale = 1e+20;
}

void Scene3D::scaleDown()
{
    m_scale = m_scale/1.1;
    if (m_scale < 1e-5)
        m_scale = 1e-5;
}

// rotation
void Scene3D::rotateUpX()
{
    m_rotateX += 1.0;
}

void Scene3D::rotateDownX()
{
    m_rotateX -= 1.0;
}

void Scene3D::rotateUpY()
{
    m_rotateY += 1.0;
}

void Scene3D::rotateDownY()
{
    m_rotateY -= 1.0;
}

void Scene3D::rotateUpZ()
{
    m_rotateZ += 1.0;
}

void Scene3D::rotateDownZ()
{
    m_rotateZ -= 1.0;
}

// translation
void Scene3D::translateDown()
{
    m_translZ -= 0.05/m_scale;
}

void Scene3D::translateUp()
{
    m_translZ += 0.05/m_scale;
}

void Scene3D::translateLeft()
{
    m_translX -= 0.05/m_scale;
}

void Scene3D::translateRight()
{
    m_translX += 0.05/m_scale;
}

// reset to default
void Scene3D::defaultScene()
{
    m_rotateX =-90;
    m_rotateY = 0;
    m_rotateZ = 0;
    m_translX = 0;
    m_translZ = 0;
    m_scale = m_scaleDefault;
}

// Draw the axis
void Scene3D::drawAxis() 
{
    if(!(m_showMask & shAxis))
        return;

    // set the line width
    glLineWidth(3.0f);
    // set the mode to 'lines'
    glBegin(GL_LINES);

    const double axisLen = 0.2/m_scale;

    // the X axis
    glColor3ub(255, 0, 0);
    glVertex3d( axisLen, 0.0, 0.0);
    glVertex3d(-axisLen, 0.0, 0.0);
    // the Y axis
    glColor3d(0, 255, 0);
    glVertex3d(0.0, axisLen, 0.0);
    glVertex3d(0.0,-axisLen, 0.0);
    // the Z axis
    glColor3ub(0, 0, 255);
    glVertex3d(0.0, 0.0, axisLen);
    glVertex3d(0.0, 0.0,-axisLen);

    // end of the 'lines' mode
    glEnd();
}

void Scene3D::setData(std::vector<common::Vertex> &&vertices,
                      std::vector<common::Triangle> &&faces)
{
    std::swap(m_vertices, vertices);
    std::swap(m_faces, faces);
}

// Calculate the aspect ratio of given mesh
void Scene3D::load()
{
    // if we have no vertices return
    if (m_vertices.empty())
        return;

    // calculate wireframe
    m_edges.clear();
    m_edges.reserve(m_faces.size());
    std::unordered_set<uint64_t> pairs;
    for (const common::Triangle &tri : m_faces)
    {
        for (int i = 0; i < 3; ++i)
        {
            uint32_t i1 = tri.coord[i];
            uint32_t i2 = tri.coord[(i+1)%3];
            uint64_t pair;
            pair = i1;
            pair = (pair << 32) + i2;
            if (pairs.find(pair) != pairs.end())
                continue;

            pair = i2;
            pair = (pair << 32) + i1;
            if (pairs.find(pair) != pairs.end())
                continue;

            pairs.insert(pair);
            m_edges.push_back({i1, i2});
        }
    }

    // initiate the total number of vertices of all parts
    m_color.resize(4 * m_vertices.size());

    // initiate the maximum and minimum values of X, Y and Z
    double mx = DBL_MAX;
    double my = DBL_MAX;
    double mz = DBL_MAX;
    double Mx =-DBL_MAX;
    double My =-DBL_MAX;
    double Mz =-DBL_MAX;
    // loop over the parts
    for (size_t i = 0; i < m_vertices.size(); ++i)
    {
        const common::Vertex &p = m_vertices[i];
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

        m_color[4*i + 0] = 50;
        m_color[4*i + 1] = 170;
        m_color[4*i + 2] = 128;
        m_color[4*i + 3] = 90;
    }

    m_scaleDefault = 1;

    auto fixScale = [&](double val)
    {
        if (val > DBL_EPSILON && m_scaleDefault > 1/val)
            m_scaleDefault = 1/val;
    };

    fixScale(Mx - mx);
    fixScale(My - my);
    fixScale(Mz - mz);

    defaultScene();
}

void Scene3D::keyPressEvent(QKeyEvent *pe)
{
    // set the actions of keyboard keys
    switch (pe->key())
    {
    case Qt::Key_Plus: scaleUp(); break;
    case Qt::Key_Equal: scaleUp(); break;
    case Qt::Key_Minus: scaleDown(); break;
    case Qt::Key_W: rotateUpX(); break;
    case Qt::Key_S: rotateDownX(); break;
    case Qt::Key_A: rotateUpZ(); break;
    case Qt::Key_D: rotateDownZ(); break;
    case Qt::Key_Q: rotateUpY(); break;
    case Qt::Key_E: rotateDownY(); break;
    case Qt::Key_Up: translateUp(); break;
    case Qt::Key_Down: translateDown(); break;
    case Qt::Key_Left: translateLeft(); break;
    case Qt::Key_Right: translateRight(); break;
    case Qt::Key_Space: defaultScene(); break;
    case Qt::Key_Escape: this->close(); break;
    }
    updateGL();
}

// Draw the facets of mesh
void Scene3D::drawFacets()
{
    if(!(m_showMask & shFacets))
        return;

    // check do the mesh exist
    if (m_vertices.empty())
        return;
    // to use the arrays of colors for drawing
    glEnableClientState(GL_COLOR_ARRAY);
    // set the vertices
    glVertexPointer(3, GL_DOUBLE, 0, m_vertices.data());
    // set the colors
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_color.data());
    // set the facets
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(3*m_faces.size()), GL_UNSIGNED_INT, m_faces.data());
}

// Draw the wireframe of mesh
void Scene3D::drawWireframe()
{
    if(!(m_showMask & shWireframe))
        return;

    // check do the mesh exist
    if (m_vertices.empty())
        return;
    // to use the arrays of colors for drawing
    glDisableClientState(GL_COLOR_ARRAY);
    // draw black wireframe
    glColor3ub(20, 20, 20);
    // set the line width
    glLineWidth(1.0f);
    // set the vertices
    glVertexPointer(3, GL_DOUBLE, 0, m_vertices.data());
    // set the edges
    glDrawElements(GL_LINES, static_cast<GLsizei>(m_edges.size())*2, GL_UNSIGNED_INT, m_edges.data());
}

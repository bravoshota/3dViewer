#include "scene3d.h"
#include "functions.h"
#include <QDebug>
#include <QMouseEvent>
#include <unordered_map>
#include <fstream>
#include <float.h>
#include <math.h>

// Initiation of Scene3D object
Scene3D::Scene3D(QWidget* parent) : QGLWidget(parent)
{
    m_scaleDefault = 1.0;
    m_totalArea = 0.0;
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
    drawTriangles();
    drawNormals();
    drawGround();
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

void Scene3D::updateForDraw()
{
    const uint8_t A = 255;

    m_drawColor.clear();
    m_drawVertices.clear();
    m_drawTriangles.clear();

    auto addTriangle = [&](uint32_t iTri, uint8_t R, uint8_t G, uint8_t B)
    {
        uint32_t index = static_cast<uint32_t>(m_drawVertices.size());
        m_drawTriangles.push_back({index, index + 1, index + 2});

        auto addTriangle1 = [&](uint8_t r, uint8_t g, uint8_t b)
        {
            const common::Triangle &tri = m_triangles[iTri];
            for (uint8_t i = 0; i < 3; ++i)
            {
                m_drawVertices.push_back(m_vertices[tri.coord[i]]);
                m_drawColor.push_back(r);
                m_drawColor.push_back(g);
                m_drawColor.push_back(b);
                m_drawColor.push_back(A);
            }
        };

        if (m_isTriangleSupported[iTri])
            addTriangle1(255, 0, 0);
        else
            addTriangle1(R, G, B);
    };

    m_drawColor.reserve(12 * m_triangles.size());
    m_drawVertices.reserve(3 * m_triangles.size());
    m_drawTriangles.reserve(m_triangles.size() - m_supportedTriangles.size());

    if (m_faces.empty())
    {
        uint8_t R = 50;
        uint8_t G = 170;
        uint8_t B = 128;

        for (uint32_t i = 0 ; i < m_triangles.size(); ++i)
        {
            addTriangle(i, R, G, B);
        }
    }
    else
    {
        for (const auto &faceTriangles : m_faces)
        {
            auto R = static_cast<uint8_t>(std::rand()*256/RAND_MAX);
            auto G = static_cast<uint8_t>(std::rand()*256/RAND_MAX);
            auto B = static_cast<uint8_t>(std::rand()*256/RAND_MAX);

            for (uint32_t i : faceTriangles)
            {
                addTriangle(i, R, G, B);
            }
        }
    }
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
    m_scale = 1.0;
    m_groundHeight = 0.0;
    m_boundBoxMin = { DBL_MAX, DBL_MAX, DBL_MAX};
    m_boundBoxMax = {-DBL_MAX,-DBL_MAX,-DBL_MAX};
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

bool Scene3D::setModel(std::vector<common::Vertex> &&vertices,
                       std::vector<common::Triangle> &&faces)
{
    std::swap(m_vertices, vertices);
    std::swap(m_triangles, faces);
    defaultScene();
    return fitModel();
}

bool Scene3D::fitModel()
{
    m_totalArea = 0.0;
    m_supportedTriangles.clear();
    m_isTriangleSupported.clear();

    // if we have no vertices return
    if (m_vertices.empty() || m_triangles.empty())
        return false;

    m_isTriangleSupported.insert(m_isTriangleSupported.begin(), m_triangles.size(), false);

    // initiate the maximum and minimum values of X, Y and Z
    m_boundBoxMin = { DBL_MAX, DBL_MAX, DBL_MAX};
    m_boundBoxMax = {-DBL_MAX,-DBL_MAX,-DBL_MAX};
    // loop over the parts
    for (size_t i = 0; i < m_vertices.size(); ++i)
    {
        const common::Vertex &p = m_vertices[i];
        // check and replace the maximum and minimum values of X, Y and Z
        if (m_boundBoxMin.x > p.x)
            m_boundBoxMin.x = p.x;
        if (m_boundBoxMax.x < p.x)
            m_boundBoxMax.x = p.x;
        if (m_boundBoxMin.y > p.y)
            m_boundBoxMin.y = p.y;
        if (m_boundBoxMax.y < p.y)
            m_boundBoxMax.y = p.y;
        if (m_boundBoxMin.z > p.z)
            m_boundBoxMin.z = p.z;
        if (m_boundBoxMax.z < p.z)
            m_boundBoxMax.z = p.z;
    }

    auto fixScale = [&](double val)
    {
        if (val > DBL_EPSILON && m_scaleDefault > 1/val)
            m_scaleDefault = 1/val;
    };

    m_scaleDefault = 1;
    fixScale(m_boundBoxMax.x - m_boundBoxMin.x);
    fixScale(m_boundBoxMax.y - m_boundBoxMin.y);
    fixScale(m_boundBoxMax.z - m_boundBoxMin.z);
    m_scale = m_scaleDefault;

    double normalLen = std::max(std::max(
            m_boundBoxMax.x - m_boundBoxMin.x,
            m_boundBoxMax.y - m_boundBoxMin.y),
            m_boundBoxMax.z - m_boundBoxMin.z) / 20;

    // calculate normals
    m_normals.resize(m_triangles.size());
    m_triangleArea.resize(m_triangles.size());
    m_normalIndices.resize(2 * m_triangles.size());
    m_normalVertices.resize(2 * m_triangles.size());
    for (uint32_t i = 0; i < m_triangles.size(); ++i)
    {
        const uint32_t *indices = m_triangles[i].coord;
        calculateNormal(m_vertices[indices[0]],
                        m_vertices[indices[1]],
                        m_vertices[indices[2]],
                        m_normals[i]);

        m_triangleArea[i] = m_normals[i].length();
        m_totalArea += m_triangleArea[i];

        if (!normalize(m_normals[i]))
            return false;

        auto I = 2 * i;
        m_normalVertices[I] = (m_vertices[indices[0]] + m_vertices[indices[1]] + m_vertices[indices[2]]) / 3;
        m_normalVertices[I + 1] = m_normalVertices[I] + m_normals[i] * normalLen;
        m_normalIndices[I    ] = I;
        m_normalIndices[I + 1] = I + 1;
    }

    // calculate vertices for ground
    {
        double minX = m_boundBoxMin.x;
        double minY = m_boundBoxMin.y;
        double maxX = m_boundBoxMax.x;
        double maxY = m_boundBoxMax.y;
        double lenY = 2.0 * (maxY - minY);
        double lenX = 2.0 * (maxX - minX);
        minX -= lenX*0.25;
        maxX += lenX*0.25;
        minY -= lenY*0.25;
        maxY += lenY*0.25;

        double stepSize = 2.0;
        size_t stepsX = static_cast<size_t>(lenX / stepSize);
        size_t stepsY = static_cast<size_t>(lenY / stepSize);
        if (stepsX < 10 || stepsY < 10)
        {
            if (stepsX < stepsY)
            {
                stepsX = 10;
                stepSize = lenX / 10;
                stepsY = static_cast<size_t>(lenY / stepSize);
            }
            else
            {
                stepsY = 10;
                stepSize = lenY / 10;
                stepsX = static_cast<size_t>(lenX / stepSize);
            }
        }

        m_groundVertices.clear();
        m_groundVertices.reserve((stepsX - 2) * (stepsY - 2) * 2);

        for (size_t i = 1; i < stepsX; ++i)
        {
            double x = minX + i*stepSize;
            m_groundVertices.push_back({x, minY, m_groundHeight});
            m_groundVertices.push_back({x, maxY, m_groundHeight});
        }
        for (size_t i = 1; i < stepsY; ++i)
        {
            double y = minY + i*stepSize;
            m_groundVertices.push_back({minX, y, m_groundHeight});
            m_groundVertices.push_back({maxX, y, m_groundHeight});
        }

        m_groundIndices.clear();
        m_groundIndices.resize(m_groundVertices.size());
        for (uint32_t i = 0; i < m_groundVertices.size(); ++i)
            m_groundIndices[i] = i;
    }

    return true;
}

// Calculate the aspect ratio of given mesh
bool Scene3D::updateAll()
{
    m_triangleFaces.clear();
    m_faces.clear();
    m_drawVertices.clear();
    m_drawTriangles.clear();
    m_drawColor.clear();
    m_edges.clear();
    m_triangleEdges.clear();

    // if we have no vertices return
    if (m_vertices.empty() || m_triangles.empty())
        return false;

    // calculate wireframe and triangle-edge connector
    m_edges.reserve(m_triangles.size());
    m_triangleEdges.reserve(3*m_triangles.size());

    std::unordered_map<uint64_t, uint32_t> pairs;
    for (const common::Triangle &tri : m_triangles)
    {
        for (int i = 0; i < 3; ++i)
        {
            uint32_t i1 = tri.coord[i];
            uint32_t i2 = tri.coord[(i+1)%3];
            uint64_t pair;
            pair = i1;
            pair = (pair << 32) + i2;
            auto it = pairs.find(pair);
            if (it != pairs.end())
            {
                m_triangleEdges.push_back(it->second);
                continue;
            }

            pair = i2;
            pair = (pair << 32) + i1;
            it = pairs.find(pair);
            if (it != pairs.end())
            {
                m_triangleEdges.push_back(it->second);
                continue;
            }

            pairs[pair] = static_cast<uint32_t>(m_edges.size());
            m_triangleEdges.push_back(static_cast<uint32_t>(m_edges.size()));
            m_edges.push_back({i1, i2});
        }
    }

    // fill edge-triangle connector
    m_edgeTriangles.clear();
    m_edgeTriangles.resize(m_edges.size());
    for (uint32_t i = 0; i < m_triangleEdges.size(); ++i)
    {
        m_edgeTriangles[m_triangleEdges[i]].push_back(i/3);
    }

    // it's time to fix triangle normals' orientations
    fixTrianglesOrientation();

    updateForDraw();

    return true;
}

void Scene3D::changeOrientation()
{
    for (auto &tri : m_triangles)
        std::swap(tri.coord[0], tri.coord[1]);

    fitModel();
    updateAll();
    updateGL();
}

bool Scene3D::poligonize(double angleInRadians)
{
    if (m_triangles.empty())
        return false;

    m_faces.clear();
    m_triangleFaces.resize(m_triangles.size());

    std::unordered_set<uint32_t> visited;
    for (uint32_t iStartTri = 0; iStartTri < m_triangles.size(); ++iStartTri)
    {
        if (visited.find(iStartTri) != visited.end())
            continue;

        std::vector<uint32_t> singleFace;
        singleFace.push_back(iStartTri);
        visited.insert(iStartTri);
        m_triangleFaces[iStartTri] = static_cast<uint32_t>(m_faces.size());
        for (uint32_t i = 0; i < singleFace.size(); ++i)
        {
            uint32_t iTri = singleFace[i];
            const common::Vector &normal = m_normals[iTri];
            uint32_t*edges = &m_triangleEdges[3*iTri];
            for (uint32_t j = 0; j < 3; ++j)
            {
                uint32_t iEdge = edges[j];
                const std::vector<uint32_t> &edgeTriangles = m_edgeTriangles[iEdge];
                for (uint32_t iNeigh : edgeTriangles)
                {
                    if (iNeigh == iTri)
                        continue;
                    if (visited.find(iNeigh) != visited.end())
                        continue;
                    const common::Vector &neighNormal = m_normals[iNeigh];
                    if (normal * neighNormal < angleInRadians)
                        continue;
                    singleFace.push_back(iNeigh);
                    visited.insert(iNeigh);
                    m_triangleFaces[iNeigh] = static_cast<uint32_t>(m_faces.size());
                }
            }
        }
        m_faces.push_back(std::move(singleFace));
    }

    updateForDraw();
    updateGL();

    return true;
}

double Scene3D::detectSupportedTriangles()
{
    double area = 0.0;
    double cosValue = -cos(45.0);
    m_supportedTriangles.clear();
    m_isTriangleSupported.clear();
    m_isTriangleSupported.reserve(m_normals.size());

    for (uint32_t i = 0; i < m_normals.size(); ++i)
    {
        const common::Vector &nor = m_normals[i];
        if (nor.z < cosValue)
        {
            m_supportedTriangles.push_back(i);
            m_isTriangleSupported.push_back(true);
            area += m_triangleArea[i];
        }
        else
        {
            m_isTriangleSupported.push_back(false);
        }
    }

    updateForDraw();
    updateGL();

    return area;
}

void Scene3D::keyPressEvent(QKeyEvent *pe)
{
    // set the actions of keyboard keys
    switch (pe->key())
    {
    case Qt::Key_Plus:  scaleUp();        break;
    case Qt::Key_Equal: scaleUp();        break;
    case Qt::Key_Minus: scaleDown();      break;
    case Qt::Key_W:     rotateDownX();    break;
    case Qt::Key_S:     rotateUpX();      break;
    case Qt::Key_A:     rotateUpZ();      break;
    case Qt::Key_D:     rotateDownZ();    break;
    case Qt::Key_Q:     rotateUpY();      break;
    case Qt::Key_E:     rotateDownY();    break;
    case Qt::Key_P:     poligonize();     break;
    case Qt::Key_Up:    translateUp();    break;
    case Qt::Key_Down:  translateDown();  break;
    case Qt::Key_Left:  translateLeft();  break;
    case Qt::Key_Right: translateRight(); break;
    case Qt::Key_Space: defaultScene();   break;
    default: return;
    }
    updateGL();
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

// Draw the facets of mesh
void Scene3D::drawTriangles()
{
    if(!(m_showMask & shTriangles))
        return;

    // check does the mesh exist
    if (m_vertices.empty())
        return;

    const common::Vertex *vert;
    const common::Triangle *tria;
    if (m_drawVertices.empty())
    {
        vert = m_vertices.data();
        tria = m_triangles.data();
    }
    else
    {
        vert = m_drawVertices.data();
        tria = m_drawTriangles.data();
    }

    glEnableClientState(GL_COLOR_ARRAY);
    // set the vertices
    glVertexPointer(3, GL_DOUBLE, 0, vert);
    // set the colors
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_drawColor.data());
    // set the facets
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(3*m_triangles.size()),
                   GL_UNSIGNED_INT, tria);}

// Draw the facets of mesh
void Scene3D::drawNormals()
{
    if(!(m_showMask & shNormals))
        return;

    // check do normals exist
    if (m_normalVertices.empty())
        return;

    glDisableClientState(GL_COLOR_ARRAY);
    glColor3ub(0, 0, 255);
    // set the vertices
    glVertexPointer(3, GL_DOUBLE, 0, m_normalVertices.data());
    // set the facets
    glDrawElements(GL_LINES, static_cast<GLsizei>(m_normalIndices.size()),
                   GL_UNSIGNED_INT, m_normalIndices.data());
}

// Draw the facets of mesh
void Scene3D::drawGround()
{
    if(!(m_showMask & shGround))
        return;

    if (m_groundVertices.empty())
        return;

    glDisableClientState(GL_COLOR_ARRAY);
    glColor4ub(100, 100, 200, 200);
    // set the vertices
    glVertexPointer(3, GL_DOUBLE, 0, m_groundVertices.data());
    // set the facets
    glDrawElements(GL_LINES, static_cast<GLsizei>(m_groundIndices.size()),
                   GL_UNSIGNED_INT, m_groundIndices.data());
}

bool Scene3D::fixTrianglesOrientation(const common::Triangle &tria1, common::Triangle &tria2,
                                      const common::Edge &edge) const
{
    bool cw1 = 0;
    for (uint8_t i = 0; i < 3; ++i)
    {
        if (tria1.coord[i] == edge.coord[0])
        {
            cw1 = (tria1.coord[(i+1)%3] == edge.coord[1]);
            break;
        }
    }

    for (uint8_t i = 0; i < 3; ++i)
    {
        if (tria2.coord[i] == edge.coord[0])
        {
            bool cw2 = (tria2.coord[(i+1)%3] == edge.coord[1]);

            if (cw1 == cw2)
            {
                if (i == 0)
                {
                    tria2.coord[0] = tria2.coord[1];
                    tria2.coord[1] = edge.coord[0];
                }
                else if (i == 1)
                {
                    tria2.coord[1] = tria2.coord[2];
                    tria2.coord[2] = edge.coord[0];
                }
                else
                {
                    tria2.coord[2] = tria2.coord[0];
                    tria2.coord[0] = edge.coord[0];
                }
                return true;
            }
            return false;
        }
    }

    return true;
}

// start with first triangle as a pattern and set all other triangle orientations to it
void Scene3D::fixTrianglesOrientation()
{
    if (m_triangles.empty())
        return;

    uint32_t fixedNumber = 0;
    std::unordered_set<uint32_t> visited;
    std::vector<uint32_t> triangles;
    triangles.reserve(m_triangles.size());
    triangles.push_back(0);
    for (uint32_t i = 0; i < triangles.size(); ++i)
    {
        uint32_t iTri = triangles[i];
        const common::Triangle &tria = m_triangles[iTri];
        std::vector<uint32_t> neighbors;

        const uint32_t *edges = &m_triangleEdges[3*iTri];
        for (uint8_t j = 0; j < 3; ++j)
        {
            uint32_t iEdge = edges[j];
            const std::vector<uint32_t> &neighbors = m_edgeTriangles[iEdge];
            for (uint32_t k = 0; k < neighbors.size(); ++k)
            {
                uint32_t iNeighbor = neighbors[k];
                if (iNeighbor == iTri || visited.find(iNeighbor) != visited.end())
                    continue;
                if (fixTrianglesOrientation(tria, m_triangles[neighbors[k]], m_edges[iEdge]))
                    ++fixedNumber;
                triangles.push_back(iNeighbor);
                visited.insert(iNeighbor);
            }
        }
    }

    if (fixedNumber > 0)
        qDebug() << fixedNumber << " triangle orientations were fixed";
}

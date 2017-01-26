/* Copyright (C) 2016 Robert Osfield
 *
 * This application is open source is published under GNU GPL license.
*/

#include "ParametricNode.h"

#include <osg/Geometry>

#include <sstream>

using namespace osgParametric;


//////////////////////////////////////////////////////////////////////////////////////////////////
//
// ParametricNode
//
ParametricNode::ParametricNode()
{
    init();
}

ParametricNode::ParametricNode(const ParametricNode& ps,const osg::CopyOp& copyop)
{
    init();
}

ParametricNode::~ParametricNode()
{
}

void ParametricNode::setDimensions(unsigned int w, unsigned int h)
{
    _parametricScene->setDimensions(w, h);
}

void ParametricNode::getDimensions(unsigned int & w, unsigned int & h) const
{
    w = _parametricScene->getWidth();
    h = _parametricScene->getHeight();
}

void ParametricNode::addSubgraphs(NodeVec const & subgraphs)
{
    for (auto subgraph : subgraphs) {
       addSubgraph(subgraph);
    }
}

void ParametricNode::addSubgraph(NodePtr const & subgraph)
{
    _subgraphs.push_back(subgraph);
}

void ParametricNode::init()
{
    _parametricScene = new ParametricScene;
    addChild(_parametricScene);
}

static osg::ref_ptr<osg::Geometry> createMesh(const osg::Vec3& origin, const osg::Vec3& uAxis, const osg::Vec3& vAxis, unsigned int uCells, unsigned vCells, bool top)
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setUseVertexBufferObjects(true);

    unsigned int numVertices = (uCells+1)*(vCells+1);

    // set up vertices
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->reserve(numVertices);

    osg::Vec3 ua = uAxis; ua /= static_cast<float>(uCells);
    osg::Vec3 va = vAxis; va /= static_cast<float>(vCells);

    for(unsigned int r=0; r<=vCells; ++r)
    {
        for(unsigned int c=0; c<=uCells; ++c)
        {
            vertices->push_back(origin + ua*static_cast<float>(c) + va*static_cast<float>(r));
        }
    }
    geometry->setVertexArray(vertices);

    // set up normal
    osg::Vec3 verticalAxis(uAxis ^ vAxis);
    verticalAxis.normalize();

    geometry->getOrCreateStateSet()->addUniform(new osg::Uniform("verticalAxis", verticalAxis));

    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array();
    normals->push_back(osg::Vec3(0.0f,0.0f,0.0f));
    geometry->setNormalArray(normals, osg::Array::BIND_OVERALL);


    // set up colour
    osg::Vec4 color(1.0,1.0,1.0,1.0);
    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    colours->push_back(color);
    geometry->setColorArray(colours, osg::Array::BIND_OVERALL);


    // set up mesh

    OSG_NOTICE<<"numVertices = "<<numVertices<<std::endl;
    OSG_NOTICE<<"numVertices>>16 = "<<(numVertices>>16)<<std::endl;


    osg::ref_ptr<osg::DrawElements> primitives;
    if ((numVertices>>16)==0) primitives = new osg::DrawElementsUShort(GL_TRIANGLES);
    else primitives = new osg::DrawElementsUInt(GL_TRIANGLES);

    geometry->addPrimitiveSet(primitives);

    for(unsigned int r=0; r<vCells; ++r)
    {
        for(unsigned int c=0; c<uCells; ++c)
        {
            unsigned int p0 = c+r*(uCells+1);
            unsigned int p1 = p0+(uCells+1);
            unsigned int p2 = p0+1;
            unsigned int p3 = p1+1;
            if (top)
            {
                primitives->addElement(p0);
                primitives->addElement(p2);
                primitives->addElement(p1);
                primitives->addElement(p2);
                primitives->addElement(p3);
                primitives->addElement(p1);
            }
            else
            {
                primitives->addElement(p0);
                primitives->addElement(p1);
                primitives->addElement(p2);
                primitives->addElement(p2);
                primitives->addElement(p1);
                primitives->addElement(p3);
            }
        }
    }

    osg::Vec3 wAxis = verticalAxis*((uAxis.length()+vAxis.length())*0.5);

    osg::BoundingBox bb;
    bb.expandBy(origin);
    bb.expandBy(origin+uAxis);
    bb.expandBy(origin+vAxis);
    bb.expandBy(origin+uAxis+vAxis);

    bb.expandBy(origin+wAxis);
    bb.expandBy(origin+uAxis+wAxis);
    bb.expandBy(origin+vAxis+wAxis);
    bb.expandBy(origin+uAxis+vAxis+wAxis);

    geometry->setInitialBound(bb);

    return geometry;
}

static osg::ref_ptr<osg::Geometry> createSideWalls(const osg::Vec3& baseOrigin, const osg::Vec3& topOrigin, const osg::Vec3& uAxis, const osg::Vec3& vAxis, int uCells, int vCells)
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setUseVertexBufferObjects(true);

    int numVertices = 2*(uCells+1) + 2*(vCells+1);


    // set up vertices
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->reserve(numVertices);

    // normals
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array();
    geometry->setNormalArray(normals, osg::Array::BIND_PER_PRIMITIVE_SET);


    osg::Vec3 ua = uAxis; ua /= static_cast<float>(uCells);
    osg::Vec3 va = vAxis; va /= static_cast<float>(vCells);

    int vn = vertices->size();
    int c=0;
    int r=0;
    for(r=0; r<=vCells; ++r)
    {
        vertices->push_back(baseOrigin + ua*static_cast<float>(c) + va*static_cast<float>(r));
        vertices->push_back(topOrigin + ua*static_cast<float>(c) + va*static_cast<float>(r));
    }
    normals->push_back(osg::Vec3(-1.0f,0.0f,0.0f));
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, vn, vertices->size()-vn));

    vn = vertices->size();
    r = vCells;
    for(c=0; c<=uCells; ++c)
    {
        vertices->push_back(baseOrigin + ua*static_cast<float>(c) + va*static_cast<float>(r));
        vertices->push_back(topOrigin + ua*static_cast<float>(c) + va*static_cast<float>(r));
    }
    normals->push_back(osg::Vec3(0.0f,1.0f,0.0f));
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, vn, vertices->size()-vn));

    vn = vertices->size();
    c = uCells;
    for(r=vCells; r>=0; --r)
    {
        vertices->push_back(baseOrigin + ua*static_cast<float>(c) + va*static_cast<float>(r));
        vertices->push_back(topOrigin + ua*static_cast<float>(c) + va*static_cast<float>(r));
    }
    normals->push_back(osg::Vec3(1.0f,0.0f,0.0f));
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, vn, vertices->size()-vn));

    vn = vertices->size();
    r = 0;
    for(c=uCells; c>=0; --c)
    {
        vertices->push_back(baseOrigin + ua*static_cast<float>(c) + va*static_cast<float>(r));
        vertices->push_back(topOrigin + ua*static_cast<float>(c) + va*static_cast<float>(r));
    }
    normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, vn, vertices->size()-vn));

    geometry->setVertexArray(vertices);

    // set up normal
    osg::Vec3 verticalAxis(uAxis ^ vAxis);
    verticalAxis.normalize();

    geometry->getOrCreateStateSet()->addUniform(new osg::Uniform("verticalAxis", verticalAxis));

    // set up colour
    osg::Vec4 color(1.0,1.0,1.0,1.0);
    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    colours->push_back(color);
    geometry->setColorArray(colours, osg::Array::BIND_OVERALL);


    // set up mesh

    OSG_NOTICE<<"numVertices = "<<numVertices<<std::endl;
    OSG_NOTICE<<"numVertices>>16 = "<<(numVertices>>16)<<std::endl;

    osg::Vec3 wAxis = verticalAxis*((uAxis.length()+vAxis.length())*0.5);

    osg::BoundingBox bb;
    bb.expandBy(baseOrigin);
    bb.expandBy(baseOrigin+uAxis);
    bb.expandBy(baseOrigin+vAxis);
    bb.expandBy(baseOrigin+uAxis+vAxis);

    bb.expandBy(baseOrigin+wAxis);
    bb.expandBy(baseOrigin+uAxis+wAxis);
    bb.expandBy(baseOrigin+vAxis+wAxis);
    bb.expandBy(baseOrigin+uAxis+vAxis+wAxis);

    bb.expandBy(baseOrigin-wAxis);
    bb.expandBy(baseOrigin+uAxis-wAxis);
    bb.expandBy(baseOrigin+vAxis-wAxis);
    bb.expandBy(baseOrigin+uAxis+vAxis-wAxis);

    geometry->setInitialBound(bb);

    return geometry;
}

void ParametricNode::setup()
{
    // From original example createParametric()
    osg::ref_ptr<osg::Group> parametric_group = new osg::Group;
    parametric_group->setName("ParametricGroup");

    osg::Vec3 origin(0.0, 0.0, 0.0);
    osg::Vec3 uAxis(1.0,0.0,0.0);
    osg::Vec3 vAxis(0.0,1.0,0.0);

    osg::Vec3 baseOrigin = origin;
    osg::Vec3 topOrigin = baseOrigin+osg::Vec3(0.0,0.0,1.0);

    // base
    if (renderBase)
    {
        osg::ref_ptr<osg::Geometry> geometry = createMesh(baseOrigin, uAxis, vAxis, uCells, vCells, false);
        parametric_group->addChild(geometry.get());
    }

    // top
    if (renderTop)
    {
        osg::ref_ptr<osg::Geometry> geometry = createMesh(topOrigin, uAxis, vAxis, uCells, vCells, true);
        parametric_group->addChild(geometry.get());
    }

    // sidewalls
    if (renderSidewalls)
    {
        osg::ref_ptr<osg::Geometry> geometry = createSideWalls(baseOrigin, topOrigin, uAxis, vAxis, uCells, vCells);
        parametric_group->addChild(geometry.get());
    }

    std::string function;
    if (!zFunction.empty()) { parametric_group->getOrCreateStateSet()->setDefine("Z_FUNCTION", zFunction); }
    if (!zBase.empty()) { parametric_group->getOrCreateStateSet()->setDefine("Z_BASE", zBase); }
    if (!zTop.empty()) { parametric_group->getOrCreateStateSet()->setDefine("Z_TOP", zTop); }


    _parametricScene->addSubgraph(parametric_group, true, true);

    _parametricScene->setup();
}

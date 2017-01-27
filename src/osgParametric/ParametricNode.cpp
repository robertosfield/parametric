/* Copyright (C) 2016 Robert Osfield
 *
 * This application is open source is published under GNU GPL license.
*/

#include "ParametricNode.h"

#include <osg/Geometry>
#include <osg/ShapeDrawable> // TEMP
#include <osgDB/ReadFile>

#include <sstream>

using namespace osgParametric;

int
replaceAll( std::string       & source,
            std::string const & oldPart,
            std::string const & newPart )
{
  std::size_t oldSize = oldPart.size();
  std::size_t newSize = newPart.size();
  std::size_t pos     = 0;
  int         num     = 0;

  while ( pos < source.size() ) { // cannot save source size because it may change
    pos = source.find( oldPart, pos );

    if ( pos != std::string::npos ) {
      source.replace( pos, oldSize, newPart );
      pos += newSize;
      ++num;
    }
  }
  return num;
} // replaceAll



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
    _width = w;
    _height = h;
}

void ParametricNode::setZFunction(std::string const & zFunction)
{
    if (zFunction == _zFunction) {
      return;
    }
    _zFunction = zFunction;
    updateShaders();
}

void ParametricNode::setZBase(std::string const & zBase)
{
    if (zBase == _zBase) {
      return;
    }
    _zBase = zBase;
    updateShaders();
}

void ParametricNode::setZTop(std::string const & zTop)
{
    if (zTop == _zTop) {
      return;
    }
    _zTop = zTop;
    updateShaders();
}

void ParametricNode::addShaders(StringVec const & shadersFilenames)
{
    for (auto shaderFilename : shadersFilenames) {
      ShaderPtr shader = osgDB::readRefShaderFile(shaderFilename);
      if (shader.valid()) {
        _shaderSources[shader->getType()] = shader->getShaderSource();
      }
    }

    updateShaders();
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
    osg::StateSet *stateSet = getOrCreateStateSet();

    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
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
    if (!_zFunction.empty()) { parametric_group->getOrCreateStateSet()->setDefine("Z_FUNCTION", _zFunction); }
    if (!_zBase.empty()) { parametric_group->getOrCreateStateSet()->setDefine("Z_BASE", _zBase); }
    if (!_zTop.empty()) { parametric_group->getOrCreateStateSet()->setDefine("Z_TOP", _zTop); }

    removeChildren(0, getNumChildren());

    _parametricScene = new ParametricScene;
    _parametricScene->setDimensions(_width, _height);
    updateShaders();

    _parametricScene->addSubgraph(parametric_group, true, true);

    for (auto subgraph : _subgraphs) {
//      _parametricScene->addSubgraph(new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(0.5,0.5,0.5), 0.4, 20.0)), visibleBoundaries, depthBoundaries);
      _parametricScene->addSubgraph(subgraph, visibleBoundaries, depthBoundaries);
    }

    _parametricScene->setup();
    addChild(_parametricScene);

}

void ParametricNode::updateShaders()
{
    if (!_parametricScene.valid()) {
      return;
    }

    osg::ref_ptr<osg::Program> program = new osg::Program;

    // Add all the shaders, replacing the function strings found
    for (ShaderSourceMap::const_iterator ssmit = _shaderSources.begin();
         ssmit != _shaderSources.end(); ++ssmit) {
       auto type = ssmit->first;
       std::string source = ssmit->second;

       if (!_zFunction.empty()) {
         replaceAll(source, "INSERT_ZFUNCTION", _zFunction);
       }
       if (!_zBase.empty()) {
         replaceAll(source, "INSERT_ZBASE", _zBase);
       }
       if (!_zTop.empty()) {
         replaceAll(source, "INSERT_ZTOP", _zTop);
       }

       if (!source.empty()) {
         program->addShader(new osg::Shader(type, source));
       }
    }

    _parametricScene->getOrCreateStateSet()->setAttribute(program);
}

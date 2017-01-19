/* Copyright (C) 2016 Robert Osfield
 *
 * This application is open source is published under GNU GPL license.
*/

#include <osg/ShapeDrawable>
#include <osg/CullFace>
#include <osg/Depth>

#include <osgGA/StateSetManipulator>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include "ParametricScene.h"


osg::ref_ptr<osg::Geometry> createMesh(const osg::Vec3& origin, const osg::Vec3& uAxis, const osg::Vec3& vAxis, unsigned int uCells, unsigned vCells, bool top)
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

osg::ref_ptr<osg::Geometry> createSideWalls(const osg::Vec3& baseOrigin, const osg::Vec3& topOrigin, const osg::Vec3& uAxis, const osg::Vec3& vAxis, unsigned int uCells, unsigned vCells)
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setUseVertexBufferObjects(true);

    unsigned int numVertices = 2*(uCells+1) + 2*(vCells+1);


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

osg::ref_ptr<osg::Program> createProgram(osg::ArgumentParser& arguments)
{
    osg::ref_ptr<osg::Program> program = new osg::Program;

    std::string filename;
    typedef std::map<osg::Shader::Type, osg::ref_ptr<osg::Shader> > ShaderMap;
    ShaderMap shaders;

    while(arguments.read("--shader",filename))
    {
        osg::ref_ptr<osg::Shader> shader = osgDB::readRefShaderFile(filename);
        if (shader.valid())
        {
            shaders[shader->getType()] = shader;
        }
    }

    for(ShaderMap::iterator itr = shaders.begin();
        itr != shaders.end();
        ++itr)
    {
        program->addShader(itr->second);
    }

    return program;
}

osg::ref_ptr<osg::Group> createParametric(osg::ArgumentParser& arguments)
{
    osg::ref_ptr<osg::Group> parametric_group = new osg::Group;
    parametric_group->setName("ParametricGroup");

    osg::Vec3 origin(0.0, 0.0, 0.0);
    osg::Vec3 uAxis(1.0,0.0,0.0);
    osg::Vec3 vAxis(0.0,1.0,0.0);

    unsigned int uCells = 10;
    unsigned int vCells = 10;

    while (arguments.read("--columns", uCells)) {}
    while (arguments.read("--rows", vCells)) {}

    osg::Vec3 baseOrigin = origin;
    osg::Vec3 topOrigin = baseOrigin+osg::Vec3(0.0,0.0,1.0);

    bool renderBase = false;
    bool renderTop = true;
    bool renderSidewalls = false;

    while (arguments.read("--base")) renderBase=true;
    while (arguments.read("--top")) renderTop=true;
    while (arguments.read("--walls")) renderSidewalls=true;
    while (arguments.read("--all")) { renderBase = true; renderTop = true; renderSidewalls = true; }

    std::string function;
    while(arguments.read("--Z_FUNCTION", function)) { parametric_group->getOrCreateStateSet()->setDefine("Z_FUNCTION", function); }
    while(arguments.read("--Z_BASE", function)) { parametric_group->getOrCreateStateSet()->setDefine("Z_BASE", function); }
    while(arguments.read("--Z_TOP", function)) { parametric_group->getOrCreateStateSet()->setDefine("Z_TOP", function); }

    std::string name;
    float value;
    while(arguments.read("--uniform",name,value))
    {
        parametric_group->getOrCreateStateSet()->addUniform(new osg::Uniform(name.c_str(), value));
    }


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

    return parametric_group;
}

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    viewer.addEventHandler(new osgViewer::StatsHandler());
    viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

    viewer.realize();

    osgViewer::ViewerBase::Windows windows;
    viewer.getWindows(windows);
    if (windows.empty())
    {
        OSG_NOTICE<<"Warning: no windows created"<<std::endl;
        return 0;
    }


    osg::ref_ptr<osgParametric::ParametricScene> ps = new osgParametric::ParametricScene;

    // provide the ParametricScene node with the dimensions of the window so it can correctly size the textures
    const osg::GraphicsContext::Traits* traits = windows.front()->getTraits();
    ps->setDimensions(traits->width, traits->height);

    // aset up the shaders to do the parametric surface placement and depth textures
    ps->getOrCreateStateSet()->setAttribute(createProgram(arguments));


    // assign the parametric surface
    ps->addSubgraph(createParametric(arguments), true, true);

    bool visibleBoundaries = false;
    while(arguments.read("-b")) visibleBoundaries = true;

    bool depthBoundaries = false;
    while(arguments.read("-d")) depthBoundaries = true;

    osg::Vec3 center;
    osg::Vec3 dimensions;
    while(arguments.read("--sphere", center.x(), center.y(), center.z(), dimensions.x()))
    {
        ps->addSubgraph(new osg::ShapeDrawable(new osg::Sphere(center, dimensions.x())), visibleBoundaries, depthBoundaries);
    }

    while(arguments.read("--box", center.x(), center.y(), center.z(), dimensions.x(), dimensions.y(), dimensions.z()))
    {
        ps->addSubgraph(new osg::ShapeDrawable(new osg::Box(center, dimensions.x(), dimensions.y(), dimensions.z())), visibleBoundaries, depthBoundaries);
    }

    while(arguments.read("--cone", center.x(), center.y(), center.z(), dimensions.x(), dimensions.y()))
    {
        ps->addSubgraph(new osg::ShapeDrawable(new osg::Cone(center, dimensions.x(), dimensions.y())), visibleBoundaries, depthBoundaries);
    }

    while(arguments.read("--capsule", center.x(), center.y(), center.z(), dimensions.x(), dimensions.y()))
    {
        ps->addSubgraph(new osg::ShapeDrawable(new osg::Capsule(center, dimensions.x(), dimensions.y())), visibleBoundaries, depthBoundaries);
    }

    while(arguments.read("--cylinder", center.x(), center.y(), center.z(), dimensions.x(), dimensions.y()))
    {
        ps->addSubgraph(new osg::ShapeDrawable(new osg::Cylinder(center, dimensions.x(), dimensions.y())), visibleBoundaries, depthBoundaries);
    }

    std::string modelFilename;
    while(arguments.read("--model", modelFilename))
    {
        osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile(modelFilename);
        if (model) ps->addSubgraph(model, visibleBoundaries, depthBoundaries);
    }


    // create the subgraphs that will do all the rendering
    ps->setup();


    viewer.setSceneData( ps.get() );

    std::string filename;
    if (arguments.read("-o",filename))
    {
        osgDB::writeNodeFile(*(viewer.getSceneData()), filename);
        return 1;
    }

    return viewer.run();
}

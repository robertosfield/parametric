/* Copyright (C) 2016 Robert Osfield
 *
 * This application is open source is published under GNU GPL license.
*/

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>
#include <osg/CoordinateSystemNode>

#include <osg/Switch>
#include <osg/Types>
#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>

#include <osgGA/Device>

#include <iostream>


osg::ref_ptr<osg::Geometry> createMesh(const osg::Vec3& origin, const osg::Vec3& uAxis, const osg::Vec3& vAxis, unsigned int uCells, unsigned vCells)
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
    osg::Vec3 normal(uAxis ^ vAxis);
    normal.normalize();

    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array();
    normals->push_back(normal);
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
            primitives->addElement(p0);
            primitives->addElement(p2);
            primitives->addElement(p1);
            primitives->addElement(p2);
            primitives->addElement(p3);
            primitives->addElement(p1);
        }
    }

    osg::Vec3 wAxis = normal*((uAxis.length()+vAxis.length())*0.5);

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

void addShaders(osg::ArgumentParser& arguments, osg::StateSet* stateset)
{
    std::string z_function;
    while(arguments.read("--Z_FUNCTION",z_function)) { stateset->setDefine("Z_FUNCTION", z_function); }


    std::string visible_function;
    while(arguments.read("--VISIBLE_FUNCTION",visible_function)) { stateset->setDefine("VISIBLE_FUNCTION", visible_function); }


    float cutOff = 0.001;
    while(arguments.read("--cutOff", cutOff)) {}
    stateset->addUniform(new osg::Uniform("cutOff",cutOff));

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

    stateset->setAttribute(program);
}

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    viewer.addEventHandler(new osgViewer::StatsHandler());
    viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));


    osg::Vec3 origin(0.0, 0.0, 0.0);
    osg::Vec3 uAxis(1.0,0.0,0.0);
    osg::Vec3 vAxis(0.0,1.0,0.0);

    unsigned int uCells = 10;
    unsigned int vCells = 10;


    while (arguments.read("--columns", uCells)) {}
    while (arguments.read("--rows", vCells)) {}


    osg::ref_ptr<osg::Geometry> geometry = createMesh(origin, uAxis, vAxis, uCells, vCells);


    addShaders(arguments, geometry->getOrCreateStateSet());



    viewer.setSceneData(geometry.get());

    std::string filename;
    if (arguments.read("-o",filename))
    {
        osgDB::writeNodeFile(*(viewer.getSceneData()), filename);
        return 1;
    }

    return viewer.run();
}

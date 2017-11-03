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

#include <osgParametric/ParametricNode.h>

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

void setupParametric(osg::ArgumentParser& arguments, osgParametric::ParametricNode & pn)
{
    while (arguments.read("--columns", pn.uCells)) {}
    while (arguments.read("--rows", pn.vCells)) {}

    while (arguments.read("--base")) pn.renderBase=true;
    while (arguments.read("--top")) pn.renderTop=true;
    while (arguments.read("--walls")) pn.renderSidewalls=true;
    while (arguments.read("--all")) { pn.renderBase = true; pn.renderTop = true; pn.renderSidewalls = true; }

    while(arguments.read("--Z_FUNCTION", pn.zFunction));
    while(arguments.read("--Z_BASE", pn.zBase));
    while(arguments.read("--Z_TOP", pn.zTop));

    std::string name;
    float value;
    while(arguments.read("--uniform",name,value))
    {
        pn.getOrCreateStateSet()->addUniform(new osg::Uniform(name.c_str(), value));
    }
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


    osg::ref_ptr<osgParametric::ParametricNode> ps = new osgParametric::ParametricNode;

    // provide the ParametricScene node with the dimensions of the window so it can correctly size the textures
    const osg::GraphicsContext::Traits* traits = windows.front()->getTraits();
    ps->setDimensions(traits->width, traits->height);

    // aset up the shaders to do the parametric surface placement and depth textures
    ps->getOrCreateStateSet()->setAttribute(createProgram(arguments));


    // assign the parametric surface
    setupParametric(arguments, *ps);

    while(arguments.read("-b")) ps->visibleBoundaries = true;

    bool depthBoundaries = false;
    while(arguments.read("-d")) ps->depthBoundaries = true;

    osg::Vec3 center;
    osg::Vec3 dimensions;
    while(arguments.read("--sphere", center.x(), center.y(), center.z(), dimensions.x()))
    {
        ps->addSubgraph(new osg::ShapeDrawable(new osg::Sphere(center, dimensions.x())));
    }

    while(arguments.read("--box", center.x(), center.y(), center.z(), dimensions.x(), dimensions.y(), dimensions.z()))
    {
        ps->addSubgraph(new osg::ShapeDrawable(new osg::Box(center, dimensions.x(), dimensions.y(), dimensions.z())));
    }

    while(arguments.read("--cone", center.x(), center.y(), center.z(), dimensions.x(), dimensions.y()))
    {
        ps->addSubgraph(new osg::ShapeDrawable(new osg::Cone(center, dimensions.x(), dimensions.y())));
    }

    while(arguments.read("--capsule", center.x(), center.y(), center.z(), dimensions.x(), dimensions.y()))
    {
        ps->addSubgraph(new osg::ShapeDrawable(new osg::Capsule(center, dimensions.x(), dimensions.y())));
    }

    while(arguments.read("--cylinder", center.x(), center.y(), center.z(), dimensions.x(), dimensions.y()))
    {
        ps->addSubgraph(new osg::ShapeDrawable(new osg::Cylinder(center, dimensions.x(), dimensions.y())));
    }

    std::string modelFilename;
    while(arguments.read("--model", modelFilename))
    {
        osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile(modelFilename);
        if (model) ps->addSubgraph(model);
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

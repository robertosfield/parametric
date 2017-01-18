/* Copyright (C) 2016 Robert Osfield
 *
 * This application is open source is published under GNU GPL license.
*/

#include "ParametricScene.h"

#include <osgUtil/CullVisitor>

#include <sstream>

using namespace osgParametric;

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Callback implementations
//
void RTTCameraCullCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);

    osg::UserDataContainer* udc = cv->getOrCreateUserDataContainer();
    unsigned int i = udc->getUserObjectIndex("ProjectionMatrix");
    osg::RefMatrix* pm = (i<udc->getNumUserObjects()) ? dynamic_cast<osg::RefMatrix*>(udc->getUserObject(i)) : 0;

    if (pm) cv->pushProjectionMatrix( pm );

    traverse(node, nv);

    if (pm) cv->popProjectionMatrix();
}

void NearFarCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);

    osg::ref_ptr<osg::RefMatrix> pm = cv->getProjectionMatrix();
    pm->setName("ProjectionMatrix");

    osg::UserDataContainer* udc = cv->getOrCreateUserDataContainer();
    udc->addUserObject(pm.get());

    traverse(node, nv);

    cv->updateCalculatedNearFar(*(cv->getModelViewMatrix()), _bb);

    // remove the matrix
    unsigned int i = udc->getUserObjectIndex(pm.get());
    udc->removeUserObject(i);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// ParametricScene::Subgraph
//
ParametricScene::Subgraph::Subgraph(parameter_ptr<osg::Node> sg, bool rrs, bool rds):
    subgraph(sg.get()),
    requiresRenderSubgraph(rrs),
    requiresDepthSubgraph(rds)
{
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//
// ParametricScene
//
ParametricScene::ParametricScene()
{
    init();
}

ParametricScene::ParametricScene(const ParametricScene& ps,const osg::CopyOp& copyop)
{
    init();
}

ParametricScene::~ParametricScene()
{
}

void ParametricScene::init()
{
    _width = 1280;
    _height = 1024;

    _renderSubgraph = new osg::Group;
    _renderSubgraph->setName("RenderSubgraph");
    addChild(_renderSubgraph.get());

    _depthSubgraph = new osg::Group;
    _depthSubgraph->setName("DepthSubgraph");
    addChild(_depthSubgraph.get());
}

void ParametricScene::addSubgraph(parameter_ptr<osg::Node> subgraph, bool requiresRenderSubgraph, bool requiresDepthSubgraph)
{
    _subgraphs.push_back(new Subgraph(subgraph, requiresRenderSubgraph, requiresDepthSubgraph));
}

void ParametricScene::setup()
{
    setupDepthSubgraphs();
    setupRenderSubgraphs();

    osg::BoundingBox bb;
    for(Subgraphs::iterator itr = _subgraphs.begin();
        itr != _subgraphs.end();
        ++itr)
    {
        Subgraph* sg = itr->get();
        if (sg->subgraph) bb.expandBy(sg->subgraph->getBound());
    }

    setCullCallback(new osgParametric::NearFarCallback(bb));
}

void ParametricScene::setupRenderSubgraphs()
{
    _renderSubgraph->removeChildren(0, _renderSubgraph->getNumChildren());

    setUpDepthStateSet(_renderSubgraph->getOrCreateStateSet(), _width, _height);

    for(Subgraphs::iterator itr = _subgraphs.begin();
        itr != _subgraphs.end();
        ++itr)
    {
        Subgraph* sg = itr->get();
        if (sg->requiresRenderSubgraph)
        {
            if ((*itr)->subgraph)
            {
                _renderSubgraph->addChild((*itr)->subgraph);
            }
        }
    }
}

void ParametricScene::setupDepthSubgraphs()
{
    _depthSubgraph->removeChildren(0, _depthSubgraph->getNumChildren());

    for(Subgraphs::iterator itr = _subgraphs.begin();
        itr != _subgraphs.end();
        ++itr)
    {
        Subgraph* sg = itr->get();
        if (sg->requiresDepthSubgraph)
        {
            osg::ref_ptr<osg::Node> boundarySubgraph = (*itr)->subgraph;

            // set up the depth texture for front face of the boundary
            osg::ref_ptr<osg::Texture2D> frontDepthTexture = createDepthTexture(_width, _height);
            osg::ref_ptr<osg::Camera> frontDepthCamera = createDepthCamera(frontDepthTexture, false);
            frontDepthCamera->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::ON);
            frontDepthCamera->addChild(boundarySubgraph);

            _depthSubgraph->addChild(frontDepthCamera.get());

            sg->frontTexture = frontDepthTexture;

            // set up the depth texture for back face of the boundary
            osg::ref_ptr<osg::Texture2D> backDepthTexture = createDepthTexture(_width, _height);
            osg::ref_ptr<osg::Camera> backDepthCamera = createDepthCamera(backDepthTexture, true);
            backDepthCamera->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT), osg::StateAttribute::ON);
            backDepthCamera->addChild(boundarySubgraph);

            _depthSubgraph->addChild(backDepthCamera.get());

            sg->backTexture = backDepthTexture;
        }
    }
}


osg::ref_ptr<osg::Texture2D> ParametricScene::createDepthTexture(unsigned int width, unsigned int height)
{
    osg::ref_ptr<osg::Texture2D> depthTexture = new osg::Texture2D;
    depthTexture->setTextureSize(width, height);
    depthTexture->setInternalFormat(GL_DEPTH_COMPONENT);
    depthTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    depthTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    depthTexture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
    depthTexture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
    depthTexture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    return depthTexture;
}

osg::ref_ptr<osg::Camera> ParametricScene::createDepthCamera(parameter_ptr<osg::Texture> depthTexture, bool backFace)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->attach(osg::Camera::DEPTH_BUFFER, depthTexture.get());
    camera->setViewport(0, 0, depthTexture->getTextureWidth(), depthTexture->getTextureHeight());

    // clear the depth and colour bufferson each clear.
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // set the camera to render before the main camera.
    camera->setRenderOrder(osg::Camera::PRE_RENDER);

    // tell the camera to use OpenGL frame buffer object where supported.
    camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

    camera->setReferenceFrame(osg::Transform::RELATIVE_RF);
    camera->setProjectionMatrix(osg::Matrixd::identity());
    camera->setViewMatrix(osg::Matrixd::identity());

    camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

    camera->setCullCallback(new osgParametric::RTTCameraCullCallback());

    if (backFace)
    {
        camera->getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::GREATER));
        camera->setClearDepth(0.0);
    }
    else
    {
//        camera->getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::LESS));
//        camera->setClearDepth(1.0);
    }

    return camera;
}

void ParametricScene::setUpDepthStateSet(osg::StateSet* stateset, unsigned int width, unsigned int height)
{
    Textures backFaceDepthTextures, frontFaceDepthTextures;

    for(Subgraphs::iterator itr = _subgraphs.begin();
        itr != _subgraphs.end();
        ++itr)
    {
        Subgraph* sg = itr->get();
        if (sg->frontTexture) frontFaceDepthTextures.push_back(sg->frontTexture.get());
        if (sg->backTexture) backFaceDepthTextures.push_back(sg->backTexture.get());
    }


    std::stringstream name;
    int unit=0;
    unsigned int numDepthTextures = std::min(backFaceDepthTextures.size(), frontFaceDepthTextures.size());
    for(unsigned int i=0; i<numDepthTextures; ++i)
    {
        name<<"frontDepthTexture"<<i;
        stateset->setTextureAttribute(unit, frontFaceDepthTextures[i].get());
        stateset->addUniform(new osg::Uniform(name.str().c_str(), unit));

        name.str("");
        ++unit;

        name<<"backDepthTexture"<<i;
        stateset->setTextureAttribute(unit, backFaceDepthTextures[i].get());
        stateset->addUniform(new osg::Uniform(name.str().c_str(), unit));

        name.str("");
        ++unit;
    }

    name.str("");
    name<<numDepthTextures;
    stateset->setDefine("NUM_DEPTH_TEXTURES", name.str());

    stateset->addUniform(new osg::Uniform("viewportDimensions",osg::Vec4(0.0f,0.0f,static_cast<float>(width),static_cast<float>(height))));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Serializers for ParametricScene
//
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( ParametricScene,
                         new osgParametric::ParametricScene,
                         osgParametric::ParametricScene,
                         "osg::Object osg::Node osgParametric::ParametricScene osg::Group" )
{
    ADD_UINT_SERIALIZER( Width, 0 );
    ADD_UINT_SERIALIZER( Height, 0 );
//    ADD_OBJECT_SERIALIZER( Settings, osgParametric::Settings, NULL );
//    ADD_VECTOR_SERIALIZER( Roads, osgParametric::RoadNetwork::Roads, osgDB::BaseSerializer::RW_OBJECT, 0 );
}


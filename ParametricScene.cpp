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
ParametricScene::Subgraph::Subgraph(parameter_ptr<osg::Node> subgrah, bool rrs, bool rds):
    subgraph(subgraph),
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
}

ParametricScene::ParametricScene(const ParametricScene& ps,const osg::CopyOp& copyop)
{
}

ParametricScene::~ParametricScene()
{
}

void ParametricScene::addSubgraph(parameter_ptr<osg::Node> subgraph, bool requiresRenderSubgraph, bool requiresDepthSubgraph)
{
    _subgraphs.push_back(new Subgraph(subgraph, requiresRenderSubgraph, requiresDepthSubgraph));
}

void ParametricScene::setupRenderSubgraphs()
{
}

void ParametricScene::setupDepthSubgraphs()
{
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

void ParametricScene::setUpDepthStateSet(osg::StateSet* stateset, Textures& backFaceDepthTextures, Textures& frontFaceDepthTextures, unsigned int width, unsigned int height)
{
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

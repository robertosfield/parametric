/* Copyright (C) 2016 Robert Osfield
 *
 * This application is open source is published under GNU GPL license.
*/

#include <osg/CullFace>
#include <osg/Depth>
#include <osg/Texture2D>
#include <osg/Camera>


namespace osgParametric
{

template<typename T>
struct parameter_ptr
{
    typedef T element_type;
    element_type* _ptr;

    parameter_ptr():_ptr(0) {}

    parameter_ptr(element_type* ptr):_ptr(ptr) {}

    template<typename P>
    parameter_ptr(osg::ref_ptr<P>& rptr):_ptr(rptr.get()) {}

    T& operator*() const { return *_ptr; }
    T* operator->() const { return _ptr; }
    T* get() const { return _ptr; }

    bool operator!() const   { return _ptr==0; } // not required
    bool valid() const       { return _ptr!=0; }

};

class RTTCameraCullCallback : public osg::NodeCallback
{
    public:

        RTTCameraCullCallback() {}

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

    protected:

        virtual ~RTTCameraCullCallback() {}
};

class NearFarCallback : public osg::NodeCallback
{
    public:

        osg::BoundingBox _bb;

        NearFarCallback(const osg::BoundingBox& bb) : _bb(bb) {}

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

    protected:

        virtual ~NearFarCallback() {}
};

class ParametricScene : public osg::Group
{
public:

    ParametricScene();

    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
    ParametricScene(const ParametricScene& ps,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

    META_Node(osgParametric, ParametricScene);

    void addSubgraph(parameter_ptr<osg::Node> subgraph, bool requiresRenderSubgraph, bool requiresDepthSubgraph);

    void setupRenderSubgraphs();

    void setupDepthSubgraphs();

protected:

    virtual ~ParametricScene();

    osg::ref_ptr<osg::Texture2D> createDepthTexture(unsigned int width, unsigned int height);

    osg::ref_ptr<osg::Camera> createDepthCamera(parameter_ptr<osg::Texture> depthTexture, bool backFace);

    typedef std::vector< osg::ref_ptr<osg::Texture2D> > Textures;

    void setUpDepthStateSet(osg::StateSet* stateset, Textures& backFaceDepthTextures, Textures& frontFaceDepthTextures, unsigned int width, unsigned int height);


    struct Subgraph : public osg::Referenced
    {
        Subgraph(parameter_ptr<osg::Node> subgrah, bool rrs, bool rds);

        osg::ref_ptr<osg::Node>     subgraph;
        bool                        requiresRenderSubgraph;
        bool                        requiresDepthSubgraph;

        osg::ref_ptr<osg::Node>     renderSubgraph;
        osg::ref_ptr<osg::Node>     depthSugraph;
        osg::ref_ptr<osg::Texture>  frontTexture;
        osg::ref_ptr<osg::Texture>  backTexture;
    };

    typedef std::vector< osg::ref_ptr<Subgraph> > Subgraphs;
    Subgraphs _subgraphs;
};

}
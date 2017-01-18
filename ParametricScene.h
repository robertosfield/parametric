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

    template<typename P>
    parameter_ptr(const osg::ref_ptr<P>& rptr):_ptr(rptr.get()) {}

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

    void setDimensions(unsigned int w, unsigned int h) { _width = w; _height = h; }

    void setWidth(unsigned int w) { _width = w; }
    unsigned int getWidth() const { return _width; }

    void setHeight(unsigned int h) { _height = h; }
    unsigned int getHeight() const { return _height; }

    void addSubgraph(parameter_ptr<osg::Node> subgraph, bool requiresRenderSubgraph, bool requiresDepthSubgraph);

    void setup();

protected:

    virtual ~ParametricScene();

    void init();

    osg::ref_ptr<osg::Texture2D> createDepthTexture(unsigned int width, unsigned int height);

    osg::ref_ptr<osg::Camera> createDepthCamera(parameter_ptr<osg::Texture> depthTexture, bool backFace);

    typedef std::vector< osg::ref_ptr<osg::Texture2D> > Textures;


    struct Subgraph : public osg::Referenced
    {
        Subgraph(parameter_ptr<osg::Node> sg, bool rrs, bool rds);

        osg::ref_ptr<osg::Node>         subgraph;

        bool                            requiresRenderSubgraph;
        bool                            requiresDepthSubgraph;

        osg::ref_ptr<osg::Texture2D>    frontTexture;
        osg::ref_ptr<osg::Texture2D>    backTexture;
    };

    typedef std::vector< osg::ref_ptr<Subgraph> > Subgraphs;

    void setupRenderStateSet(Subgraph* sgToExclude, osg::StateSet* stateset, unsigned int width, unsigned int height);

    void setupRenderSubgraphs();

    void setupDepthSubgraphs();

    unsigned int _width;
    unsigned int _height;

    Subgraphs _subgraphs;

    osg::ref_ptr<osg::Group> _renderSubgraph;
    osg::ref_ptr<osg::Group> _depthSubgraph;
};

}
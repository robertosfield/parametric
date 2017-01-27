/* Copyright (C) 2016 Robert Osfield
 *
 * This application is open source is published under GNU GPL license.
*/
#pragma once

#include <osgParametric/Export>
#include <osgParametric/ParametricScene.h>

#include <osg/Group>

#include <vector>

namespace osgParametric
{

using NodePtr = osg::ref_ptr<osg::Node>;
using NodeVec = std::vector<NodePtr>;

class OSGPARAMETRIC_EXPORT ParametricNode : public osg::Group
{
public:

    ParametricNode();

    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
    ParametricNode(const ParametricNode& ps,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

    META_Node(osgParametric, ParametricNode);

    void setDimensions(unsigned int w, unsigned int h);
    void getDimensions(unsigned int & w, unsigned int & h) const { w = _width; h = _height; }

    void setNumCells(osg::Vec2 const & numCells) { uCells = static_cast<unsigned int>(numCells[0]);
                                                   vCells = static_cast<unsigned int>(numCells[1]); }
    osg::Vec2 getNumCells() const { return osg::Vec2(static_cast<float>(uCells), static_cast<float>(vCells)); }
    unsigned int uCells    = 10;
    unsigned int vCells    = 10;

    bool renderBase        = false;
    bool renderTop         = true;
    bool renderSidewalls   = false;

    bool visibleBoundaries = false;
    bool depthBoundaries   = false;

    std::string zFunction;
    std::string zBase;
    std::string zTop;

    void addSubgraphs(NodeVec const & subgraphs);
    void addSubgraph(NodePtr const & subgraph);
    NodeVec const & getSubgraphs() const { return _subgraphs; }

    void setup();

protected:

    virtual ~ParametricNode();

    void init();

    osg::ref_ptr<ParametricScene> _parametricScene;

    unsigned int _width = 800;
    unsigned int _height = 600;
    NodeVec _subgraphs;
};

}
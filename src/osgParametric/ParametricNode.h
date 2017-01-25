/* Copyright (C) 2016 Robert Osfield
 *
 * This application is open source is published under GNU GPL license.
*/
#pragma once

#include <osgParametric/Export>
#include <osgParametric/ParametricScene.h>

#include <osg/Group>

namespace osgParametric
{

class OSGPARAMETRIC_EXPORT ParametricNode : public osg::Group
{
public:

    ParametricNode();

    /** Copy constructor using CopyOp to manage deep vs shallow copy. */
    ParametricNode(const ParametricNode& ps,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

    META_Node(osgParametric, ParametricNode);

    void setDimensions(unsigned int w, unsigned int h);
    void getDimensions(unsigned int & w, unsigned int & h) const;

    void setNumCells(unsigned int uc, unsigned int vc) { uCells = uc; vCells = vc; }
    void getNumCells(unsigned int & uc, unsigned int & vc) const { uc = uCells; vc = vCells; }
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

    void addSubgraph(osg::ref_ptr<osg::Node> const & subgraph);

    void setup();

protected:

    virtual ~ParametricNode();

    void init();

    osg::ref_ptr<ParametricScene> _parametricScene;

};

}
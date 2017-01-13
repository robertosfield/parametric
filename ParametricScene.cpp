/* Copyright (C) 2016 Robert Osfield
 *
 * This application is open source is published under GNU GPL license.
*/

#include "ParametricScene.h"

using namespace osgParametric;

ParametricScene::Subgraph::Subgraph(parameter_ptr<osg::Node> subgrah, bool rrs, bool rds):
    subgraph(subgraph),
    requiresRenderSubgraph(rrs),
    requiresDepthSubgraph(rds)
{
}


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

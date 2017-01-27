#include "ParametricNodeModule.hpp"
#include <igs/modules/osg/scenegraph/GroupModule.hpp>

#include <igs/modules/osg/OSGDefs.hpp>
#include <igs/modules/osg/SmartPtrOSGDefs.hpp>
#include <igs/modules/osg/SmartPtrInfoOSGDefs.hpp>
#include <igs/database/ModuleDevSource.hpp>

// ----------------------------------------------------------------------------
// ParametricNode Module definitions
// ----------------------------------------------------------------------------

namespace osgParametric {

sg::Vec2 getNumCells(ParametricNode const * pn)
{
  sg::Vec2 numCells;
  numCells[0] = static_cast<float>(pn->uCells);
  numCells[1] = static_cast<float>(pn->vCells);
  return numCells;
}

void setNumCells(ParametricNode * pn, sg::Vec2 const & numCells)
{
  pn->uCells = std::max(10u, std::min(1000u, static_cast<unsigned int>(numCells[0])));
  pn->vCells = std::max(10u, std::min(1000u, static_cast<unsigned int>(numCells[1])));
}

} // namespace osgParametric

namespace igs {

template <>
ClassInfo const &
classInfo<osgParametric::ParametricNode>()
{
  static ClassInfo info = ClassInfo().init<osgParametric::ParametricNode>()

      .addConstructor( constructor< osgParametric::ParametricNode, osg::ref_ptr<osgParametric::ParametricNode> >() )

      .addProperty(
          Property( "Dimensions", Property::Persistent,
              Method("getDimensions", &osgParametric::ParametricNode::getDimensions,
                  Parameter("width", 1000, Parameter::Out),
                  Parameter("height", 1000, Parameter::Out)
              ),
              Method( "setDimensions", &osgParametric::ParametricNode::setDimensions,
                  Parameter("width", 1000, Parameter::In),
                  Parameter("height", 1000, Parameter::In)
              ),
              Signal()
          )
      )

      .addProperty(
          Property( "NumCells", Property::Persistent,
              BoundFunction("getNumCells", &osgParametric::getNumCells,
                  Return("numCells", igs::math::vec<sg::Vec2>(25.0, 25.0), Return::Out)
              ),
              BoundFunction( "setNumCells", &osgParametric::setNumCells,
                  Parameter("numCells", igs::math::vec<sg::Vec2>(25.0, 25.0), Parameter::In)
              ),
              Signal()
          )
      )

      .addProperty(
          Property( "RenderBase", Property::Persistent,
              Member("getRenderBase", &osgParametric::ParametricNode::renderBase,
                  Return("renderBase", false, Return::Out)
              ),
              Member("setRenderBase", &osgParametric::ParametricNode::renderBase,
                  Parameter("renderBase", false, Parameter::In)
              ),
              Signal()
          )
      )

      .addProperty(
          Property( "RenderTop", Property::Persistent,
              Member("getRenderTop", &osgParametric::ParametricNode::renderTop,
                  Return("renderTop", true, Return::Out)
              ),
              Member("setRenderTop", &osgParametric::ParametricNode::renderTop,
                  Parameter("renderTop", true, Parameter::In)
              ),
              Signal()
          )
      )

      .addProperty(
          Property( "RenderSidewalls", Property::Persistent,
              Member("getRenderSidewalls", &osgParametric::ParametricNode::renderSidewalls,
                  Return("renderSidewalls", false, Return::Out)
              ),
              Member("setRenderSidewalls", &osgParametric::ParametricNode::renderSidewalls,
                  Parameter("renderSidewalls", false, Parameter::In)
              ),
              Signal()
          )
      )

      .addProperty(
          Property( "VisibleBoundaries", Property::Persistent,
              Member("getVisibleBoundaries", &osgParametric::ParametricNode::visibleBoundaries,
                  Return("visibleBoundaries", false, Return::Out)
              ),
              Member("setVisibleBoundaries", &osgParametric::ParametricNode::visibleBoundaries,
                  Parameter("visibleBoundaries", false, Parameter::In)
              ),
              Signal()
          )
      )

      .addProperty(
          Property( "DepthBoundaries", Property::Persistent,
              Member("getDepthBoundaries", &osgParametric::ParametricNode::depthBoundaries,
                  Return("depthBoundaries", false, Return::Out)
              ),
              Member("setDepthBoundaries", &osgParametric::ParametricNode::depthBoundaries,
                  Parameter("depthBoundaries", false, Parameter::In)
              ),
              Signal()
          )
      )

      .addProperty(
          Property( "ZFunction", Property::Persistent,
              Member("getZFunction", &osgParametric::ParametricNode::zFunction,
                  Return("zFunction", std::string(), Return::Out)
              ),
              Member("setZFunction", &osgParametric::ParametricNode::zFunction,
                  Parameter("zFunction", std::string(), Parameter::In)
              ),
              Signal()
          )
      )

      .addProperty(
          Property( "ZBase", Property::Persistent,
              Member("getZBase", &osgParametric::ParametricNode::zBase,
                  Return("zBase", std::string(), Return::Out)
              ),
              Member("setZBase", &osgParametric::ParametricNode::zBase,
                  Parameter("zBase", std::string(), Parameter::In)
              ),
              Signal()
          )
      )

      .addProperty(
          Property( "ZTop", Property::Persistent,
              Member("getZTope", &osgParametric::ParametricNode::zTop,
                  Return("zTop", std::string(), Return::Out)
              ),
              Member("setZTop", &osgParametric::ParametricNode::zTop,
                  Parameter("zTop", std::string(), Parameter::In)
              ),
              Signal()
          )
      )

      .addProperty(
          Property( "Subgraphs", Property::Persistent,
              Method("getSubgraphs", &osgParametric::ParametricNode::getSubgraphs,
                  Return("subgraphs", osgParametric::NodeVec(), Return::Out)
              ),
              Method("addSubgraphs", &osgParametric::ParametricNode::addSubgraphs,
                  Parameter("subgraphs", osgParametric::NodeVec(), Parameter::In)
              ),
              Signal()
          )
      )

      .addProperty(
          Property( "Setup", Property::Persistent,
              Operation(),
              Method("setup", &osgParametric::ParametricNode::setup),
              Signal()
          )
      )
  ;
  return info;
} // classInfo<osgParametric::ParametricNode>

} // namespace igs

DEFINE_MODULE_CLASS( osgParametric::ParametricNode, sg, ParametricNode );

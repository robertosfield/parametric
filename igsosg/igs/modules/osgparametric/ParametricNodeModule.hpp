#pragma once

#include <igs/modules/osgparametric/osgParametricModulesAPI.h>

#include <igs/modules/osg/scenegraph/GroupModule.hpp>
#include <osgParametric/ParametricNode.h>

#include <igs/database/ModuleDevHeader.hpp>

// ----------------------------------------------------
// ParametricNode Module
// ----------------------------------------------------

DECLARE_MODULE_CLASS(osgParametric::ParametricNode, sg, ParametricNode, OSGPARAMETRICMODULES_API, osg::Group);
TYPETRAITS(osgParametric::ParametricNode, true, false, false, false);



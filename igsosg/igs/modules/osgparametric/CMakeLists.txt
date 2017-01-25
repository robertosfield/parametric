# -------------------------------
# Set up the osgParametricModules
# -------------------------------

igs_version(
    osgParametricModules
    ModulePackage
    osgparametricmodules
    OSGPARAMETRICMODULES_VERSION_SOURCES
    GLOBAL
    OSGPARAMETRICMODULES_API
    igs/modules/osgparametric
    osgParametricModulesAPI.h
)

igs_build_target(
    BASENAME osgparametricmodules
    TYPE     Library
    GROUP    OpenSceneGraph
    COMPILE_FLAGS
        ${IGS_COMPILEFLAGS}
    DEFINES
    INCLUDE_DIRS
         ${CMAKE_SOURCE_DIR}
         ${CMAKE_SOURCE_DIR}/igsosg
    LIBRARIES
        osgParametric
        optimized igsosgmodules debug igsosgmodules${CMAKE_DEBUG_POSTFIX}
        optimized igsosg        debug igsosg${CMAKE_DEBUG_POSTFIX}
        ${OPENIGS_LIBRARIES}
        ${OPENSCENEGRAPH_LIBRARIES}
    HEADERS_PUBLIC
        osgParametricModulesAPI.h
        ParametricNodeModule.hpp
    SOURCES
        ParametricNodeModule.cpp
    FLAT_SOURCE_TREE
    HEADER_INSTALL_DIR include/igs/modules/osgparametric
)

install(
    FILES
        ${CMAKE_SOURCE_DIR}/src/osgParametric/parametric.vert
        ${CMAKE_SOURCE_DIR}/src/osgParametric/parametric3.vert
        ${CMAKE_SOURCE_DIR}/src/osgParametric/parametric.frag
        ${CMAKE_SOURCE_DIR}/README.md
        osgparametric.igsml
    DESTINATION
       share/openigs/examples/osg/parametric
)

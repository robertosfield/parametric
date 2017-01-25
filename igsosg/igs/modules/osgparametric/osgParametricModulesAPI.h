#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** @cond */
/**
 * @defgroup osg_parametric Modules : OSGParametric
 */
/** @endcond */

/**
 * @file osgParametricModulesAPI.h
 * @brief Linkage definitions for OSG Parametric Module API
 *
 */

/* Definitions for exporting or importing the OSG Parametric Modules API */
#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#  if defined osgparametricmodules_STATIC
#    define OSGPARAMETRICMODULES_API
#  elif defined osgparametricmodules_EXPORTS
#    define OSGPARAMETRICMODULES_API __declspec( dllexport )
#  else
#    define OSGPARAMETRICMODULES_API __declspec( dllimport )
#  endif
#else
#  define OSGPARAMETRICMODULES_API
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

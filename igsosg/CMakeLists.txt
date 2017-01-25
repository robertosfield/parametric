# -----------------------------
# Set up OpenIGS kernel package
# -----------------------------
# NOTE: OPENIGS_ROOT must have been set before this
# i.e. on the command line: -DOPENIGS_ROOT:PATH="/path/to/openigs"
# or must already be in the cache file.
if ( NOT OPENIGS_ROOT )
  ## TODO: Use canonical path depending on platform (there is hopefully a cmake command for this)
  set( OPENIGS_ROOT $ENV{OPENIGS_ROOT} )
  if ( NOT OPENIGS_ROOT )
    message( FATAL_ERROR "OPENIGS_ROOT not set or found. Set it either in the environment or as a CMake variable. Aborting...." )
    exit()
  endif()
endif()
list( APPEND CMAKE_MODULE_PATH 
      ${OPENIGS_ROOT}/share/openigs/cmake
)
include( IGSSetupOpenIGS )
include( IGSSetupVersion )

# --------------------
# Set up Boost package
# --------------------
include( IGSSetupBoost )
include( IGSSetupSaccadesLicensing )

include( IGSSetupOSG )

add_subdirectory( igs )

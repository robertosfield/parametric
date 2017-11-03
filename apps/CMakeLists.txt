# -----------------------------
# osgparametricscene executable
# -----------------------------

ADD_EXECUTABLE(
    osgparametricscene
    osgparametricscene.cpp
)

TARGET_LINK_LIBRARIES(
    osgparametricscene
    osgParametric
    ${OSG_LIBRARIES}
    ${OSGVIEWER_LIBRARIES}
    ${OSGUTIL_LIBRARIES}
    ${OSGDB_LIBRARIES}
    ${OSGGA_LIBRARIES}
    ${OPENTHREADS_LIBRARIES}
)

SET_TARGET_PROPERTIES(
    osgparametricscene
    PROPERTIES
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
)

INSTALL(
    TARGETS osgparametricscene
    RUNTIME DESTINATION bin COMPONENT Runtime
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
 )

# -----------------------------
# osgparametricnode executable
# -----------------------------

ADD_EXECUTABLE(
    osgparametricnode
    osgparametricnode.cpp
)

TARGET_LINK_LIBRARIES(
    osgparametricnode
    osgParametric
    ${OSG_LIBRARIES}
    ${OSGVIEWER_LIBRARIES}
    ${OSGUTIL_LIBRARIES}
    ${OSGDB_LIBRARIES}
    ${OSGGA_LIBRARIES}
    ${OPENTHREADS_LIBRARIES}
)

SET_TARGET_PROPERTIES(
    osgparametricnode
    PROPERTIES
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
)

INSTALL(
    TARGETS osgparametricnode
    RUNTIME DESTINATION bin COMPONENT Runtime
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
 )


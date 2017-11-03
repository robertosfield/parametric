# -----------------------------
# parametric executable
# -----------------------------

ADD_EXECUTABLE(
    parametric
    parametric.cpp
)

TARGET_LINK_LIBRARIES(
    parametric
    osgParametric
    ${OSG_LIBRARIES}
    ${OSGVIEWER_LIBRARIES}
    ${OSGUTIL_LIBRARIES}
    ${OSGDB_LIBRARIES}
    ${OSGGA_LIBRARIES}
    ${OPENTHREADS_LIBRARIES}
)

SET_TARGET_PROPERTIES(
    parametric
    PROPERTIES
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
)

INSTALL(
    TARGETS parametric
    RUNTIME DESTINATION bin COMPONENT Runtime
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
 )
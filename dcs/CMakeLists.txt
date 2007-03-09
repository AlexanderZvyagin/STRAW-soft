PROJECT(STRAW-dcs)

SET(ROOT_DIR ${PROJECT_SOURCE_DIR}/../.cmake )
FIND_PACKAGE(ROOT REQUIRED)

INCLUDE_DIRECTORIES( ${ROOT_INCLUDE_DIR} ${DaqDataDecoding_INCLUDE_DIR})

SET(DCS_SRC ChannelStatus.cc db_read.cc dcs.cc draw.cc nominal_HV.cc
            db_detectors_channels.cc db_write.cc MainCanvas.cc time-f.cc
            dcs_cint.cc)

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dcs_cint.cc
   COMMAND rootcint
   ARGS  -f ${CMAKE_CURRENT_BINARY_DIR}/dcs_cint.cc -c dcs.h MainCanvas.h LinkDef.h
   DEPENDS dcs.h MainCanvas.h LinkDef.h
)

ADD_EXECUTABLE(dcs ${DCS_SRC})
TARGET_LINK_LIBRARIES(dcs ${ROOT_LIBRARIES} popt)

INSTALL(TARGETS dcs
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)


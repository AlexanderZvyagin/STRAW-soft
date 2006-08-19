PROJECT(STRAW-soft)
SUBDIRS(dcs)

#SET(ROOT_DIR ${PROJECT_SOURCE_DIR}/.cmake )
#FIND_PACKAGE(ROOT REQUIRED)

#SET(DaqDataDecoding_DIR ${PROJECT_SOURCE_DIR}/.cmake )
#FIND_PACKAGE(DaqDataDecoding REQUIRED)


#FILE(GLOB lib_py ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.py)
SET(lib_py lib/__init__.py lib/names.py lib/mapping.py)

INSTALL(FILES ${lib_py} DESTINATION lib/STRAW)

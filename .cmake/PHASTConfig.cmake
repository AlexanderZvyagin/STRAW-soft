SET( PHAST_ROOT_DIR $ENV{PHAST} )

IF( "${PHAST_ROOT_DIR}" STREQUAL "" )
    MESSAGE(FATAL_ERROR "PHAST environment variable is not set!")
ELSE( "${PHAST_ROOT_DIR}" STREQUAL "" )
    #EXEC_PROGRAM(${PHAST_ROOT_DIR}/phast)
ENDIF( "${PHAST_ROOT_DIR}" STREQUAL "" )


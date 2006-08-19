SET( DaqDataDecoding_ROOT_DIR $ENV{DaqDataDecoding} )

IF   ( "${DaqDataDecoding_ROOT_DIR}" STREQUAL "" )
    MESSAGE(FATAL_ERROR "DaqDataDecoding environment variable is not set!")
ELSE ( "${DaqDataDecoding_ROOT_DIR}" STREQUAL "" )
    SET(DaqDataDecoding_INCLUDE_DIR ${DaqDataDecoding_ROOT_DIR}/src)
    SET(DaqDataDecoding_LIBRARIES "-L${DaqDataDecoding_ROOT_DIR}/src -lDaqDataDecoding")
ENDIF( "${DaqDataDecoding_ROOT_DIR}" STREQUAL "" )

MESSAGE(${DaqDataDecoding_INCLUDE_DIR})
MESSAGE(${DaqDataDecoding_LIBRARIES})

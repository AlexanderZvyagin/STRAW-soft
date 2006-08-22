SET( CORAL_ROOT_DIR $ENV{CORAL} )

IF( "${CORAL_ROOT_DIR}" STREQUAL "" )
    MESSAGE(FATAL_ERROR "CORAL environment variable is not set!")
ELSE( "${CORAL_ROOT_DIR}" STREQUAL "" )
    SET(CORAL_INCLUDE_DIR ${CORAL_ROOT_DIR}/include)
    SET(CORAL_LIBRARIES_DIR "-L${CORAL_ROOT_DIR}/lib/Linux 
                             -lCsBase 
                             -lCsBeam 
                             -lCsCalorim 
                             -lCsEvent 
                             -lCsEvmc 
                             -lCsG3CallFile 
                             -lCsGeom 
                             -lCsHist
                             -lCsObjStore 
                             -lCsOraStore 
                             -lCsRCEvdis
                             -lCsRich1 
                             -lCsSpacePoint 
                             -lCsTrack 
                             -lCsUtils 
                             -lCsVertex 
                             -lDaqDataDecoding 
                             -lDetectors 
                             -lexpat 
                             -lFileDB 
                             -lLattice 
                             -lMySQLDB 
                             -lRecon 
                             -lReco 
                             -lTraffic 
                             -lTrkMCTrue 
                             -lVrtKalman 
                             -lVrtRoland")
ENDIF( "${CORAL_ROOT_DIR}" STREQUAL "" )

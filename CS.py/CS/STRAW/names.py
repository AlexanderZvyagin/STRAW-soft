##  @addtogroup STRAW
##  @{

##  STRAW drift chamber detector short names for a given year.
#   
#   A short name is without ending two characteds, ex: \a ST03X1.
#   @return List of detectors
#
def straw_short_names(year):
    if year in [2002]:
        names = \
        ['ST03X1','ST03Y1','ST03U1',    \
         'ST03V1','ST03Y2','ST03X2',    \
         'ST04V1','ST04Y1','ST04X1']
    elif year in [2003,2004]:
        names = \
        ['ST03X1','ST03Y1','ST03U1',    \
         'ST03V1','ST03Y2','ST03X2',    \
         'ST04V1','ST04Y1','ST04X1',    \
         'ST05X1','ST05Y1','ST05U1',    \
         'ST06V1','ST06Y1','ST06X1']
    elif year in [2006]:
        names = \
        ['ST02X1','ST02Y1','ST02U1',    \
         'ST02V1','ST02Y2','ST02X2',    \
         'ST03X1','ST03Y1','ST03U1',    \
         'ST03V1','ST03Y2','ST03X2',    \
         'ST05X1','ST05U1','ST05Y2']
    elif year in [2007]:
        names = \
        ['ST02X1','ST02Y1','ST02U1',    \
         'ST02V1','ST02Y2','ST02X2',    \
         'ST03X1','ST03Y1','ST03U1',    \
         'ST03V1','ST03Y2','ST03X2',    \
         'ST05X1','ST05U1','ST05Y2']
    else:
        raise 'The straw detector names for the year %d are not known' % year

    return names

##  STRAW drift chamber detector full names for a given year.
#   
#   An example of a full name: \a ST03X1db.
#   @return Generates detector names.
#
def straw_full_names(year):
    for n in straw_short_names(year):
        for ud in 'ud':
            for abc in 'abc':
                yield n+ud+abc

##  @}

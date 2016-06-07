/*******************************************************************\

Module: Inspector Utilities

Author:

\*******************************************************************/


#ifndef INSPECTOR_INSPECTOR_UTIL_H_
#define INSPECTOR_INSPECTOR_UTIL_H_

typedef unsigned locationt;
typedef std::vector<locationt> locationst;
typedef std::vector<std::pair<locationt, locationst> > reaching_automatat;


#define contains(locations, location) \
  (std::find(locations.begin(), locations.end(), location) != locations.end())

#define Forall_locations(it, locations) \
  for(locationst::iterator it=(locations).begin(); \
      it!=(locations).end(); it++)

#define forall_locations(it, locations) \
  for(locationst::const_iterator it=(locations).begin(); \
      it!=(locations).end(); it++)

/*  TODO : Should have its scope altered.
 * Possibly even just macroed. */


#endif /* INSPECTOR_INSPECTOR_UTIL_H_ */

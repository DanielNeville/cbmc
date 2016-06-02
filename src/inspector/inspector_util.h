/*******************************************************************\

Module: Inspector Utilities

Author:

\*******************************************************************/


#ifndef INSPECTOR_INSPECTOR_UTIL_H_
#define INSPECTOR_INSPECTOR_UTIL_H_

typedef unsigned locationt;
typedef std::vector<locationt> locationst;

#define contains(locations, location) \
  (std::find(locations.begin(), locations.end(), location) != locations.end())

/*  TODO : Should have its scope altered.
 * Possibly even just macroed. */


#endif /* INSPECTOR_INSPECTOR_UTIL_H_ */

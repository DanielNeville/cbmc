/*******************************************************************\

Module: Reachability analysis

Author:

\*******************************************************************/

#ifndef INSPECTOR_REACHABILITY_ANALYSIS_H_
#define INSPECTOR_REACHABILITY_ANALYSIS_H_

#include <goto-programs/goto_functions.h>
#include <goto-programs/cfg.h>
#include <util/message.h>
#include <analyses/is_threaded.h>
#include <analyses/cfg_dominators.h>

/* To be correctly linked later when exact includes determined.
 * This avoids changing Makefiles for now. */
#include <path-symex/locs.h>
/* End includes */


#include "inspector_util.h"



/*******************************************************************\

   Class: From reachability_slicert

 Purpose:

\*******************************************************************/

class reachability_analysist
{
public:
  reachability_analysist(locationst &entry_locations_,
      locationst &input_locations_,
      locationst &output_locations_,
      message_handlert &message_handler_):
        entry_locations(entry_locations_),
        input_locations(input_locations_),
        output_locations(output_locations_),
        message(message_handler_)
{
    interaction_locations = input_locations;
#if 0
  /* This is (as far as I'm aware) valid.  But my Eclipse throws its toys out of the pram with it.
   * Using the below to avoid Eclipse issues.  */
  interaction_locations.insert(interaction_locations.end(), output_locations.begin(), output_locations.end());
#else
  for(auto it = output_locations.begin(); it != output_locations.end(); it++) {
    interaction_locations.push_back(*it);
  }

  all_locations = interaction_locations;
  for(auto it = entry_locations.begin(); it != entry_locations.end(); it++) {
    all_locations.push_back(*it);
  }
#endif
}

  void operator()(goto_functionst &goto_functions,
      std::vector<std::pair<locationt, locationst> > &interaction_reaches,
      std::vector<std::pair<locationt, locationst> > &all_reaches,
)
  {
    cfg(goto_functions);
    fixedpoint(interaction_reaches, all_reaches);
  }


protected:
  struct reachability_entryt
  {
    reachability_entryt():reaches_assertion(false)
    {
    }

    bool reaches_assertion;
  };

  typedef cfg_baset<reachability_entryt> cfgt;
  cfgt cfg;

  typedef std::stack<cfgt::entryt> queuet;


  void fixedpoint(
      std::vector<std::pair<locationt, locationst> > &interaction_reaches,
      std::vector<std::pair<locationt, locationst> > &alL_reaches);

  void slice(goto_functionst &goto_functions);

  locationst entry_locations;
  locationst input_locations;
  locationst output_locations;

  messaget message;

  /* Union of input + output. */
  locationst interaction_locations;

  /* Union of input, output, entry. */
  locationst all_locations;
};

void reachability_analysis(goto_functionst &goto_functions,
    std::vector<std::pair<locationt, locationst> > &interaction_reaches,
    std::vector<std::pair<locationt, locationst> > &all_reaches,
    locationst &entry_locations_,
    locationst &input_locations_,
    locationst &output_locations_,
    message_handlert &message_handler_);

#endif /* INSPECTOR_REACHABILITY_ANALYSIS_H_ */

/*******************************************************************\

Module: Reachability analysis

Author:

\*******************************************************************/

#include <iostream>
#include <stack>

#include <util/i2string.h>

#include <goto-programs/remove_skip.h>
#include <goto-programs/remove_unreachable.h>
#include <goto-programs/cfg.h>

#include "reachability_analysis.h"

/*******************************************************************\

Function: reachability_analysist::fixedpoint_assertions

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void reachability_analysist::fixedpoint(
    std::vector<std::pair<locationt, locationst> > &interaction_reaches,
    std::vector<std::pair<locationt, locationst> > &all_reaches)
{
  Forall_locations(location, all_locations) {
    locationst reached;
    locationst interactions_reached;
    locationst frontier;

    frontier.push_back(*location);

    while(!frontier.empty()) {
      locationt item = frontier.back();
      frontier.pop_back();
      reached.push_back(item);

      if(contains(interaction_locations, item))
        interactions_reached.push_back(item);

      for(auto it : cfg[item].out) {
        if(!contains(reached, it.first)) {
          frontier.push_back((unsigned) it.first);
        }
      }
    }

    all_reaches.push_back(std::make_pair(*location, reached));
  }
}


/*******************************************************************\

Function: reachability_analysis

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void reachability_analysis(goto_functionst &goto_functions,
    std::vector<std::pair<locationt, locationst> > &interaction_reaches,
    std::vector<std::pair<locationt, locationst> >  &all_reaches,
    locationst &entry_locations,
    locationst &input_locations,
    locationst &output_locations,
    message_handlert &message_handler)
{
  reachability_analysist(entry_locations, input_locations, output_locations, message_handler)
      (goto_functions, interaction_reaches, all_reaches);
}

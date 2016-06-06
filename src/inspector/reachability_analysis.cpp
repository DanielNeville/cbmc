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
  const is_threadedt &is_threaded)
{
  queuet queue;

  message.status() << "___\n" << message.eom;

  for(cfgt::entry_mapt::iterator
      e_it=cfg.entry_map.begin();
      e_it!=cfg.entry_map.end();
      e_it++) {

    if(is_threaded(e_it->first)){
      message.error() << "Threaded programs not currently supported." << message.eom;
      return;
    }

    locationt location = (locationt) e_it->first->location_number;

    std::cout << e_it->first->location_number << "\n";
    if(contains(entry_locations, (locationt) location)) {
//      queue.push(e_it->second);
      std::cout << "CONTAINED!" << location << "\n";
    }
  }



//
//  while(!queue.empty())
//  {
//    cfgt::entryt e=queue.top();
//    cfgt::nodet &node=cfg[e];
//    queue.pop();
//
//    if(node.reaches_assertion) continue;
//
//    node.reaches_assertion=true;
//
//    for(cfgt::edgest::const_iterator
//        p_it=node.in.begin();
//        p_it!=node.in.end();
//        p_it++)
//    {
//      queue.push(p_it->first);
//    }
//  }
}

/*******************************************************************\

Function: reachability_analysist::slice

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void reachability_analysist::slice(goto_functionst &goto_functions)
{
  // now replace those instructions that do not reach any assertions
  // by self-loops
  return;

  Forall_goto_functions(f_it, goto_functions)
    if(f_it->second.body_available())
    {
      Forall_goto_program_instructions(i_it, f_it->second.body)
      {
        const cfgt::nodet &e=cfg[cfg.entry_map[i_it]];
        if(!e.reaches_assertion &&
           !i_it->is_end_function())
          i_it->make_goto(i_it);
      }

      // replace unreachable code by skip
      remove_unreachable(f_it->second.body);
    }

  // remove the skips
  remove_skip(goto_functions);
  goto_functions.update();
}

/*******************************************************************\

Function: reachability_analysis

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void reachability_analysis(goto_functionst &goto_functions,
    std::vector<std::pair<locationt, locationst> >  &reached,
    locationst &entry_locations,
    locationst &input_locations,
    locationst &output_locations,
    message_handlert &message_handler)
{
  reachability_analysist(entry_locations, input_locations, output_locations, message_handler)
      (goto_functions, reached);
}

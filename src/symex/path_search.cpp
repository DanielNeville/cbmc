/*******************************************************************\

Module: Path-based Symbolic Execution

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <util/time_stopping.h>

#include <solvers/flattening/bv_pointers.h>
#include <solvers/sat/satcheck.h>

#include <path-symex/path_symex.h>
#include <path-symex/build_goto_trace.h>

#include <analyses/dependence_graph.h>
#include <analyses/cfg_dominators.h>




//

#include <stack>

#include <util/i2string.h>
#include <util/simplify_expr.h>

#include <goto-programs/remove_skip.h>
#include <goto-programs/remove_unreachable.h>
#include <goto-programs/cfg.h>
#include <iostream>
#include <algorithm>
#include <path-symex/step_wp.h>
//

#include "path_search.h"

/*******************************************************************\

Function: path_searcht::operator()

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

path_searcht::resultt path_searcht::operator()(
  const goto_functionst &goto_functions)
{
  var_mapt var_map(ns);
  
  locs.build(goto_functions);

  /* Useful to have CFG too */
  cfg(goto_functions);
  for(int i = 0; i < locs.size(); i++) {
    assumptions[i] = nil_exprt();
  }
  /*// */

  // this is the container for the history-forest  
  path_symex_historyt history;
  
  queue.push_back(initial_state(var_map, locs, history));

  initialize_property_map(goto_functions);


  dependence_graph(goto_functions, ns);

  dependence_graph.output(ns, goto_functions, std::cout);

  reachability(goto_functions);

  calculate_failure_locations(goto_functions);

  // set up the statistics
  number_of_dropped_states=0;
  number_of_paths=0;
  number_of_VCCs=0;
  number_of_steps=0;
  number_of_feasible_paths=0;
  number_of_infeasible_paths=0;
  number_of_VCCs_after_simplification=0;
  number_of_failed_properties=0;
  number_of_locs=locs.size();

  // stop the time
  start_time=current_time();
  

  while(!queue.empty())
  {
    number_of_steps++;
  
    // Pick a state from the queue,
    // according to some heuristic.
    // The state moves to the head of the queue.
    pick_state();
    
    // move into temporary queue
    queuet tmp_queue;
    tmp_queue.splice(
      tmp_queue.begin(), queue, queue.begin(), ++queue.begin());
    
    try
    {
      statet &state=tmp_queue.front();
      
      // record we have seen it
      loc_data[state.get_pc().loc_number].visited=true;

      debug() << "Loc: #" << state.get_pc().loc_number
              << ", queue: " << queue.size()
              << ", depth: " << state.get_depth();
      for(const auto & s : queue)
        debug() << ' ' << s.get_depth();
        
      debug() << eom;

      if(fails[state.get_pc().loc_number].size() > 0) {
        handle_fails(state, goto_functions);
      }
    
      if(drop_state(state))
      {
        number_of_dropped_states++;
        number_of_paths++;
        continue;
      }
      
      if(!state.is_executable())
      {
        number_of_paths++;
        continue;
      }

      if(eager_infeasibility &&
         state.last_was_branch() &&
         !is_feasible(state))
      {
        number_of_infeasible_paths++;
        number_of_paths++;
        continue;
      }
      
      if(number_of_steps%1000==0)
      {
        status() << "Queue " << queue.size()
                 << " thread " << state.get_current_thread()
                 << '/' << state.threads.size()
                 << " PC " << state.pc() << messaget::eom;
      }

      // an error, possibly?
      if(state.get_instruction()->is_assert())
      {
        if(show_vcc)
          do_show_vcc(state);
        else
        {
          check_assertion(state);
          
          // all assertions failed?
          if(number_of_failed_properties==property_map.size())
            break;
        }
      }

      // execute
      path_symex(state, tmp_queue);

      state.history->learnt_clause = assumptions[state.history->pc.loc_number];

      // put at head of main queue
      queue.splice(queue.begin(), tmp_queue);
    }
    catch(const std::string &e)
    {
      error() << e << eom;
      number_of_dropped_states++;
    }
    catch(const char *e)
    {
      error() << e << eom;
      number_of_dropped_states++;
    }
    catch(int)
    {
      number_of_dropped_states++;
    }
  }
  
  report_statistics();
  
  return number_of_failed_properties==0?SAFE:UNSAFE;
}

/*******************************************************************\

Function: path_searcht::report_statistics

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void path_searcht::report_statistics()
{
  std::size_t number_of_visited_locations=0;
  for(const auto & l : loc_data)
    if(l.visited)
      number_of_visited_locations++;

  #if 0
  for(unsigned l=0; l<loc_data.size(); l++)
    if(!loc_data[l].visited) status() << "NV: " << l << eom;
  #endif

  // report a bit
  status() << "Number of visited locations: "
           << number_of_visited_locations << " (out of "
           << number_of_locs << ')' << messaget::eom;

  status() << "Number of dropped states: "
           << number_of_dropped_states << messaget::eom;

  status() << "Number of paths: "
           << number_of_paths << messaget::eom;

  status() << "Number of infeasible paths: "
           << number_of_infeasible_paths << messaget::eom;

  status() << "Generated " << number_of_VCCs << " VCC(s), "
           << number_of_VCCs_after_simplification
           << " remaining after simplification"
           << messaget::eom;
  
  time_periodt total_time=current_time()-start_time;
  status() << "Runtime: " << total_time << "s total, "
           << sat_time << "s SAT" << messaget::eom;
}

/*******************************************************************\

Function: path_searcht::pick_state

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void path_searcht::pick_state()
{
  switch(search_heuristic)
  {
  case search_heuristict::DFS:
    // Picking the first one (most recently added) is a DFS.
    return;
  
  case search_heuristict::BFS:
    // Picking the last one is a BFS.
    if(queue.size()>=2)
      // move last to first position
      queue.splice(queue.begin(), queue, --queue.end(), queue.end());
    return;
    
  case search_heuristict::LOCS:
    return;
  }  
}

/*******************************************************************\

Function: path_searcht::do_show_vcc

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void path_searcht::do_show_vcc(statet &state)
{
  // keep statistics
  number_of_VCCs++;
  
  const goto_programt::instructiont &instruction=
    *state.get_instruction();
    
  mstreamt &out=result();

  if(instruction.source_location.is_not_nil())
    out << instruction.source_location << '\n';
  
  if(instruction.source_location.get_comment()!="")
    out << instruction.source_location.get_comment() << '\n';
    
  unsigned count=1;
  
  std::vector<path_symex_step_reft> steps;
  state.history.build_history(steps);

  for(std::vector<path_symex_step_reft>::const_iterator
      s_it=steps.begin();
      s_it!=steps.end();
      s_it++)
  {      
    if((*s_it)->guard.is_not_nil())
    {
      std::string string_value=from_expr(ns, "", (*s_it)->guard);
      out << "{-" << count << "} " << string_value << '\n';
      count++;
    }

    if((*s_it)->ssa_rhs.is_not_nil())
    {
      equal_exprt equality((*s_it)->ssa_lhs, (*s_it)->ssa_rhs);
      std::string string_value=from_expr(ns, "", equality);
      out << "{-" << count << "} " << string_value << '\n';
      count++;
    }
  }

  out << "|--------------------------" << '\n';
  
  exprt assertion=state.read(instruction.guard);

  out << "{" << 1 << "} "
      << from_expr(ns, "", assertion) << '\n';

  if(!assertion.is_true())
    number_of_VCCs_after_simplification++;
  
  out << eom;
}

/*******************************************************************\

Function: path_searcht::drop_state

  Inputs:

 Outputs:

 Purpose: decide whether to drop a state

\*******************************************************************/

bool path_searcht::drop_state(const statet &state) const
{
  // depth limit
  if(depth_limit_set && state.get_depth()>depth_limit)
    return true;
  
  // context bound
  if(context_bound_set && state.get_no_thread_interleavings()>context_bound)
    return true;
  
  // branch bound
  if(branch_bound_set && state.get_no_branches()>branch_bound)
    return true;
  
  // unwinding limit -- loops
  if(unwind_limit_set && state.get_instruction()->is_backwards_goto())
  {
    for(path_symex_statet::unwinding_mapt::const_iterator
        it=state.unwinding_map.begin();
        it!=state.unwinding_map.end();
        it++)
      if(it->second>unwind_limit)
        return true;
  }
  
  // unwinding limit -- recursion
  if(unwind_limit_set && state.get_instruction()->is_function_call())
  {
    for(path_symex_statet::recursion_mapt::const_iterator
        it=state.recursion_map.begin();
        it!=state.recursion_map.end();
        it++)
      if(it->second>unwind_limit)
        return true;
  }
  
  return false;
}

/*******************************************************************\

Function: path_searcht::check_assertion

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void path_searcht::check_assertion(statet &state)
{
  // keep statistics
  number_of_VCCs++;
  
  const goto_programt::instructiont &instruction=
    *state.get_instruction();

  irep_idt property_name=instruction.source_location.get_property_id();
  property_entryt &property_entry=property_map[property_name];
  
  if(property_entry.status==FAILURE)
    return; // already failed
  else if(property_entry.status==NOT_REACHED)
    property_entry.status=SUCCESS; // well, for now!

  // the assertion in SSA
  exprt assertion=
    state.read(instruction.guard);

  if(assertion.is_true()) return; // no error, trivially

  // keep statistics
  number_of_VCCs_after_simplification++;
  
  status() << "Checking property " << property_name << eom;

  // take the time
  absolute_timet sat_start_time=current_time();

  satcheckt satcheck;
  bv_pointerst bv_pointers(ns, satcheck);
  
  satcheck.set_message_handler(get_message_handler());
  bv_pointers.set_message_handler(get_message_handler());

  if(!state.check_assertion(bv_pointers))
  {
    build_goto_trace(state, bv_pointers, property_entry.error_trace);
    property_entry.status=FAILURE;
    number_of_failed_properties++;
  }
  
  sat_time+=current_time()-sat_start_time;
}

/*******************************************************************\

Function: path_searcht::is_feasible

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bool path_searcht::is_feasible(statet &state)
{
  status() << "Feasibility check" << eom;

  // take the time
  absolute_timet sat_start_time=current_time();

  satcheckt satcheck;
  bv_pointerst bv_pointers(ns, satcheck);
  
  satcheck.set_message_handler(get_message_handler());
  bv_pointers.set_message_handler(get_message_handler());

  bool result=state.is_feasible(bv_pointers);
  
  sat_time+=current_time()-sat_start_time;
  
  return result;
}

/*******************************************************************\

Function: path_searcht::initialize_property_map

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void path_searcht::initialize_property_map(
  const goto_functionst &goto_functions)
{
  unsigned int counter = 1;
  for(goto_functionst::function_mapt::const_iterator
      it=goto_functions.function_map.begin();
      it!=goto_functions.function_map.end();
      it++)
    if(!it->second.is_inlined())
    {
      const goto_programt &goto_program=it->second.body;
    
      for(goto_programt::instructionst::const_iterator
          it=goto_program.instructions.begin();
          it!=goto_program.instructions.end();
          it++)
      {
        if(!it->is_assert())
          continue;
      
        const source_locationt &source_location=it->source_location;
      
        irep_idt property_name=source_location.get_property_id();
        
        property_entryt &property_entry=property_map[property_name];
        property_entry.status=NOT_REACHED;
        property_entry.description=source_location.get_comment();
        property_entry.source_location=source_location;
        property_entry.counter_id=counter;
        property_entry.location_number=it->location_number;
        counter++;
      }
    }    
}

void path_searcht::calculate_failure_locations(const goto_functionst &goto_functions) {
  for(cfgt::entry_mapt::iterator
      e_it=cfg.entry_map.begin();
      e_it!=cfg.entry_map.end();
      e_it++) {
    std::set<unsigned> reaches = reachability.reaches(e_it->first->location_number);
    cfgt::nodet &node=cfg[e_it->second];

    std::set<unsigned> all_reaches;

    in_nodes[e_it->first->location_number] = node.in.size();

    for(cfgt::edgest::const_iterator
        p_it=node.in.begin();
        p_it!=node.in.end();
        p_it++) {
      std::set<unsigned> in_reaches = reachability.reaches(cfg[p_it->first].PC->location_number);
      for(auto i : in_reaches) {
        all_reaches.insert(i);
      }
    }

    std::set<int> result;
    std::set_difference(all_reaches.begin(), all_reaches.end(), reaches.begin(), reaches.end(),
        std::inserter(fails[e_it->first->location_number], fails[e_it->first->location_number].end()));
  }
}

void path_searcht::handle_fails(statet &state, const goto_functionst &goto_functions) {
  assert(fails[state.get_instruction()->location_number].size() > 0);
  for(auto f : triggered_assertion_failures) { // Could change to find.  Barely matters.
    if(f == state.get_instruction()->location_number)
      return; // Just trigger once for now.
  }

  triggered_assertion_failures.push_back(state.get_instruction()->location_number);
  unsigned assertion_failure = *fails[state.get_instruction()->location_number].begin();
  // Just get the first for now.

  std::cout << "********** Failed assert: " << assertion_failure << " **********\n";

  unsigned property_id = 0; // property ids are from 1..n;

  for(auto p:property_map) {
    if(p.second.location_number == assertion_failure) {
      property_id = p.second.counter_id;
      break;
    }
  }
  assert(property_id); // Must be non-zero.
  symbolt symbol = ns.lookup("__CPROVER_initialize::fails_" + i2string(property_id));

  exprt assumption = state.read_no_propagate(not_exprt(symbol.symbol_expr()));

  path_symex_step_reft history_step = state.history;

  while(!history_step.is_nil()) {
    unsigned loc = history_step->pc.loc_number;

    if(locs[history_step->pc].target->is_assert()) {
      /* Must handle assertion case, manual weakest pre. */
      assumption = implies_exprt(state.read_no_propagate(locs[history_step->pc].target->guard), assumption);
    } else {
      assert(!locs[history_step->pc].target->is_assert());
      assumption = step_wp(*history_step, assumption, ns);
    }

//    std::cout << "\n\n--" << loc << "--\n\n" <<  assumption.pretty() << "\n\n\n** SIMPLIFIES TO **\n\n" << simplify_expr(assumption, ns).pretty();

    if(state.get_instruction()->is_goto()) {
      if(assumptions[loc].is_nil()) {
        assumptions[loc] = simplify_expr(assumption, ns);
      } else {
        assumptions[loc] = simplify_expr(and_exprt(assumptions[loc], assumption), ns);
      }
    }



    --history_step; //previous
  }
}

/////

/*******************************************************************\

Function: reachability_slicert::fixedpoint_assertions

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void reachabilityt::fixedpoint_assertions(std::vector<unsigned> &locations, unsigned location)
{
  queuet queue;

  for(cfgt::entry_mapt::iterator
      e_it=cfg.entry_map.begin();
      e_it!=cfg.entry_map.end();
      e_it++) {
    cfg[e_it->second].reaches_assertion = false;
    if(e_it->first->location_number == location) {
      queue.push(e_it->second);
    }
  }
  // Should change to
  //     const cfgt::nodet &e=cfg[cfg.entry_map[i_it]];

  while(!queue.empty())
  {
    cfgt::entryt e=queue.top();
    cfgt::nodet &node=cfg[e];
    queue.pop();

    if(node.reaches_assertion) continue;

    node.reaches_assertion=true;
    locations.push_back(node.PC->location_number);

    for(cfgt::edgest::const_iterator
        p_it=node.in.begin();
        p_it!=node.in.end();
        p_it++)
    {
      queue.push(p_it->first);
    }
  }
}


void reachabilityt::output(std::ostream &out) {
  for(int i = 0; i < reachable_assertions.size(); i++) {
    out << i << " reaches :";
    for(auto l: reachable_assertions[i]) {
      out << l << ", ";
    }
    out << "\n";
  }

}

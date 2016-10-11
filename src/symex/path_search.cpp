/*******************************************************************\

Module: Path-based Symbolic Execution

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <util/time_stopping.h>

#include <solvers/flattening/bv_pointers.h>
#include <solvers/sat/satcheck.h>

#include <path-symex/path_symex.h>
#include <path-symex/build_goto_trace.h>

// Move to appropriate file.
#include <util/simplify_expr.h>
#include <util/base_type.h>

#include <iostream>

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

  // this is the container for the history-forest  
  path_symex_historyt history;
  
  queue.push_back(initial_state(var_map, locs, history));
  
  // set up the statistics
  number_of_dropped_states=0;
  number_of_paths=0;
  number_of_VCCs=0;
  number_of_steps=0;
  number_of_feasible_paths=0;
  number_of_infeasible_paths=0;
  number_of_VCCs_after_simplification=0;
  number_of_failed_properties=0;
  number_of_merged_states=0;
  number_of_locs=locs.size();

  // stop the time
  start_time=current_time();
  
  initialize_property_map(goto_functions);
  
  while(!queue.empty())
  {
    number_of_steps++;

    at_merge_point(*queue.begin());

    merge_states();
  
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

  if(merge_heuristic != merge_heuristict::NONE)
  {
    status() << "Number of merges: "
             << number_of_merged_states << messaget::eom;
  }

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

  case search_heuristict::FAST_FORWARD:
    // Very simple fast forward strategy.

    if(queue.size() > 1) {
      for (auto it = queue.begin(); it != queue.end(); it++)
      {
        if(!at_merge_point(*it))
        {
          // Move states not at merge point to front.
          // Breaks iterator.
          queue.splice(queue.begin(), queue, it);
          // But for now, only one.  We broke our iterator.
          break;
        }
      }
    }
  }  
}
/*******************************************************************\

Function: path_searcht::merge_states

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void path_searcht::merge_states()
{
  if(queue.size() < 2)
    return;

  switch (merge_heuristic)
  {
    case merge_heuristict::NONE:
      return;

    case merge_heuristict::AGGRESSIVE:
      // The last state inserted, is the last state
      // we manipulated.
      queuet::iterator current=queue.begin();

      queuet::iterator it=queue.begin();
      assert(queue.size() >= 2 && it != queue.end());

      // Move it to the point the state beyond the last state.
      it++;

      for (; it != queue.end(); it++)
      {
        if(it->pc() != current->pc()
            ||
           it->get_current_thread() != current->get_current_thread()
           )
        {
          // Only merge when same PC.  (Otherwise fast-forward?)
          continue;
        }

        debug() << "Considering to merge at " << it->pc() << eom;

        // Own class with inheritance eventually.
        if(do_aggressive_merge(it, current))
        {
          merge(current, it);
          number_of_merged_states++;
// //     Disabled for testing.
          queue.erase(it);
        }
      }

      return;
  }
}

bool path_searcht::do_aggressive_merge(
    queuet::iterator &state, queuet::iterator &cmp_state)
{
  return true; // much aggressive.
}

bool path_searcht::do_qce_merge(
    queuet::iterator &state, queuet::iterator &cmp_state)
{
  /* To do. */

  return false;
}

void path_searcht::merge(
    queuet::iterator &state, queuet::iterator &cmp_state)
{
  assert(state->pc() == cmp_state->pc());

  assert(state->get_current_thread() == cmp_state->get_current_thread());

  progress() << "Merging states at program location: " << state->pc() << eom;
  // Specific semantics.  State is updated to be the 'new' state.
  // Cmp_state will be destroyed.

  /* So it should be noted that two states to merge will have some
   * common prefix, and some common suffix (albeit possibly empty)
   *
   * Maintaining the common prefix is particularly important, so let's
   * consider how to calculate.
   */

  /* Generate forward iterable histories for both states */
  state_historyt state_history;
  state->history.build_history(state_history);
  state_historyt::iterator state_it = state_history.begin();

  state_historyt cmp_state_history;
  cmp_state->history.build_history(cmp_state_history);
  state_historyt::iterator cmp_state_it = cmp_state_history.begin();

  unsigned steps = 0;
  /* This code places both states at the first location of their divergence. */

  while (state_it != state_history.end()
      && cmp_state_it != cmp_state_history.end())
  {

    if((*state_it)->pc != (*cmp_state_it)->pc) // Consider if different guard.
      break;

    steps++;

    state_it++;
    cmp_state_it++;
  }

  if(steps == 0)
  {
    throw "Error.\n";
  }

  /*
   * One will have guard: g, and one will have not_exprt(g).
   * This isn't relevant or assumed.  Both are treated as "just some" guard.
   * tl;dr: It is not assumed that g1 = not(g2) or vice-versa.
   */

  /* History.
   *
   * Each expression is conjuncted onto the current focal point.
   * Each guard generates a new conjunction of
   */

  exprt state_guarded_expression=true_exprt();
  exprt cmp_state_guarded_expression=true_exprt();

  path_symex_step_reft reverse_step=state->history;
  path_symex_step_reft cmp_reverse_step=cmp_state->history;

  and_exprt state_conjunction_guards;
  and_exprt cmp_state_conjunction_guards;


  /* This alters reverse_step! */
  construct_guarded_expression(state_guarded_expression,
      state_history.size() - steps, reverse_step,
      state_conjunction_guards.operands());

  construct_guarded_expression(cmp_state_guarded_expression,
      cmp_state_history.size() - steps, cmp_reverse_step,
      cmp_state_conjunction_guards.operands());

  exprt guarded_expression=and_exprt(state_guarded_expression,
      cmp_state_guarded_expression);

  guarded_expression=simplify_expr(guarded_expression, ns);

  /* Update history */
  /* Reverse step was moved by construct_guarded_expression! */
  reverse_step->guard=nil_exprt();
  reverse_step->full_lhs=true_exprt();
  reverse_step->ssa_lhs=symbol_exprt();
  reverse_step->ssa_rhs=nil_exprt();
  reverse_step->arbitrary_expr=guarded_expression;

  /* Rewrite history */
  state->history=reverse_step;
  /* This step changes history! */

  /* Update internal variable mappings */

  // a and !b
  exprt cond=and_exprt(state_conjunction_guards,
      not_exprt(cmp_state_conjunction_guards));

//  cond=simplify_expr(cond, ns);

  int i = 0;

  std::cout << "Size: " << state->var_map.id_map.size() << "\n";

//  for (var_mapt::id_mapt::iterator it=state->var_map.id_map.begin();
//      it != state->var_map.id_map.end(); it++)
//  {
//    if(!(state->get_var_state(it->second).value
//        == cmp_state->get_var_state(it->second).value))
//    {
//      if_exprt phi_node=if_exprt();
//
//      phi_node.cond()=cond;
//
//      if(state->get_var_state(it->second).value.is_not_nil()
//          && cmp_state->get_var_state(it->second).value.is_not_nil())
//      {
//        phi_node.type()=state->get_var_state(it->second).value.type();
//        phi_node.true_case()=state->get_var_state(it->second).value;
//        phi_node.false_case()=cmp_state->get_var_state(it->second).value;
//      }
//      else if(state->get_var_state(it->second).value.is_not_nil()
//          && cmp_state->get_var_state(it->second).value.is_nil())
//      {
//        phi_node.type()=state->get_var_state(it->second).value.type();
//        phi_node.true_case()=state->get_var_state(it->second).value;
//        phi_node.false_case()=it->second.ssa_symbol();
//      }
//      else if(cmp_state->get_var_state(it->second).value.is_not_nil()
//          && state->get_var_state(it->second).value.is_nil())
//      {
//        phi_node.type()=cmp_state->get_var_state(it->second).value.type();
//        phi_node.true_case()=it->second.ssa_symbol();
//        phi_node.false_case()=cmp_state->get_var_state(it->second).value;
//      }
//      else
//      {
//        assert(0); // They're both the same via the previous IF, so if they're both NIL, they're the same.
//        // This should not be reached.
//      }
////      base_type_eq(phi_node.true_case(), phi_node.false_case(), ns);
//      state->get_var_state(it->second).value=phi_node;
//
//      std::cout << phi_node.pretty() << "\n\n\n\n\n\n\n\n\n______\n\n\n\n\n\n";
//    }
//  }

  /* Update statistics */
  // Max depth.
  state->set_depth(
      (state->get_depth() < cmp_state->get_depth()) ?
          state->get_depth() : cmp_state->get_depth());
  // ...
}

void path_searcht::construct_guarded_expression(exprt &expr,
    int reverse_steps, path_symex_step_reft &reverse_step,
    exprt::operandst &guards)
{
  // Condition at bottom.
   while (true)
   {
     /* We could generate the guarded expression through backwards recursion. */
     /* TODO:  The above. */
     if(reverse_step->guard.is_not_nil()) {
       expr = implies_exprt(reverse_step->guard, expr);
       guards.push_back(reverse_step->guard);
     }

     if(reverse_step->ssa_rhs.is_not_nil()) {
       expr=and_exprt(expr,
           equal_exprt(reverse_step->ssa_lhs, reverse_step->ssa_rhs));
     }

     if(reverse_step->arbitrary_expr.is_not_nil()) {
       expr=and_exprt(expr,
                 reverse_step->arbitrary_expr);
     }

     if(reverse_steps > 0) {
       --reverse_step;
       --reverse_steps;
       /* This means we end up in the right place. */
     } else {
       break;
     }
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
      }
    }    
}

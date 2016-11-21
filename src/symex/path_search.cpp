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
#include <util/replace_expr.h>
#include <util/prefix.h>
#include <util/base_type.h>
#include <math.h>
#include <goto-programs/goto_functions.h>
#include <goto-programs/cfg.h>
#include <analyses/cfg_dominators.h>

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

  time_periodt merging_time;

  initialize_property_map(goto_functions);

  if(qce_set)
  {
    calculate_qce_hotsets(goto_functions);
  }

  if(our_qce_set)
  {
    calculate_our_qce_hotsets(goto_functions);
  }

  while(!queue.empty())
  {
    number_of_steps++;

//    at_merge_point(*queue.begin());

    absolute_timet merging_start = current_time();
    merge_states();
    merging_time += current_time() - merging_start;

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

//      std::cout << state.get_pc().loc_number << " " << state.get_instruction()->type << " -- Number of states: " << queue.size() << "\n";

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

  std::cout << "Merging time: " << merging_time.as_string() << "s\n";

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

  if(merge_heuristic == merge_heuristict::NONE)
    return;

  queuet::iterator current=queue.begin();

  queuet::iterator it=queue.begin();
  assert(queue.size() >= 2 && it != queue.end());

  // Move it to the point the state beyond the last state.
  it++;

  for (; it != queue.end(); it++)
  {
    if(it->pc() != current->pc()
        || it->get_current_thread() != current->get_current_thread())
    {
      // Only merge when same PC.  (Otherwise fast-forward?)
      continue;
    }

    // Own class with inheritance eventually.
    bool do_merge=false;

    switch (merge_heuristic)
    {
      case merge_heuristict::AGGRESSIVE:
        debug() << "Considering to aggressive merge at " << it->pc() << eom;
        do_merge=do_aggressive_merge(it, current);
        break;

      case merge_heuristict::OUR_QCE:
        do_merge=do_our_qce_merge(it, current);
        break;

      case merge_heuristict::QCE:
        do_merge=do_qce_merge(it, current);
        break;

      case merge_heuristict::CONSTANTS:
        do_merge=do_cc_merge(it, current);
        break;

      default:
        do_merge=false;
    }

    if(do_merge)
    {
      debug() << "Merge returned TRUE." << eom;
      merge(current, it);
      number_of_merged_states++;
      queue.erase(it);
    }
  }
}

bool path_searcht::do_aggressive_merge(
    queuet::iterator &state, queuet::iterator &cmp_state)
{
  return true; // much aggressive.
}


bool path_searcht::do_cc_merge(queuet::iterator &state,
    queuet::iterator &cmp_state)
{
  debug() << "Considering CC merge at " << state->get_pc().loc_number << eom;

  assert(state->get_pc().loc_number == cmp_state->get_pc().loc_number);

  for (auto var_ptr : state->var_map.id_map)
  {
    var_mapt::var_infot &var=state->var_map.id_map[var_ptr.first];
    var_mapt::var_infot &cmp_var=cmp_state->var_map.id_map[var_ptr.first];

    if(state->get_var_state(var).value.is_constant()
        && cmp_state->get_var_state(cmp_var).value.is_constant()
        && (state->get_var_state(var).value.get(ID_value)
            != cmp_state->get_var_state(var).value.get(ID_value)))
    {
      debug() << "Returning false on CC calculation." << eom;
      return false;
    }
  }

  debug() << "Returning True on CC calculation." << eom;
  return true;
}

bool path_searcht::do_qce_merge(
    queuet::iterator &state, queuet::iterator &cmp_state)
{
  debug() << "Considering QCE merge at " << state->get_pc().loc_number << eom;

  assert(state->get_pc().loc_number == cmp_state->get_pc().loc_number);

  for (auto var_ptr : state->var_map.id_map)
  {
    var_mapt::var_infot &var=state->var_map.id_map[var_ptr.first];
    var_mapt::var_infot &cmp_var=cmp_state->var_map.id_map[var_ptr.first];

    location_symbol_pairt loc_pair =
        std::make_pair(state->get_pc().loc_number, var.full_identifier);

    if(q_add[loc_pair] > alpha * q_tot[state->get_instruction()])
    {
      debug() << var.full_identifier << " in the hot-set." << eom;

      if(state->get_var_state(var).value.is_constant()
          &&
          cmp_state->get_var_state(cmp_var).value.is_constant()
        &&
        (state->get_var_state(var).value.get(ID_value)
            !=
        cmp_state->get_var_state(var).value.get(ID_value)))
      {
        debug() << "Returning false on QCE calculation." << eom;
        return false;
      }
    }
  }

  debug() << "Returning True on QCE calculation." << eom;
  return true;
}


bool path_searcht::do_our_qce_merge(
    queuet::iterator &state, queuet::iterator &cmp_state)
{
  debug() << "Considering QCE merge at " << state->get_pc().loc_number << eom;

  assert(state->get_pc().loc_number == cmp_state->get_pc().loc_number);

  typedef std::vector<std::pair<irep_idt, double > > hotsett;
  hotsett hotset;

  for (auto var_ptr : state->var_map.id_map)
  {

    location_symbol_pairt loc_pair =
        std::make_pair(state->get_pc().loc_number,
        var_ptr.second.full_identifier);

    unsigned value=q_reaches[loc_pair];

    if(q_reaches[loc_pair] > 0)
    {
      hotsett::iterator it = hotset.begin();

      while(it != hotset.end()
          && value > it->second)
      {
        it++;
      }
      hotset.insert(it, std::make_pair(var_ptr.second.full_identifier, q_reaches[loc_pair]));
    }
  }

  if(hotset.empty()) {
    return true;
  }

  unsigned remove = (2 * queue.size() * hotset.size()) / max;

  while(remove--)
    hotset.pop_back();

  for (hotsett::iterator it = hotset.begin();
      it != hotset.end();
      it++)
  {
    var_mapt::var_infot &var=state->var_map.id_map[it->first];
    var_mapt::var_infot &cmp_var=cmp_state->var_map.id_map[it->first];

    if(state->get_var_state(var).value.is_constant()
        &&
        cmp_state->get_var_state(cmp_var).value.is_constant()
      &&
      (state->get_var_state(var).value.get(ID_value)
          !=
      cmp_state->get_var_state(cmp_var).value.get(ID_value)))
    {
      debug() << "Returning false on Our QCE calculation." << eom;
      return false;
    }
  }

  debug() << "Returning True on Our QCE calculation." << eom;
  return true;
}

void path_searcht::merge(
    queuet::iterator &state, queuet::iterator &cmp_state)
{
  /* The steps here are:
   * 1) Assert the states are in a position to be merged.
   * 2) Find the common prefix of both states, and the number of steps required.
   * 3) Generate
   *
   */

  assert(state->pc() == cmp_state->pc());
  assert(state->get_current_thread() == cmp_state->get_current_thread());
  assert(state->threads.size() == 1);
  assert(cmp_state->threads.size() == 1);

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
//
//  debug() << "First " << steps << " steps are contained within both states to be merged." << eom;
//
//  debug() << "First different steps: " << (*state_it)->pc.loc_number << " and " << (*cmp_state_it)->pc.loc_number << eom;

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

  and_exprt state_guarded_expression;
  and_exprt cmp_state_guarded_expression;

  path_symex_step_reft reverse_step=state->history;
  path_symex_step_reft cmp_reverse_step=cmp_state->history;

  and_exprt state_conjunction_guards;
  and_exprt cmp_state_conjunction_guards;

  /* This alters reverse_step! */
  construct_guarded_expression(state_guarded_expression.operands(),
      state_history.size() - steps, reverse_step,
      state_conjunction_guards.operands());

  construct_guarded_expression(cmp_state_guarded_expression.operands(),
      cmp_state_history.size() - steps, cmp_reverse_step,
      cmp_state_conjunction_guards.operands());

  /* Update the SSA */
  /* Var map SHARED! */
  exprt cond = state_conjunction_guards;

  for (auto var_ptr : state->var_map.id_map)
  {
    var_mapt::var_infot &var=state->var_map.id_map[var_ptr.first];
    var_mapt::var_infot &cmp_var=cmp_state->var_map.id_map[var_ptr.first];


    if(state->get_var_state(var).ssa_symbol != var.ssa_symbol())
    {
      state_guarded_expression.operands().push_back(
          equal_exprt(state->get_var_state(var).ssa_symbol, var.ssa_symbol()));
      state->get_var_state(var).ssa_symbol=var.ssa_symbol();

//      exprt what=state->get_var_state(var).ssa_symbol;
//      exprt by=var.ssa_symbol();
//
//      replace_expr(what, by, state_guarded_expression);
    }

    if(cmp_state->get_var_state(cmp_var).ssa_symbol != cmp_var.ssa_symbol())
    {
      cmp_state_guarded_expression.operands().push_back(
          equal_exprt(cmp_state->get_var_state(cmp_var).ssa_symbol,
              cmp_var.ssa_symbol()));

//      exprt what=cmp_state->get_var_state(cmp_var).ssa_symbol;
//      exprt by=var.ssa_symbol();
//
//      replace_expr(what, by, cmp_state_guarded_expression);
    }

    if(state->get_var_state(var).value != cmp_state->get_var_state(cmp_var).value)
    {
      if_exprt ternary;

      ternary.cond() = cond;
      ternary.true_case() = state->get_var_state(var).value;
      ternary.false_case() = cmp_state->get_var_state(cmp_var).value;

      if(path_symext::propagate(ternary)
        &&
      base_type_eq(ternary.true_case(), ternary.false_case(), ns)) {
        state->get_var_state(var).value = ternary;
      } else {
        state->get_var_state(var).value = nil_exprt();
      }
    }
  }

  debug() << "Removed guard: " << from_expr(reverse_step->guard) << eom;

  exprt guarded_expression=or_exprt(simplify_expr(state_guarded_expression, ns),
      simplify_expr(cmp_state_guarded_expression, ns));

  /* Update history */
  /* Reverse step was moved by construct_guarded_expression! */
  reverse_step->guard=guarded_expression;
  reverse_step->full_lhs=true_exprt();
  reverse_step->ssa_lhs=symbol_exprt();
  reverse_step->ssa_rhs=nil_exprt();
//  reverse_step->arbitrary_expr=guarded_expression;

  /* Rewrite history */
  state->history=reverse_step;
  /* This step changes history! */

//  debug() << "New guard: " << from_expr(state->history->guard) << eom;

  /* Update statistics */
  // Max depth.
  state->set_depth(
      (state->get_depth() < cmp_state->get_depth()) ?
          state->get_depth() : cmp_state->get_depth());
}

void path_searcht::construct_guarded_expression(exprt::operandst &expr,
    int reverse_steps, path_symex_step_reft &reverse_step,
    exprt::operandst &guards)
{
  // Condition at bottom.
   while (true)
   {
     /* We could generate the guarded expression through backwards recursion. */
     /* TODO:  The above. */
     if(reverse_step->guard.is_not_nil()) {
       expr.push_back(reverse_step->guard);
       guards.push_back(reverse_step->guard);
     }

     if(reverse_step->ssa_rhs.is_not_nil()) {
       expr.push_back(equal_exprt(reverse_step->ssa_lhs, reverse_step->ssa_rhs));
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

bool path_searcht::calculate_qce_tot(goto_programt::const_targett &l) {
  goto_programt::const_targett l_jump;
  goto_programt::const_targett l_next;

  /* This function is used to perform a bottom-up calculation
   * of q_tot given some location within the program.
   *
   * It deliberately ignores function calls.
   *
   * If the required information to calculate q_tot is not available,
   * it returns False and queues this for later.
   *
   * Because this is bottom-up, END_FUNCTION has already been
   * assigned in the calling function and does not need to be
   * handled.
   *
   * The underlying code is assumed to be loop-free due to earlier unwinding.
   */

  /* The calculated equation is thus:
   *
   *  q(l') =
   *    {  b * q(succ(l')) + b *  q(l'') + 1 // for if(e) goto l''
   *    {  0  // for HALT or END_FUNCTION
   *    {  q(succ(l'))   // otherwise
   *
   *    In other words, for GOTOs, a fixed calculation is used.
   *    For HALT, 0 is returned (which is transfered up through the
   *    double-recursion tree, and for anything else, the existing
   *    value is propagated.
   *
   *    See Page 5 of Efficient State Merging paper.
   */

  switch(l->type) {
    case GOTO:

      assert(l->targets.size() == 1);

      if(l->guard.is_false())
      {
        l_next = l;
        l_next++;

        if(q_tot[l_next] < 0) {
          return false;
        }

        q_tot[l] = q_tot[l_next];

        return true;
      }

      if(l->guard.is_true())
      {
        l_jump = *(l->targets.begin());

        if(q_tot[l_jump] < 0) {
          return false;
        }

        q_tot[l]=q_tot[l_jump];

        return true;
      }

      assert(!l->guard.is_false() && !l->guard.is_true());

      l_next=l;
      l_next++;

      l_jump = *(l->targets.begin());

      if(q_tot[l_next] < 0 || q_tot[l_jump] < 0)
      {
        return false;
      }

      q_tot[l]= beta * q_tot[l_next] + beta * q_tot[l_jump] + 1;

      return true;

    case END_FUNCTION:
      assert(0); // Should be handled.
      break;

    default:
      if(l->targets.size() > 0) {
        assert(0); // Should be GOTO.
      }

      l_next = l;
      l_next++;

      if(q_tot[l_next] < 0) {
        return false;
      }

      if(l->type == ASSERT)
      {
        q_tot[l] = q_tot[l_next] + 1;
      } else {
        q_tot[l] = q_tot[l_next];
      }
      return true;
  }
}


bool path_searcht::calculate_qce_add(goto_programt::const_targett &l) {
  goto_programt::const_targett l_jump;
  goto_programt::const_targett l_next;
  switch(l->type) {
    case GOTO:

      assert(l->targets.size() == 1);

      if(l->guard.is_false())
      {
        l_next = l;
        l_next++;

        if(q_tot[l_next] < 0) {
          return false;
        }

        q_tot[l] = q_tot[l_next];

        return true;
      }

      if(l->guard.is_true())
      {
        l_jump = *(l->targets.begin());

        if(q_tot[l_jump] < 0) {
          return false;
        }

        q_tot[l]=q_tot[l_jump];

        return true;
      }

      assert(!l->guard.is_false() && !l->guard.is_true());

      l_next=l;
      l_next++;

      l_jump = *(l->targets.begin());

      if(q_tot[l_next] < 0 || q_tot[l_jump] < 0)
      {
        return false;
      }

      q_tot[l]= beta * q_tot[l_next] + beta * q_tot[l_jump] + 1;//dependencies();
      /* Massive to do */
      return true;

    case END_FUNCTION:
      assert(0); // Should be handled.
      break;

    default:
      if(l->targets.size() > 0) {
        assert(0); // Should be GOTO.
      }

      l_next = l;
      l_next++;

      if(q_tot[l_next] < 0) {
        return false;
      }

      if(l->type == ASSERT)
      {
        q_tot[l] = q_tot[l_next] + 1;
      } else {
        q_tot[l] = q_tot[l_next];
      }
      return true;
  }
}


void path_searcht::calculate_q_tot(const goto_functionst &goto_functions)
{

  forall_goto_functions(it_f, goto_functions)
  {
    if(!it_f->second.body_available() ||
        it_f->second.body.instructions.size() < 1)
    {
      continue;
    }

    auto it = it_f->second.body.instructions.end();
    it--;

    if(it->type != END_FUNCTION)
    continue;

    std::vector<goto_programt::const_targett> work;

    work.push_back(it);
    q_tot[it] = 0;

    int size;

    size = 0;

    while(work.size() > size)
    {
      size = work.size();

      for(auto w_it: work)
      {
        for(goto_programt::const_targett t_it : w_it->incoming_edges)
        {
          if(std::find(work.begin(), work.end(), t_it) == work.end()
              &&
              t_it->function == it->function)
          {
            work.push_back(t_it);
            q_tot[t_it] = -1;
          }
        }

      }
    }

    work.erase(work.begin()); // Remove the END_FUNCTION.  We've handled it.

    while(!work.empty())
    {
      goto_programt::const_targett item = work.front();
      work.erase(work.begin());

      if(!calculate_qce_tot(item))
      {
        work.push_back(item);
      }
    }
  }
}

unsigned path_searcht::add_symbol_table_to_goto_functions(
    goto_functionst &goto_functions)
{
  unsigned inserted_symbols=0;

  goto_programt::targett new_instruction;

  Forall_goto_functions(it_f, goto_functions)
  {
    if(!it_f->second.body_available() ||
        it_f->second.body.instructions.size() < 1)
    {
      continue;
    }

    Forall_goto_program_instructions(it, it_f->second.body)
    {
      if(relevant_location(it))
      {
        for(auto &symbols : ns.get_symbol_table().symbols)
        {

          if(has_prefix(symbols.second.display_name().c_str(), "__CPROVER")
              ||
              symbols.second.type.id() == ID_code)
          {
            continue;
          }

          if(symbols.second.type.id() == ID_array)
          {
            // Do array specific stuff after John's changes.
            continue;
          }

          new_instruction = it_f->second.body.insert_before(it);
          new_instruction->make_assignment();
          code_assignt code(symbols.second.symbol_expr(), symbols.second.symbol_expr());
          new_instruction->code=code;
          new_instruction->code.set(ID_generate_assign, ID_true);
          new_instruction->function=it->function;
          inserted_symbols++;
        }

//        for(auto incoming_edge : it->incoming_edges) {
//          if(incoming_edge->targets.size() > 0 &&
//              incoming_edge->get_target() == it) {
//            incoming_edge->set_target(new_instruction);
//          }
//        }
      }
    }
  }

  goto_functions.update();

  return inserted_symbols;
}


void path_searcht::take_transitive_closure()
{
  /* Take the transitive closure */
  bool fixedpoint = false;

  while(!fixedpoint)
  {
    fixedpoint=true;

    for(std::map<goto_programt::const_targett,
        dep_graph_domaint::depst>::reverse_iterator ptr =
        data_deps_in.rbegin();  ptr != data_deps_in.rend(); ptr++)
    {
      unsigned size = ptr->second.size();

      for(auto existing_target: ptr->second)
      {
        for(auto &new_target: data_deps_in[existing_target])
        {
          data_deps_in[ptr->first].insert(new_target);
        }
      }

      if(data_deps_in[ptr->first].size() > size)
      {
        fixedpoint=false;
      }
    }
  }

  for(std::map<goto_programt::const_targett,
      dep_graph_domaint::depst>::iterator ptr =
      data_deps_in.begin();  ptr != data_deps_in.end(); ptr++)
  {
    for (auto &item : ptr->second)
    {
      if((ptr->first->is_goto() || ptr->first->is_assert()))
//          && ptr->first->function==item->function)
      {
        data_deps_out[item].insert(ptr->first);
      }
    }
  }
}

void path_searcht::calculate_our_q_reaches(
    const goto_functionst &goto_functions, goto_functionst &new_goto_functions)
{

  forall_goto_functions(f_it, goto_functions)
  {
    if(!f_it->second.body_available())
    continue;

    auto insts =f_it->second.body.instructions;
    auto new_insts = new_goto_functions.function_map.find(f_it->first)->second.body.instructions;

    goto_programt::instructionst::const_iterator it=insts.begin();
    goto_programt::instructionst::const_iterator new_it=new_insts.begin();

    bool reached_end_of_function = false;

    while(!reached_end_of_function)
    {
      if(new_it == new_insts.end() || it == insts.end())
      {
        reached_end_of_function=true;
      }
      else
      {
        while(new_it->code.get(ID_generate_assign) == ID_true)
        {
          new_it++;
        }

        if(relevant_location(it))
        {
          /* Do some work*/
          std::cout << "Working at " << it->location_number << " - " << new_it->location_number << "\n";

          assert(relevant_location(new_it));

          goto_programt::instructionst::const_iterator data_extraction = new_it;

          data_extraction--;

          while(data_extraction->code.get(ID_generate_assign) == ID_true)
          {
            assert(data_extraction->code.get(ID_generate_assign) == ID_true);
            assert(data_extraction->is_assign());
            assert(data_extraction->code.op0() == data_extraction->code.op1());

            symbol_exprt symbol = to_symbol_expr(data_extraction->code.op0());

            double q_add_calculation = 0;

            for(auto finder : data_deps_out)
            {
              if(finder.first->location_number==data_extraction->location_number)
              {
                q_add_calculation=finder.second.size();
                break;
            }
          }

            location_symbol_pairt location_symbol_pair = std::make_pair(it->location_number, symbol.get_identifier());

            q_reaches[location_symbol_pair] = q_add_calculation;
            data_extraction--;
          }
        }

        it++;
        new_it++;
      }
    }
  }
}

void path_searcht::output_q_values(
    const goto_functionst &goto_functions)
{
  std::cout << "Outputting symbol reachability:\n";

  forall_goto_functions(f_it, goto_functions){
    if(!f_it->second.body_available())
      continue;

    forall_goto_program_instructions(p_it, f_it->second.body)
    {
      std::cout << p_it->location_number << " : " << q_tot[p_it] << "\n";

      for(auto &find : q_add) {
        if(find.first.first == p_it->location_number)
        {
          std::cout << p_it->location_number << " : " <<
              find.first.second << " : " << find.second << "\n";
        }
      }
    }
  }
}




void path_searcht::calculate_qce_hotsets(const goto_functionst &goto_functions)

{
  status() << "[QCE Hotsets] * Calculating QCE hotsets." << eom;

  goto_functionst new_goto_functions;
  new_goto_functions.copy_from(goto_functions);

  /* To do: Statically calculate merge-points */

  debug() << "[QCE Hotsets] * Inserting symbols at merge-point locations." << eom;

  add_symbol_table_to_goto_functions(new_goto_functions);

  debug() << "[QCE Hotsets] * Calculating dependence graph." << eom;

  dependence_grapht dependence_graph(ns);
  dependence_graph(new_goto_functions, ns);

  debug() << "[QCE Hotsets] * Generating backwards edge graph." << eom;

  Forall_goto_functions(f_it, new_goto_functions){
    if(!f_it->second.body_available())
      continue;

    Forall_goto_program_instructions(p_it, f_it->second.body)
    {
      data_deps_in[p_it]=dependence_graph[p_it].get_data_deps();
    }
  }

  debug() << "[QCE Hotsets] * Taking transitive closure." << eom;

  take_transitive_closure();

  locst locs_(ns);
  locs_.build(new_goto_functions);
  locs_.output(std::cout);

//  debug() << "[QCE Hotsets] * Calculating reached branches." << eom;

//  Forall_goto_functions(f_it, new_goto_functions){
//    if(!f_it->second.body_available())
//      continue;
//
//    Forall_goto_program_instructions(it, f_it->second.body)
//    {
//       branches_hit[it]=-1;
//    }
//
//    goto_programt::const_targett start_inst = f_it->second.body.instructions.begin();
//    branches_hit[start_inst]=0;
//    calculate_branches(start_inst);
//  }


  status() << "[QCE Hotsets] * Calculating Q_Tot." << eom;

  calculate_q_tot(goto_functions);

  debug() << "[QCE Hotsets] * Calculating symbol reachability." << eom;
//
//  calculate_symbol_reachability(goto_functions, new_goto_functions, true);
//
//  debug() << "[QCE Hotsets] * Hotset calculation complete. *" << eom;
//
//  output_q_values(goto_functions);
}



void path_searcht::calculate_our_qce_hotsets(
    const goto_functionst &goto_functions)

{
  status() << "Calculating our QCE hotsets (method 1)." << eom;

  goto_functionst new_goto_functions;
  new_goto_functions.copy_from(goto_functions);

  add_symbol_table_to_goto_functions(new_goto_functions);

  debug() << "Calculating dependence graph." << eom;

  dependence_grapht dependence_graph(ns);
  dependence_graph(new_goto_functions, ns);

  Forall_goto_functions(f_it, new_goto_functions){
    if(!f_it->second.body_available())
      continue;

    Forall_goto_program_instructions(p_it, f_it->second.body)
    {
      data_deps_in[p_it]=dependence_graph[p_it].get_data_deps();
    }
  }

  take_transitive_closure();

  calculate_our_q_reaches(goto_functions, new_goto_functions);
}


//
//void path_searcht::calculate_branches(goto_programt::const_targett &location)
//{
//  std::vector<goto_programt::const_targett> work;
//
//  /* This relies on the program being LOOP-FREE. */
//
//  work.push_back(location);
//
//  std::cout << location->location_number<<"\n";
//
//  goto_programt::const_targett l;
//  goto_programt::const_targett new_l;
//
//  while(!work.empty())
//  {
//    l=work.back();
//
//    assert(!l->is_backwards_goto());
//
//    std::cout << l->location_number << "\n";
//
//    /* Take the last element */
//    unsigned branches_at_location=branches_hit[l];
//
//    switch(l->type)
//    {
//      case GOTO:
//      {
//        /* Must have a target!  We *specifically ignore*
//         * whether we can determine through constant
//         * prop whether it's true or false.
//         *
//         * Every branch is possible either way! */
//        assert(l->targets.size() == 1);
//
//
//        std::cout << l->location_number << " -- ";
//
//
//        new_l=l->get_target();
//        (work.back())++;
//
//        std::cout << work.back()->location_number << " -- ";
//
//        branches_hit[work.back()]=branches_at_location+1;
//
//        work.push_back(new_l);
//        branches_hit[new_l]=branches_at_location+1;
//
//        std::cout << work.back()->location_number << "\n";
//
//        std::cout << "GOTO: " << work.size() << "\n";
//
//        break;
//      }
//
//      case END_FUNCTION:
//      {
//        work.pop_back();
//
//
//        std::cout << "END_FUNC: " << work.size() << "\n";
//        break;
//      }
//
//      default:
//      {
//        (work.back())++;
//        branches_hit[work.back()]=branches_at_location+1;
//
//        std::cout << "DEFAULT: " << work.size() << "\n";
//
//      }
//    }
//  }

//  std::vector<goto_programt::const_targett> work;
//
//  work.push_back(location);
//
//  while (!work.empty())
//  {
//    goto_programt::const_targett l=work.back();
//    work.pop_back();
//
//    /* deliberately flow-"insensitive" */
//    unsigned current_branches = branches_hit[l];
//
//    switch (l->type)
//    {
//      case GOTO:
//      {
//        assert(l->targets.size() == 1);
//
//        goto_programt::const_targett jump;
//        jump=l;
//        jump=l->get_target();
//
//        l++;
//
//        work.push_back(l);
//        branches_hit[l]=current_branches+1;
//        work.push_back(jump);
//        branches_hit[jump]=current_branches+1;
//        break;
//      }
//
//      case END_FUNCTION:
//        /* Remove worker */
//        break;
//        /* Deliberately ignore function call */
//
//      default:
//        assert(l->targets.size() == 0); // Should be GOTO.
//        l++;
//        branches_hit[l]=current_branches;
//        work.push_back(l);
//    }
//  }
//}

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

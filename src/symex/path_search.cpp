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
#include <analyses/dependence_graph.h>



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

//  calculate_hotsets(goto_functions);
//
//  return SAFE;

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

      std::cout << state.get_pc().loc_number << " " << state.get_instruction()->type << " -- Number of states: " << queue.size() << "\n";

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
//
//  std::cout << "Left hand side.\n";
//
//  int i = 0;
//  for(auto it: state_history) {
//    if(it->ssa_rhs.is_not_nil())
//      std::cout << ++i << " : "
//          << equal_exprt(it->ssa_lhs, it->ssa_rhs).pretty() << "\n\n";
//    if(it->guard.is_not_nil())
//      std::cout << ++i << " : "
//          << it->guard.pretty() << "\n\n";
//  }

//  std::cout << "Right hand side.\n";
//
//  i = 0;
//   for(auto it: cmp_state_history) {
//     if(it->ssa_rhs.is_not_nil())
//       std::cout << ++i << " : "
//           << equal_exprt(it->ssa_lhs, it->ssa_rhs).pretty() << "\n\n";
//     if(it->guard.is_not_nil())
//       std::cout << ++i << " : "
//           << it->guard.pretty() << "\n\n";
//   }




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

//  std::cout << "Guarded expression\n\n:";

  construct_guarded_expression(cmp_state_guarded_expression.operands(),
      cmp_state_history.size() - steps, cmp_reverse_step,
      cmp_state_conjunction_guards.operands());

  /* Update the SSA */
  /* Var map SHARED! (Combine to one loop, but not yet -- readability) */
  for (auto var_ptr : state->var_map.id_map)
  {
    var_mapt::var_infot &var=state->var_map.id_map[var_ptr.first];

    if(state->get_var_state(var).ssa_symbol != var.ssa_symbol())
    {
//      replace_expr(state->get_var_state(var).ssa_symbol, var.ssa_symbol(), state_guarded_expression);

      state_guarded_expression.operands().push_back(
          equal_exprt(state->get_var_state(var).ssa_symbol, var.ssa_symbol()));
      state->get_var_state(var).ssa_symbol=var.ssa_symbol();
    }
  }

  for (auto var_ptr : cmp_state->var_map.id_map)
  {
    var_mapt::var_infot &var=cmp_state->var_map.id_map[var_ptr.first];

    if(cmp_state->get_var_state(var).ssa_symbol != var.ssa_symbol())
    {
//      replace_expr(cmp_state->get_var_state(var).ssa_symbol, var.ssa_symbol(), cmp_state_guarded_expression);
      cmp_state_guarded_expression.operands().push_back(
          equal_exprt(cmp_state->get_var_state(var).ssa_symbol,
              var.ssa_symbol()));
    }
  }

  exprt guarded_expression=or_exprt(state_guarded_expression,
      cmp_state_guarded_expression);

//  std::cout << guarded_expression.pretty() << "\n\n";

//  guarded_expression=simplify_expr(guarded_expression, ns);

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

//  state->history.build_history(state_history);
//  i = 0;
//  for(auto it: state_history) {
//    if(it->ssa_rhs.is_not_nil())
//      std::cout << ++i << " : "
//          << equal_exprt(it->ssa_lhs, it->ssa_rhs).pretty() << "\n\n";
//    if(it->guard.is_not_nil())
//      std::cout << ++i << " : "
//          << it->guard.pretty() << "\n\n";
//  }

  /* Update internal variable mappings */

  // a and !b
//  exprt cond=and_exprt(state_conjunction_guards,
//      not_exprt(cmp_state_conjunction_guards));
//  exprt cond = state_conjunction_guards;

//  cond=simplify_expr(cond, ns);

  /* We do not handle concurrency yet. */




//  for (auto cmp_var_ptr : cmp_state->var_map.id_map)
//  {
//    var_mapt::var_infot &var = state->var_map.id_map[cmp_var_ptr.first];
//    var_mapt::var_infot &cmp_var = cmp_var_ptr.second;
//
//    std::cout << cmp_var_ptr.first << " MAINTAINED: " << var.ssa_identifier()
//        << " AND " << state->get_var_state(var).ssa_symbol.get_identifier() << "\n";
////    << " VS "
////        << " REMOVED" << cmp_var.ssa_identifier() << " AND "
////        << cmp_state->get_var_state(cmp_var).ssa_symbol.get_identifier() << "\n";
//
//
//
//  }


//  std::cout << "Size!: " << state->var_map.id_map.size() << "\n";
//
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
////      std::cout << phi_node.pretty() << "\n\n\n\n\n\n\n\n\n______\n\n\n\n\n\n";
//    }
//  }

  /* Update statistics */
  // Max depth.
  state->set_depth(
      (state->get_depth() < cmp_state->get_depth()) ?
          state->get_depth() : cmp_state->get_depth());
  // ...
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

     if(reverse_step->arbitrary_expr.is_not_nil()) {
       expr.push_back(reverse_step->arbitrary_expr);
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

void path_searcht::calculate_hotset(
    unsigned location,
    const goto_functionst &goto_functions,
    std::vector<symbolt> &symbols)
{
  // q(l, c) = b * q(succ(l', c) + b * q(l'', c) + c(l', e).
  // 0 halt
  // q(succ(l', c)) else

  // A recursive calculation here would kill everything.
  path_symex_statet &state = queue.back();

  dependence_grapht dependence_graph(ns);
  dependence_graph(goto_functions, ns);

  forall_goto_functions(f_it, goto_functions)
  {
    if(f_it->second.body_available())
    {
      std::cout << "////" << std::endl;
      std::cout << "//// Function: " << f_it->first << std::endl;
      std::cout << "////" << std::endl;
      std::cout << std::endl;
      dependence_graph.output(ns, f_it->second.body, std::cout);
    }
  }

  dependence_graph.output_dot(std::cout);

  forall_goto_functions(f_it, goto_functions)
  {
    if(!f_it->second.body_available())
      continue;

    forall_goto_program_instructions(it, f_it->second.body) {
      std::cout << "At: " << it->location_number << " control: ";
      for(auto control : dependence_graph[it].get_control_deps()) {
        std::cout << control->location_number << ",";
      }
      std::cout << ".  Data: ";
      for(auto data : dependence_graph[it].get_data_deps()) {
        std::cout << data->location_number << ",";
      }


      std::cout << "\n";
    }
  }

  return;


  std::map<unsigned int, searchert> searchers;
  std::map<unsigned int, searchert> completed_searchers;

  unsigned int states = 0;

  searchert searcher(location);
  searchers.insert(std::make_pair(states++, searcher));

  std::cout << "Locs:" << locs.loc_vector.size() << " from " << location << "\n";

  while(!searchers.empty()) {
    std::map<unsigned int, searchert>::iterator item = searchers.begin();

    unsigned id = item->first;
    searchert current = item->second;

    searchers.erase(searchers.begin());

    std::cout << locs[current.location].target->type << " at " << current.location << "\n";


    switch(locs[current.location].target->type) {
      case GOTO:
      {
        const exprt &guard = locs[current.location].target->guard;


        if(locs[current.location].target->is_backwards_goto()) {
          current.loop_count++;
          if(current.loop_count > 0) {
            current.work = false;
            completed_searchers.insert(std::make_pair(id, current));
            std::cout << "Backwards at " << current.location << "\n";
            break;
          }
        }

        if(guard.is_true())
        {
          assert(!locs[current.location].target->targets.empty());

          current.location=
              (*locs[current.location].target->targets.begin())->location_number;
          searchers.insert(std::make_pair(id, current));

          std::cout << "(Always true -> " << current.location << ")\n";
          break;
        }

        if(guard.is_false())
        {
          current.location++;
          searchers.insert(std::make_pair(id, current));

          std::cout << "(Always false -> " << current.location << ")\n";

          break;
        }

        std::cout << state.read_no_propagate(guard).pretty();

        searchert lhs = current;
        searchert rhs = current;

        assert(!locs[current.location].target->targets.empty());

        lhs.location = (*locs[current.location].target->targets.begin())->location_number;
        rhs.location++;

//        std::cout << "(Could be both: LHS -> " << lhs.location << " -- RHS ->" << rhs.location << "\n";

        current.branches_found++;

        unsigned lhs_id = states++;
        unsigned rhs_id = states++;

        current.lhs = lhs_id;
        current.rhs = rhs_id;

        lhs.branches++;
        rhs.branches++;

        current.work = false;

        /* Calculate value */
//        current.value = current.value + 1;

        completed_searchers.insert(std::make_pair(id, current));
        searchers.insert(std::make_pair(lhs_id, lhs));
        searchers.insert(std::make_pair(rhs_id, rhs));

        break;
      }
      case FUNCTION_CALL:
      {
        code_function_callt function_call=to_code_function_call(
            locs[current.location].target->code);

        if(function_call.function().id() == ID_symbol)
        {
          const irep_idt &function_identifier=
              to_symbol_expr(function_call.function()).get_identifier();

          // find the function
          locst::function_mapt::const_iterator f_it=locs.function_map.find(
              function_identifier);

          if(f_it == locs.function_map.end())
            throw "failed to find `" + id2string(function_identifier)
                + "' in function_map";

          const locst::function_entryt &function_entry=f_it->second;

          loc_reft function_entry_point=function_entry.first_loc;

          // do we have a body?
          if(function_entry_point == loc_reft())
          {
            current.location++;
            searchers.insert(std::make_pair(id, current));
            // no body

            // this is a skip
            break;
          }

          current.returns.push_back(current.location + 1);
          current.location=function_entry_point.loc_number;
          searchers.insert(std::make_pair(id, current));
        }
        else
        {
          throw "Function w/o symbol.";
        }
        break;
      }


      case END_FUNCTION:
        if(current.returns.empty())
        {
          current.work = false;
          completed_searchers.insert(std::make_pair(id, current));
          break;
        }

        current.location=current.returns.back();
        current.returns.pop_back();
        searchers.insert(std::make_pair(id, current));

        break;

      default:
        current.location++;
        searchers.insert(std::make_pair(id, current));
        break;
    }
  }

  std::cout << "number of completed workers: " << completed_searchers.size() << "\n";
  for(auto it:completed_searchers) {
    std::cout << it.first << ":" << it.second.location << " - " << it.second.branches_found << "\n";
  }

  /* q_tot */
  double q_tot = 0;

  std::map<unsigned int, searchert>::iterator item;
  item = completed_searchers.begin();

  std::vector<unsigned> to_add;
  to_add.push_back(item->first);

  while(!to_add.empty()) {
    unsigned ptr_id = to_add.back();
    to_add.pop_back();

    item = completed_searchers.find(ptr_id);

    q_tot +=
        std::pow(0.8, item->second.branches) * item->second.branches_found;

    if(item->second.lhs > -1 && item->second.rhs > -1)
    {
      to_add.push_back(item->second.lhs);
      to_add.push_back(item->second.rhs);
    }
  }

  std::cout << "\n\nSymbol table.\n";

  for (auto symbol : ns.get_symbol_table().symbols)
  {
    /* For now */
    if(has_prefix(symbol.first.c_str(), "__CPROVER"))
      continue;



    std::cout << symbol.first << "\n";
  }

  std::cout << "q_tot:" << q_tot << "\n";
}



void path_searcht::calculate_hotsets(const goto_functionst &goto_functions)

{
  std::cout << "Calculating hotsets\n";

  goto_functionst goto_functions_copy;
  goto_functions_copy.copy_from(goto_functions);

  forall_goto_functions(it_f, goto_functions){

  if(!it_f->second.body_available() ||
      it_f->second.body.instructions.size() < 1)
  {
    continue;

  }

  std::vector<symbolt> symbols;
//      calculate_hotset(it->location_number, goto_functions, symbols);
  calculate_hotset(it_f->second.body.instructions.begin()->location_number, goto_functions, symbols);
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
    state.read_no_propagate(instruction.guard);

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
//    bv_pointers.print_assignment(std::cout);
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

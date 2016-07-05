/*
 * path_symex_hoisting.cpp
 *
 *  Created on: 29 Jun 2015
 *      Author: danlle
 */

#include <iostream>
#include "path_symex_hoisting.h"
#include "step_wp.h"
#include <util/std_code.h>

#include <util/simplify_expr.h>



path_symex_hoistingt::path_symex_hoistingt() {
  // TODO Auto-generated constructor stub

}

path_symex_hoistingt::~path_symex_hoistingt() {
  // TODO Auto-generated destructor stub
}

bool path_symex_hoistingt::terminate_early(
    path_symex_step_reft &history_step,
    locst &locs) {
  if(locs[history_step->pc].target->is_goto()) {
    // branch.
    std::cout << "BREAK ON BRANCH " << history_step->pc << "\n";
    return true;
  }

  return false;
}

bool path_symex_hoistingt::hoist_further(
    path_symex_step_reft &history_step,
    locst &locs) {

  if(history_step.is_nil()) {
    return false;
  }

  if(locs[history_step->pc].target->is_function_call()) {
    std::cout << "BREAK ON FUNC CALL: " << history_step->pc << " \n";
    return false; // went too far.
  }
  return true;
}



bool path_symex_hoistingt::hoist(
    exprt &assertion,
    loc_reft &pc,
    std::map<loc_reft, exprt> &hoisted_asserts,
    path_symex_statet &state,
    locst &locs,
    const namespacet &ns)
{
  std::cout << "!!! Entering hoist function (PC: " << state.pc() << ") !\n";


  std::cout << "\nWritten to:";

  assert(state.get_instruction()->is_assert());
  exprt wp = assertion;
  path_symex_step_reft history_step = state.history;


  do {
     pc = history_step->pc;

     if(hoisted_asserts[pc].is_false()) {
       wp = false_exprt();
     } else {
       if(!hoisted_asserts[pc].is_true()) {
         wp = simplify_expr(and_exprt(wp, hoisted_asserts[pc]), ns);
       }
       wp = step_wp(*history_step, wp, ns);
     }

     hoisted_asserts[pc] = wp;

     std::cout << pc << ",";

//     std::cout << "PC: " << pc << "\n" << wp.pretty() << "\n\n\n";

     if(terminate_early(history_step, locs))
       break;

    --history_step;
  } while(hoist_further(history_step, locs));

  assertion = wp;

  /* TO DO:
   * GIven PC, find instruction
   * If irrelevant, bring the assertion forwards one more.
   * Repeat until can't bring forward (contains checks?)
   */

  return false;
}


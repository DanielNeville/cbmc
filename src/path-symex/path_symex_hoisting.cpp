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



path_symex_hoisting::path_symex_hoisting() {
  // TODO Auto-generated constructor stub

}

path_symex_hoisting::~path_symex_hoisting() {
  // TODO Auto-generated destructor stub
}

void path_symex_hoisting::insert_skips(goto_functionst &goto_functions) {
  for(goto_functionst::function_mapt::iterator
      it_f=goto_functions.function_map.begin();
      it_f !=goto_functions.function_map.end();
      it_f++)
  {
    goto_programt &goto_program=it_f->second.body;

    for(goto_programt::instructionst::iterator
        it=goto_program.instructions.begin();
        it!=goto_program.instructions.end();
        it++)
    {
      bool previous = false;

      if(it->is_assert() || it->is_assume() || it->is_skip() || it->code.get(ID_hoist) == ID_hoist) {
        it->code.set(ID_hoist, ID_hoist);
        previous = true;
      } else {
        if(!previous) {
          goto_programt::targett t=
              goto_program.insert_before(it);
          t->make_skip();
          t->code.set(ID_hoist, ID_hoist);
          previous = false;
        }
      }
    }
  }
}

unsigned path_symex_hoisting::calculate_hoist_steps(
    path_symex_statet &state,
    locst &locs,
    const namespacet &ns) {

  unsigned steps = 0;

  path_symex_step_reft history_step = state.history;

  std::cout << steps << " - " << history_step->pc << "\n";

  path_symex_step_reft previous;

  do{

    if(locs[history_step->pc].target->is_function_call()) {
      std::cout << "BREAK ON FUNC CALL: " << history_step->pc << " \n";
      return steps - 1; // went too far.

    }

    if(locs[history_step->pc].target->code.get(ID_merge_point) == ID_merge_point && steps > 0) {
      std::cout << "MERGE POINT AT " << history_step->pc << " -- RETURNING " << steps << "\n";
      return steps;
    }
    // before step back

//    previous = history_step;
    --history_step;
    steps++;
    // step back

  } while(!history_step.is_nil());

  return steps;
}



bool path_symex_hoisting::hoist(
    exprt &assertion,
    loc_reft &pc,
    std::map<loc_reft, exprt> &hoisted_asserts,
    unsigned steps,
    path_symex_statet &state,
    const namespacet &ns)
{
  std::cout << "HOIST!\n";

  assert(state.get_instruction()->is_assert());

  exprt wp = assertion;

  //  exprt assertion = state.get_instruction()->guard.op0();

  path_symex_step_reft history_step = state.history;

  unsigned iterations = 0;

  bool repeat = false;

  do {
    iterations++;
    //    std::cout << "*********** ITERATION " << iterations << " ************\n";

    if(repeat) {
      --history_step;
      repeat = false;
    }

    if(history_step.is_nil()) {
      break;
    }

    std::cout << "PC" << history_step->pc << "\n";
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


    if(iterations < steps) {
      repeat = true;
    }

    //    while((history_step)->ssa_lhs.is_nil()){ /* In case of emtpy steps */
    //      history_step.operator --();
    //    }

    //    std::cout << "STEP:\n";
    //    std::cout << history_step->ssa_lhs.pretty() << "\n === \n" << history_step->ssa_rhs.pretty() << "\n\n";
    //    std::cout << "ASSERTION:\n" << assertion.pretty() << "\n\n";

    //  std::cout << "STEP_WP:\n" << step_wp(*history_step, assertion, ns).pretty() << "\n\n";

    //    std::cout << "END HOIST!\n";


  } while(repeat);

  std::cout << "TOTAL ITERATIONS:" << iterations << "\n";

  assertion = wp;

  /* TO DO:
   * GIven PC, find instruction
   * If irrelevant, bring the assertion forwards one more.
   * Repeat until can't bring forward (contains checks?)
   */

  return false;
}


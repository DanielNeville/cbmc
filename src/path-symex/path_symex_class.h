/*******************************************************************\

Module: Concrete Symbolic Transformer

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_PATH_SYMEX_CLASS_H
#define CPROVER_PATH_SYMEX_CLASS_H

#include "path_symex.h"

class path_symext
{
public:
  inline path_symext()
  {
  }

  virtual void operator()(
    path_symex_statet &state,
    std::list<path_symex_statet> &furter_states);

  virtual void operator()(path_symex_statet &state);

  void do_goto(
    path_symex_statet &state,
    bool taken);
    
  virtual void do_assert_fail(path_symex_statet &state)
  {
    const goto_programt::instructiont &instruction=
      *state.get_instruction();
    
    state.record_step();
    state.next_pc();
    exprt guard=state.read(not_exprt(instruction.guard));
    state.history->guard=guard;
  }  
  
  typedef path_symex_stept stept;

protected:
  void do_goto(
    path_symex_statet &state,
    std::list<path_symex_statet> &further_states);

  void function_call(
    path_symex_statet &state,
    const code_function_callt &call,
    std::list<path_symex_statet> &further_states)
  {
    exprt f=state.read(call.function());
    function_call_rec(state, call, f, further_states);
  }
    
  void function_call_rec(
    path_symex_statet &state,
    const code_function_callt &function_call,
    const exprt &function,
    std::list<path_symex_statet> &further_states);
    
  void return_from_function(path_symex_statet &state);
  
  void set_return_value(path_symex_statet &, const exprt &);

  void symex_malloc(
    path_symex_statet &state,
    const exprt &lhs,
    const side_effect_exprt &code);

  void assign(
    path_symex_statet &state,
    const exprt &lhs,
    const exprt &rhs);

  inline void assign(
    path_symex_statet &state,
    const code_assignt &assignment)
  {
    assign(state, assignment.lhs(), assignment.rhs());
  }

  void assign_rec(
    path_symex_statet &state,
    exprt::operandst &guard, // instantiated
    const exprt &ssa_lhs, // instantiated, recursion here
    const exprt &ssa_rhs); // instantiated


  inline void recursive_taint_extraction(const exprt &expr,
      taintt &taint, path_symex_statet &state) {
//    std::cout << "Find_taint ENTERED!  Operand count : " << expr.operands().size() << "\n";

    if(expr.id() == ID_symbol) {
//      std::cout << "Entering symbol branch.\n";
      symbol_exprt symbol = to_symbol_expr(expr);
      const irep_idt &full_identifier=symbol.get(ID_C_full_identifier);
      var_mapt::var_infot &var_info=state.var_map[full_identifier];
      assert(var_info.full_identifier==full_identifier);
      path_symex_statet::var_statet &var_state=state.get_var_state(var_info);
      taint = state.taint_engine.meet(expr.id(), taint, var_state.taint);
    }

    forall_operands(it, expr) {
      recursive_taint_extraction(*it, taint, state);
    }
  }

  static bool propagate(const exprt &src);
};


#endif

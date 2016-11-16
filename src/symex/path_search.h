/*******************************************************************\

Module: Path-based Symbolic Execution

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_PATH_SEARCH_H
#define CPROVER_PATH_SEARCH_H

#include <util/time_stopping.h>
#include <util/expanding_vector.h>
#include <tuple>

#include <goto-programs/safety_checker.h>

#include <path-symex/path_symex_state.h>
#include <path-symex/path_symex_class.h>

// Todo: Move
typedef std::vector<path_symex_step_reft> state_historyt;
#include <analyses/dependence_graph.h>


class path_searcht:public safety_checkert
{
public:
  explicit inline path_searcht(const namespacet &_ns):
    safety_checkert(_ns),
    show_vcc(false),
    eager_infeasibility(false),
    locs(_ns),
    depth_limit_set(false), // no limit
    context_bound_set(false),
    unwind_limit_set(false),
    branch_bound_set(false),
    qce_set(false),
    search_heuristic(search_heuristict::DFS),
    merge_heuristic(merge_heuristict::NONE)
  {
  }

  virtual resultt operator()(
    const goto_functionst &goto_functions);
    
  void set_depth_limit(unsigned limit)
  {
    depth_limit_set=true;
    depth_limit=limit;
  }

  void set_context_bound(unsigned limit)
  {
    context_bound_set=true;
    context_bound=limit;
  }

  void set_branch_bound(unsigned limit)
  {
    branch_bound_set=true;
    branch_bound=limit;
  }

  void set_unwind_limit(unsigned limit)
  {
    unwind_limit_set=true;
    unwind_limit=limit;
  }

  bool show_vcc;
  bool eager_infeasibility;
  
  locst locs;

  // statistics
  unsigned number_of_dropped_states;
  unsigned number_of_paths;
  unsigned number_of_steps;
  unsigned number_of_feasible_paths;
  unsigned number_of_infeasible_paths;
  unsigned number_of_VCCs;
  unsigned number_of_VCCs_after_simplification;
  unsigned number_of_failed_properties;
  unsigned number_of_merged_states;
  std::size_t number_of_locs;
  absolute_timet start_time;
  time_periodt sat_time;

  enum statust { NOT_REACHED, SUCCESS, FAILURE };

  struct property_entryt
  {
    statust status;
    irep_idt description;
    goto_tracet error_trace;
    source_locationt source_location;
    
    inline bool is_success() const { return status==SUCCESS; }
    inline bool is_failure() const { return status==FAILURE; }
    inline bool is_not_reached() const { return status==NOT_REACHED; }
  };
  
  inline void set_dfs() { search_heuristic=search_heuristict::DFS; }
  inline void set_bfs() { search_heuristic=search_heuristict::BFS; }
  inline void set_locs() { search_heuristic=search_heuristict::LOCS; }
  // Really should support FF & DFS/BFS/etc.
  inline void set_fast_forward() { search_heuristic=search_heuristict::FAST_FORWARD; }
  
  inline void set_aggressive_merging() { merge_heuristic=merge_heuristict::AGGRESSIVE; }

  inline void set_qce()
  {
    qce_set=true;
    search_heuristic=search_heuristict::FAST_FORWARD;
    merge_heuristic=merge_heuristict::QCE;
  }

  typedef std::map<irep_idt, property_entryt> property_mapt;
  property_mapt property_map;

  typedef path_symex_statet statet;
  typedef std::list<statet> queuet;

protected:

  // State queue. Iterators are stable.
  // The states most recently executed are at the head.
  queuet queue;
  
  // search heuristic
  void pick_state();

  struct loc_datat
  {
    bool visited;
    loc_datat():visited(false) { }
  };

  expanding_vector<loc_datat> loc_data;
  
  bool execute(queuet::iterator state);
  
  void check_assertion(statet &state);
  bool is_feasible(statet &state);
  void do_show_vcc(statet &state);
  
  bool drop_state(const statet &state) const;
  
  void report_statistics();
  
  void initialize_property_map(
    const goto_functionst &goto_functions);

  unsigned depth_limit;
  unsigned context_bound;
  unsigned branch_bound;
  unsigned unwind_limit;
  bool depth_limit_set, context_bound_set, unwind_limit_set, branch_bound_set;
  bool qce_set;

  enum class search_heuristict { DFS, BFS, LOCS, FAST_FORWARD } search_heuristic;
  enum class merge_heuristict { AGGRESSIVE, NONE, QCE } merge_heuristic;

  void merge_states();

  /* Following to be moved to separate class? */
  bool do_aggressive_merge(
      queuet::iterator &state, queuet::iterator &current_state);

  bool do_qce_merge(
      queuet::iterator &state, queuet::iterator &cmp_state);

  void merge(
      queuet::iterator &state, queuet::iterator &cmp_state);

  bool inline at_merge_point(
      statet &state)
  {
    return locs[state.pc()].target->incoming_edges.size() > 1;
  }

  void construct_guarded_expression(exprt::operandst &expr,
      int reverse_steps, path_symex_step_reft &reverse_step,
      exprt::operandst &guards);

  void calculate_hotsets(const goto_functionst &goto_functions);

  void calculate_q_tot(const goto_functionst &goto_functions);

  void take_transitive_closure();

  unsigned add_symbol_table_to_goto_functions(
      goto_functionst &goto_functions);

  void calculate_symbol_reachability(
      const goto_functionst &goto_functions,
      goto_functionst &new_goto_functions);

  void output_q_values(
      const goto_functionst &goto_functions);

  std::map<goto_programt::const_targett,
    dep_graph_domaint::depst> data_deps_in;

  std::map<goto_programt::const_targett,
    dep_graph_domaint::depst> data_deps_out;

  std::map<goto_programt::const_targett, unsigned> branches_hit;

  std::map<goto_programt::const_targett, double> q_tot;
  typedef std::pair<unsigned, irep_idt> location_symbol_pairt;
  std::map<location_symbol_pairt, double> q_add;

  double alpha = 0.000000000001; // 10^-12.
  double beta = 0.8;

  bool calculate_qce_tot(goto_programt::const_targett &l);
  bool calculate_qce_add(goto_programt::const_targett &l);
  void calculate_branches(goto_programt::const_targett &location,
      std::map<goto_programt::const_targett, unsigned> &branches_hit);

  bool relevant_location(goto_programt::const_targett &l)
  {
    return l->incoming_edges.size() > 1;
  }

  bool relevant_location(goto_programt::targett &l)
  {
    return l->incoming_edges.size() > 1;
  }
};

#endif

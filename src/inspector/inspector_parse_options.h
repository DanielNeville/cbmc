/*******************************************************************\

Module: Inspector Command Line Option Processing

Author:

\*******************************************************************/

#ifndef CPROVER_CBMC_PARSEOPTIONS_H
#define CPROVER_CBMC_PARSEOPTIONS_H

#include <util/ui_message.h>
#include <util/parse_options.h>

#include <util/time_stopping.h>
#include <util/expanding_vector.h>

#include <goto-programs/safety_checker.h>

#include <path-symex/path_symex_state.h>

#include <langapi/language_ui.h>

#include "inspector_util.h"

#include <symex/path_search.h>


class bmct;
class goto_functionst;
class optionst;

#define INSPECTOR_OPTIONS \
  "(function):" \
  "D:I:(std89)(std99)(std11)" \
  "(classpath):(cp):(main-class):" \
  "(16)(32)(64)(LP64)(ILP64)(LLP64)(ILP32)(LP32)" \
  "(little-endian)(big-endian)" \
  "(show-goto-functions)(show-loops)" \
  "(show-symbol-table)(show-parse-tree)" \
  "(show-properties)(show-reachable-properties)(property):" \
  "(verbosity):(version)" \
  "(gcc)(arch):" \
  "(taint):(show-taint)" \
  "(show-local-may-alias)" \
  "(json):(xml):" \
  "(taint-locations):(show-taint-locations)(strict-rules)" \
  "(entry-locations):(show-entry-locations)" \
  "(output-automata):" \
  "(unreachable-instructions)" \
  "(intervals)(show-intervals)" \
  "(non-null)(show-non-null)" \
  "(show-locs)"

class inspector_parse_optionst:
  public parse_options_baset,
  public language_uit
{
public:
  virtual int doit();
  virtual void help();

  inspector_parse_optionst(int argc, const char **argv);

protected:
  virtual void register_languages();

  virtual void get_command_line_options(optionst &options);

  virtual int get_goto_program(
    const optionst &options,
    goto_functionst &goto_functions);

  virtual bool process_goto_program(
    const optionst &options,
    goto_functionst &goto_functions);
    
  bool set_properties(goto_functionst &goto_functions);
  

  void report_success();
  void report_failure();
  void report_properties(const path_searcht::property_mapt &);
  void show_counterexample(const class goto_tracet &);

  void eval_verbosity();
  
  bool has_entry_point;
};



#endif

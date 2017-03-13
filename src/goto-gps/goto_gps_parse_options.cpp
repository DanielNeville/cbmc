/*******************************************************************\

Module: Goto-GPS Command Line Option Processing

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <cstdlib> // exit()
#include <iostream>
#include <fstream>
#include <memory>

#include <ansi-c/ansi_c_language.h>
#include <cpp/cpp_language.h>
#include <java_bytecode/java_bytecode_language.h>
#include <jsil/jsil_language.h>

#include <goto-programs/set_properties.h>
#include <goto-programs/remove_function_pointers.h>
#include <goto-programs/remove_virtual_functions.h>
#include <goto-programs/remove_returns.h>
#include <goto-programs/remove_vector.h>
#include <goto-programs/remove_complex.h>
#include <goto-programs/remove_asm.h>
#include <goto-programs/goto_convert_functions.h>
#include <goto-programs/show_properties.h>
#include <goto-programs/show_symbol_table.h>
#include <goto-programs/read_goto_binary.h>
#include <goto-programs/write_goto_binary.h>
#include <goto-programs/goto_inline.h>
#include <goto-programs/link_to_library.h>

#include <analyses/goto_check.h>
#include <analyses/local_may_alias.h>

#include <langapi/mode.h>

#include <util/language.h>
#include <util/options.h>
#include <util/config.h>
#include <util/string2int.h>
#include <util/unicode.h>

#include <cbmc/version.h>

#include "goto_gps_parse_options.h"

/*******************************************************************\

Function: goto_gps_parse_optionst::goto_analyzer_parse_optionst

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

goto_gps_parse_optionst::goto_gps_parse_optionst(int argc, const char **argv):
  parse_options_baset(GOTO_GPS_OPTIONS, argc, argv),
  language_uit(cmdline, ui_message_handler),
  ui_message_handler(cmdline, "GOTO-GPS " CBMC_VERSION)
{
}
  
/*******************************************************************\

Function: goto_gps_parse_optionst::register_languages

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void goto_gps_parse_optionst::register_languages()
{
  register_language(new_ansi_c_language);
  register_language(new_cpp_language);
  register_language(new_java_bytecode_language);
  register_language(new_jsil_language);
}

/*******************************************************************\

Function: goto_gps_parse_optionst::eval_verbosity

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void goto_gps_parse_optionst::eval_verbosity()
{
  // this is our default verbosity
  unsigned int v=messaget::M_STATISTICS;
  
  if(cmdline.isset("verbosity"))
  {
    v=unsafe_string2unsigned(cmdline.get_value("verbosity"));
    if(v>10) v=10;
  }
  
  ui_message_handler.set_verbosity(v);
}

/*******************************************************************\

Function: goto_gps_parse_optionst::get_command_line_options

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void goto_gps_parse_optionst::get_command_line_options(optionst &options)
{
  if(config.set(cmdline))
  {
    usage_error();
    exit(1);
  }

  #if 0
  if(cmdline.isset("c89"))
    config.ansi_c.set_c89();

  if(cmdline.isset("c99"))
    config.ansi_c.set_c99();

  if(cmdline.isset("c11"))
    config.ansi_c.set_c11();

  if(cmdline.isset("cpp98"))
    config.cpp.set_cpp98();

  if(cmdline.isset("cpp03"))
    config.cpp.set_cpp03();

  if(cmdline.isset("cpp11"))
    config.cpp.set_cpp11();
  #endif

  #if 0
  // check array bounds
  if(cmdline.isset("bounds-check"))
    options.set_option("bounds-check", true);
  else
    options.set_option("bounds-check", false);

  // check division by zero
  if(cmdline.isset("div-by-zero-check"))
    options.set_option("div-by-zero-check", true);
  else
    options.set_option("div-by-zero-check", false);

  // check overflow/underflow
  if(cmdline.isset("signed-overflow-check"))
    options.set_option("signed-overflow-check", true);
  else
    options.set_option("signed-overflow-check", false);

  // check overflow/underflow
  if(cmdline.isset("unsigned-overflow-check"))
    options.set_option("unsigned-overflow-check", true);
  else
    options.set_option("unsigned-overflow-check", false);

  // check overflow/underflow
  if(cmdline.isset("float-overflow-check"))
    options.set_option("float-overflow-check", true);
  else
    options.set_option("float-overflow-check", false);

  // check for NaN (not a number)
  if(cmdline.isset("nan-check"))
    options.set_option("nan-check", true);
  else
    options.set_option("nan-check", false);

  // check pointers
  if(cmdline.isset("pointer-check"))
    options.set_option("pointer-check", true);
  else
    options.set_option("pointer-check", false);

  // check for memory leaks
  if(cmdline.isset("memory-leak-check"))
    options.set_option("memory-leak-check", true);
  else
    options.set_option("memory-leak-check", false);

  // check assertions
  if(cmdline.isset("no-assertions"))
    options.set_option("assertions", false);
  else
    options.set_option("assertions", true);

  // use assumptions
  if(cmdline.isset("no-assumptions"))
    options.set_option("assumptions", false);
  else
    options.set_option("assumptions", true);

  // magic error label
  if(cmdline.isset("error-label"))
    options.set_option("error-label", cmdline.get_values("error-label"));
  #endif
}

/*******************************************************************\

Function: goto_gps_parse_optionst::doit

  Inputs:

 Outputs:

 Purpose: invoke main modules

\*******************************************************************/

int goto_gps_parse_optionst::doit()
{
  if(cmdline.isset("version"))
  {
    std::cout << CBMC_VERSION << std::endl;
    return 0;
  }
  
  //
  // command line options
  //

  optionst options;
  get_command_line_options(options);
  eval_verbosity();

  //
  // Print a banner
  //
  status() << "GOTO-GPS version " CBMC_VERSION " "
           << sizeof(void *)*8 << "-bit "
           << config.this_architecture() << " "
           << config.this_operating_system() << eom;

  register_languages();
  
  goto_model.set_message_handler(get_message_handler());

  if(goto_model(cmdline.args))
    return 6;
    
  if(process_goto_program(options))
    return 6;


  /* Do things here */

  /* Input / Output */

  /* Input: CProver program + User options (e.g. what to prioritise)
   * Output: Suggested schedule
   * (Iterative) Input:  Status of explored paths / Generated results   *
   *    i.e. (Iterative) Input:  Path + Status (reachable/etc)
   */

  /* Ideas:
   *
   * 2LS
   *  - K-Induction
   *  - Termination Analysis via Ranking Function
   *  - Context-Sensitive
   *  - Preconditions
   *  - Intervals/Equalities/Zone/Octagon Domains
   *
   * Abstract Interpretation
   *  - Via Goto Analyser interface provides taint, reachability and intervals/non-null.
   *  - Via internals provides:
   *      - Constant prop.
   *      - Flow insensitive analysis framework
   *      - Custom bitbector analysis framework
   *      - Dependence graph
   *
   * Termination analysis
   *
   * Loop acceleration
   *
   * Randomised testing
   *
   * Code segment characterisation
   *
   */

  if(cmdline.isset("block-exits"))
  {
    Forall_goto_functions(f_it, goto_model.goto_functions) {
      if(!f_it->second.body_available())
        continue;

      Forall_goto_program_instructions(it, f_it->second.body) {
        switch(it->type) {
          case FUNCTION_CALL:
          case RETURN:
          case END_FUNCTION:
          {
            auto instr = f_it->second.body.insert_before(it);
            instr->make_assumption(false_exprt());
            break;
          }

          default:
          {
            break;
          }
        }
      }
    }
    goto_model.goto_functions.update();
  }


  if(cmdline.isset("print-function-names"))
  {
    Forall_goto_functions(f_it, goto_model.goto_functions) {
      std::cout << f_it->first << "\n";
    }
  }

  if(cmdline.isset("o"))
  {
    status() << "Writing GOTO program to `" << cmdline.get_value("o") << "'" << eom;

    if(write_goto_binary(
        cmdline.get_value("o"), goto_model.symbol_table, goto_model.goto_functions, get_message_handler()))
      return 1;
    else
      return 0;
  }

  error() << "no analysis option given -- consider reading --help"
          << eom;
  return 6;
}

/*******************************************************************\

Function: goto_gps_parse_optionst::set_properties

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bool goto_gps_parse_optionst::set_properties()
{
  try
  {
    if(cmdline.isset("property"))
      ::set_properties(goto_model, cmdline.get_values("property"));
  }

  catch(const char *e)
  {
    error() << e << eom;
    return true;
  }

  catch(const std::string e)
  {
    error() << e << eom;
    return true;
  }
  
  catch(int)
  {
    return true;
  }
  
  return false;
}

/*******************************************************************\

Function: goto_gps_parse_optionst::process_goto_program

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
  
bool goto_gps_parse_optionst::process_goto_program(
  const optionst &options)
{
  try
  {
    #if 0
    // Remove inline assembler; this needs to happen before
    // adding the library.
    remove_asm(goto_model);

    // add the library
    status() << "Adding CPROVER library (" 
             << config.ansi_c.arch << ")" << eom;
    link_to_library(goto_model, ui_message_handler);
    #endif

    // remove function pointers
    status() << "Removing function pointers and virtual functions" << eom;
    remove_function_pointers(goto_model, cmdline.isset("pointer-check"));
    remove_virtual_functions(goto_model);

    // do partial inlining
    status() << "Partial Inlining" << eom;
    goto_partial_inline(goto_model, ui_message_handler);
    
    // remove returns, gcc vectors, complex
    remove_returns(goto_model);
    remove_vector(goto_model);
    remove_complex(goto_model);

    #if 0
    // add generic checks
    status() << "Generic Property Instrumentation" << eom;
    goto_check(options, goto_model);
    #endif
    
    // recalculate numbers, etc.
    goto_model.goto_functions.update();

    // add loop ids
    goto_model.goto_functions.compute_loop_numbers();
    
    // show it?
    if(cmdline.isset("show-goto-functions"))
    {
      namespacet ns(goto_model.symbol_table);

      goto_model.goto_functions.output(ns, std::cout);
      return true;
    }

    // show it?
    if(cmdline.isset("show-symbol-table"))
    {
      ::show_symbol_table(goto_model, get_ui());
      return true;
    }
  }

  catch(const char *e)
  {
    error() << e << eom;
    return true;
  }

  catch(const std::string e)
  {
    error() << e << eom;
    return true;
  }
  
  catch(int)
  {
    return true;
  }
  
  catch(std::bad_alloc)
  {
    error() << "Out of memory" << eom;
    return true;
  }
  
  return false;
}

/*******************************************************************\

Function: goto_gps_parse_optionst::help

  Inputs:

 Outputs:

 Purpose: display command line help

\*******************************************************************/

void goto_gps_parse_optionst::help()
{
  std::cout <<
    "\n"
    "* * GOTO-GPS " CBMC_VERSION " - Copyright (C) 2016 ";
    
  std::cout << "(" << (sizeof(void *)*8) << "-bit version)";
    
  std::cout << " * *\n";
    
  std::cout <<
    "* *                Daniel Kroening, DiffBlue                * *\n"
    "* *                 kroening@kroening.com                   * *\n"
    "\n"
    "Usage:                       Purpose:\n"
    "\n"
    " goto-gps [-h] [--help]  show help\n"
    " goto-gps file.c ...     source file names\n"
    "\n"
    "\n";
}

/*******************************************************************\

Module: Computing a WP of a step

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_STEP_WP_H
#define CPROVER_STEP_WP_H

#include "path-symex/path_symex_state.h"
#include "path-symex/path_symex_history.h"

exprt step_wp(
  const path_symex_stept &step,
  const exprt &cond,
  const namespacet &ns);
  
#endif

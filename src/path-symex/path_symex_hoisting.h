/*
 * path_symex_hoisting.h
 *
 *  Created on: 29 Jun 2015
 *      Author: danlle
 */

#ifndef PATH_SYMEX_PATH_SYMEX_HOISTING_H_
#define PATH_SYMEX_PATH_SYMEX_HOISTING_H_

#include <goto-programs/goto_functions.h>
#include "path-symex/path_symex_history.h"
#include "path-symex/path_symex_state.h"


class path_symex_hoisting {
public:
  path_symex_hoisting();
  virtual ~path_symex_hoisting();

  void insert_skips(goto_functionst &goto_functions);

  bool hoist(exprt &assertion,
      loc_reft &pc,
      std::map<loc_reft, exprt> &hoisted_asserts,
      unsigned steps,
      path_symex_statet &state,
    const namespacet &ns);

  unsigned calculate_hoist_steps(
      path_symex_statet &state,
      locst &locs,
      const namespacet &ns);

//  exprt step_wp(
//    path_symex_stept &,
//    exprt &,
//    const namespacet &
//  );
};



#endif /* PATH_SYMEX_PATH_SYMEX_HOISTING_H_ */

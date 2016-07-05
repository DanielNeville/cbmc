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


class path_symex_hoistingt {
public:
  path_symex_hoistingt();
  virtual ~path_symex_hoistingt();

  void insert_skips(goto_functionst &goto_functions);

  bool hoist(exprt &assertion,
      loc_reft &pc,
      std::map<loc_reft, exprt> &hoisted_asserts,
      path_symex_statet &state,
      locst &locs,
    const namespacet &ns);

  bool hoist_further(path_symex_step_reft &history_step,
      locst &locs);

  bool terminate_early(
      path_symex_step_reft &history_step,
      locst &locs);




//  exprt step_wp(
//    path_symex_stept &,
//    exprt &,
//    const namespacet &
//  );
};



#endif /* PATH_SYMEX_PATH_SYMEX_HOISTING_H_ */

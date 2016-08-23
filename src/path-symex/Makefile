SRC = locs.cpp var_map.cpp path_symex_history.cpp path_symex_state.cpp \
      path_symex.cpp build_goto_trace.cpp path_replay.cpp \
      path_symex_state_read.cpp path_symex_hoisting.cpp \
      step_wp.cpp impara_join.cpp quantifier_elimination.cpp

INCLUDES= -I ..

include ../config.inc
include ../common

CLEANFILES = path-symex$(LIBEXT)

all: path-symex$(LIBEXT)

###############################################################################

path-symex$(LIBEXT): $(OBJ)
	$(LINKLIB)


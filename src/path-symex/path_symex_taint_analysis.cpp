/*******************************************************************
 Module: Taint Engine Module

 Author: Daniel Neville,	daniel.neville@cs.ox.ac.uk
 John Galea,				john.galea@cs.ox.ac.uk

 \*******************************************************************/

#include "path_symex_taint_analysis.h"

virtual path_symex_taint_analysis_enginet::~path_symex_taint_analysis_enginet() {
}

inline const static taintt path_symex_taint_analysis_enginet::get_top_elem() {
	return 0;
}

path_symex_no_taint_analysis_enginet::~path_symex_no_taint_analysis_enginet() {
}

inline const taintt path_symex_no_taint_analysis_enginet::get_bottom_elem() const {
	return 0;
}

inline taintt path_symex_no_taint_analysis_enginet::meet(irep_idt id,
		const taintt taint_1, const taintt taint_2) const {
	return 0;
}

inline const std::string path_symex_no_taint_analysis_enginet::get_taint_analysis_name() const {
	return "None";
}

inline taintt path_symex_no_taint_analysis_enginet::parse_taint(
		const std::string taint_name) const {
	return 0;
}

inline std::string path_symex_no_taint_analysis_enginet::get_taint_name(
		const taintt taint) const {
	return "";
}

path_symex_simple_taint_analysis_enginet::~path_symex_simple_taint_analysis_enginet() {
}

inline const taintt path_symex_simple_taint_analysis_enginet::get_bottom_elem() const {
	return TAINTED;
}

inline taintt path_symex_simple_taint_analysis_enginet::meet(irep_idt id,
		const taintt taint1, const taintt taint2) const {
	// Perform checks on passed taint types.
	if (taint1 != UNTAINTED && taint1 != TAINTED) {
		throw "First taint type  passed to meet function is invalid.";
	}

	if (taint2 != UNTAINTED && taint2 != TAINTED) {
		throw "Second taint type passed to meet function is invalid.";
	}

	// If either taint state is tainted, then the result is tainted.
	return (taint1 == TAINTED || taint2 == TAINTED) ? TAINTED : UNTAINTED;
}

inline const std::string path_symex_simple_taint_analysis_enginet::get_taint_analysis_name() const {
	return "Simple taint domain.";
}

inline taintt path_symex_simple_taint_analysis_enginet::parse_taint(
		const std::string taint_name) const {
	/* Parse from strings -> taint types */
	if (taint_name == "untainted")
		return UNTAINTED;
	if (taint_name == "tainted")
		return TAINTED;
	throw "Taint type not recognised";
}

inline std::string path_symex_simple_taint_analysis_enginet::get_taint_name(
		const taintt taint) const {
	/* Parse from taint -> strings types */
	switch (taint) {
	case UNTAINTED:
		return "untainted";
	case TAINTED:
		return "tainted";
	}
	throw "Taint type not recognised";
}

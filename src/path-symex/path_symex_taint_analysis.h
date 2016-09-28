/*******************************************************************
 Module: We define taint lattices to represent taint types (the domain). In order to implement analysis using a new taint domain,
 one needs to implement the path_symex_taint_analysist interface. This file contains defines the interface class, as well
 as its implementors.

 Author:

 \*******************************************************************/

#ifndef CPROVER_PATH_SYMEX_TAINT_ANALYSIS_H
#define CPROVER_PATH_SYMEX_TAINT_ANALYSIS_H

#include <iostream>

/* Future work.
 *
 * Consider whether a goto_check for printf not-tainted should be inserted
 * (but under what taint engine?)
 *
 * Make taint JSON file syntactically 'similar' to A.I. taint file.
 *
 * Consider tainting function calls under some semantics.
 */

// TODO: code-efficient, const, code-clean up, references when possible, comment, etc
// TODO: Pretty output
// We represent positions in a taint lattice as unsigned short value.
// 0 always represents top (UT)
typedef unsigned short taintt;

class taint_datat;

/**
 * 	Interface for taint analysis, which differ in their considered domains.
 */
class path_symex_taint_analysis_enginet
{

public:

  virtual ~path_symex_taint_analysis_enginet()
  {
  }
  ;

  // Returns the maximal/top element of the lattice.
  inline const static taintt get_top_elem()
  {
    return 0;
  }

  // Returns the minimal/lowest element of the lattice.
  virtual const taintt get_bottom_elem() const = 0;

  // Given two taint types, the meet of the two is returned.
  virtual taintt meet(irep_idt id, taintt taint_1, taintt taint_2) const = 0;

  // Returns the name of the taint analysis engine
  virtual const std::string get_taint_analysis_name() const = 0;

  // Returns the taint type corresponding to the string
  virtual taintt parse_taint(std::string taint_name) const = 0;

  // Returns the name of a given taint type
  virtual std::string get_taint_name(taintt taint) const = 0;

  taint_datat *taint_data;

  // A flag specifying whether the taint engine is to be used.
  bool enabled=false;
};

typedef path_symex_taint_analysis_enginet taint_enginet;

class path_symex_no_taint_analysis_enginet: public taint_enginet
{
public:
  inline const taintt get_bottom_elem() const
  {
    return 0;
  }

  inline taintt meet(irep_idt id, taintt taint_1, taintt taint_2) const
  {
    return 0;
  }

  inline const std::string get_taint_analysis_name() const
  {
    return "None";
  }

  inline taintt parse_taint(std::string taint_name) const
  {
    return 0;
  }

  inline std::string get_taint_name(taintt taint) const
  {
    return "";
  }
};

class path_symex_simple_taint_analysis_enginet: public taint_enginet
{
public:
  // For simple taint analysis, we solely consider two taint types.
  static const taintt UNTAINTED=0; // 0 = max.
  static const taintt TAINTED=1;

  ~path_symex_simple_taint_analysis_enginet()
  {
  }

  inline const taintt get_bottom_elem() const
  {
    return TAINTED;
  }

  taintt meet(irep_idt id, taintt taint1, taintt taint2) const
  {
    // Perform checks on passed taint types.
    if (taint1 != UNTAINTED && taint1 != TAINTED)
    {
      throw "First taint type  passed to meet function is invalid.";
    }

    if (taint2 != UNTAINTED && taint2 != TAINTED)
    {
      throw "Second taint type passed to meet function is invalid.";
    }

    // If either taint state is tainted, then the result is tainted.
    return (taint1 == TAINTED || taint2 == TAINTED) ? TAINTED : UNTAINTED;
  }

  inline const std::string get_taint_analysis_name() const
  {
    return "Simple taint domain.";
  }

  inline taintt parse_taint(std::string taint_name) const
  {
    /* Parse from strings -> taint types */
    if (taint_name == "untainted")
      return UNTAINTED;
    if (taint_name == "tainted")
      return TAINTED;
    throw "Taint type not recognised";
  }

  inline std::string get_taint_name(taintt taint) const
  {
    /* Parse from taint -> strings types */
    switch (taint)
    {
      case UNTAINTED:
        return "untainted";
      case TAINTED:
        return "tainted";
    }
    throw "Taint type not recognised";
  }
};

#endif

/*******************************************************************\

Module: Input Locations Parser

Author:

\*******************************************************************/

#ifndef CPROVER_INPUT_LOCATION_PARSER_H
#define CPROVER_INPUT_LOCATION_PARSER_H

#include <string>
#include <list>
#include <iosfwd>

#include <util/message.h>
#include <util/irep.h>

#include <path-symex/locs.h>

#include "inspector_util.h"

/* To be correctly linked later when exact includes determined.
 * This avoids changing Makefiles for now. */
// BLANK
/* End .cpp includes */

class input_location_parse_treet
{
public:
  class rulet
  {
  public:
	unsigned int loc;

    enum { COMPLETE, CONSTANT, OTHER } taint;
    enum { INPUT, OUTPUT, NONE } flow;

    std::string message;

    void output(std::ostream &) const;

    inline rulet(): loc(0), taint(OTHER), flow(NONE)
    {

    }
  };

  typedef std::list<rulet> rulest;
  rulest rules;

  void output(std::ostream &) const;
};

bool parse_input_locations(
  const std::string &taint_file_name,
  input_location_parse_treet &,
  message_handlert &);

bool parse_entry_locations(
  const std::string &entry_location_file,
  locationst &entry_locations,
  message_handlert &);

bool check_rules(
  input_location_parse_treet::rulest &rules,
  locst &locs,
  std::ostream & warnings,
  bool errors,
  locationst &input_locations,
  locationst &output_locations);

void calculate_entry_locations(
		goto_functionst &goto_functions,
		locationst &entry_locations,
		bool &have_entry_point);

#endif

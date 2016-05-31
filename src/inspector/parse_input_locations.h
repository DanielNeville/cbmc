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

#endif

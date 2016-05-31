/*******************************************************************\

Module: Taint Parser

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <ostream>

#include <util/string2int.h>

#include <json/json_parser.h>

#include "parse_input_locations.h"

/*******************************************************************\

Function: taint_parser

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bool parse_input_locations(
  const std::string &file_name,
  input_location_parse_treet &dest,
  message_handlert &message_handler)
{
  /* Format
	Array of Objects.

	Each Object:

	Loc: <Program location s.t. LHS is tainted>
	Taint: <Taint value>
	Flow: <Input or Output taint>
  */

  jsont json;

  if(parse_json(file_name, message_handler, json))
  {
    messaget message(message_handler);
    message.error() << "input location file is not a valid json file"
                    << messaget::eom;
    return true;
  }

  if(!json.is_array())
  {
    messaget message(message_handler);
    message.error() << "expecting an array in the input location file, but got "
                    << json << messaget::eom;
    return true;
  }

  for(jsont::arrayt::const_iterator
      it=json.array.begin();
      it!=json.array.end();
      it++)
  {
    if(!it->is_object())
    {
      messaget message(message_handler);
      message.error() << "expecting an array of objects in the input location file, but got "
                      << *it << messaget::eom;
      return true;
    }

    input_location_parse_treet::rulet rule;

    const std::string taint=(*it)["taint"].value;
    const std::string flow=(*it)["flow"].value;
    const std::string loc=(*it)["loc"].value;
    const std::string message=(*it)["message"].value;

    if(taint=="complete")
      rule.taint=input_location_parse_treet::rulet::COMPLETE;
    else if(taint=="constant")
      rule.taint=input_location_parse_treet::rulet::CONSTANT;
    else if(taint=="other")
      rule.taint=input_location_parse_treet::rulet::OTHER;
    else
    {
      messaget message(message_handler);
      message.error() << "taint rule must have \"kind\" which is "
                         "\"complete\" or \"constant\" or \"none\""
                      << messaget::eom;
      return true;
    }

    if(flow=="input")
      rule.flow=input_location_parse_treet::rulet::INPUT;
    else if(flow=="output")
      rule.flow=input_location_parse_treet::rulet::OUTPUT;
    else if(flow=="none")
      rule.flow=input_location_parse_treet::rulet::NONE;
    else
    {
      messaget message(message_handler);
      message.error() << "FLOW rule must have \"kind\" which is "
                         "\"input\" or \"output\" or \"none\""
                      << messaget::eom;
      return true;
    }

    if(loc.empty()) {
        messaget message(message_handler);
        message.error() << "location must have \"unsigned int\""
                        << messaget::eom;
        return true;
    } else {
    	rule.loc = safe_string2unsigned(std::string(loc, 0, std::string::npos));

    }

    rule.message=message;

    dest.rules.push_back(rule);
  }

  return false;
}

/*******************************************************************\

Function: taint_parse_treet::rulet::output

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void input_location_parse_treet::rulet::output(std::ostream &out) const
{
  out << "pc loc " << loc << ":";

  switch(taint)
  {
  case COMPLETE: out << " complete "; break;
  case CONSTANT: out << " constant "; break;
  case NONE: out << " none "; break;
  }

  out << "taint on";

  switch(flow)
  {
  case INPUT: out << "input"; break;
  case OUTPUT: out << "output"; break;
  case NONE: out << "none"; break;
  }

  out << '\n';
}

/*******************************************************************\

Function: taint_parse_treet::output

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void input_location_parse_treet::output(std::ostream &out) const
{
  for(const auto & rule : rules)
    rule.output(out);
}


/*******************************************************************\

Module: Taint Parser

Author:

\*******************************************************************/

#include <ostream>
#include <iostream>

#include <util/string2int.h>

#include <json/json_parser.h>
#include <util/message.h>

#include "parse_input_locations.h"
#include "reachability_analysis.h"

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

  out << "taint on ";

  switch(flow)
  {
  case INPUT: out << "input"; break;
  case OUTPUT: out << "output"; break;
  case NONE: out << "none"; break;
  }

  out << "\n";
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



bool check_rules(
    input_location_parse_treet::rulest &rules,
    locst &locs,
    std::ostream & warnings,
    bool errors,
    locationst &input_locations,
    locationst &output_locations) {

  bool found_error = false;

  for(auto rule: rules) {
    if(!warnings)
      return true;

    if(rule.loc >= locs.size()) {
      warnings << "Following rule outside program scope:" << "\n";
      rule.output(warnings);
      warnings << "\n";

      found_error = true;
    }

    goto_programt::const_targett inst = locs[rule.loc].target;

    if(inst->is_assign() || inst->is_decl() || inst->is_function_call())
    {
      if(inst->is_function_call()) {
        code_function_callt function_call = to_code_function_call(inst->code);
        if(function_call.lhs().is_nil()) {
          warnings << "Following rule refers to function call with nil left-hand side:" << "\n";
          rule.output(warnings);
          warnings << "\n";

          if(errors) {
            return false;
          }
        }
      }
    } else
    {
      warnings << "Following rule refers to an unsupported op (" << inst->type << ")\n";
      rule.output(warnings);
      warnings << "\n";

      found_error = true;
    }

    if(rule.flow == input_location_parse_treet::rulet::INPUT) {
      input_locations.push_back(rule.loc);
    }
    if(rule.flow  == input_location_parse_treet::rulet::OUTPUT) {
      output_locations.push_back(rule.loc);
    }
  }

  return !(errors && found_error);
}

bool parse_entry_locations(
    const std::string &entry_location_file,
    locationst &entry_locations,
    message_handlert &message_handler) {

  jsont json;

  if(parse_json(entry_location_file, message_handler, json))
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
    unsigned int location = safe_string2unsigned(std::string((*it)["loc"].value, 0, std::string::npos));
    entry_locations.push_back(location);
  }
  return false;
}



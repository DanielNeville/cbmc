/*******************************************************************\
Module: Taint Parser
Author:
\*******************************************************************/

#include <ostream>
#include <iostream>

#include <util/string2int.h>

#include <json/json_parser.h>
#include <util/message.h>

/*******************************************************************\
Function: taint_datat
  Inputs:
 Outputs:
 Purpose:
\*******************************************************************/

typedef path_symex_simple_taint_analysist::taintt taintt;
// TODO:  Remove this when templated.


class taint_datat
{
public:

  class taint_rulet
  {
  public:
    // TODO : Consider get / set / operator
    unsigned int loc;
    taintt taint;

    inline void output(std::ostream &out) const {
      path_symex_simple_taint_analysist taint_engine;

      out << "Location: " << loc << " set to " <<
          taint_engine.parser(taint);
    }

    inline taint_rulet(): loc(0), taint(UNTAINTED)
    {

    }
  };

  typedef std::set<taint_rulet> datat;
  datat data;

  inline void add(unsigned loc, taintt taint) {
    taint_rulet rule;
    rule.loc = loc;
    rule.taint = taint;
    data.insert(rule);
  }

  inline void output(std::ostream &out) const {
    // TODO Templated later.

    int i = 0;
    for(auto taint: data) {
      out << ++i << ": ";
      taint.output(out);
      out << "\n";
      // TODO EOM.
    }
  }
};

bool parse_input_locations(
    const std::string &file_name,
    path_symex_simple_taint_analysist &taint_engine,
    message_handlert &message_handler,
    taint_datat &taint_data
)
{
  /* Format
  Array of Objects.
  Each Object:
  Loc: <Program location s.t. LHS is tainted>
  Taint: <Taint value>
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

    const std::string taint_string =(*it)["taint"].value;
    const std::string loc_string =(*it)["loc"].value;

    taintt taint;
    unsigned int loc;

    try {
      taint =  taint_engine.parser(taint_string);
    } catch(...) {
      messaget message(message_handler);
      message.error() << "Taint type not recognised."
          << messaget::eom;
    }

    if(loc_string.empty()) {
      messaget message(message_handler);
      message.error() << "location must have \"unsigned int\""
          << messaget::eom;
      return true;
    } else {
      loc = safe_string2unsigned(std::string(loc_string, 0, std::string::npos));
    }

    taint_data.add(loc, taint);
  }

  return false;
}



bool check_rules(
    taint_datat &rules,
    locst &locs,
    std::ostream & warnings,
    bool errors) {

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
        }
      }
    } else
    {
      warnings << "Following rule refers to an unsupported op (" << inst->type << ")\n";
      rule.output(warnings);
      warnings << "\n";

      found_error = true;
    }

  }

  return found_error;
}

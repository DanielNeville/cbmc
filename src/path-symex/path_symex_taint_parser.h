/*
 * path_symex_taint_parser.h
 *
 *  Created on: 22 Sep 2016
 *  Author: Daniel Neville
 */

#ifndef PATH_SYMEX_PATH_SYMEX_TAINT_PARSER_H_
#define PATH_SYMEX_PATH_SYMEX_TAINT_PARSER_H_

#include <ostream>
#include <iostream>

#include <util/string2int.h>

#include <json/json_parser.h>
#include <util/message.h>
#include <path-symex/path_symex_taint_data.h>

bool parse_taint_file(
    const std::string &file_name,
    message_handlert &message_handler,
    taint_datat &taint_data
)
{
  /* Format

  Taint Analysis Option.

  Array of Objects.
  Each Object:
  Loc: <Program location s.t. LHS is tainted>
  Taint: <Taint value>
   */

  jsont json;

  // First parse the file.
  if(parse_json(file_name, message_handler, json))
  {
    messaget message(message_handler);
    message.error() << "input location file is not a valid json file" << messaget::eom;
    return true;
  }

  // Perform array check.
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
      path_symex_simple_taint_analysist taint_engine;
      // TODO: Handle catch better.  Add templating.
      taint =  taint_engine.parser(taint_string);
    } catch(...) {
      messaget message(message_handler);
      message.error() << "Taint type not recognised." << messaget::eom;
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

#endif /* PATH_SYMEX_PATH_SYMEX_TAINT_PARSER_H_ */

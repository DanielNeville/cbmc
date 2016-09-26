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

	// Perform an array check.
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
		// Check if object
		if(!it->is_object())
		{
			messaget message(message_handler);
			message.error() << "expecting an array of objects in the input location file, but got "
					<< *it << messaget::eom;
			return true;
		}

		// Fetch raw data from JSON.
		// TODO: Handle these Fields!
		const std::string taint_str =(*it)["taint"].value;
		const std::string loc_str =(*it)["loc"].value;
		const std::string array_flg_str =(*it)["array"].value;
		const std::string array_idx_str =(*it)["array_idx"].value;
		const std::string struct_flg_str =(*it)["struct"].value;
		const std::string struct_memb_str =(*it)["struct_memb"].value;
		const std::string func_flg_str =(*it)["func"].value;
		const std::string func_param_str =(*it)["func_param"].value;

		// Parsed Fields.
		taintt taint;
		unsigned int loc;
		bool array_flg;
		unsigned int array_idx;
		bool struct_flg;
		unsigned int struct_memb;
		bool func_flg;
		unsigned int func_param;

		// Check taint string
		try {
			taint =  taint_data.taint_analysis.parse_taint(taint_str);
		} catch(...) {
			messaget message(message_handler);
			message.error() << "Taint type not recognised." << messaget::eom;
		}

		// Check loc string
		if(loc_str.empty()) {
			messaget message(message_handler);
			message.error() << "location must have \"unsigned int\""
					<< messaget::eom;
			return true;
		} else {
			loc = safe_string2unsigned(std::string(loc_string, 0, std::string::npos));
		}

		//	Array Flag
		if(array_flg_str.empty()) {
		messaget message(message_handler);
		message.error() << "location must have \"unsigned int\""
				<< messaget::eom;
		return true;
	} else {
		loc = safe_string2unsigned(std::string(array_flg_str, 0, std::string::npos));
	}



		taint_data.add(loc, taint, array_flg, array_idx, struct_flg, struct_memb);
	}

	return false;
}


#endif /* PATH_SYMEX_PATH_SYMEX_TAINT_PARSER_H_ */

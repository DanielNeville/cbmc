/*******************************************************************\

Module: Reachability analysis

Author:

\*******************************************************************/

#include <iostream>
#include <stack>

#include <util/i2string.h>

#include <goto-programs/remove_skip.h>
#include <goto-programs/remove_unreachable.h>

#include "reachability_analysis.h"

void reachability_analysist::simple_cfg(
      goto_functionst &goto_functions) {

  Forall_locations(location, entry_locations) {
    location_sett reached;
    location_sett interactions_reached;
    location_sett frontier;

    frontier.insert(*location);

    while(!frontier.empty())
    {
      location_sett out;
      locationt loc = *frontier.begin();
      frontier.erase(frontier.begin());

      reached.insert(loc);

      auto target = locs[loc].target;
      goto_programt::const_targett next(target);
      next++;

      goto_functionst::function_mapt::const_iterator this_f_it=
        goto_functions.function_map.find(locs[loc].function);

      if(this_f_it == goto_functions.function_map.end() ||
          !this_f_it->second.body_available())
      {
        std::cout << "ERROR!!!!\n";
      }
      goto_programt::const_targett end_pt = this_f_it->second.body.instructions.end();


      std::cout << target->location_number << " ";

      if(target->is_function_call()) {
        std::cout << "function call.";
        const exprt &function=
           to_code_function_call(target->code).function();

         if(function.id()!=ID_symbol)
           return;

         const irep_idt &identifier=
           to_symbol_expr(function).get_identifier();

         goto_functionst::function_mapt::const_iterator f_it=
           goto_functions.function_map.find(identifier);

         if(f_it!=goto_functions.function_map.end() &&
            f_it->second.body_available())
         {
           // get the first instruction
           goto_programt::const_targett i_it=
             f_it->second.body.instructions.begin();

           goto_programt::const_targett e_it=
             f_it->second.body.instructions.end();

           goto_programt::const_targett last_it=e_it; last_it--;

           if(i_it!=e_it)
           {
             // nonempty function
//             this->add_edge(entry, entry_map[i_it]);
             out.insert(i_it->location_number);

             // add the last instruction as predecessor of the return location
             if(next != end_pt) {
               out_edges[last_it->location_number].push_back(next->location_number);
             }
//             if(next_PC!=goto_program.instructions.end())
//               this->add_edge(entry_map[last_it], entry_map[next_PC]);
           }
           else if(next!=end_pt)
           {
             // empty function
             out.insert(next->location_number);
           }
         }
         else if(next!=end_pt)
           out.insert(next->location_number);



      }
      else if(target->is_goto()) {
        std::cout << "goto";
        for(auto it: target->targets) {
          out.insert(it->location_number);
        }

        if(next!=end_pt)
        {
          out.insert(next->location_number);
        }
      }
      else if(target->is_return()) {
        std::cout << "return";
      } else if(target->is_end_function()) {
        for(auto it: out_edges[target->location_number]) {
          out.insert(it);
        }
      }
      else{
        std::cout << "other " << target->type;
        if(loc + 1 < locs.size()) {
          out.insert(loc + 1);
        }
      }

      for(auto it: out) {
        frontier.insert(it);
        std::cout << "   " << loc << " -> " << it;
      }
      std::cout << "\n";
    }

    locationst out;






  }

//  Forall_goto_functions(f_it, goto_functions) {
//
//    if(!f_it->second.body_available())
//      continue;
//
//
//
//    Forall_goto_program_instructions(it, f_it->second.body) {
//      std::cout << it->location_number << " " << locs[it->location_number].target->location_number << "\n";
//    }
//  }


}

locationst reachability_analysist::get_next(locationt location) {
//  auto it = cfg[location].PC;
//  locationst out;
//
//  std::cout << it->location_number;
//
//  if(it->targets.size() > 0 && it->guard.is_true()) {
//    out.push_back((*it->targets.begin())->location_number);
//    return out;
//  }
//  if(it->targets.size() > 0 && !it->guard.is_false()) {
//    // Only accept one target.
//    out.push_back((*it->targets.begin())->location_number);
//  }
//  if(it->is_end_function()) {
//
//  }
//  if(it->is_function_call()) {
//
//  }
//
//
//  std::cout << "\n";
}

/*******************************************************************\

Function: reachability_analysist::fixedpoint

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void reachability_analysist::fixedpoint(
    reaching_automatat &interaction_reaches,
    reaching_automatat &all_reaches)
{
//  cfg.output_dot(std::cout);

  Forall_locations(location, all_locations) {
    locationst reached;
    locationst interactions_reached;
    locationst frontier;

    frontier.push_back(*location);

    while(!frontier.empty()) {
      locationt item = frontier.back();
      frontier.pop_back();
      reached.push_back(item);

      if(contains(interaction_locations, item))
        interactions_reached.push_back(item);

//      get_next(item);

//      for(auto it : cfg[item].out) {
//        if(!contains(reached, it.first)) {
//          frontier.push_back((unsigned) it.first);
//          //          std::cout << item << " -> " << it.first << "\n";
//        }
//      }
    }

    all_reaches.push_back(std::make_pair(*location, reached));
    interaction_reaches.push_back(std::make_pair(*location, interactions_reached));
  }

}

/*******************************************************************\

Function: reachability_analysist::fixedpoint_assertions

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void reachability_analysist::output(std::ostream& out,
    output_typet output_type)
{
  switch(output_type) {
  case PLAINTEXT: {
      out << "Entry locations: \n";
      output_plaintext(entry_locations, out);
      out << "Input locations: \n";
      output_plaintext(input_locations, out);
      out << "Output locations: \n";
      output_plaintext(output_locations, out);
      break;
    }
  case DOT: {
    output_dot(out);
    break;
  }
  case JSON: {
    out << "Not implemented.\n";
    break;
  }

  }

}

void reachability_analysist::output_plaintext(
    locationst &locations,
    std::ostream& out)
{
  Forall_locations(location, locations) {

    locationst *reaches = NULL;
    out << *location << " -> ";

    for(auto it = interaction_reaches_.begin();
    it != interaction_reaches_.end();
    it++) {
      if(it->first == *location) {
       reaches = &it->second;
      }
    }

    if(reaches == NULL) {
      out << "<NONE>";
    } else {
      bool first = false;
      Forall_locations(it, *reaches) {
        if(first) {
          out << ", ";
        }
        first = true;
        out << *it;
      }
    }

    out << "\n";
  }
}

void reachability_analysist::output_dot(
    locationst &locations,
    std::ostream &out)
{
  Forall_locations(location, locations) {

     bool found = false;

     for(auto it = interaction_reaches_.begin();
     it != interaction_reaches_.end() && !found;
     it++) {
       if(it->first == *location) {
        Forall_locations(reached_location, it->second) {
          out << "\t\"" << *location << "\" -> \"" << *reached_location << "\"\n";
          found = true;
        }
       }
     }
   }
}

void reachability_analysist::output_dot(
    std::ostream& out)
{
  out << "digraph G {\n";

  Forall_locations(it, entry_locations) {
    out << "\t\"" << *it << "\" [shape=square]" << "\n";
  }
  Forall_locations(it, input_locations) {
    out << "\t\"" << *it << "\" [shape=diamond]" << "\n";
  }
  Forall_locations(it, output_locations) {
    out << "\t\"" << *it << "\" [shape=oval]" << "\n";
  }

  output_dot(entry_locations, out);
  output_dot(input_locations, out);
  output_dot(output_locations, out);

  out << "}";
}


/*******************************************************************\

Module: Reachability analysis

Author:

\*******************************************************************/

#include <iostream>
#include <stack>

#include <util/i2string.h>

#include <goto-programs/remove_skip.h>
#include <goto-programs/remove_unreachable.h>
#include <goto-programs/cfg.h>

#include "reachability_analysis.h"

/*******************************************************************\

Function: reachability_analysist::fixedpoint_assertions

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void reachability_analysist::fixedpoint(
    reaching_automatat &interaction_reaches,
    reaching_automatat &all_reaches)
{
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

      for(auto it : cfg[item].out) {
        if(!contains(reached, it.first)) {
          frontier.push_back((unsigned) it.first);
        }
      }
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


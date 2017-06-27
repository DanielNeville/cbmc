/*******************************************************************\
 Module: Unit tests for variable/sensitivity/abstract_object::merge
 Author: DiffBlue Limited. All rights reserved.
\*******************************************************************/

#include <catch.hpp>

#include <analyses/interval.h>
#include <util/std_types.h>
#include <util/std_expr.h>
#include <util/symbol_table.h>
#include <util/arith_tools.h>

#define V(X)   (binary2integer(X.get(ID_value).c_str(), 2))
#define V_(X)  (binary2integer(X.c_str(), 2))


SCENARIO("add interval domain",
  "[core][analyses][interval][add]")
{
  GIVEN("Two simple signed intervals")
  {
    const typet type=signedbv_typet(32);
    symbol_tablet symbol_table;
    namespacet ns(symbol_table);

    source_locationt source_location;


    constant_exprt zero = to_constant_expr(from_integer(0, type));
    std::map<int, constant_exprt> values;

    for(int i = -100; i <= 100; i++)
    {
      constant_exprt expr = zero;
      expr.set_value(integer2string(i, 2));
      values[i]=expr;
    }

    WHEN("Both are positive [2,4]+[6,8]")
    {
      intervalt left(values[2], values[4]);
      intervalt right(values[6], values[8]);

      intervalt result = left.add(right);

      THEN("Domain is consistent")
      {
        REQUIRE(string2integer(left.get_lower().get(ID_value).c_str(), 2) == 2);
        REQUIRE(string2integer(left.get_upper().get(ID_value).c_str(), 2) == 4);
        REQUIRE(string2integer(right.get_lower().get(ID_value).c_str(), 2) == 6);
        REQUIRE(string2integer(right.get_upper().get(ID_value).c_str(), 2) == 8);
      }


      THEN("The result is [8, 12]")
      {
        REQUIRE(string2integer(result.get_lower().get(ID_value).c_str(), 2) == 8);
        REQUIRE(string2integer(result.get_upper().get(ID_value).c_str(), 2) == 12);
      }
    }
  }
}

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

SCENARIO("multiply interval domain",
  "[core][analyses][interval][multiply]")
{
  GIVEN("A selection of constant_exprts in a std::vector and map")
  {
    const typet type=signedbv_typet(32);
    symbol_tablet symbol_table;
    namespacet ns(symbol_table);

    std::map<int, constant_exprt> values;
    std::vector<exprt> ve;

    for(int i = -100; i <= 100; i++)
    {
      values[i] = from_integer(mp_integer(i), type);
      ve.push_back(exprt(from_integer(mp_integer(i), type)));
    }


    WHEN("Both are positive [2,5]*[7,11]")
    {
      intervalt a(values[2], values[5]);
      intervalt b(values[7], values[11]);

      intervalt result = a.multiply(b);

      THEN("Domain is consistent")
      {
        CHECK(V(a.get_lower()) == 2);
        CHECK(V(a.get_upper()) == 5);
        CHECK(V(b.get_lower()) == 7);
        CHECK(V(b.get_upper()) == 11);
      }

      CAPTURE(result);

      THEN("The result is [14, 55]")
      {
        CHECK(V(result.get_lower()) == 14);
        CHECK(V(result.get_upper()) == 55);
      }
    }

    WHEN("One is entirely negative [-2,-5]*[7,11]")
     {
       intervalt a(values[-2], values[-5]);
       intervalt b(values[7], values[11]);

       intervalt result = a.multiply(b);

       THEN("Domain is consistent")
       {
         CHECK(V(a.get_lower()) == -2);
         CHECK(V(a.get_upper()) == -5);
         CHECK(V(b.get_lower()) == 7);
         CHECK(V(b.get_upper()) == 11);
       }

       CAPTURE(result);


       THEN("The result is [-55, -14]")
       {
         CHECK(V(result.get_lower()) == mp_integer(-55));
         CHECK(V(result.get_upper()) == -14);
       }
     }

    WHEN("Range contains and extends from zero [-2,5]*[7,11]")
     {
       intervalt a(values[-2], values[5]);
       intervalt b(values[7], values[11]);

       intervalt result = a.multiply(b);

       THEN("Domain is consistent")
       {
         CHECK(V(a.get_lower()) == -2);
         CHECK(V(a.get_upper()) == 5);
         CHECK(V(b.get_lower()) == 7);
         CHECK(V(b.get_upper()) == 11);
       }

       CAPTURE(result);


       THEN("The result is [-22, 55]")
       {
         CHECK(V(result.get_lower()) == mp_integer(-22));
         CHECK(V(result.get_upper()) == 55);
       }
     }


    WHEN("One domain is infinite and other crosses zero [-2,5]*[7,INF]")
       {
         intervalt a(values[-2], values[5]);
         intervalt b(values[7], max_exprt(type));

         intervalt result = a.multiply(b);

         THEN("Domain is consistent")
         {
           CHECK(V(a.get_lower()) == -2);
           CHECK(V(a.get_upper()) == 5);
           CHECK(V(b.get_lower()) == 7);
           CHECK(intervalt::is_max(b.get_upper()));
         }

         CAPTURE(result);

         THEN("The result is [-INF, INF]")
         {
           CHECK(intervalt::is_max(result.get_upper()));
           CHECK(intervalt::is_min(result.get_lower()));
         }
       }


    WHEN("One domain is infinite and other is positive [2,5]*[7,INF]")
       {
         intervalt a(values[2], values[5]);
         intervalt b(values[7], max_exprt(type));
         intervalt result = a.multiply(b);

         THEN("Domain is consistent")
         {
           CHECK(V(a.get_lower()) == 2);
           CHECK(V(a.get_upper()) == 5);
           CHECK(V(b.get_lower()) == 7);
           CHECK(b.is_max());
         }

         THEN("The result is [14, INF]")
         {
           CAPTURE(result);

           CHECK(result.is_max());
           CHECK(V(result.get_lower()) == 14);
         }
       }
  }


}
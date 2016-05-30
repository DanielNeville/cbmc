/*******************************************************************\

Module: Inspector Main Module

Author:

\*******************************************************************/

#include <util/unicode.h>

#include "inspector_parse_options.h"

/*******************************************************************\

Function: main / wmain

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

#ifdef _MSC_VER
int wmain(int argc, const wchar_t **argv_wide)
{
  const char **argv=narrow_argv(argc, argv_wide);
#else
int main(int argc, const char **argv)
{
#endif
	inspector_parse_optionst parse_options(argc, argv);

  return parse_options.main();
}

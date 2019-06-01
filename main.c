/* main.c
   authors: Simon Lovell Bart      (microchips-n-dip)
            David Robinson Garland (davidgarland)
   description: The main program for the Chaos compiler.
   date: 2019-05-31
   */

#include <chaos.h>

int main(int argc, char **argv) {
  assert(argc > 1);
  
  struct unit unit = unit_init();
  
  printf("\n -- unit_read --\n\n");
  unit_read(&unit, argv[1]);
  printf("%s", unit.src);
  
  printf("\n -- unit_lex --\n\n");
  unit_lex(&unit);
  unit_lex_print(&unit);
  
  unit_free(&unit);
  return 0;
}

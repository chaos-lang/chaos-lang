/* main.c
   authors: Simon Lovell Bart      (microchips-n-dip)
            David Robinson Garland (davidgarland)
   description: The main program for the Chaos compiler.
   date: 2019-05-31
   */

#define _POSIX_C_SOURCE 200809L

#include <chaos.h>
#include <time.h>

struct timespec
diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

int main(int argc, char **argv) {
  assert(argc > 1);
  
  struct unit unit = unit_init();
  
  printf("\n -- unit_read --\n\n");
  unit_read(&unit, argv[1]);

  printf("%s", unit.src + 1);
  
  printf("\n -- unit_lex --\n\n");

  struct timespec start, stop;

  clock_gettime(CLOCK_MONOTONIC, &start);

  unit_lex(&unit);

  clock_gettime(CLOCK_MONOTONIC, &stop);

  printf("elapsed time: %zu:%zu\n", diff(start, stop).tv_sec, diff(start, stop).tv_nsec);

  unit_lex_print(&unit);
  
  unit_free(&unit);
  return 0;
}

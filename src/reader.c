/* reader.c
   authors: Simon Lovell Bart       (microchips-n-dip)
            David Robinson Garland  (davidgarland)
   description: The file reader for the Cheshire compiler.
   date: 2019-05-31
   */

/* Dependencies */

#include <cheshire.h>

/* Main Reader */

void 
unit_read(struct unit *unit, char *filename) {
  assert(unit != NULL);
  assert(filename != NULL);

  /* Save the filename. */ 
  if (unit->filename == NULL)
    unit->filename = XNEWVEC(char, strlen(filename));
  else
    unit->filename = XRESIZEVEC(char, unit->filename, strlen(filename));
  assert(unit->filename != NULL);
  strcpy(unit->filename, filename);

  /* Calculate the size of the file. */
  FILE *fp = fopen(filename, "r");
  assert(fp != NULL);
  fseek(fp, 0, SEEK_END);
  size_t len = (size_t) ftell(fp);
  rewind(fp);

  /* Allocate the source buffer. */
  if (unit->src == NULL)
    unit->src = XNEWVEC(char, len + 2);
  else
    unit->src = XRESIZEVEC(char, unit->src, len + 2);
  assert(unit->src != NULL);
  unit->src_len = len;

  /* Read the file into the buffer. */
  fread(unit->src + 1, len, 1, fp);
  fclose(fp);
  unit->src[0] = ' ';
  unit->src[len + 1] = '\0';
}

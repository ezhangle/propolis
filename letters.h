#include "hexcode.h"
extern BIT16 letters[38];
/* The extra six letters are for borders.
 *     11212112
 *    22      20
 *   22        20
 *  01          04
 *   23        25
 *    23      25
 *     0a24240c
 */
extern int debugletters;
extern BIT16 invletters[4096];

void degauss();
void fillinvletters();

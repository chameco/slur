#include <stddef.h>

#include "../include/slur.h"
#include "../include/modules/prelude.h"

int main(int argc, char **argv) {
	if (argc < 2) return EXIT_FAILURE;
	prelude(&GLOBAL);
	value *v = eval_str(&GLOBAL, argv[1]);
	if (v != NULL) free(v);
	return EXIT_SUCCESS;
}

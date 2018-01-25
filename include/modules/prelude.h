#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "slur.h"

value *v__print(value **args) {
	for (size_t i = 0; args[i] != NULL; ++i) {
		switch (args[i]->type) {
			case FUNCTION:  printf("Func\n"); break;
			case PRESERVED: printf("Lambda\n"); break;
			case INTEGER:   printf("%d\n", args[i]->integer); break;
			case STRING:    printf("%s\n", args[i]->string); break;
			case ARBITRARY: printf("<%s at %p>\n", args[i]->name, args[i]->arbitrary); break;
		}
	}
	FARGS(args);
	return NULL;
}

value *v__set(value **args) {
	if (args[0] == NULL || args[1] == NULL) return NULL;
	if (args[0]->type != STRING) return NULL;
	insert(&GLOBAL, args[0]->string, *args[1]);
	FARGS(args);
	return NULL;
}

value *v__get(value **args) {
	if (args[0] == NULL) return NULL;
	if (args[0]->type != STRING) return NULL;
	value *ret = lookup(&GLOBAL, args[0]->string);
	FARGS(args);
	return ret;
}

value *v__add(value **args) {
	value *ret = (value *) malloc(sizeof(value));
	ret->type = INTEGER; ret->integer = 0;
	for (size_t i = 0; args[i] != NULL; ++i) {
		if (args[i]->type == INTEGER) ret->integer += args[i]->integer;
	}
	FARGS(args);
	return ret;
}

value *v__neg(value **args) {
	if (args[0] == NULL) return NULL;
	if (args[0]->type != INTEGER) return NULL;
	value *ret = (value *) malloc(sizeof(value));
	ret->type = INTEGER; ret->integer = -args[0]->integer;
	FARGS(args);
	return ret;
}

void prelude(scope *s) {
	insert(s, ".", (value){FUNCTION, .function = v__print});
	insert(s, "#", (value){FUNCTION, .function = v__set});
	insert(s, "@", (value){FUNCTION, .function = v__get});
	insert(s, "+", (value){FUNCTION, .function = v__add});
	insert(s, "-", (value){FUNCTION, .function = v__neg});
}

#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define FNULL(x) if (x != NULL) { free(x); x = NULL; } else;
#define FARGS(args) for (size_t i = 0; args[i] != NULL; ++i) FNULL(args[i]);

typedef struct {
	enum { END=0, OPEN, CLOSE, OLAM, CLAM, OBLK, CBLK, WORD, INT, STR } type;
	void *data;
} token;
typedef struct node {
	enum { CALL, LAMBDA, BLOCK, GET, VAL } type;
	union {
		struct { struct node *call; struct node **args; };
		struct { char *lambda; struct node *body; };
		struct node **block;
		char *get;
		struct value *val;
	};
} node;
typedef struct scope {
	struct scope *parent;
	struct entry *dict;
	size_t dict_size;
	size_t dict_p;
} scope;
typedef struct value {
	enum { FUNCTION, PRESERVED, INTEGER, STRING, ARBITRARY } type;
	union {
		struct value *(*function)(struct value **);
		struct { char *lambda; struct scope closure; node *body; };
		int integer;
		char *string;
		struct { char *name; void *arbitrary; };
	};
} value;
typedef struct entry { char *name; value val; } entry;
scope GLOBAL = {.parent = NULL, .dict = NULL, .dict_size = 0, .dict_p = 0};

#define _DEFINE_READ_UNTIL(name, param, check)\
	char *name(const char *src, size_t *src_pos, param) {\
		size_t ret_size = 1024;\
		char *ret = (char *) calloc(ret_size, sizeof(char));\
		size_t j = 0;\
		while (check (ret[j++] = src[++*src_pos])) {\
			if (ret[j - 1] == EOF) { --*src_pos; break; }\
			else if (j >= ret_size - 1) ret = realloc(ret, sizeof(char) * (ret_size *= 2));\
		}\
		ret[j - 1] = 0;\
		return ret;\
}
_DEFINE_READ_UNTIL(read_until, char end, end !=);
_DEFINE_READ_UNTIL(read_until_not, char end, end ==);
_DEFINE_READ_UNTIL(read_until_pred, int (*f)(int), !f);
_DEFINE_READ_UNTIL(read_until_not_pred, int (*f)(int), f);

int iswordchar(int c) { return c != '(' && c != ')' && c != '[' && c != ']' && c != '{' && c != '}' && c != '"' && !isspace(c); }

token *lex(const char *buf, size_t buf_size) {
	size_t ret_size = 256;
	token *ret = (token *) calloc(ret_size, sizeof(token));
	size_t i = 0, ti = 0;
	for (i = 0; i < buf_size && buf[i] != 0; ++i) {
		switch (buf[i]) {
			case ' ': continue;
			case '(': ret[ti] = (token){OPEN, NULL}; break;
			case ')': ret[ti] = (token){CLOSE, NULL}; break;
			case '[': ret[ti] = (token){OLAM, NULL}; break;
			case ']': ret[ti] = (token){CLAM, NULL}; break;
			case '{': ret[ti] = (token){OBLK, NULL}; break;
			case '}': ret[ti] = (token){CBLK, NULL}; break;
			case '"': ret[ti].type = STR;
					  ret[ti].data = read_until(buf, &i, '"');
					  break;
			default:  if (isdigit(buf[i])) {
						  ret[ti].type = INT;
						  --i;
						  char *num = read_until_not_pred(buf, &i, isdigit);
						  --i;
						  ret[ti].data = malloc(sizeof(int));
						  *(int *) ret[ti].data = atoi(num);
						  FNULL(num);
					  } else if (!isspace(buf[i])) {
						  ret[ti].type = WORD;
						  --i;
						  ret[ti].data = read_until_not_pred(buf, &i, iswordchar);
						  --i;
					  }
					  break;
		}
		++ti;
		if (ti >= ret_size - 1) ret = realloc(ret, sizeof(token) * (ret_size *= 2));
	}
	return ret;
}

node *parse_one(token *in, size_t *ip) {
	size_t args_size;
	node *ret = (node *) malloc(sizeof(node));
	switch (in[*ip].type) {
		case OPEN:  ++*ip;
					if (in[*ip].type == CLOSE || in[*ip].type == END) { FNULL(ret); break; }
					ret->type = CALL;
				    ret->call = parse_one(in, ip);
				    args_size = 8;
				    ret->args = (node **) calloc(args_size, sizeof(node *));
				    for (size_t j = 0; in[*ip].type != CLOSE; ++j) {
					    if (j >= args_size - 1) ret->args = realloc(ret->args, sizeof(node *) * (args_size *= 2));
					    if ((ret->args[j] = parse_one(in, ip)) == NULL) --j;
				    }
					break;
		case CLOSE: FNULL(ret); ret = NULL; break;
		case OLAM:  ++*ip;
					if (in[*ip].type == CLAM || in[*ip].type == END) { FNULL(ret); break; }
					ret->type = LAMBDA;
					ret->lambda = (char *) in[*ip].data;
					++*ip;
					if (in[*ip].type == CLAM) ret->body = NULL;
					else {
						ret->body = parse_one(in, ip);
						for (size_t i = *ip; in[i].type != CLAM; ++i) {
							*ip = i;
						}
					}
					break;
		case CLAM:  FNULL(ret); break;
		case OBLK:  ++*ip;
					if (in[*ip].type == CBLK || in[*ip].type == END) { FNULL(ret); break; }
					ret->type = BLOCK;
				    args_size = 8;
				    ret->block = (node **) calloc(args_size, sizeof(node *));
				    for (size_t j = 0; in[*ip].type != CBLK; ++j) {
					    if (j >= args_size - 1) ret->args = realloc(ret->args, sizeof(node *) * (args_size *= 2));
					    if ((ret->block[j] = parse_one(in, ip)) == NULL) --j;
				    }
					break;
		case CBLK:  FNULL(ret); break;
		case WORD:  ret->type = GET;
				    ret->get = (char *) in[*ip].data;
				    break;
		case INT:   ret->type = VAL;
				    ret->val = malloc(sizeof(value));
				    *((value *) ret->val) = (value){INTEGER, .integer = *(int *) in[*ip].data};
				    FNULL(in[*ip].data);
				    break;
		case STR:   ret->type = VAL;
				    ret->val = malloc(sizeof(value));
				    *((value *) ret->val) = (value){STRING, .string = (char *) in[*ip].data};
				    break;
		case END:   FNULL(ret); break;
	}
	++*ip;
	return ret;
}

void scopecpy(scope *dst, scope *src) {
	dst->parent = src->parent;
	dst->dict_size = src->dict_size;
	dst->dict_p = src->dict_p;
	dst->dict = (entry *) calloc(dst->dict_size, sizeof(entry));
	memcpy(dst->dict, src->dict, dst->dict_size * sizeof(entry));
}

value *lookup(scope *s, char *name) {
	value *ret = NULL;
	for (size_t i = 0; i < s->dict_p; ++i) if (strcmp(s->dict[i].name, name) == 0) {
		ret = (value *) malloc(sizeof(value));
		memcpy(ret, &s->dict[i].val, sizeof(value));
		break;
	}
	if (ret == NULL && s->parent != NULL) ret = lookup(s->parent, name);
	else free(name);
	return ret;
}

void insert(scope *s, char *name, value value) {
	for (size_t i = 0; i < s->dict_p; ++i) if (strcmp(s->dict[i].name, name) == 0) { s->dict[i] = (entry){name, value}; return; }
	if (s->dict_size < 256) s->dict = realloc(s->dict, sizeof(entry) * (s->dict_size = 256));
	if (s->dict == NULL || s->dict_p >= s->dict_size) s->dict = realloc(s->dict, sizeof(entry) * (s->dict_size *= 2));
	s->dict[s->dict_p++] = (entry){name, value};
}

value *eval(scope *s, node *n) {
	value *ret = NULL, *tmp;
	size_t i;
	if (n == NULL) return NULL;
	else switch (n->type) {
		case CALL:   tmp = eval(s, n->call);
					 if (tmp == NULL) return NULL;
					 if (tmp->type == FUNCTION) {
						 for (i = 0; n->args[i] != NULL; ++i) n->args[i] = (void *) eval(s, n->args[i]);
						 ret = tmp->function((value **) n->args); break;
					 } else if (tmp->type == PRESERVED) {
						 ret = eval(s, n->args[0]);
						 if (ret != NULL) insert(&tmp->closure, tmp->lambda, *ret);
						 FNULL(ret);
						 ret = eval(&tmp->closure, tmp->body);
					 } else return NULL;
					 break;
		case LAMBDA: ret = (value *) malloc(sizeof(value));
					 ret->type = PRESERVED;
					 ret->lambda = n->lambda;
					 scopecpy(&ret->closure, s);
					 ret->body = n->body;
					 break;
		case BLOCK:  if (n->block[0] == NULL) return NULL;
					 for (i = 0; n->block[i+1] != NULL; ++i) { ret = eval(s, n->block[i]); FNULL(ret); }
					 ret = eval(s, n->block[i]);
					 break;
		case GET:    ret = lookup(s, n->get); break;
		case VAL:    ret = n->val; break;
	}
	FNULL(n);
	return ret;
}

value *eval_str(scope *s, const char *e) {
	token *ts = lex(e, strlen(e));
	size_t i = 0;
	node *n = parse_one(ts, &i);
	value *v = n == NULL ? NULL : eval(s, n);
	FNULL(ts);
	return v;
}

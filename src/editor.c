#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "slur.h"
#include "modules/prelude.h"

typedef struct line {
	char *data; // Does not include the newline
	size_t len;
	struct line *prev, *next;
	struct buffer *buf;
} line;

typedef struct buffer {
	char *lwp; // Last written path
	line *head, *tail;
} buffer;

line *make_line(const char *data) {
	line *ret = (line *) malloc(sizeof(line));
	ret->data = strdup(data);
	ret->prev = ret->next = NULL;
	ret->buf = NULL;
	return ret;
}

void destroy_line(line *l) {
	if (l == NULL) return;
	if (l->prev != NULL) l->prev->next = l->next;
	if (l->next != NULL) l->next->prev = l->prev;
	if (l->data != NULL) free(l->data);
	free(l);
}

void insert_line_before(line *itr, line *l) {
	if (l == NULL || itr == NULL) return;
	if (l->prev != NULL) l->prev->next = l->next;
	if (l->next != NULL) l->next->prev = l->prev;
	if (itr->prev != NULL) itr->prev->next = l;
	l->prev = itr->prev;
	l->next = itr;
	itr->prev = l;
	l->buf = itr->buf;
	if (l->buf != NULL && l->prev == NULL) l->buf->head = l;
	if (itr->buf != NULL && itr->next == NULL) itr->buf->tail = itr;
}

void insert_line_after(line *itr, line *l) {
	if (l == NULL || itr == NULL) return;
	if (l->prev != NULL) l->prev->next = l->next;
	if (l->next != NULL) l->next->prev = l->prev;
	if (itr->next != NULL) itr->next->prev = l;
	l->prev = itr;
	l->next = itr->next;
	itr->next = l;
	l->buf = itr->buf;
	if (itr->buf != NULL && itr->prev == NULL) itr->buf->head = itr;
	if (l->buf != NULL && l->next == NULL) l->buf->tail = l;
}

void write_buffer(buffer *b) {
	for (line *l = b->head; l != NULL; l = l->next) if (l->data != NULL) puts(l->data);
}

int main(int argc, char **argv) {
	buffer b = (buffer){.lwp=NULL, .head=NULL, .tail=NULL};
	line *fst = make_line("foo");
	b.tail = b.head = fst;
	fst->buf = &b;
	line *snd = make_line("bar");
	insert_line_after(fst, snd);
	insert_line_after(snd, fst);
	write_buffer(&b);
	return EXIT_SUCCESS;
}

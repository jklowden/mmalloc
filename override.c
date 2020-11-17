/*
 * To override libc malloc without modifying source code. 
 */

#include <mmalloc.h>

void *
malloc(size_t size) {
  return  mmalloc(size);
}

void *
calloc(size_t nmemb, size_t size) {
  return  mcalloc(nmemb, size);
}

void *
realloc(void *ptr, size_t size) {
  return  mrealloc(ptr, size);
}

void
free(void *ptr) {
  mfree(ptr);
}

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

void *mmalloc(size_t size);
void *mcalloc(size_t nmemb, size_t size);
void *mrealloc(void *ptr, size_t size);
void mfree(void *ptr);

#if MALLOC_IS_MMALLOC

void mlog( const char func[], int line, const void *ptr );

static inline void *
mmalloc_s( const char func[], int line, size_t size ) {
    void *p = mmalloc(size);
    mlog(func, line, p);
    return p;
}
#define malloc(size) mmalloc_s(__func__, __LINE__, size)

static inline void *
mcalloc_s(const char func[], int line, size_t n, size_t size) {
    void *p = mcalloc(n, size); 
    mlog(func, line, p);
    return p;
}
#define calloc(n, size) mcalloc_s(__func__, __LINE__, n, size)

static inline void *
mrealloc_s(const char func[], int line, void *ptr, size_t size) {
    void *p = mrealloc(ptr, size);
    mlog(func, line, p);
    return p;
}
#define realloc(p, size) mrealloc_s(__func__, __LINE__, p, size)

#define free(ptr) mfree(ptr); 

#endif

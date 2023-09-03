#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

typedef struct {
    char obj[MAX_OBJECT_SIZE];
    char uri[MAXLINE];
    int LRU;
    int size;

    int read_cnt;
    sem_t w, wg, mutex;
} block;

typedef struct {
    block data[MAX_CACHE_SIZE / MAX_OBJECT_SIZE];
    int num;
} cache_t;

void cache_init(cache_t *cache);
int get_cache(cache_t *cache, char *uri, int fd);
void add_cache(cache_t *cache, char *obj, int size, char *uri);
void update_lru(cache_t *cache, int ind);
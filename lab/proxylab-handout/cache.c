#include "cache.h"
#include "csapp.h"

void cache_init(cache_t *cache) {
    for (int i = 0; i < MAX_CACHE_SIZE / MAX_OBJECT_SIZE; i++) {
        cache->data[i].LRU = i;
        cache->data[i].read_cnt = 0;
        sem_init(&cache->data[i].mutex, 0, 1);
        sem_init(&cache->data[i].w, 0, 1);
        sem_init(&cache->data[i].wg, 0, 1);
    }
    cache->num = 0;
    return;
}

int get_cache(cache_t *cache, char *uri, int fd) {
    for (int i = 0; i < cache->num; i++) {
        if (strcmp(cache->data[i].uri, uri) == 0) {
            P(&cache->data[i].mutex);
            cache->data[i].read_cnt++;
            if (cache->data[i].read_cnt == 1)
                P(&cache->data[i].w);
            V(&cache->data[i].mutex);
            Rio_writen(fd, cache->data[i].obj, cache->data[i].size);
            P(&cache->data[i].mutex);
            cache->data[i].read_cnt--;
            if (cache->data[i].read_cnt == 0)
                V(&cache->data[i].w);
            V(&cache->data[i].mutex);
            update_lru(cache, i);
            return 1;
        }
    }
    return 0;
}

void add_cache(cache_t *cache, char *obj, int size, char *uri) {
    if (cache->num < MAX_CACHE_SIZE / MAX_OBJECT_SIZE) {
        P(&cache->data[cache->num].wg);
        P(&cache->data[cache->num].w);
        strcpy(cache->data[cache->num].obj, obj);
        strcpy(cache->data[cache->num].uri, uri);
        update_lru(cache, cache->num);
        cache->data[cache->num].size = size;
        V(&cache->data[cache->num].w);
        V(&cache->data[cache->num].wg);
        cache->num++;
        return;
    }
    for (int i = 0; i < MAX_CACHE_SIZE / MAX_OBJECT_SIZE; i++) {
        if (cache->data[i].LRU == MAX_CACHE_SIZE / MAX_OBJECT_SIZE - 1) {
            P(&cache->data[i].wg);
            P(&cache->data[i].w);
            strcpy(cache->data[i].obj, obj);
            strcpy(cache->data[i].uri, uri);
            update_lru(cache, i);
            cache->data[i].size = size;
            V(&cache->data[i].w);
            V(&cache->data[i].wg);
            return;
        }
    }
}

void update_lru(cache_t *cache, int ind) {
    int level = cache->data[ind].LRU;
    for (int i = 0; i < cache->num; i++) {
        if (cache->data[i].LRU < level) {
            cache->data[i].LRU++;
        }
    }
    cache->data[ind].LRU = 0;
    return;
}
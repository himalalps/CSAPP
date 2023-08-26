#include "cachelab.h"
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void printHelp() {
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n"
           "Options:\n"
           "  -h         Print this help message.\n"
           "  -v         Optional verbose flag.\n"
           "  -s <num>   Number of set index bits.\n"
           "  -E <num>   Number of lines per set.\n"
           "  -b <num>   Number of block offset bits.\n"
           "  -t <file>  Trace file.\n\n"
           "Examples:\n"
           "  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n"
           "  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void simulateCache(int set, int line, int block, char *tracefile, int verbose) {
    int hits = 0, misses = 0, evictions = 0;
    const int TOTAL_BLOCKS = (1 << set) * line;
    long long **cache =
        (long long **)malloc(sizeof(long long *) * TOTAL_BLOCKS);
    int **lruTable = (int **)malloc(sizeof(int *) * (1 << set));

    for (int i = 0; i < TOTAL_BLOCKS; i++) {
        cache[i] = (long long *)malloc(sizeof(long long) * 2);
        cache[i][0] = 0;
    }

    for (int i = 0; i < 1 << set; i++) {
        lruTable[i] = (int *)malloc(sizeof(int) * line);
        for (int j = 0; j < line; j++) {
            lruTable[i][j] = j;
        }
    }

    FILE *fp = fopen(tracefile, "r");
    assert(fp);

    char op;
    unsigned long long address;
    int size;

    while (fscanf(fp, " %c %llx,%d", &op, &address, &size) == 3) {
        // printf("%c %lld,%d ", op, address, size);
        if (op == 'I') {
            continue;
        }
        unsigned long long tag = address >> (set + block);
        int setIndex = (address >> block) & ((1 << set) - 1);
        int flag = 0;
        for (int i = 0; i < line; i++) {
            if (cache[setIndex * line + i][0] &&
                cache[setIndex * line + i][1] == tag) {
                if (op == 'M') {
                    hits += 2;
                    if (verbose) {
                        printf("%c %llx,%d hit hit\n", op, address, size);
                    }
                } else {
                    hits++;
                    if (verbose) {
                        printf("%c %llx,%d hit\n", op, address, size);
                    }
                }
                if (line > 1 && lruTable[setIndex][0] != i) {
                    for (int j = 1; j < line; j++) {
                        if (lruTable[setIndex][j] == i) {
                            while (j) {
                                lruTable[setIndex][j] =
                                    lruTable[setIndex][j - 1];
                                j--;
                            }
                            lruTable[setIndex][0] = i;
                            break;
                        }
                    }
                }
                flag = 1;
                break;
            } else if (!cache[setIndex * line + i][0]) {
                cache[setIndex * line + i][0] = 1;
                cache[setIndex * line + i][1] = tag;
                misses++;
                if (op == 'M') {
                    hits++;
                    if (verbose) {
                        printf("%c %llx,%d miss hit\n", op, address, size);
                    }
                } else {
                    if (verbose) {
                        printf("%c %llx,%d miss\n", op, address, size);
                    }
                }
                if (line > 1 && lruTable[setIndex][0] != i) {
                    for (int j = 1; j < line; j++) {
                        if (lruTable[setIndex][j] == i) {
                            while (j) {
                                lruTable[setIndex][j] =
                                    lruTable[setIndex][j - 1];
                                j--;
                            }
                            lruTable[setIndex][0] = i;
                            break;
                        }
                    }
                }
                flag = 1;
                break;
            }
        }
        if (flag) {
            continue;
        }
        if (line == 1) {
            cache[setIndex * line][1] = tag;
        } else {
            cache[setIndex * line + lruTable[setIndex][line - 1]][1] = tag;
            int temp = lruTable[setIndex][line - 1];
            for (int j = line - 1; j > 0; j--) {
                lruTable[setIndex][j] = lruTable[setIndex][j - 1];
            }
            lruTable[setIndex][0] = temp;
        }
        misses++;
        evictions++;
        if (op == 'M') {
            hits++;
            if (verbose) {
                printf("%c %llx,%d miss eviction hit\n", op, address, size);
            }
        } else {
            if (verbose) {
                printf("%c %llx,%d miss eviction\n", op, address, size);
            }
        }
    }

    fclose(fp);

    printSummary(hits, misses, evictions);

    for (int i = 0; i < TOTAL_BLOCKS; i++) {
        free(cache[i]);
    }

    for (int i = 0; i < 1 << set; i++) {
        free(lruTable[i]);
    }

    free(lruTable);
    free(cache);

    return;
}

int main(int argc, char *argv[]) {
    int opt;
    const char *optstring = "hvs:E:b:t:";
    int verbose = 0;
    int s = 0, E = 0, b = 0;
    char *tracefile = NULL;
    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
        case 'h':
            printHelp();
            return 0;
        case 'v':
            verbose = 1;
            break;
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            tracefile = optarg;
            break;
        case '?':
            printHelp();
            return 0;
        }
    }
    if (s == 0 || E == 0 || b == 0 || tracefile == NULL) {
        printf("./csim: Missing required command line argument\n");
        printHelp();
        return 0;
    }

    simulateCache(s, E, b, tracefile, verbose);

    return 0;
}

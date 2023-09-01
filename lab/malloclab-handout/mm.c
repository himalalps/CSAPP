/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    "",
};

#define NEXT_FIT

/* single word or double word alignment */
#define WSIZE 4
#define DSIZE 8
#define ALIGNMENT 8

/* extend heap by CHUNKSIZE (bytes) */
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* pack a size and an allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))
#define PACK_PREV(size, alloc, prev) ((size) | (alloc) | ((prev) << 1))

/* read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* read the size and allocated bit from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREV_ALLOC(p) ((GET(p) & 0x2) >> 1)

/* given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE((char *)(bp)-DSIZE))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

void *heap_listp;

#ifdef NEXT_FIT
void *pre_listp;
#endif

static void *coalesce(void *bp);
static void *extend_heap(size_t size);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

/**
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1) {
        return -1;
    }
    PUT(heap_listp, 0);                                  // alignment padding
    PUT(heap_listp + WSIZE, PACK_PREV(DSIZE, 1, 1));     // prologue header
    PUT(heap_listp + 2 * WSIZE, PACK_PREV(DSIZE, 1, 1)); // prologue footer
    PUT(heap_listp + 3 * WSIZE, PACK_PREV(0, 1, 1));     // epilogue block
    heap_listp += 2 * WSIZE;
    if (extend_heap(CHUNKSIZE) == NULL) {
        return -1;
    }
#ifdef NEXT_FIT
    pre_listp = heap_listp;
#endif
    return 0;
}

/**
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    size_t asize;
    size_t extendsize;
    void *bp;

    if (size == 0) {
        return NULL;
    }

    asize = ALIGN(size + WSIZE);

    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize)) == NULL) {
        return NULL;
    }
    place(bp, asize);
#ifdef NEXT_FIT
    pre_listp = bp;
#endif
    return bp;
}

/**
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    size_t size = GET_SIZE(HDRP(ptr));
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(ptr));
    PUT(HDRP(ptr), PACK_PREV(size, 0, prev_alloc));
    PUT(FTRP(ptr), PACK_PREV(size, 0, prev_alloc));
#ifdef NEXT_FIT
    pre_listp = coalesce(ptr);
#else
    coalesce(ptr);
#endif
    return;
}

/**
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    void *newptr;
    size_t copySize;
    size_t csize;

    if (ptr == NULL) {
        return mm_malloc(size);
    } else if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    copySize = ALIGN(size + WSIZE);
    csize = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
    if (csize >= copySize && !GET_ALLOC(HDRP(NEXT_BLKP(ptr)))) {
        PUT(HDRP(ptr), PACK_PREV(csize, 1, GET_PREV_ALLOC(HDRP(ptr))));
        place(ptr, copySize);
        return ptr;
    } else if (GET_SIZE(HDRP(NEXT_BLKP(ptr))) == 0) {
        copySize = MAX(copySize - csize, CHUNKSIZE);
        extend_heap(copySize);
        PUT(HDRP(ptr),
            PACK_PREV(csize + copySize, 1, GET_PREV_ALLOC(HDRP(ptr))));
        place(ptr, ALIGN(size + WSIZE));
        return ptr;
    } else if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) &&
               GET_SIZE(HDRP(NEXT_BLKP(NEXT_BLKP(ptr)))) == 0) {
        copySize = MAX(copySize - csize, CHUNKSIZE);
        extend_heap(copySize);
        PUT(HDRP(ptr),
            PACK_PREV(csize + copySize, 1, GET_PREV_ALLOC(HDRP(ptr))));
        place(ptr, ALIGN(size + WSIZE));
        return ptr;
    }

    newptr = mm_malloc(size);
    copySize = GET_SIZE(HDRP(ptr)) - WSIZE;
    if (size < copySize) {
        copySize = size;
    }
    memcpy(newptr, ptr, copySize);
    mm_free(ptr);
    return newptr;
}

/**
 * extend_heap - Extends the heap with a new free block
 */
static void *extend_heap(size_t size) {
    void *bp;
    size_t prev_alloc;

    if ((bp = mem_sbrk(ALIGN(size))) == (void *)-1) {
        return NULL;
    }

    prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    PUT(HDRP(bp), PACK_PREV(size, 0,
                            prev_alloc)); // free block header
    PUT(FTRP(bp), PACK_PREV(size, 0,
                            prev_alloc));         // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK_PREV(0, 1, 0)); // new epilogue block

    return coalesce(bp);
}

/**
 * coalesce - Coalesces adjacent free blocks
 *             and returns a pointer to the coalesced block
 */
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        return bp;
    } else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK_PREV(size, 0, prev_alloc));
        PUT(FTRP(bp), PACK_PREV(size, 0, prev_alloc));
    } else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        prev_alloc = GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK_PREV(size, 0, prev_alloc));
        PUT(HDRP(PREV_BLKP(bp)), PACK_PREV(size, 0, prev_alloc));
        bp = PREV_BLKP(bp);
    } else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        prev_alloc = GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK_PREV(size, 0, prev_alloc));
        PUT(FTRP(NEXT_BLKP(bp)), PACK_PREV(size, 0, prev_alloc));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

/**
 * find_fit - Finds a free block that fits the size
 *              using first fit search
 */
static void *find_fit(size_t asize) {
#ifdef NEXT_FIT
    void *bp = pre_listp;
    while (GET_SIZE(HDRP(bp))) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
        bp = NEXT_BLKP(bp);
    }
    bp = heap_listp;
    while (bp != pre_listp && GET(HDRP(bp))) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
        bp = NEXT_BLKP(bp);
    }
    return NULL;
#else
    void *bp = NEXT_BLKP(heap_listp);

    while (GET_SIZE(HDRP(bp))) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
        bp = NEXT_BLKP(bp);
    }

    return NULL;
#endif
}

/**
 * place - Places a block of size asize at the start of bp
 *          and splits the block if the remainder is at least
 *          the minimum block size
 */
static void place(void *bp, size_t asize) {
    size_t csize = GET_SIZE(HDRP(bp));
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));

    if (csize - asize >= DSIZE) {
        PUT(HDRP(bp), PACK_PREV(asize, 1, prev_alloc));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK_PREV(csize - asize, 0, 1));
        PUT(FTRP(bp), PACK_PREV(csize - asize, 0, 1));
    } else {
        PUT(HDRP(bp), PACK_PREV(csize, 1, prev_alloc));
        csize = GET_SIZE(HDRP(NEXT_BLKP(bp)));
        prev_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(NEXT_BLKP(bp)), PACK_PREV(csize, prev_alloc, 1));
        if (!prev_alloc) {
            PUT(FTRP(NEXT_BLKP(bp)), PACK_PREV(csize, prev_alloc, 1));
        }
    }
    return;
}

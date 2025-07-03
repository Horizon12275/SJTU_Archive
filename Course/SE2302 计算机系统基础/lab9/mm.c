/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * 1. Firstly, I just follow the textbook to implement the malloc lab.
 * I implement the malloc, free, realloc, and heap checker function based on the implicit free list.
 * The first fit policy I used is first fit. This policy finds the first block that fits the size requirement.
 * And the final score is: 'Perf index = 43 (util) + 15 (thru) = 58/100'.
 *
 * 2. Secondly, I tried another fit policy, which is the next fit policy.
 * When finding the suitable free block, we start at the position where the last search ended
 * and go to the next block until find a suitable one or hit the end of the free list.
 * By the way, when considering splitting a block, we first check if the current free block is large enough to be split.
 * In other words, we check if the remaining block after the split is big enough to form a new block.
 * If the remaining block is large enough, we proceed with the split.
 * And the final score is: 'Perf index = 42 (util) + 38 (thru) = 80/100'.
 *
 * 3. Thirdly, I tried to implement the segregated free list to improve the performance.
 * The segregated free list is a list of free blocks that are segregated by size.
 * The free list is divided into several lists, each of which contains blocks within a certain size range.
 * When searching for a free block, we first search the free list that contains blocks of the same size range as the requested size.
 * If we cannot find a suitable block in that list, we move on to the next list.
 * And the final score is: 'Perf index = 56 (util) + 40 (thru) = 96/100'.
 */

// include the necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "mm.h"
#include "memlib.h"

// Define the basic constant
#define WSIZE 8             // Word Size : 8 bytes in a 64-bit system
#define DSIZE 16            // Double Word Size: 16 bytes in a 64-bit system
#define CHUNKSIZE (1 << 13) // Extend heap by this amount (8KB)

// Define the basic function
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

// Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// Read and write a word at address p
#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))

// Read the size and allocated fields from address p
#define GET_SIZE(p) (GET(p) & ~0x7)

// Get the allocate bit from the address p
#define GET_ALLOC(p) (GET(p) & 0x1)

// Given block ptr bp, compute address of its header and footer
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// Given block ptr bp, compute address of next and previous blocks
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

// Given block ptr bp, compute address of the predecessor and successor
#define PRED_PTR(bp) ((char *)bp)
#define SUCC_PTR(bp) ((char *)bp + WSIZE)

// Get the address of the list pointer
#define GET_LIST_PTR(bp) (*(char **)bp)

// Get the address of the predecessor and successor
#define GET_PREDECESSOR(bp) (GET_LIST_PTR(bp))
#define GET_SUCCESSOR(bp) (GET_LIST_PTR(SUCC_PTR(bp)))

// Set the address of the list pointer to the new address
#define SELECT_SEG_ENTRY(i) (*(seg_free_listp + i))
#define MAXSEGENTRY 16

// Function prototypes
static char *heap_listp = 0;      // Pointer to the first block
static char **seg_free_listp = 0; // Pointer to the first free block

// Internal function prototypes for the malloc/free
__attribute__((always_inline)) static inline void *coalesce(void *bp);
__attribute__((always_inline)) static inline void *extend_heap(size_t words);
__attribute__((always_inline)) static inline void insert_node(void *bp, size_t size);
__attribute__((always_inline)) static inline void delete_node(void *bp);
__attribute__((always_inline)) static inline void *place(void *bp, size_t asize);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // Create the initial empty heap
    if ((seg_free_listp = mem_sbrk(MAXSEGENTRY * sizeof(void *))) == (void *)(-1))
        return -1;

    // Initialize the free list
    for (int i = 0; i < MAXSEGENTRY; i++)
        SELECT_SEG_ENTRY(i) = NULL;

    // Create the initial empty heap
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)(-1))
        return -1;

    // Initialize the heap with the prologue block and epilogue block
    PUT(heap_listp, 0);
    PUT((heap_listp + WSIZE), PACK(DSIZE, 1));
    PUT((heap_listp + (2 * WSIZE)), PACK(DSIZE, 1));
    PUT((heap_listp + (3 * WSIZE)), PACK(0, 1));
    heap_listp += (2 * WSIZE);

    // Extend the heap with a free block of CHUNKSIZE bytes
    if (extend_heap((1 << 9) / WSIZE) == NULL)
        return -1;

    // mm_check(0, __LINE__);
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp = NULL;

    // Ignore the spurious request
    if (heap_listp == 0)
        mm_init();

    // Adjust the block size to include overhead and alignment requirements
    if (size <= 0)
        return NULL;
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    // Search the free list to find the suitable free block
    int list_entry_num = 0;
    size_t search_size = asize;
    while (list_entry_num < MAXSEGENTRY)
    {
        if (((list_entry_num == MAXSEGENTRY - 1) || (search_size <= 1)) &&
            (SELECT_SEG_ENTRY(list_entry_num) != NULL))
        {
            bp = SELECT_SEG_ENTRY(list_entry_num);
            // Find the suitable free block
            while ((bp != NULL) && (GET_SIZE(HDRP(bp))) < asize)
                bp = GET_SUCCESSOR(bp);
            if (bp != NULL)
            {
                bp = place(bp, asize);
                return bp;
            }
        }
        search_size = search_size >> 1;
        list_entry_num++;
    }

    // No suitable free block found, extend the heap
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;

    bp = place(bp, asize);

    // mm_check(0, __LINE__);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    // Return if the block pointer is NULL
    if (bp == 0)
        return;

    // Get the block size
    if (heap_listp == 0)
        mm_init();

    // Get the block size
    size_t block_size = (size_t)GET_SIZE(HDRP(bp));

    // Mark the block as free
    PUT(HDRP(bp), PACK(block_size, 0));
    PUT(FTRP(bp), PACK(block_size, 0));
    coalesce(bp);

    // mm_check(0, __LINE__);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *bp, size_t size)
{
    void *new_block = bp;

    // If the block pointer is NULL, call mm_malloc
    if (bp == NULL)
        return mm_malloc(size);

    // If the size is 0, call mm_free
    if (size == 0)
    {
        mm_free(bp);
        return NULL;
    }

    // Adjust the block size to include overhead and alignment requirements
    if (size <= DSIZE)
        size = 2 * DSIZE;
    else
        size = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    // Get the old block size
    size_t old_size = (size_t)GET_SIZE(HDRP(bp));
    if (old_size >= size)
        return bp;

    // Check the next block
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t next_blk_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));

    // Check if the next block can be merged
    if (!next_alloc && ((old_size + next_blk_size) >= size))
    {
        delete_node(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(old_size + next_blk_size, 1));
        PUT(FTRP(bp), PACK(old_size + next_blk_size, 1));
        return bp;
    }

    // Allocate new block
    new_block = mm_malloc(size);
    if (new_block == NULL)
        return NULL;

    // Copy the old data to the new block
    if (size < old_size)
        old_size = size;
    memcpy(new_block, bp, old_size);
    mm_free(bp);

    // mm_check(0, __LINE__);
    return new_block;
}

// Extend the heap with the size of words
__attribute__((always_inline)) static inline void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    // Extend the heap with the size of words
    if ((bp = mem_sbrk(size)) == (void *)-1)
        return NULL;

    // Initialize the free block header/footer and the new epilogue header
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    GET_PREDECESSOR(bp) = NULL;
    GET_SUCCESSOR(bp) = NULL;

    return coalesce(bp);
}

// Coalesce the free block with the adjacency free block
__attribute__((always_inline)) static inline void *coalesce(void *bp)
{
    size_t is_prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t is_next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    // Case 1: Both previous block and next block are allocated
    if (is_prev_alloc && is_next_alloc)
    {
        insert_node(bp, size);
        return bp;
    }
    // Case 2: Previous block are freed but the next block are allocated
    else if (is_prev_alloc && !is_next_alloc)
    {
        // Coalesce the current block and the next block
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_node(NEXT_BLKP(bp));
        // Update the header and footer of the current block
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    // Case 3: Previous block are allocated but the next block are freed
    else if (!is_prev_alloc && is_next_alloc)
    {
        // Coalesce the current block and the previous block
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete_node(PREV_BLKP(bp));
        // Update the header and footer of the previous block
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    // Case 4: Both previous block and next block are freed
    else if (!is_next_alloc && !is_prev_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        // Remove the previous block and next block from the free list
        delete_node(PREV_BLKP(bp));
        delete_node(NEXT_BLKP(bp));
        // Update the header and footer of the previous block
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    GET_PREDECESSOR(bp) = NULL;
    GET_SUCCESSOR(bp) = NULL;
    insert_node(bp, size);
    return bp;
}

// Insert the free block into the free list
__attribute__((always_inline)) static inline void insert_node(void *bp, size_t size)
{
    int list_entry_num = 0; // Keep the track the free list entry number
    void *insert_ptr = bp;  // Keep the track the insert pointer
    void *next_ptr = NULL;  // Keep the track the next pointer

    // Select the suitable free list
    while ((list_entry_num < (MAXSEGENTRY - 1)) && (size > 1))
    {
        size = size >> 1;
        list_entry_num++;
    }
    insert_ptr = SELECT_SEG_ENTRY(list_entry_num);
    if (insert_ptr != NULL)
        next_ptr = GET_SUCCESSOR(insert_ptr);
    else
        next_ptr = NULL;

    // Find the suitable position to insert the free block
    while ((insert_ptr != NULL) && (size > GET_SIZE(HDRP(insert_ptr))))
    {
        next_ptr = insert_ptr;
        insert_ptr = GET_SUCCESSOR(insert_ptr);
    }

    // Just like insertion, we need to separate insertion in separate situation
    if (insert_ptr != NULL)
    {
        // When the previous block is not NULL
        if (next_ptr != NULL)
        {
            GET_PREDECESSOR(bp) = insert_ptr;
            GET_SUCCESSOR(insert_ptr) = bp;
            GET_SUCCESSOR(bp) = next_ptr;
            GET_PREDECESSOR(next_ptr) = bp;
        }
        // When the previous block is NULL
        else
        {
            GET_PREDECESSOR(bp) = insert_ptr;
            GET_SUCCESSOR(insert_ptr) = bp;
            GET_SUCCESSOR(bp) = NULL;
        }
    }
    // When the previous block is NULL
    else
    {
        // When the next block is not NULL
        if (next_ptr != NULL)
        {
            GET_PREDECESSOR(bp) = NULL;
            GET_SUCCESSOR(bp) = next_ptr;
            GET_PREDECESSOR(next_ptr) = bp;
            SELECT_SEG_ENTRY(list_entry_num) = bp;
        }
        // When the next block is NULL
        else
        {
            GET_PREDECESSOR(bp) = NULL;
            GET_SUCCESSOR(bp) = NULL;
            SELECT_SEG_ENTRY(list_entry_num) = bp;
        }
    }
}

// Delete the free block from the free list
__attribute__((always_inline)) static inline void delete_node(void *bp)
{
    int list_entry_num = 0;           // Keep the track the free list entry number
    size_t size = GET_SIZE(HDRP(bp)); // Get the size of the free block

    // Select the suitable free list
    while ((list_entry_num < (MAXSEGENTRY - 1)) && (size > 1))
    {
        size = size >> 1;
        list_entry_num++;
    }

    // Just like deletion, we need to separate deletion in separate situation
    if (GET_PREDECESSOR(bp) != NULL)
    {
        // When the previous block is not NULL
        if (GET_SUCCESSOR(bp) != NULL)
        {
            GET_SUCCESSOR(GET_PREDECESSOR(bp)) = GET_SUCCESSOR(bp);
            GET_PREDECESSOR(GET_SUCCESSOR(bp)) = GET_PREDECESSOR(bp);
        }
        // When the next block is NULL
        else
        {
            GET_SUCCESSOR(GET_PREDECESSOR(bp)) = NULL;
        }
    }
    // When the previous block is NULL
    else
    {
        // When the next block is not NULL
        if (GET_SUCCESSOR(bp) != NULL)
        {
            SELECT_SEG_ENTRY(list_entry_num) = GET_SUCCESSOR(bp);
            GET_PREDECESSOR(GET_SUCCESSOR(bp)) = NULL;
        }
        // When the next block is NULL
        else
        {
            SELECT_SEG_ENTRY(list_entry_num) = NULL;
        }
    }
}

// Place the block into the free list
__attribute__((always_inline)) static inline void *place(void *bp, size_t asize)
{
    size_t block_size = GET_SIZE(HDRP(bp));
    size_t remaining_size = block_size - asize;
    delete_node(bp);

    // The current block don't need to be splitted
    if (remaining_size <= 2 * DSIZE)
    {
        PUT(HDRP(bp), PACK(block_size, 1));
        PUT(FTRP(bp), PACK(block_size, 1));
    }
    // The current block can't be splitted
    else if (asize >= 96)
    {
        /**
         * 96 is empirical size inspired by the following link:
         * https://github.com/sally20921/malloclab/blob/master/malloclab-handout/src/mm.c
         */
        PUT(HDRP(bp), PACK(remaining_size, 0));
        PUT(FTRP(bp), PACK(remaining_size, 0));
        GET_PREDECESSOR(bp) = NULL;
        GET_SUCCESSOR(bp) = NULL;
        PUT(HDRP(NEXT_BLKP(bp)), PACK(asize, 1));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(asize, 1));
        coalesce(bp);
        return NEXT_BLKP(bp);
    }
    // The current block can be splitted
    else
    {
        // Mark the old block pointer as allocated
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        // Mark the new block pointer as free
        PUT(HDRP(NEXT_BLKP(bp)), PACK(remaining_size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(remaining_size, 0));
        GET_PREDECESSOR(NEXT_BLKP(bp)) = NULL;
        GET_SUCCESSOR(NEXT_BLKP(bp)) = NULL;
        coalesce(NEXT_BLKP(bp));
    }
    return bp;
}

// Debugging function prototypes
void check_block(void *bp);
void print_block(void *bp);

/*
 * mm_check - Check the heap for correctness
 */
void mm_check(int verbose, int lineno)
{
    // Check the heap for consistency and print the result
    void *bp = heap_listp;
    int list_entry_num = 0;
    size_t prev_size = 0;
    size_t prev_alloc = 1;

    if (verbose)
        printf("Heap (%p):\n", heap_listp);
    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");

    // Check each block in the heap
    check_block(heap_listp);

    // Check the free list for consistency
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (verbose)
            print_block(bp);
        check_block(bp);
        // Check if the block is in the free list and the predecessor and successor pointers are consistent
        if (GET_ALLOC(HDRP(bp)) == 0)
        {
            if (prev_alloc == 0)
                printf("Two consecutive free blocks: %p and %p\n", PREV_BLKP(bp), bp);
            if (GET_PREDECESSOR(bp) != NULL && GET_SUCCESSOR(bp) != NULL)
            {
                if (GET_SUCCESSOR(GET_PREDECESSOR(bp)) != bp)
                    printf("Inconsistent predecessor and successor pointers: %p\n", bp);
                if (GET_PREDECESSOR(GET_SUCCESSOR(bp)) != bp)
                    printf("Inconsistent predecessor and successor pointers: %p\n", bp);
            }
            list_entry_num = 0;
            while (list_entry_num < MAXSEGENTRY)
            {
                if (SELECT_SEG_ENTRY(list_entry_num) == bp)
                    break;
                list_entry_num++;
            }
            if (list_entry_num == MAXSEGENTRY)
                printf("Free block not in free list: %p\n", bp);
        }
        prev_size = GET_SIZE(HDRP(bp));
        prev_alloc = GET_ALLOC(HDRP(bp));
    }

    // Check the epilogue block
    if (verbose)
        print_block(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");
}

// To check the block whether it is aligned and the header/footer is matched
void check_block(void *bp)
{
    if ((size_t)bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);

    if (GET(HDRP(bp)) != GET(FTRP(bp)))
        printf("Error: header does not match footer\n");
}

// To print the block information in the heap
void print_block(void *bp)
{
    size_t hsize, halloc, fsize, falloc; // Header size, Header allocate, Footer size, Footer allocate

    // Get the block information
    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    printf("%p: header: [%zu:%c] footer: [%zu:%c]\n", bp, hsize, (halloc ? 'a' : 'f'), fsize, (falloc ? 'a' : 'f'));
}
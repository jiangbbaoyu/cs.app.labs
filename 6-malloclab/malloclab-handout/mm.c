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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

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
    ""
};

/* single word (4 bytes) or double word (8 bytes) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Size of word and double word */
#define WSIZE 4
#define DSIZE 8

/* initialize the heap with this size . 64bytes */
#define  INITCHUNKSIZE (1<<6)

/* extend the heap with this size (4k bytes)*/
#define CHUNKSIZE (1<<12)

/* free lists size */
#define LISTSIZE 16

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/*pack a size and allocated/free bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

// Read and write a word into address p
#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) ((*(unsigned int *)(p)) = (val))

/* store ptr value to the location specified by p */
#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned  int )(ptr))

// Read the size and allocated fields from address p
#define GET_SIZE(p) (GET(p) & ~0x7) // latest 3 bits not used for represent size
#define GET_ALLOC(p) (GET(p) & 0x1) // latest 1 bit used for indicate allocated or not

// given block pointer ptr, compute  block header and footer
#define HDRP(ptr) ((char*)(ptr) - WSIZE)
#define FTRP(ptr) ((char*)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE )

// given current block pointer ptr, compute the prev block pointer and the next block pointer.
#define NEXT_BLK_PTR(ptr) ((char*)(ptr) + GET_SIZE(HDRP(ptr)))
#define PREV_BLK_PTR(ptr) ((char*)(ptr) - GET_SIZE((char*)(ptr) - DSIZE) )

/* The successor free block pointer's pointer and the predecessor free block pointer's pointer `in the current free block`*/
#define PRED_PTR(ptr) ((char *)(ptr))
#define SUCC_PTR(ptr) ((char *)(ptr) + WSIZE)

// The successor free block pointer and the predecessor free block pointer.
#define PRED(ptr) ( (void *)(GET(ptr)))
#define SUCC(ptr) ( (void *)(GET(SUCC_PTR(ptr))))


/* Data structure

Allocated block:

                            31  30  29  ... 5   4   3   2   1   0
                            +-------------------------+-------+--+
    Header:                 |   Size of the block     |       | A|
        block pointer +-->  +-------------------------+-------+--+
                            |                                    |
                            |   Payload                          |
                            |                                    |
                            +------------------------------------+
                            |   Padding(optional)                |
                            +-------------------------+-------+--+
    Footer:                 |   Size of the block     |       | A|
                            +-------------------------+-------+--+

Free block:

                            31  30  29  ... 5   4   3   2   1   0
                            +-------------------------+-------+--+
    Header:                 |   size of the block     |       | A|
        block pointer +-->  +-------------------------+-------+--+
                            |   pointer to its predecessor       |
                            +------------------------------------+
                            |   pointer to its successor         |
                            +------------------------------------+
                            |                                    |
                            |   Payload                          |
                            |                                    |
                            +------------------------------------+
                            |   Padding(optional)                |
                            +-------------------------+-------+--+
    Footer:                 |   size of the block     |       | A|
                            +-------------------------+-------+--+


Heap:
                            31  30  29  ... 5   4   3   2   1   0
    Start of heap:          +-------------------------+-------+--+
                            |   0(Padding)            |       |  |
                            +-------------------------+-------+--+  <--+
                            |   8                     |       | A|     |
static char *heap_listp +-> +-------------------------+-------+--+     +--  Prologue block
                            |   8                     |       | A|     |
                            +-------------------------+-------+--+  <--+
                            |                                    |
                            |                                    |
                            |   Blocks                           |
                            |                                    |
                            |                                    |
                            +-------------------------+-------+--+  <--+
    Footer:                 |   0                     |       | A|     +--  Epilogue block
                            +-------------------------+-------+--+  <--+

*/


/*
 Segregated free lists:

                            +---+---+---+---+---+---+---+---+---+
                            |   |   |   |   |   |   |   |   |   |
                            +-+-+---+---+---+---+---+---+---+---+
                              |       |           |
                              v       v           v
                            +---+   +---+       +---+
                            |   |   |   |       |   |
                            +---+   +---+       +---+
                              |
                              v
                            +---+
                            |   |
                            +---+
 */
void* segregated_free_lists [LISTSIZE];

// Extend the heap
static void* extend_heap(size_t size);
// Insert the free block to the free list
static void insert_node(void* bp);
// Delete the free block from the free list
static void delete_node(void *block_ptr);

// Coalesce adjacent free block if exists
static void* coalesce(void *bp);
// Place a block with this size to the free block ptr
static void* place(void* bp,size_t size);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    char* heap;
    for (int i=0;i<LISTSIZE;i++)
    {
        segregated_free_lists[i] = NULL;
    }

    /*initialize heap */
    if((long)(heap = mem_sbrk(4 * WSIZE)) == -1)
    {
        return -1;
    }

    PUT(heap,0); // alignment padding
    PUT(heap+(1*WSIZE), PACK(DSIZE,1)); //prologue header
    PUT(heap+(2*WSIZE), PACK(DSIZE,1)); //prologue footer
    PUT(heap+(3*WSIZE), PACK(0,1));     // Epilogue header

    if (extend_heap(CHUNKSIZE)== NULL)
    {
        return -1;
    }

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if (size <= 0)
        return NULL;

    if (size < DSIZE)
        size = 2*DSIZE;
    else
        size = ALIGN(size) +DSIZE;

    size_t search_size = size;
    void* bp = NULL;
    for (int target_list_no = 0; target_list_no < LISTSIZE; ++target_list_no) {

        if (search_size <=1 && segregated_free_lists[target_list_no] != NULL)
        {
            // found the suitable-sized free list ( segregated_free_lists[target_list_no, LISTSIZE]  are all matched free lists )
            bp = segregated_free_lists[target_list_no];
            while (bp != NULL && GET_SIZE(HDRP(bp)) < size )
            {
                bp = SUCC(bp);
            }

            if (bp != NULL)
                break;  // GET_SIZE(HDRP(bp)) >=  size, found the suitable-sized block

            // if reach here, this free list don't contains a block which size is larger than or equal to `size`
            // then search the next larger free list if exists
        }
        search_size >>= 1;
    }

    // There are no suitable block in the free lists, extend the heap
    if (bp == NULL)
    {
        if ((bp = extend_heap(MAX(size,CHUNKSIZE))) == NULL)
            return NULL; // extend failed
    }

    bp = place(bp,size);
    return bp;
}

/*
 * mm_free - Freeing a block
 */
void mm_free(void *ptr)
{
    size_t size =GET_SIZE(HDRP(ptr));

    // Reset header and footer for current block
    PUT(HDRP(ptr), PACK(size,0));
    PUT(FTRP(ptr), PACK(size,0));

    // Insert current block to free list
    insert_node(ptr);

    // Coalesce adjacent free block if exists
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if (size <-0)
        return NULL;

    if (size < DSIZE)
        size = 2*DSIZE;
    else
        size = ALIGN(size) + 2*DSIZE;

    size_t current_size = GET_SIZE(HDRP(ptr));

    // 1. If target size is smaller than or equal to current size, return the block pointer directly
    if ( size <= current_size)
        return ptr;

    // 2. Target size is bigger than current size
    // 2.1. If the next block is the epilogue block, extend the heap directly
    if (GET_SIZE(HDRP(NEXT_BLK_PTR(ptr))) == 0){

        size_t extend_size = MAX(size - current_size,CHUNKSIZE);
        if (extend_heap(extend_size) == NULL)
            return NULL;


        size_t new_blk_size = extend_size + current_size;
        PUT(HDRP(ptr), PACK(new_blk_size,1));
        PUT(FTRP(ptr), PACK(new_blk_size,1));

        place(ptr,size);
        return ptr;
    }

    // 2.2. If the next block is a free block, and the size if enough to resize the new block
    if (!GET_ALLOC(HDRP(NEXT_BLK_PTR(ptr))))
    {
        size_t next_free_blk_size = GET_SIZE(HDRP(NEXT_BLK_PTR(ptr)));
        size_t new_blk_size = next_free_blk_size + current_size;
        if (new_blk_size >= size)
        {
            PUT(HDRP(ptr), PACK(new_blk_size,1));
            PUT(FTRP(ptr), PACK(new_blk_size,1));

            place(ptr,size);
            return ptr;
        }
    }

    // 2.3. Allocate a new block with the target size
    void *new_block = mm_malloc(size);
    memcpy(new_block, ptr, GET_SIZE(HDRP(ptr)));
    mm_free(ptr);
    return new_block;
}



static void* extend_heap(size_t size)
{
    void* bp;
    size = ALIGN(size);// rounds up to the nearest multiple of ALIGNMENT
    if ((bp=mem_sbrk(size)) == (void*)-1)
    {
        return NULL;
    }
    /* init the header and the footer of new free block*/
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));

    /*Set the epilogue block of the heap*/
    PUT(HDRP(NEXT_BLK_PTR(bp)), PACK(0,1));
    insert_node(bp);

    return coalesce(bp);
}

static void insert_node(void* bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    int target_list_no =0;

    void* succ_blk_ptr = NULL;
    void* prev_blk_ptr = NULL;

    // Find the corresponding free list for current block
    /*                              size
     * segregated_free_lists[0] ->  1
     * segregated_free_lists[1] ->  [2,3]
     * segregated_free_lists[2] ->  [4,7]
     * segregated_free_lists[3] ->  [8,15]
     * ....
     */
    while ( (target_list_no < LISTSIZE -1) && (size >1) )
    {
        size >>= 1;
        target_list_no ++;
    }

    // Find the insert position for block, keep free list in ascending order
    succ_blk_ptr = segregated_free_lists[target_list_no];
    while ((succ_blk_ptr != NULL) && GET_SIZE(HDRP(succ_blk_ptr))<size)
    {
        prev_blk_ptr = succ_blk_ptr;
        succ_blk_ptr = SUCC(succ_blk_ptr);
    }

    /* There are 4 situations */
    if (succ_blk_ptr == NULL){
        // 1. The list is empty
        if (prev_blk_ptr == NULL)
        {
            SET_PTR(PRED_PTR(bp),NULL); // Set current block's predecessor pointer
            SET_PTR(SUCC_PTR(bp),NULL); // Set current block's successor pointer
            segregated_free_lists[target_list_no] = bp;
        }else{
            // 2. The insert position is the end of the list
            SET_PTR(PRED_PTR(bp),prev_blk_ptr); // Set current block's predecessor pointer
            SET_PTR(SUCC_PTR(bp),NULL); // Set current block's successor pointer

            // SET_PTR(SUCC_PTR(PRED(bp)),bp);
            SET_PTR(SUCC_PTR(prev_blk_ptr),bp); // Set successor of the block's predecessor
        }
    }else{
        // 3. The insert position is the beginning of the list, and list is not empty
        if (prev_blk_ptr == NULL)
        {
            SET_PTR(PRED_PTR(bp),NULL); // Set current block's predecessor pointer
            SET_PTR(SUCC_PTR(bp),succ_blk_ptr); // Set current block's successor pointer

            //SET_PTR(PRED_PTR(SUCC(bp)),bp);
            SET_PTR(PRED_PTR(succ_blk_ptr),bp); // Set predecessor of the block's  successor

            segregated_free_lists[target_list_no] = bp;
        }else{
            // 4. The insert position is neither the beginning nor the end of the list
            SET_PTR(PRED_PTR(bp),prev_blk_ptr); // Set current block's predecessor pointer
            SET_PTR(SUCC_PTR(bp),succ_blk_ptr); // Set current block's successor pointer

            SET_PTR(SUCC_PTR(prev_blk_ptr),bp); // Set predecessor block's successor
            SET_PTR(PRED_PTR(succ_blk_ptr),bp); // Set successor block's predecessor
        }
    }
}

// Coalesce adjacent free block if exists
static void* coalesce(void *bp)
{

    _Bool prev_blk_allocated = GET_ALLOC(HDRP(PREV_BLK_PTR(bp)));
    _Bool next_blk_allocated = GET_ALLOC(HDRP(NEXT_BLK_PTR(bp)));
    size_t curr_blk_size = GET_SIZE(HDRP(bp));

    /* There are 4 situations */
    if (prev_blk_allocated && next_blk_allocated)
    {
        // 1. The previous block and the next block are both allocated
        return bp;
    }
    else if(prev_blk_allocated && !next_blk_allocated)
    {
        // 2. The previous block was allocated, the next block is free
        size_t netx_blk_size = GET_SIZE(HDRP(NEXT_BLK_PTR(bp)));
        size_t coalesced_size = netx_blk_size + curr_blk_size;

        // Delete current block and the next block from free list
        delete_node(bp);
        delete_node(NEXT_BLK_PTR(bp));
        // Reset header and footer for the new coalesced block
        PUT(HDRP(bp), PACK(coalesced_size,0));
        PUT(FTRP(bp), PACK(coalesced_size,0)); // use updated new size to locate the footer
    }
    else if (!prev_blk_allocated && next_blk_allocated)
    {
        // 3. The previous block is free, the next block was allocated
        size_t prev_blk_size = GET_SIZE(HDRP(PREV_BLK_PTR(bp)));
        size_t coalesced_size = prev_blk_size + curr_blk_size;

        // Delete current block and the previous block from free list
        delete_node(bp);
        delete_node(PREV_BLK_PTR(bp));
        // Reset header and footer for the new coalesced block
        PUT(HDRP(PREV_BLK_PTR(bp)), PACK(coalesced_size,0)); // header
        PUT(FTRP(PREV_BLK_PTR(bp)), PACK(coalesced_size,0)); // footer
        // alternative for footer
        // PUT(FTRP(bp), PACK(coalesced_size,0));

        // Reset current block's pointer to the new coalesced block
        bp = PREV_BLK_PTR(bp);
    }
    else
    {
        // 4. The previous block and the next block are both free
        size_t prev_blk_size = GET_SIZE(HDRP(PREV_BLK_PTR(bp)));
        size_t next_blk_size = GET_SIZE(HDRP(NEXT_BLK_PTR(bp)));
        size_t coalesced_size = prev_blk_size + next_blk_size + curr_blk_size;

        // Delete current block / previous block/ next block from free list
        delete_node(bp);
        delete_node(PREV_BLK_PTR(bp));
        delete_node(NEXT_BLK_PTR(bp));
        // Reset header and footer for the new coalesced block
        PUT(HDRP(PREV_BLK_PTR(bp)), PACK(coalesced_size,0));
        PUT(FTRP(NEXT_BLK_PTR(bp)), PACK(coalesced_size,0));

        // Reset current block's pointer to the new coalesced block
        bp = NEXT_BLK_PTR(bp);
    }

    // Insert this new coalesced block into the free lists
    insert_node(bp);

    return bp;
}

static void delete_node(void* bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    int target_list_no =0;
    // Find the corresponding free list for current block
    while ( (target_list_no <LISTSIZE - 1) && (size > 1) )
    {
        size >>= 1;
        target_list_no ++;
    }

    // there are 4 cases
    if (PRED(bp) == NULL)
    {
        if (SUCC(bp) == NULL)
        {
            // 1. target free list only contains this block, set the free list empty
            segregated_free_lists[target_list_no] = NULL;
        }
        else
        {
            // 2. this block is the first block of target free list, set the free list's first block to the successor of this block
            SET_PTR(PRED_PTR(SUCC(bp)), NULL);
            SET_PTR(SUCC_PTR(bp), NULL);
            segregated_free_lists[target_list_no] = SUCC(bp);
        }
    }else
    {
        if (SUCC(bp) == NULL)
        {
            // 3. The current block is the end of the list
            SET_PTR(SUCC_PTR(PRED(bp)),NULL);
            SET_PTR(PRED_PTR(bp),NULL);
        }
        else
        {
            // 4. The current block is neither the beginning nor the end of the list
            SET_PTR(SUCC_PTR(PRED(bp)), SUCC(bp));
            SET_PTR(PRED_PTR(bp),NULL);

            SET_PTR(PRED_PTR(SUCC(bp)), PRED(bp));
            SET_PTR(SUCC_PTR(bp),NULL);
        }
    }
}

static void* place(void* bp,size_t size)
{
    size_t blk_size = GET_SIZE(HDRP(bp));
    size_t frag_size = blk_size -size;

    delete_node(bp);

    if (frag_size >= 2*DSIZE)
    {
        // split
        //1. set the current block allocated
        PUT(HDRP(bp), PACK(size,1));
        PUT(FTRP(bp), PACK(size,1));

        //2. treat inner fragment as a new free block
        PUT(HDRP(NEXT_BLK_PTR(bp)), PACK(frag_size,0));
        PUT(FTRP(NEXT_BLK_PTR(bp)), PACK(frag_size,0));

        //3. insert into free list
        insert_node(NEXT_BLK_PTR(bp));

        // 4. do possible coalesce
        coalesce(NEXT_BLK_PTR(bp));

    }
    else
    {
        PUT(HDRP(bp), PACK(blk_size,1));
        PUT(FTRP(bp), PACK(blk_size,1));
    }

    return bp;
}












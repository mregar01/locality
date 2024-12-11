/*
 *     uarray2b.c
 *     by Prithviraj Singh Shahani (pshaha01) and Max Regardie (mregar01), 
 *     2/22/23
 *     
 *     locality
 *
 *     This is the implementation file for our UArray2b interface.
 *
 */

#include "uarray2b.h"
#include "uarray2.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define T UArray2b_T

struct T {
        int width; 
        int height; 
        int size; 
        int blocksize;
        int totalblocks;
        UArray2_T elems;
};

extern T UArray2b_new (int width, int height, int size, int blocksize)
{
        assert(blocksize > 0);
        assert(width > 0);
        assert(height > 0);

        T toReturn = malloc(sizeof(struct T));
        toReturn->width = width;
        toReturn->height = height;
        toReturn->size = size;
        toReturn->blocksize = blocksize;

        /* ceiling to get upper bound of how many blocks we need */
        int blockwidth = ceil((float)width / blocksize);
        int blockheight = ceil((float)height / blocksize);
        int totalblocks = (blockheight * blockwidth);
        
        UArray2_T blocks = UArray2_new(blockwidth, blockheight, 
                (sizeof(UArray2_T)));
        
        int col, row;

        /* Initializing the blocks (UArray2s) that store elements */
        for (int i = 0; i < totalblocks; i++) {
                row = i / blockwidth;
                if (row == 0) {
                        col = i;
                        
                } else {
                        col = (i % blockwidth);
                }
                                
                UArray2_T curr = UArray2_new(blocksize, blocksize, size);
                *(UArray2_T *)UArray2_at(blocks, col, row) = curr;
        }

        toReturn->totalblocks = totalblocks;
        toReturn->elems = blocks;

        return toReturn;
}

extern T UArray2b_new_64K_block(int width, int height, int size)
{
        assert(width > 0);
        assert(height > 0);
        int blocktotal = (64000*1024)/size;
        int maxsize = sqrt(blocktotal);
        T toreturn = UArray2b_new(width, height, size, maxsize);
        return toreturn;
}

extern void UArray2b_free (T *array2b) 
{
        assert(array2b && *array2b);
        int col, row;
        int width = UArray2b_width(*array2b);
        int blocksize = UArray2b_blocksize(*array2b);
        int blockwidth = ceil((float)width / blocksize);
        int size = (*array2b)->totalblocks;

        /* free each block (UArray2) */ 
        for (int i = 0; i < size; i++) {
                row  = i / blockwidth;
                if (row == 0) {
                        col = i;
                } else {
                        col = (i % blockwidth);
                }
                UArray2_free((UArray2_T*) UArray2_at(((*array2b)->elems), 
                                                                col, row));
        }

        UArray2_free(&(*array2b)->elems);
        free (*array2b);
}

extern int UArray2b_width (T  array2b)
{
        assert(array2b != NULL);
        return array2b->width;
}

extern int UArray2b_height (T  array2b)
{
        assert(array2b != NULL);
        return array2b->height;
}

extern int UArray2b_size (T  array2b)
{
        assert(array2b != NULL);
        return array2b->size;
}

extern int UArray2b_blocksize (T  array2b)
{
        assert(array2b != NULL);
        return array2b->blocksize;
}

extern void *UArray2b_at (T array2b, int column, int row)
{
        assert(array2b != NULL);

        int width = array2b->width; 
        int height = array2b->height;
        assert(column >= 0 && column < width);
        assert(row >= 0 && row < height);

        int blocksize = array2b->blocksize;

        /* Find block */
        UArray2_T *currBlock = (UArray2_T*) UArray2_at((array2b->elems),
                                (column / blocksize), (row / blocksize));
        
        /* Find index within block */
        void *currVal = UArray2_at(*currBlock, (column % blocksize), 
                                                        (row % blocksize));
        return currVal;
}

extern void UArray2b_map (T array2b,
        void apply(int col, int row, T array2b, void *elem, void *cl),
        void *cl)
{

        assert(array2b != NULL);
        assert(apply != NULL);
        int blocksize = array2b->blocksize;
        int blockRow, blockCol, cellRow, cellCol, overallRow, overallCol;

        int width = UArray2b_width(array2b);
        int height = UArray2b_height(array2b);
        int numblocks = ceil((float)width / blocksize);
        int size = array2b->totalblocks;

        /* find block to map into */
        for (int i = 0; i < size; i++) {
                blockRow  = i / numblocks;
                if (blockRow == 0) {
                        blockCol = i;
                } else {
                        blockCol = (i % numblocks);
                }

                UArray2_T currBlock = *(UArray2_T*) UArray2_at(
                                (array2b->elems), blockCol, blockRow);

                int blockwidth = UArray2_width(currBlock);

                /* find index of block to map into */
                for (int i = 0; i < (blocksize * blocksize); i++) {
                        cellRow = i / blockwidth;
                        if (cellRow == 0) {
                                cellCol = i;
                        } else {
                                cellCol = (i % blockwidth);
                        }
                        overallRow = (blockRow * blocksize) + cellRow;
                        overallCol = (blockCol * blocksize) + cellCol;
                        
                        if (overallRow < height && overallCol < width) {
        
                                void *curr = UArray2_at(currBlock, 
                                                        cellCol, cellRow);
                                apply(overallCol, overallRow, 
                                                array2b, curr, cl);
                        }
                }
        }
}
#undef T

/*
 *     uarray2.c
 *     by Prithviraj Singh Shahani (pshaha01) and Max Regardie (mregar01), 
 *     2/13/22
 *     
 *     iii
 *
 *     This is the implementation file for our UArray2 interface.
 *
 */

#include <uarray.h>
#include <assert.h>
#include "uarray2.h"
#include <stdlib.h>
#include <stdio.h>

struct UArray2_T {
        int width; /* The number of cols to represent the elems, at least 1*/
        int height; /* The number of rows to represent the elems, at least 1*/
        int size; /* number of bytes in one element */
        UArray_T elems; //Note, Hanson defines this as a pointer, no need to
        //define this as a pointer, but keep in mind when inserting into this
};

UArray2_T UArray2_new (int width, int height, int size) {
        
        int length = width * height;
        assert(width > 0);
        assert(height > 0);
        UArray_T ourArray = UArray_new(length, size);
        UArray2_T UArray2 = malloc(sizeof(ourArray) * (sizeof(width)));
        assert(UArray2);
        UArray2->elems = ourArray;
        UArray2->width = width;
        UArray2->height = height;
        UArray2->size = size;
        return UArray2;
}

void *UArray2_at(UArray2_T UArray2, int col, int row) {
        assert(UArray2);
        assert(col < UArray2_width(UArray2));
        assert(row < UArray2_height(UArray2));
        assert(col > -1 && row > -1);

        int index = (UArray2->width)*row + col;
        return UArray_at(UArray2->elems, index);
}

int UArray2_size(UArray2_T UArray2) {
        assert(UArray2);
        return UArray2->size;
}

int UArray2_width(UArray2_T UArray2) {
        assert(UArray2);
        return UArray2->width;
}

int UArray2_height(UArray2_T UArray2) {
        assert(UArray2);
        return UArray2->height;
}

void UArray2_map_row_major(UArray2_T UArray2, void apply(int col, int row, 
        UArray2_T UArray2, void *curr, void *cl), void *cl) {

        int currCol;
        int currRow;
        assert(apply);
        assert(UArray2);
        assert(cl);

        int width = UArray2_width(UArray2);

        int length = UArray_length(UArray2->elems);
        for(int i = 0; i < length; i++) {
                currRow = i / width;
                if (currRow == 0) {
                        currCol = i;
                        
                } else {
                        currCol = (i % width);
                }
                
                void *curr = UArray2_at(UArray2, currCol, currRow);
                apply(currCol, currRow, UArray2, curr, cl);
        }
}

void UArray2_map_col_major(UArray2_T UArray2, void apply(int col, int row, 
        UArray2_T UArray2, void *curr, void *cl), void *cl){
        
        assert(apply);
        assert(UArray2);
        assert(cl);
        int currCol;
        int currRow;
        int height = UArray2_height(UArray2);
        

        int length = UArray_length(UArray2->elems);
        for(int i = 0; i < length; i++) {
                if (height == 0){
                        currCol = 0;
                } else {
                        currCol = i / height;
                }
                if (((height) * currCol) == 0){
                        currRow = i;
                } else {
                        currRow = i % (height * currCol);
                }
                void *curr = UArray2_at(UArray2, currCol, currRow);
                apply(currCol, currRow, UArray2, curr, cl);
        }

}

void UArray2_free(UArray2_T *UArray2) {
        assert(UArray2 && *UArray2);

        UArray_free(&(*UArray2)->elems);
        free(*UArray2);
}


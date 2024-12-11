/*
 *     UArray2.h
 *     by Prithviraj Singh Shahani (pshaha01) and Max Regardie (mregar01), 
 *     2/13/22
 *     
 *     iii
 *
 *     This is the header file for the UArray2 interface. 
 *     
 */

#ifndef UARRAY2_INCLUDE
#define UARRAY2_INCLUDED


typedef struct UArray2_T *UArray2_T;


/**********UArray2_new********
 *
 * Creates and returns a UArray2
 * Inputs: number of columns and number of rows and size of each element
 * Return: A UArray2 of the designated dimensions to store the designated type
 *      of element
 * Expects:
 *      Size to reflect the size of a single element of the desired data type
 *              Width and height to accurately reflect the dimensions of the 
 *              desired UArray2 and that they are both greater than 0.
 *      Total number of elements stored in UArray2 doesn't exceed width*height
 *
 * Notes:
 *
 ************************/
 UArray2_T UArray2_new (int width, int height, int size);



/**********UArray2_at********
 *
 * Finds the element stored at the given col/row
 * Inputs: The UArray2, the col, and the row of the element to be found
 * Return: A void pointer pointing to the element at the given row/col
 * Expects
 *      The col/row parameters to be between 0 and the width/height of the
 *      UArray - 1.
 *              A nonnull UArray2
 *
 * Notes:
 *
 ************************/
void *UArray2_at(UArray2_T UArray2, int col, int row);


/**********UArray2_size********
 *
 * Returns the number of bytes used to store one element in UArray2
 * Inputs: The UARaay2
 * Return: Size of an element in bytes 
 * Expects: UArray2 to be nonnull 
 * 
 * Notes:
 *
 ************************/
int UArray2_size(UArray2_T UArray2);


/**********UArray2_width********
 *
 * Find and return the width of a UArray2
 * Inputs: UArray2 to retrive width from
 * Return: Width of UArray 2 as an int
 * Expects: UArray2 to be nonnull 
 *
 * Notes: Will throw a checked runtime error if the UArray2 is empty
 *
 ************************/
int UArray2_width(UArray2_T UArray2);


/**********UArray2_height********
 *
 * Find and return the height of a UArray2
 * Inputs: UArray2 to retrive height from
 * Return: Height of UArray 2 as an int
 * Expects: UArray2 to be nonnull 
 *
 * Notes: 
 *
 ************************/
int UArray2_height(UArray2_T UArray2);


/**********UArray2_map_row_major********
 *
 * Applies a function onto the elements one by one in order of row major
 * Inputs: The UArray2 storing the elements, the function to apply, and a void
 *      pointer indicating the closure of the apply function
 * Return: nothing
 * Expects: 
 *      Nonnull UArray2
 *              Working apply function
 *      Closure causes apply function to stop
 * Notes:
 *
 ************************/
void UArray2_map_row_major(UArray2_T UArray2, void apply(int col, int row, 
                        UArray2_T UArray2, void *curr, void *cl), void *cl);


/**********UArray2_map_col_major********
 *
 * Applies a function onto the elements one by one in order of col major
 * Inputs: The UArray2 storing the elements, the function to apply, and a void
 *      pointer indicating the closure of the apply function
 * Return: nothing
 * Expects: 
 *      Nonnull UArray2
 *              Working apply function
 *      Closure causes apply function to stop
 * Notes:
 *
 ************************/
void UArray2_map_col_major(UArray2_T UArray2, void apply(int col, int row, 
                        UArray2_T UArray2, void *curr, void *cl), void *cl);


/**********UArray2_free********
 *
 * Frees up all space allocated by a UArray 2
 * Inputs: pointer to an instance of a UArray 2
 * Return: nothing
 * Expects: UArray2 to be nonnull and not freed already
 *
 * Notes: none
 *
 ************************/
void UArray2_free(UArray2_T *UArray2);

#undef UArray2_T
#endif
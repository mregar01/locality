/*
 *     ppmtrans.c
 *     by Prithviraj Singh Shahani (pshaha01) and Max Regardie (mregar01), 
 *     2/23/23
 *     
 *     locality
 *
 *     This file performs rotations on ppm images and records timing data to
 *     teach us information about locality.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "cputiming.h"

#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (false)

void rotateimage(Pnm_ppm Image, int rotationDegree, A2Methods_T methods);

void applyrotation90(int col, int row, A2Methods_UArray2 currArray, void* curr,
void* newArray );

void applyrotation180(int col, int row, A2Methods_UArray2 currArray, void* curr,
void* newArray );

void applyrotation0(int col, int row, A2Methods_UArray2 currArray, void* curr, 
void* newArray );

void applyrotation270(int col, int row, A2Methods_UArray2 currArray, void* curr,
void* newArray );

void applyhorizontal(int col, int row, A2Methods_UArray2 currArray, void* curr,
void* newArray );

void applyvertical(int col, int row, A2Methods_UArray2 currArray, void* curr, 
void* newArray );

void applytranspose(int col, int row, A2Methods_UArray2 currArray, void* curr, 
void* newArray );


static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] [filename]\n",
                        progname);
        exit(1);
}

int main(int argc, char *argv[]) 
{
        Except_T cantopen = {"Can't open file\n"};
        char *time_file_name = NULL;
        int   rotation       = 0;
        int   i;
        FILE *fp = NULL;
        char *flip;
        bool isfile = false;
        bool timerOn = false;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map);

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
                                        "Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-time") == 0) {
                        time_file_name = argv[++i];
                        timerOn = true;
                } else if (strcmp(argv[i], "-flip") == 0) {
                        flip = argv[++i];
                        if (!(strcmp(flip, "horizontal") == 0 || strcmp(flip, 
                                                        "vertical") == 0)) {
                                fprintf(stderr,  
                                "Flip must be horizontal or vertical\n");
                                usage(argv[0]);
                        }
                        if (strcmp(flip, "horizontal") == 0) {
                                rotation = 360;
                        } else {
                                rotation = 450;
                        }
                } else if (strcmp(argv[i], "-transpose") == 0) {
                        rotation = 540;
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                        usage(argv[0]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        fp = fopen(argv[i], "r");
                        isfile = true;
                }
        }
        if (!isfile) {
                fp = stdin;
        } 
        
        if (fp == NULL) {
                RAISE(cantopen);
        }

        Pnm_ppm pixmap = Pnm_ppmread(fp, methods); 
        assert(pixmap != NULL);

        /*If a timer file is included, record time for rotation,
                otherwise just do the rotate*/
        if (timerOn) {
                double time_used;
                CPUTime_T timer;
                
                timer = CPUTime_New();
                CPUTime_Start(timer);
                
                rotateimage(pixmap, rotation, methods);

                time_used = CPUTime_Stop(timer);
                int pixelsperns = (time_used/((pixmap->width) * 
                                                (pixmap->height)));
                FILE *timingOutput = NULL;
                timingOutput = fopen(time_file_name, "a");

                if (timingOutput == NULL) {
                        RAISE(cantopen);
                } else {
                        fprintf(timingOutput,
                                "Total Time: %0.f ns, Time/Pixel: %d ns\n", 
                                time_used, pixelsperns);
                        CPUTime_Free(&timer);
                        fclose(timingOutput);

                }
        } else {
                rotateimage(pixmap, rotation, methods);
        }
        Pnm_ppmfree(&pixmap);
        fclose(fp);
        exit(EXIT_SUCCESS);
}

/**********rotateimage********
 *
 * function that calls different apply functions based on rotation
 * Inputs: Pnm_ppm Image, int rotationDegree, A2Methods_T methods
 * Return: none
 * 
 * Expects:
 *      *newArray to be a Pnm_ppm
 * Notes:
 *      prints out ppm after rotation has been completed
 *      This function also handles flip horizontal/vertical and transpose
 *      rotation = 360 is flip horizontal
 *      rotation = 450 is flip vertical
 *      rotation = 540 is transpose
 ************************/
void rotateimage(Pnm_ppm Image, int rotationDegree, A2Methods_T methods)
{
        assert(Image != NULL);
        assert(methods != NULL);
        unsigned width = Image->width;
        unsigned height = Image->height;

        A2Methods_UArray2 initial = Image->pixels;

        Pnm_ppm newPpm = malloc(sizeof(struct Pnm_ppm));
        assert(newPpm != NULL);

        newPpm->methods = methods;
        newPpm->denominator = Image->denominator;
        
        /*Axes are swapped for 90, 270, transpose, same as original otherwise*/
        if (rotationDegree == 90) {
                newPpm->width = height;
                newPpm->height = width;
                newPpm->pixels = methods->new(height, width, 
                                                sizeof(struct Pnm_rgb));
                methods->map_default(initial, applyrotation90, newPpm);
        } else if (rotationDegree == 180) {
                newPpm->width = width;
                newPpm->height = height;
                newPpm->pixels = methods->new(width, height, 
                                                sizeof(struct Pnm_rgb));
                methods->map_default(initial, applyrotation180, newPpm);
        } else if (rotationDegree == 0) {
                newPpm->width = width;
                newPpm->height = height;
                newPpm->pixels = methods->new(width, height, 
                                                sizeof(struct Pnm_rgb));
                methods->map_default(initial, applyrotation0, newPpm);
        } else if (rotationDegree == 270) {
                newPpm->width = height;
                newPpm->height = width;
                newPpm->pixels = methods->new(height, width, 
                                                sizeof(struct Pnm_rgb));
                methods->map_default(initial, applyrotation270, newPpm);
        } else if (rotationDegree == 360) {
                newPpm->width = width;
                newPpm->height = height;
                newPpm->pixels = methods->new(width, height, 
                                                sizeof(struct Pnm_rgb));
                methods->map_default(initial, applyhorizontal, newPpm);
        } else if (rotationDegree == 450) {
                newPpm->width = width;
                newPpm->height = height;
                newPpm->pixels = methods->new(width, height, 
                                                sizeof(struct Pnm_rgb));
                methods->map_default(initial, applyvertical, newPpm);
        } else if (rotationDegree == 540) {
                newPpm->width = height;
                newPpm->height = width;
                newPpm->pixels = methods->new(height, width, 
                                                sizeof(struct Pnm_rgb));
                methods->map_default(initial, applytranspose, newPpm);
        }
        
        Pnm_ppmwrite(stdout, newPpm);
        Pnm_ppmfree(&newPpm);
}


/**********applyrotation90********
 *
 * apply function for doing a 90 degree rotation
 * Inputs: col, row, A2Methods_UArray2 array, curr value, cl pointer
 * Return: none
 * 
 * Expects:
 *      *newArray to be a Pnm_ppm
 * Notes:
 *      
 ************************/
void applyrotation90(int col, int row, A2Methods_UArray2 currArray, void* curr,
                                                void* newArray ) 
{        
        Pnm_ppm PpmImage = (Pnm_ppm) newArray;
        assert(PpmImage != NULL);
        int height = PpmImage->methods->height(currArray);
        Pnm_rgb currPixel = (Pnm_rgb) curr;
        *(Pnm_rgb) PpmImage->methods->at(PpmImage->pixels, (height - row - 1),
                                                col) = *currPixel; 
}

/**********applyrotation180********
 *
 * apply function for doing a 180 degree rotation
 * Inputs: col, row, A2Methods_UArray2 array, curr value, cl pointer
 * Return: none
 * 
 * Expects:
 *      *newArray to be a Pnm_ppm
 * Notes:
 *      
 ************************/
void applyrotation180(int col, int row, A2Methods_UArray2 currArray,
                                        void* curr, void* newArray )
{       
        Pnm_ppm PpmImage = (Pnm_ppm) newArray;
        assert(PpmImage != NULL);
        int height = PpmImage->methods->height(currArray);
        int width = PpmImage->methods->width(currArray);
        Pnm_rgb currPixel = (Pnm_rgb) curr;
        *(Pnm_rgb) PpmImage->methods->at(PpmImage->pixels, (width - col - 1),
                                (height - row - 1)) = *currPixel; 
}

/**********applyrotation0********
 *
 * apply function for doing a 0 degree rotation
 * Inputs: col, row, A2Methods_UArray2 array, curr value, cl pointer
 * Return: none
 * 
 * Expects:
 *      *newArray to be a Pnm_ppm
 * Notes:
 *      
 ************************/
void applyrotation0(int col, int row, A2Methods_UArray2 currArray, void* curr,
                                                void* newArray )
{
        (void)currArray;
        Pnm_ppm PpmImage = (Pnm_ppm) newArray;
        assert(PpmImage != NULL);
        Pnm_rgb currPixel = (Pnm_rgb) curr;
        *(Pnm_rgb) PpmImage->methods->at(PpmImage->pixels, col, 
                                                row) = *currPixel; 
}

/**********applyrotation270********
 *
 * apply function for doing a 270 degree rotation
 * Inputs: col, row, A2Methods_UArray2 array, curr value, cl pointer
 * Return: none
 * 
 * Expects:
 *      *newArray to be a Pnm_ppm
 * Notes:
 *      
 ************************/
void applyrotation270(int col, int row, A2Methods_UArray2 currArray,
                                                void* curr, void* newArray )
{        
        Pnm_ppm PpmImage = (Pnm_ppm) newArray;
        assert(PpmImage != NULL);
        int width = PpmImage->methods->width(currArray);
        Pnm_rgb currPixel = (Pnm_rgb) curr;
        *(Pnm_rgb) PpmImage->methods->at(PpmImage->pixels, row, 
                                        (width - col - 1)) = *currPixel; 
}

/**********applyhorizontal********
 *
 * apply function for doing a horizontal flip
 * Inputs: col, row, A2Methods_UArray2 array, curr value, cl pointer
 * Return: none
 * 
 * Expects:
 *      *newArray to be a Pnm_ppm
 * Notes:
 *      
 ************************/
void applyhorizontal(int col, int row, A2Methods_UArray2 currArray, 
                                        void* curr, void* newArray )
{        
        Pnm_ppm PpmImage = (Pnm_ppm) newArray;
        Pnm_rgb currPixel = (Pnm_rgb) curr;
        int width = PpmImage->methods->width(currArray);
        *(Pnm_rgb) PpmImage->methods->at(PpmImage->pixels, 
                                ((width - 1) - col), row) = *currPixel; 
}

/**********applyvertical********
 *
 * apply function for doing a vertical flip
 * Inputs: col, row, A2Methods_UArray2 array, curr value, cl pointer
 * Return: none
 * 
 * Expects:
 *      *newArray to be a Pnm_ppm
 * Notes:
 *      
 ************************/
void applyvertical(int col, int row, A2Methods_UArray2 currArray, void* curr, 
                                                void* newArray )
{        
        Pnm_ppm PpmImage = (Pnm_ppm) newArray;
        Pnm_rgb currPixel = (Pnm_rgb) curr;
        int height = PpmImage->methods->height(currArray);
        *(Pnm_rgb) PpmImage->methods->at(PpmImage->pixels, 
                                        col, (height - 1 - row)) = *currPixel; 
}

/**********applytranspose********
 *
 * apply function for doing a transpose rotation
 * Inputs: col, row, A2Methods_UArray2 array, curr value, cl pointer
 * Return: none
 * 
 * Expects:
 *      *newArray to be a Pnm_ppm
 * Notes:
 *      Inverse of rotate 0, swap row and col just like in matrix
 ************************/
void applytranspose(int col, int row, A2Methods_UArray2 currArray, void* curr,
                                                void* newArray )
{        
        (void)currArray;
        Pnm_ppm PpmImage = (Pnm_ppm) newArray;
        Pnm_rgb currPixel = (Pnm_rgb) curr;
        *(Pnm_rgb) PpmImage->methods->at(PpmImage->pixels, row, 
                                                col) = *currPixel;  
}

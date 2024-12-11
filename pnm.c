#include <setjmp.h>

#include <netpbm/ppm.h>

#include "mem.h"
#include "pnm.h"
#include <assert.h>

#include "stackoverflow.h"

const Except_T Pnm_Badformat = {"Input is not a correctly formatted pnm map"};

static char *argv[] = {"bogus"};
static int argc = sizeof(argv) / sizeof(argv[0]);
static int initialized = 0;

Pnm_ppm Pnm_ppmread(FILE *fp, A2Methods_T methods)
{
        assert(fp != NULL && methods != NULL);
        assert(methods->new);
        assert(methods->at);

        int width, height;
        pixval denominator;

        jmp_buf jmpbuf;
        int rc = setjmp(jmpbuf);
        if (rc == 0)
        {
                pixel **pixels = ppm_readppm(fp, &width, &height, &denominator);
                volatile Pnm_ppm ppm;
                NEW(ppm);
                ppm->width = width;
                ppm->height = height;
                ppm->denominator = denominator;
                ppm->pixels = methods->new (width, height, sizeof(struct Pnm_rgb));
                ppm->methods = methods;
                for (int row = 0; row < height; row++)
                {
                        pixel *thisrow = pixels[row];
                        for (int col = 0; col < width; col++)
                        {
                                pixel p = thisrow[col];
                                struct Pnm_rgb rgb = {PPM_GETR(p), PPM_GETG(p), PPM_GETB(p)};
                                struct Pnm_rgb *ipixel = methods->at(ppm->pixels, col, row);
                                *ipixel = rgb;
                        }
                }
                ppm_freearray(pixels, height);
                return ppm;
        }
        else
        {
                RAISE(Pnm_Badformat);
                return NULL; // let there be no compiler warnings
        }
}

static void copy_pixel(int i, int j, A2Methods_UArray2 a, void *elem, void *cl)
{
        (void)a;
        pixel **pixels = cl;
        Pnm_rgb pixel = elem;
        PPM_ASSIGN(pixels[j][i], pixel->red, pixel->green, pixel->blue);
}

void Pnm_ppmwrite(FILE *fp, Pnm_ppm pixmap)
{
        assert(fp != NULL && pixmap != NULL);
        const struct A2Methods_T *methods = pixmap->methods;
        assert(methods != NULL);
        assert(methods->map_default);

        int w = pixmap->width;
        int h = pixmap->height;
        assert(w > 0 && h > 0); // the library can't handle empty pixmaps
        pixel **pixels = ppm_allocarray(w, h);
        methods->map_default(pixmap->pixels, copy_pixel, pixels);
        ppm_writeppm(fp, pixels, w, h, pixmap->denominator, 0);
        ppm_freearray(pixels, h);
}

void Pnm_ppmfree(Pnm_ppm *ppmp)
{
        assert(ppmp != NULL);
        Pnm_ppm pixmap = *ppmp;
        assert(pixmap != NULL);
        const struct A2Methods_T *methods = pixmap->methods;
        assert(methods != NULL);
        assert(methods->free);
        methods->free(&pixmap->pixels);
        FREE(*ppmp);
}

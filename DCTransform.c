/*
 * Madeleine Street (mstree03) and Fergus Ferguson (mfergu05)
 * Comp 40 - Homework 4
 *
 * DCTransform.c
 *
 * Implementation for DCTransform. Performs discrete cosign transform functions
 * on each block stored in a PixBlock_img struct (which represents a 2 x 2
 * pixel block of YPbPr values for a 2 x 2 group of pixels).
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "arith40.h"
#include "a2plain.h"
#include "a2methods.h"
#include "DCTransform.h"
#include "PixBlock.h"


/******************************************************************************
 **                              DECLARATIONS                                **
 *****************************************************************************/

struct DCT_img {
    int height, width;
    A2Methods_UArray2 pixels;
    A2Methods_T methods;
};

/* NEW PIXEL HELPER FUNCTIONS */
DCTBlock DCT_newDCTBlock(pix_block block);
pix_block DCT_newPixBlock(DCTBlock block);

/* HELPER TRANSFORMATION FUNCTIONS */
uint64_t DCT_quantChroma(float chroma);
float DCT_unQuantChroma(uint64_t chroma);

uint64_t DCT_calcA(float Y1, float Y2, float Y3, float Y4);
int64_t DCT_calcB(float Y1, float Y2, float Y3, float Y4);
int64_t DCT_calcC(float Y1, float Y2, float Y3, float Y4);
int64_t DCT_calcD(float Y1, float Y2, float Y3, float Y4);

float DCT_calcY1(uint64_t a, int64_t b, int64_t c, int64_t d);
float DCT_calcY2(uint64_t a, int64_t b, int64_t c, int64_t d);
float DCT_calcY3(uint64_t a, int64_t b, int64_t c, int64_t d);
float DCT_calcY4(uint64_t a, int64_t b, int64_t c, int64_t d);

float DCT_bound(float lower, float upper, float num);
void DCT_printBlock(DCTBlock block);

void DCT_pixToDCTApply(int col, int row, A2Methods_UArray2 array2, void *elem,
    void *cl);
void DCT_DCTToPixApply(int col, int row, A2Methods_UArray2 array2, void *elem,
    void *cl);

/******************************************************************************
 **                             NEW AND FREE                                 **
 *****************************************************************************/


 /*
  * function DCT_newDCTImg
  * Parameters:
  *      - PixBlocked_img image used to create DCT_img
  * Returns: a DCT_img created based on the source image
  * Preconditions: image is non null
  * Does: converts a PixBlocked_img image into a DCT_img. Caller is responsible
  *       for deallocating the DCT_img
  */
DCT_img DCT_newDCTImg(PixBlocked_img image)
{
    assert(image != NULL);

    /* Allocate space */
    DCT_img new_image;
    new_image = malloc(sizeof(struct DCT_img));
    assert(new_image != NULL);

    /* Set up values */
    new_image -> methods = uarray2_methods_plain;
    new_image -> height = PixBlock_height(image);
    new_image -> width = PixBlock_width(image);
    new_image -> pixels = new_image -> methods -> new(new_image -> width,
                          new_image -> height, sizeof(struct DCTBlock));
    assert(new_image != NULL);

    /* Map function */
    A2Methods_mapfun *map = new_image -> methods -> map_row_major;

    /* TRANSFORM */
    map(new_image -> pixels, DCT_pixToDCTApply, image);

    return new_image;
}



/*
 * function DCT_newPixBlockedImg
 * Parameters:
 *      - DCT_img image used to create PixBlocked_img
 * Returns: a PixBlocked_img created based on the source image
 * Preconditions: image is non null
 * Does: converts a DCT_img image into a PixBlocked_img. Caller is responsible
 *       for deallocating the PixBlocked_img
 */
PixBlocked_img DCT_newPixBlockedImg(DCT_img image)
{
    assert(image != NULL);

    /* Allocate space */
    PixBlocked_img new_image = PixBlock_newEmpty(image -> height,
                                                 image -> width);

    /* Map function */
    A2Methods_mapfun *map = image -> methods -> map_default;

    /* TRANSFORM! */
    map(image -> pixels, DCT_DCTToPixApply, new_image);

    return new_image;
}



/*
 * function DCT_newDCTEmpty
 * Parameters:
 *      - int height and width for the dimensions of the desired image
 * Returns: a new unfilled DCT_img
 * Does: Allocates and returns a new DCT_img
 * Preconditions:
 *      - height and width are greater than 0
 */
DCT_img DCT_newDCTEmpty(int height, int width)
{
    DCT_img new_image;
    new_image = malloc(sizeof(struct DCT_img));
    assert(new_image != NULL);

    new_image -> height = height;
    new_image -> width = width;
    new_image -> methods = uarray2_methods_plain;
    new_image -> pixels = new_image -> methods -> new(width, height,
                                                      sizeof(struct DCTBlock));
    assert(new_image -> pixels != NULL);

    return new_image;
}


/*
 * function DCT_free
 * Parameters:
 *      - DCT_img image to be freed
 * Returns: None
 * Preconditions:
 *      - image is not-NULL
 * Does: frees the DCT transformed image
 */
void DCT_free(DCT_img image)
{
    assert(image != NULL);

    image -> methods -> free(&(image->pixels));

    free(image);
}

/******************************************************************************
 **                         GETTERS AND SETTERS                              **
 *****************************************************************************/

 /*
  * function DCT_at
  * Parameters:
  *    - DCT_image - image being indexed
  *    - int col
  *    - int row
  * Returns: DCT_Block pointer
  * Preconditions:
  *    - image must be non-null
  *    - col and row must be valid indexes of image
  * Does: returns the DCT_Block at the given index of a given image.
  */
DCTBlock * DCT_at(DCT_img image, int col, int row)
{
    assert (image != NULL);
    assert (col < image -> width && row < image -> height);

    DCTBlock * block;

    block = image -> methods -> at(image -> pixels, col, row);

    return block;
}



/*
 * function DCT_height
 * Parameters: DCT_img image for the image to look at
 * Returns: the height of the given image as an int
 * Preconditions: image is non null
 * Does: gets and returns the image's height
 */
int DCT_height(DCT_img image)
{
    assert (image != NULL);

    return image -> height;
}



/*
 * function DCT_width
 * Parameters: DCT_img image for the image to look at
 * Returns: the width of the given image as an int
 * Preconditions: image is non null
 * Does: gets and returns the image's width
 */
int DCT_width(DCT_img image)
{
    assert (image != NULL);

    return image -> width;
}

/*****************************************************************************
 **                                 HELPERS                                 **
 *****************************************************************************/

/********************** NEW PIXEL HELPER FUNCTIONS ***************************/

/*
 * function DCT_newDCTBlock
 * Parameters:
 *      - pix_block block for the block to convert to a DCT_block
 * Returns: DCTBlock
 * Preconditions: pix_block is initialized and has correct values.
 * Does: Creates a new DCTBlock struct using the values in a given pix_block.
 *       Performs discrete cosine transform on values and quantizes the
 *       results
 */
DCTBlock DCT_newDCTBlock(pix_block block)
{
    DCTBlock dBlock;

    dBlock.a = DCT_calcA(block.Y1, block.Y2, block.Y3, block.Y4);
    dBlock.b = DCT_calcB(block.Y1, block.Y2, block.Y3, block.Y4);
    dBlock.c = DCT_calcC(block.Y1, block.Y2, block.Y3, block.Y4);
    dBlock.d = DCT_calcD(block.Y1, block.Y2, block.Y3, block.Y4);

    dBlock.Pb_avg = DCT_quantChroma(block.Pb_avg);
    dBlock.Pr_avg = DCT_quantChroma(block.Pr_avg);

    return dBlock;
}

/*
 * function DCT_newPixBlock
 * Parameters:
 *      - DCT_Block block for the block to convert into a pix_block
 * Returns: a new pix_block created from the given DCTransform block
 * Does: creates and returns a new pix_block who's values come from the
 *       inverse discrete transformation of the given block
 * Preconditions: block is inizialised and contains correct values
 */
pix_block DCT_newPixBlock(DCTBlock block)
{
    pix_block pBlock;

    /* Set new pix_block values */
    pBlock.Y1 = DCT_calcY1(block.a, block.b, block.c, block.d);
    pBlock.Y2 = DCT_calcY2(block.a, block.b, block.c, block.d);
    pBlock.Y3 = DCT_calcY3(block.a, block.b, block.c, block.d);
    pBlock.Y4 = DCT_calcY4(block.a, block.b, block.c, block.d);

    pBlock.Pb_avg = DCT_unQuantChroma(block.Pb_avg);
    pBlock.Pr_avg = DCT_unQuantChroma(block.Pr_avg);

    return pBlock;
}



/********************* HELPER TRANSFORMATION FUNCTIONS **********************/

/*
 * function DCT_calcA
 * Parameters:
 *    - float Y1
 *    - float Y2
 *    - float Y3
 *    - float Y4
 * Returns: uint64_t - quantized a value
 * Preconditions: All given y values are in the correct ranges of 0 - 1
 * Does: Calculates and quantizes the value a from the given y values.
 */
uint64_t DCT_calcA(float Y1, float Y2, float Y3, float Y4)
{
    assert(Y1 >= 0 && Y1 <= 1);
    assert(Y2 >= 0 && Y2 <= 1);
    assert(Y3 >= 0 && Y3 <= 1);
    assert(Y4 >= 0 && Y4 <= 1);

    float a = (Y4 + Y3 + Y2 + Y1) / 4.0;
    a = DCT_bound(0.0, 1.0, a);

    uint64_t quantized = roundf(a * 511.0);
    quantized = (uint64_t)DCT_bound(0.0, 511.0, (float)quantized);

    return quantized;
}



/*
 * function DCT_calcB
 * Parameters:
 *    - float Y1
 *    - float Y2
 *    - float Y3
 *    - float Y4
 * Returns: uint64_t - quantized b value
 * Preconditions: All given y values are in the correct ranges of 0 - 1
 * Does: Calculates and quantizes the value b from the given y values.
 */
int64_t DCT_calcB(float Y1, float Y2, float Y3, float Y4)
{
    assert(Y1 >= 0 && Y1 <= 1);
    assert(Y2 >= 0 && Y2 <= 1);
    assert(Y3 >= 0 && Y3 <= 1);
    assert(Y4 >= 0 && Y4 <= 1);

    float b = (Y4 + Y3 - Y2 - Y1) / 4.0;
    b = DCT_bound(-0.3, 0.3, b);

    int64_t quantized = roundf(b * 50.0);
    quantized = (uint64_t)DCT_bound(-15.0, 15.0, (float)quantized);

    return quantized;
}



/*
 * function DCT_calcA
 * Parameters:
 *    - float Y1
 *    - float Y2
 *    - float Y3
 *    - float Y4
 * Returns: uint64_t - quantized c value
 * Preconditions: All given y values are in the correct ranges of 0 - 1
 * Does: Calculates and quantizes the value c from the given y values.
 */
int64_t DCT_calcC(float Y1, float Y2, float Y3, float Y4)
{
    assert(Y1 >= 0 && Y1 <= 1);
    assert(Y2 >= 0 && Y2 <= 1);
    assert(Y3 >= 0 && Y3 <= 1);
    assert(Y4 >= 0 && Y4 <= 1);

    float c = (Y4 - Y3 + Y2 - Y1) / 4.0;
    c = DCT_bound(-0.3, 0.3, c);

    int64_t quantized = roundf(c * 50.0);
    quantized = (uint64_t)DCT_bound(-15.0, 15.0, (float)quantized);

    return quantized;
}



/*
 * function DCT_calcD
 * Parameters:
 *    - float Y1
 *    - float Y2
 *    - float Y3
 *    - float Y4
 * Returns: uint64_t - quantized d value
 * Preconditions: All given y values are in the correct ranges of 0 - 1
 * Does: Calculates and quantizes the value d from the given y values.
 */
int64_t DCT_calcD(float Y1, float Y2, float Y3, float Y4)
{
    assert(Y1 >= 0 && Y1 <= 1);
    assert(Y2 >= 0 && Y2 <= 1);
    assert(Y3 >= 0 && Y3 <= 1);
    assert(Y4 >= 0 && Y4 <= 1);

    float d = (Y4 - Y3 - Y2 + Y1) / 4.0;
    d = DCT_bound(-0.3, 0.3, d);

    int64_t quantized = roundf(d * 50.0);
    quantized = (uint64_t)DCT_bound(-15.0, 15.0, (float)quantized);

    return quantized;
}



/*
 * function DCT_calcY1
 * Parameters:
 *      - uint64_t a, b, c, d for the quantized values returned from the DCT
 * Returns: a float in the range 0 - 1 representing Y1
 * Does: dequantizes a, b, c and d and performs the inverse Discrete cosine
 *       transform
 */
float DCT_calcY1(uint64_t a, int64_t b, int64_t c, int64_t d)
{
    /*
    assert(a <= 511);
    assert(b >= -15 && b <= 15);
    assert(c >= -15 && c <= 15);
    assert(d >= -15 && d <= 15); */

    /* Dequantize values and bound them into the correct range */
    float A = a / 511.0;
    A = DCT_bound(0.0, 1.0, A);
    float B = b / 50.0;
    B = DCT_bound(-0.3, 0.3, B);
    float C = c / 50.0;
    C = DCT_bound(-0.3, 0.3, C);
    float D = d /50.0;
    D = DCT_bound(-0.3, 0.3, D);

    /* Inverse discrete cosine transform */
    float Y1 = A - B - C + D;
    Y1 = DCT_bound(0.0, 1.0, Y1);

    return Y1;
}



/*
 * function DCT_calcY2
 * Parameters:
 *      - uint64_t a, b, c, d for the quantized values returned from the DCT
 * Returns: a float in the range 0 - 1 representing Y2
 * Does: dequantizes a, b, c and d and performs the inverse Discrete cosine
 *       transform
 */
float DCT_calcY2(uint64_t a, int64_t b, int64_t c, int64_t d)
{
    /*
    assert(a <= 511);
    assert(b >= -15 && b <= 15);
    assert(c >= -15 && c <= 15);
    assert(d >= -15 && d <= 15); */

    /* Dequantize values and bound them into the correct range */
    float A = a / 511.0;
    A = DCT_bound(0.0, 1.0, A);
    float B = b / 50.0;
    B = DCT_bound(-0.3, 0.3, B);
    float C = c / 50.0;
    C = DCT_bound(-0.3, 0.3, C);
    float D = d /50.0;
    D = DCT_bound(-0.3, 0.3, D);

    /* Inverse discrete cosine transform */
    float Y2 = A - B + C - D;
    Y2 = DCT_bound(0.0, 1.0, Y2);

    return Y2;
}



/*
 * function DCT_calcY3
 * Parameters:
 *      - uint64_t a, b, c, d for the quantized values returned from the DCT
 * Returns: a float in the range 0 - 1 representing Y3
 * Does: dequantizes a, b, c and d and performs the inverse Discrete cosine
 *       transform
 */
float DCT_calcY3(uint64_t a, int64_t b, int64_t c, int64_t d)
{
    /*
    assert(a <= 511);
    assert(b >= -15 && b <= 15);
    assert(c >= -15 && c <= 15);
    assert(d >= -15 && d <= 15); */

    /* Dequantize values and bound them into the correct range */
    float A = a / 511.0;
    A = DCT_bound(0.0, 1.0, A);
    float B = b / 50.0;
    B = DCT_bound(-0.3, 0.3, B);
    float C = c / 50.0;
    C = DCT_bound(-0.3, 0.3, C);
    float D = d /50.0;
    D = DCT_bound(-0.3, 0.3, D);

    /* Inverse discrete cosine transform */
    float Y3 = A + B - C - D;
    Y3 = DCT_bound(0.0, 1.0, Y3);

    return Y3;
}



/*
 * function DCT_calcY4
 * Parameters:
 *      - uint64_t a, b, c, d for the quantized values returned from the DCT
 * Returns: a float in the range 0 - 1 representing Y4
 * Does: dequantizes a, b, c and d and performs the inverse Discrete cosine
 *       transform
 */
float DCT_calcY4(uint64_t a, int64_t b, int64_t c, int64_t d)
{
    /*
    assert(a <= 511);
    assert(b >= -15 && b <= 15);
    assert(c >= -15 && c <= 15);
    assert(d >= -15 && d <= 15); */

    /* Dequantize values and bound them into the correct range */
    float A = a / 511.0;
    A = DCT_bound(0.0, 1.0, A);
    float B = b / 50.0;
    B = DCT_bound(-0.3, 0.3, B);
    float C = c / 50.0;
    C = DCT_bound(-0.3, 0.3, C);
    float D = d /50.0;
    D = DCT_bound(-0.3, 0.3, D);

    /* Inverse discrete cosine transform */
    float Y4 = A + B + C + D;
    Y4 = DCT_bound(0.0, 1.0, Y4);

    return Y4;
}



/*
 * function DCT_bound
 * Parameters:
 *    - float lower: lower acceptable bound
 *    - float upper: upper acceptable bound
 *    - float num: num to be bound
 * Returns: float - the num, limited to the given range if it was out of
 * bounds, otherwise it will remain unchanged.
 * Preconditions: None.
 * Does: Determines if a given num is within a given range. If not, it is
 * limited to the upper or lower bounds of that range -- whichever is closest.
 * Otherwise the num is returned as is.
 */
float DCT_bound(float lower, float upper, float num)
{
    if(num < lower) {
        return lower;
    }
    else if (num > upper) {
        return upper;
    }
    else {
        return num;
    }
}



/*
 * function DCT_quantChroma
 * Parameters: float chroma - chroma value to be quantized
 * Returns: uint64_t - the quantized chroma value
 * Preconditions: chroma must be between the range -0.5 and 0.5 (function
 * assumes it is receiving pr or pb value)
 * Does: Quantizes either a pr or pb value depending on what it is given. Also
 * converts the chroma from float to uint64_t.
 */
uint64_t DCT_quantChroma(float chroma)
{
    assert(chroma >= -0.5 && chroma <= 0.5);

    chroma = DCT_bound(-0.3, 0.3, chroma);

    unsigned quantChroma = Arith40_index_of_chroma(chroma);

    return (uint64_t)quantChroma;
}



/*
 * function DCT_unQuantChroma
 * Parameters: uint64_t - the quantized chroma value
 * Returns: float deQuant - unquantized chroma value
 * Preconditions: chroma must be between the range -0.5 and 0.5 (function
 * assumes it is receiving pr or pb value)
 * Does: Dequantizes either a pr or pb value depending on what it is given.
 * Also converts the chroma from uint64_t to float.
 */
float DCT_unQuantChroma(uint64_t chroma)
{
    float deQuant = Arith40_chroma_of_index(chroma);

    deQuant = DCT_bound(-0.5, 0.5, deQuant);

    return deQuant;
}



/*
 * function DCT_pixToDCTApply
 * Parameters:
 *    - int col - current col index
 *    - int row - current row index
 *    - A2Methods_UArray2 array2
 *    - void * elem - contains DCTBlock
 *    - void * cl - closure argument, contains the PixBlock_img
 * Returns: None
 * Preconditions: PixBlock_img must be non-null
 * Does: Fills in a DCT_img struct from an existing PixBlock_img struct
 */
void DCT_pixToDCTApply(int col, int row, A2Methods_UArray2 array2, void *elem,
    void *cl)
{
    assert(cl != NULL);
    assert(array2 != NULL);
    /* Get pixBlocked image and destination DCTBlock */
    PixBlocked_img piximg = cl;
    DCTBlock * dest = elem;


    /* Get Original pix_block */
    pix_block * old_block = PixBlock_at(piximg, col, row);

    *dest = DCT_newDCTBlock(*old_block);
}



/*
 * function DCT_DCTToPixApply
 * Parameters:
 *    - int col - current col index
 *    - int row - current row index
 *    - A2Methods_UArray2 array2
 *    - void * elem - contains pix_block
 *    - void * cl - closure argument, contains the DCT_img
 * Returns: None
 * Preconditions: DCT_img must be non-null
 * Does: Fills in a PixBlock_img struct from an existing DCT_img struct
 */
void DCT_DCTToPixApply(int col, int row, A2Methods_UArray2 array2, void *elem,
    void *cl)
{
    assert(cl != NULL);
    assert(array2 != NULL);
    /* Get pixBlocked image and source DCT block */
    PixBlocked_img piximg = cl;
    DCTBlock * source_block = elem;

    /* Get dest block */
    pix_block * dest = PixBlock_at(piximg, col, row);
    *dest = DCT_newPixBlock(*source_block);
}


/************************ TESTING PURPOSES ONLY *****************************/
void DCT_printBlock(DCTBlock block)
{
    fprintf(stderr, "Chroma Values: \n");
    fprintf(stderr, "   Pr - %d    Pb - %d\n", block.Pr_avg, block.Pb_avg);
    fprintf(stderr, "A B C D Values:\n");
    fprintf(stderr, "   a - %ld     b - %ld     c - %ld     d - %ld\n",
            block.a, block.b, block.c, block.d);
    fprintf(stderr, "\n");

}

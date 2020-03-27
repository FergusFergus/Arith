/*
 * Madeleine Street (mstree03) and Fergus Ferguson (mfergu05)
 * COMP 40 Homework 4: Arith
 * PixBlock.c
 *
 * Implementation for the PixBlock Steps of the transformation. Combines each
 * 2 x 2 block of pixels in an image into one struct by averaging the Pr and
 * Pb values. If the image has an odd width or height, the corresponding
 * dimensions are reduced by 1 to make them even.
 */

 #include <stdlib.h>
 #include <stdio.h>
 #include <stdbool.h>
 #include <stdint.h>
 #include <assert.h>
 #include "except.h"
 #include "YPbPr.h"
 #include "PixBlock.h"

/******************************************************************************
 **                              DECLARATIONS                                **
 *****************************************************************************/

struct PixBlocked_img
{
    int height, width;
    A2Methods_UArray2 pixels; /* UArray2 of YPbPr_pixels */
    A2Methods_T methods;
};

/*=========================== HELPER FUNCTIONS ==============================*/
float PixBlock_computeAvg(float pr1, float pr2, float pr3, float pr4);

void PixBlock_print(PixBlocked_img image);
void PixBlock_printBlock(pix_block block);

pix_block PixBlock_newPixelBlock(YPbPr_img image, int col, int row);
YPbPr_pixel PixBlock_newYPPPixel(pix_block pixel, int pix_num);
float PixBlock_bound(float lower, float upper, float num);


/******************************************************************************
 **                             NEW AND FREE                                 **
 *****************************************************************************/

/*
 * function PixBlock_new
 * Parameters:
 *      - YPbPr_img image for the image to turn into a pixblocked array
 * Returns: a PixBlocked_img made from the given image
 * Preconditions: image is non null
 * Does: Converts the component color image to an image where pixels are
 *       into groups of 4. If the height or width of the original image is
 *       odd, the Pix blocked image removes the last row/col
 */
PixBlocked_img PixBlock_new(YPbPr_img image)
{
    assert(image != NULL);

    int height = YPbPr_getHeight(image);
    int width = YPbPr_getWidth(image);

    // figure out what to do if image 0 x 0 or smthing
    assert(height >= 2 && width >= 2);

    if (height % 2 != 0) {
        height--;
    }
    if (width % 2 != 0) {
        width--;
    }

    /* Create pixblock array */
    PixBlocked_img new_image;
    new_image = malloc(sizeof(struct PixBlocked_img));
    assert(new_image != NULL);
    new_image -> height = height / 2;
    new_image -> width = width / 2;
    new_image -> methods = uarray2_methods_plain;

    new_image -> pixels = new_image -> methods -> new(new_image -> width,
        new_image -> height, sizeof(struct pix_block));

    /* fill pixbloc array */
    for (int row = 0; row < height; row += 2) {
        for (int col = 0; col < width; col += 2) {
            pix_block curr_block = PixBlock_newPixelBlock(image, col, row);
            pix_block * dest = new_image -> methods -> at(new_image -> pixels,
                                col / 2, row / 2);
            *dest = curr_block;
        }
    }

    return new_image;
}

/*
 * function PixBlock_newYPP
 * Parameters:
 *      - PixBlocked_img image for the image to expand
 * Returns: a YPbPr_img created based on the source image
 * Preconditions: image is non null
 * Does: converts a PixBlocked image into a YPbPr_img. Caller is responsible
 *       for deallocating the YPbPr_img
 */
YPbPr_img PixBlock_newYPP(PixBlocked_img image)
{
    assert(image != NULL);

    /* Allocate and set up values of new YPbPr_img */
    YPbPr_img new_image;
    new_image = YPbPr_newEmpty((image -> height) * 2, (image -> width) * 2);
    assert(new_image != NULL);

    /* Fill array */
    for (int row = 0; row < image -> height; row++) {
        for (int col = 0; col < image -> width; col++) {

            pix_block curr_block = *PixBlock_at(image, col, row);

            YPbPr_pixel new_pix = PixBlock_newYPPPixel(curr_block, 1);
            YPbPr_setPixel(new_image, new_pix, (col * 2), (row * 2));

            new_pix = PixBlock_newYPPPixel(curr_block, 2);
            YPbPr_setPixel(new_image, new_pix, (col * 2) + 1, (row * 2));

            new_pix = PixBlock_newYPPPixel(curr_block, 3);
            YPbPr_setPixel(new_image, new_pix, (col * 2), (row * 2) + 1);

            new_pix = PixBlock_newYPPPixel(curr_block, 4);
            YPbPr_setPixel(new_image, new_pix, (col * 2) + 1, (row * 2) + 1);
        }
    }

    return new_image;
}



/*
 * function PixBlock_newEmpty
 * Parameters:
 *      - int height
 *      - int width
 * Returns: a PixBlocked_img made
 * Preconditions: PixBlock_newPixBlockSingle
 * Does: Initializes an empty PixBlocked_img struct of the given dimensions
 */
PixBlocked_img PixBlock_newEmpty(int height, int width)
{
    PixBlocked_img new_image;
    new_image = malloc(sizeof(struct PixBlocked_img));
    assert(new_image != NULL);

    new_image -> height = height;
    new_image -> width = width;

    new_image -> methods = uarray2_methods_plain;

    new_image -> pixels = new_image -> methods -> new(new_image -> width,
        new_image -> height, sizeof(struct pix_block));

    return new_image;
}



/*
 * function PixBlock_free
 * Parameters:
 *      - PixBlocked image to be freed
 * Returns: None
 * Preconditions:
 *      - image is not-NULL
 * Does: frees the pix blocked image
 */
void PixBlock_free(PixBlocked_img image)
{
    assert(image != NULL);

    image -> methods -> free(&(image->pixels));

    free(image);
}


/******************************************************************************
 **                         GETTERS AND SETTERS                              **
 *****************************************************************************/

/*
 * function PixBlock_at
 * Parameters:
 *      - PixBlocked_img image - image being indexed
 *      - int col
 *      - int row
 * Returns: pointer to a pix_block in the given image
 * Preconditions:
 *      - image is not-NULL
 *      - int col and row are in range
 * Does: returns the address of a pix_block at a given index in a given image
 */
pix_block * PixBlock_at(PixBlocked_img image, int col, int row)
{
    assert(image != NULL);
    assert(col < image -> width && row < image -> height);

    pix_block * pixels;
    pixels = image -> methods -> at(image -> pixels, col, row);

    return pixels;
}


/*
 * function PixBlock_at
 * Parameters:
 *      - PixBlocked_img image
 * Returns: int - height of the given image
 * Preconditions:
 *      - image is not-NULL
 * Does: returns the height of a given image
 */
int PixBlock_height(PixBlocked_img image)
{
    assert(image != NULL);

    return image -> height;
}


/*
 * function PixBlock_at
 * Parameters:
 *      -
 * Returns: int - width of the given image
 * Preconditions:
 *      - image is not-NULL
 * Does: returns the width of a given image
 */
int PixBlock_width(PixBlocked_img image)
{
    assert(image != NULL);

    return image -> width;
}

/*****************************************************************************
 **                                 HELPERS                                 **
 *****************************************************************************/

/*
 * function PixBlock_newPixelBlock
 * Parameters:
 *      - YPbPr_img image for the image containing the desired pixels
 *      - int col and row representinf the coordinates of the upper left corner
 *        pixel to be contained in the new pix_block
 * Returns: a pix_block created from the pixels in image
 * Preconditions: image is non null and col and row are in range
 * Does: creats and returns a new pix_block based on the pixles at (col, row)
 *       (col + 1, row), (col, row + 1) and (col + 1, row + 1)
 */
pix_block PixBlock_newPixelBlock(YPbPr_img image, int col, int row)
{
    assert (image != NULL);

    pix_block new_pixel;
    int height = YPbPr_getHeight(image);
    int width = YPbPr_getWidth(image);

    /* get pixels */
    YPbPr_pixel pix1 = YPbPr_getPixel(image, col, row);
    YPbPr_pixel pix2;
    if (col + 1 < width) {
        pix2 = YPbPr_getPixel(image, col + 1, row);
    }
    YPbPr_pixel pix3;
    if (row + 1 < height) {
        pix3 = YPbPr_getPixel(image, col, row + 1);
    }
    YPbPr_pixel pix4;
    if (col + 1 < width && row + 1 < height) {
        pix4 = YPbPr_getPixel(image, col + 1, row + 1);
    }

    /* calculate chroma */
    new_pixel.Pr_avg = PixBlock_computeAvg(pix1.pr, pix2.pr, pix3.pr, pix4.pr);
    new_pixel.Pr_avg = PixBlock_bound(-0.5, 0.5, new_pixel.Pr_avg);
    new_pixel.Pb_avg = PixBlock_computeAvg(pix1.pb, pix2.pb, pix3.pb, pix4.pb);
    new_pixel.Pb_avg =PixBlock_bound(-0.5, 0.5, new_pixel.Pb_avg);

    new_pixel.Y1 = pix1.y;
    new_pixel.Y2 = pix2.y;
    new_pixel.Y3 = pix3.y;
    new_pixel.Y4 = pix4.y;

    return new_pixel;
}


/*
 * function PixBlock_newYPPPixel
 * Parameters:
 *      - pix_block pixel representing the four pixels to unpack into separate
 *        pixels
 *      - int pix_num: representing the pixel to extract
 * Returns: a single YPbPr_pixel
 * Preconditions:
 *      - pix_block is non-empt
 *      - pix_num is from 1 to 4 where pixel 1 is the upper left, and pixel 4
 *        is the lower right
 * Does: Extracts a single YPbPr_pixel from a pix_block
 */
YPbPr_pixel PixBlock_newYPPPixel(pix_block pixel, int pix_num)
{
    assert(pix_num > 0 && pix_num <= 4);

    YPbPr_pixel new_pixel;

    // first pixel in pix_block
    if (pix_num == 1) {
        new_pixel.y = pixel.Y1;
    // second pixel in pix_block
    } else if (pix_num == 2) {
        new_pixel.y = pixel.Y2;
    // third pixel in pix_block
    } else if (pix_num == 3) {
        new_pixel.y = pixel.Y3;
    // fourth pixel in pix_block
    } else {
        new_pixel.y = pixel.Y4;
    }

    new_pixel.pb = pixel.Pb_avg;
    new_pixel.pr = pixel.Pr_avg;

    return new_pixel;
}



/*
 * function PixBlock_computerAvg
 * Parameters:
 *      - floats pr1, pr2, p3, and pr4 representing the four color channel
 *         values to average together
 * Returns: the mean of the four parameters as a float
 * Preconditions:
 *      - pr1-4 are within the range -0.5 - 0.5
 * Does: Computes the average of the four given float values.
 */
float PixBlock_computeAvg(float pr1, float pr2, float pr3, float pr4)
{
    assert(pr1 <= 0.5 && pr1 >= -0.5);
    assert(pr2 <= 0.5 && pr2 >= -0.5);
    assert(pr3 <= 0.5 && pr3 >= -0.5);
    assert(pr4 <= 0.5 && pr4 >= -0.5);

    float average = (pr1 + pr2 + pr3 +pr4);
    average /= 4.0;

    return average;
}

/*
 * function PixBlock_bound
 * Paramters:
 *      - float lower for the lower bound
 *      - float upper for the upper bound
 *      - float num for the num to bound
 * Returns: num if it is bewteen upper and lower, lower if is less than lower
 *          higher if it is more than higher
 */
 float PixBlock_bound(float lower, float upper, float num)
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

/* TESTING FUNCTION
void PixBlock_print(PixBlocked_img image)
{
    assert(image != NULL);

    FILE * out = stderr;

    for (int col = 0; col < image -> width; col++) {
        for (int row = 0; row < image -> height; row++) {
            pix_block * to_print = image -> methods -> at(image -> pixels,
            col, row);
            fprintf(out, "y1: %f\t y2: %f\t y3: %f\t y4: %f\t pbAvg: %f\t prAvg: %f\n",
            to_print -> Y1, to_print -> Y2, to_print -> Y3,
            to_print -> Y4, to_print -> Pb_avg, to_print -> Pr_avg);
        }
    }
}



void PixBlock_printBlock(pix_block block)
{
    fprintf(stderr, "Chroma Values: \n");
    fprintf(stderr, "   Pr - %f    Pb - %f\n", block.Pr_avg, block.Pb_avg);
    fprintf(stderr, "Y1 Y2 Y3 Y4 Values:\n");
    fprintf(stderr, "   y1 - %f     y2 - %f     y3 - %f     y4 - %f\n",
            block.Y1, block.Y2, block.Y3, block.Y4);
    fprintf(stderr, "\n");
}*/

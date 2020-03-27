/*
 * Madeleine Street (mstree03) and Fergus Ferguson (mfergu05)
 * COMP 40 Homework 4: Arith
 * YPbPr.c
 *
 * Implementation for the PixBlock Steps of the transformation
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "YPbPr.h"
#include "a2plain.h"
#include "a2methods.h"
#include "math.h"

/******************************************************************************
 **                              DECLARATIONS                                **
 *****************************************************************************/
struct YPbPr_img
{
    int height, width;
    A2Methods_UArray2 pixels; /* UArray2 of YPbPr_pixels */
    A2Methods_T methods;
};

/* Used only to pass to the newYPP map functions */
struct newYPPcl {
    YPbPr_img image;
    int denominator;
    A2Methods_T methods;
};

/* Used only to pass to the newYPP map functions */
struct newRGBcl {
    Pnm_ppm image;
    int denominator;
    A2Methods_T methods;
};

float YPbPr_calcY(int red, int green, int blue, int denominator);
float YPbPr_calcPb(int red, int green, int blue, int denominator);
float YPbPr_calcPr(int red, int green, int blue, int denominator);

int YPbPr_calcR(float y, float pb, float pr, int denominator);
int YPbPr_calcG(float y, float pb, float pr, int denominator);
int YPbPr_calcB(float y, float pb, float pr, int denominator);

YPbPr_pixel YPbPr_newYPPPix (struct Pnm_rgb rgbPix, int denominator);
struct Pnm_rgb YPbPr_newRGBpix (YPbPr_pixel yppPix, int denominator);

float YPbPr_normalize(int value, int denominator);

void YPbPr_newYPPApply(int col, int row, A2Methods_UArray2 array2, void *elem,
    void *cl);
void YPbPr_newRGBApply(int col, int row, A2Methods_UArray2 array2, void *elem,
    void *cl);
float bound(float lower, float upper, float num);

/*void print_vals(int col, int row, A2Methods_UArray2 array2, void *elem,
    void *cl);*/

/******************************************************************************
 **                             NEW AND FREE                                 **
 *****************************************************************************/
/*
 * function YPbPr_new
 * Parameters:
 *      - Pnm_ppm image representing the pnm image to convert to component
 *        video color space
 * Returns: a YPbPr_img representing the image in the new color space
 * Does: allocates a YPbPr_img and uses it to store a converted pnm image in
 *       component video color spcae
 * Preconditions:
 *      - image is non-NULL
 * Note: caller is responsible for freeing YPbPr_img with YPbPr_free
 */
YPbPr_img YPbPr_new(Pnm_ppm image)
{
    assert(image != NULL);

    /* Malloc Space */
    YPbPr_img new_image;
    new_image = malloc(sizeof(struct YPbPr_img));
    assert(new_image != NULL);

    /* Get height and width values */
    new_image -> height = image -> height;
    new_image -> width = image -> width;

    /* Set up A2Methods */
    new_image -> methods = uarray2_methods_plain;
    assert(new_image -> methods != NULL);
    A2Methods_mapfun *map = new_image -> methods -> map_default;

    /* Create pixels array */
    new_image -> pixels = new_image -> methods -> new(new_image -> width,
        new_image -> height, sizeof(struct YPbPr_pixel));
    assert(new_image -> pixels != NULL);

    /* Fill pixels array with converted pixels */
    struct newYPPcl cl = {new_image, image -> denominator,
        new_image -> methods};
    map(image -> pixels, YPbPr_newYPPApply, &cl);

    return new_image;
}


/*
 * function YPbPr_newEmpty
 * Parameter:
 *      - height and width for the size of the desired image
 * Returns: a YPbPr_img with no pixel values inserted
 * Does: Creates a new YPbPr_img and returns it. Caller is responsible for
 *       deallocating image.
 * Preconditions:
 *      - height and width are greater than 0
 */
YPbPr_img YPbPr_newEmpty(int height, int width)
{
    assert(height > 0 && width > 0);

    /* Malloc Space */
    YPbPr_img new_image = malloc(sizeof(struct YPbPr_img));
    assert(new_image != NULL);

    /* set height and width */
    new_image -> height = height;
    new_image -> width = width;

    /* Set up methods */
    new_image -> methods = uarray2_methods_plain;
    assert(new_image -> methods != NULL);

    /* Create pixels array */
    new_image -> pixels = new_image -> methods -> new(new_image -> width,
        new_image -> height, sizeof(struct YPbPr_pixel));
    assert(new_image -> pixels != NULL);

    return new_image;
}


/*
 * function YPbPr_newPnm
 * Parameters:
 *      - YPbPr image representing the component video color space image to
 *        convert to a pnm
 * Returns: a Pnm_ppm stored in RGB color space
 * Does: allocates a new Pnm_ppm and stores the converted image inside it
 * Preconditions:
 *      - image is non-null
 * Note: caller is responsible for deallocating Pnm_ppm
 */
Pnm_ppm YPbPr_newPnm(YPbPr_img image)
{
    assert(image != NULL);

    /* Allocate new Pnm_ppm */
    Pnm_ppm new_image;
    new_image = malloc(sizeof(struct Pnm_ppm));
    assert(new_image != NULL);

    /* set up Pnm_ppm */
    new_image -> height = image -> height;
    new_image -> width = image -> width;

    new_image -> methods = uarray2_methods_plain;
    assert(new_image -> methods != NULL);

    A2Methods_mapfun *map = new_image -> methods -> map_default;

    /* Create pixels array */
    new_image -> pixels = new_image -> methods -> new(new_image -> width,
        new_image -> height, sizeof(struct Pnm_rgb));
    assert(new_image -> pixels != NULL);

    /* Fill array with converted pixels */
    A2Methods_T methods = uarray2_methods_plain;
    struct newRGBcl cl = {new_image, 255, methods};
    map(image -> pixels, YPbPr_newRGBApply, &cl);

    new_image -> denominator = 255;
    Pnm_ppmwrite(stdout, new_image);
    //map(new_image -> pixels, print_vals, NULL);


    return new_image;
}


/*
 * function YPbPr_free
 * Parameters:
 *      - YPbPr_img image representing the image to free
 * Returns: nothing
 * Does: frees memory associated with image and sets it to NULL
 * Preconditions:
 *      - image is non-null
 */
void YPbPr_free(YPbPr_img image)
{
    assert(image != NULL);

    image -> methods -> free(&(image->pixels));

    free(image);
}


/******************************************************************************
 **                         GETTERS AND SETTERS                              **
 *****************************************************************************/
/*
 * function YPbPr_getpixel
 * Parameters:
 *      - YPbPr_img image for the image to extract from
 *      - ints col and row for the location of the desired pixel
 * Returns: a YPbPr_pixel representing the pixel at (col, row) of the image
 * Does: gets and returns a pixel from image
 * Preconditions:
 *      - image is non-NULL
 *      - col and row are in range
 */
YPbPr_pixel YPbPr_getPixel(YPbPr_img image, int col, int row)
{
    assert(image != NULL);
    assert(col < image -> width && col >= 0);
    assert(row < image -> height && row >= 0);
    
    YPbPr_pixel *to_return = image -> methods -> at(image -> pixels, col, row);

    return *to_return;
}


/*
 * function YPbPr_setpixel
 * Parameters:
 *      - YPbPr_img image for the image to extract from
 *      - YPbPr_pixel which will be put in the given YPbPr image
 *      - ints col and row for the location of the desired pixel
 * Returns: None
 * Does: sets a pixel in the image to a given value
 * Preconditions:
 *      - image is non-NULL
 *      - col and row are in range
 */
void YPbPr_setPixel(YPbPr_img image, YPbPr_pixel pix, int col, int row)
{
    assert (image != NULL);
    assert (col < image -> width && row < image-> height);

    YPbPr_pixel * to_set = image -> methods -> at(image -> pixels, col, row);
    *to_set = pix;
}


/*
 * function YPbPr_getWidth
 * Parameters:
 *      - YPbPr_img image for the image to look at
 * Returns: the width of the given image
 * Does: gets and returns the width of an image
 * Preconditions:
 *      - image is non-null
 */
int YPbPr_getWidth(YPbPr_img image)
{
    assert(image != NULL);
    return image->width;
}


/*
 * function YPbPr_getHeight
 * Parameters:
 *      - YPbPr_img image for the image to look at
 * Returns: the height of the given image
 * Does: gets and returns the height of an image
 * Preconditions:
 *      - image is non-null
 */
int YPbPr_getHeight(YPbPr_img image)
{
    assert(image != NULL);
    return image->height;
}


/*****************************************************************************
 **                                 HELPERS                                 **
 *****************************************************************************/

/*
 * function YPbPr_normalize
 * Parameters:
 *      - int value: value to normalize based on the denominator
 *      - int denominator: the number to adjust the value by
 * Returns: a float representing the normalized value of value
 * Does: normalizes a single value based on the given denominator
 * Preconditions:
 *      - denominator is larger than value
 *      - denominator is greater than zero
 */
float YPbPr_normalize(int value, int denominator)
{
    assert(value <= denominator && denominator > 0);
    float result = ((float)value) / ((float)denominator);

    assert(result >= 0 && result <= 1);

    return result;
}


/*
 * function YPbPr_calcY
 * Parameters:
 *      - int red, green, blue representing the color values of a given pixel
 *      - int denominator for the maximum size of red/green/blue
 * Returns: a float representing the Y value of the YPbPr representation of the
 *          given pixel
 * Does: calculates and return the Y (brightness) value
 * Preconditions:
 *      - red, green and blue are between 0 and denominator (inclusive)
 */
float YPbPr_calcY(int red, int green, int blue, int denominator)
{
    /* assert preconditions */
    assert(red >= 0 && red <= denominator);
    assert(green >= 0 && green <= denominator);
    assert(blue >= 0 && blue <= denominator);

    /* Normalize values */
    float redN = YPbPr_normalize(red, denominator);
    redN = bound(0.0, 1.0, redN);

    float greenN = YPbPr_normalize(green, denominator);
    greenN = bound(0.0, 1.0, greenN);

    float blueN = YPbPr_normalize(blue, denominator);
    blueN = bound(0.0, 1.0, blueN);

    float y = (0.299 * redN) + (0.587 * greenN) + (0.114 * blueN);
    y = bound(0.0, 1.0, y);

    assert(y <= 1 && y >= 0);

    return y;
}


/*
 * function YPbPr_calcPb
 * Parameters:
 *      - int red, green, blue representing the color values of a given pixel
 *      - int denominator for the maximum size of red/green/blue
 * Returns: a float representing the Pb value of the YPbPr representation of
 *          the given pixel
 * Does: calculates and return the Pb value
 * Preconditions:
 *      - red, green and blue are between 0 and denominator (inclusive)
 */
float YPbPr_calcPb(int red, int green, int blue, int denominator)
{
    /* assert preconditions */
    assert(red >= 0 && red <= denominator);
    assert(green >= 0 && green <= denominator);
    assert(blue >= 0 && blue <= denominator);

    /* Normalize values */
    float redN = YPbPr_normalize(red, denominator);
    redN = bound(0.0, 1.0, redN);
    float greenN = YPbPr_normalize(green, denominator);
    greenN = bound(0.0, 1.0, greenN);
    float blueN = YPbPr_normalize(blue, denominator);
    blueN = bound(0.0, 1.0, blueN);

    float pb = (-0.168736 * redN) + (-0.331264 * greenN) + (0.5 * blueN);
    pb = bound(-0.5, 0.5, pb);

    assert(pb <= 0.5 && pb >= -0.5);

    return pb;
}


/*
 * function YPbPr_calcPr
 * Parameters:
 *      - int red, green, blue representing the color values of a given pixel
 *      - int denominator for the maximum size of red/green/blue
 * Returns: a float representing the Pr value of the YPbPr representation of
 *          the given pixel
 * Does: calculates and return the Pr value
 * Preconditions:
 *      - red, green and blue are between 0 and denominator (inclusive)
 */
float YPbPr_calcPr(int red, int green, int blue, int denominator)
{
    /* assert preconditions */
    assert(red >= 0 && red <= denominator);
    assert(green >= 0 && green <= denominator);
    assert(blue >= 0 && blue <= denominator);

    /* Normalize values */
    float redN = YPbPr_normalize(red, denominator);
    redN = bound(0.0, 1.0, redN);
    float greenN = YPbPr_normalize(green, denominator);
    greenN = bound(0.0, 1.0, greenN);
    float blueN = YPbPr_normalize(blue, denominator);
    blueN = bound(0.0, 1.0, blueN);

    float pr = (0.5 * redN) + (-0.418688 * greenN) + (-0.081312 * blueN);
    pr = bound(-0.5, 0.5, pr);

    assert(pr <= 0.5 && pr >= -0.5);

    return pr;
}


/*
 * function YPbPr_calcR
 * Parameters:
 *      - float y: brightness value
 *      - float pb: blue color difference signal
 *      - float pr: red color difference signal
 *      - int denominator: number to use to scale return value by
 * Returns: an int representing the red value based on the given YPbPr values
 * Does: calculate the value of the Red component of a RGB pixel
 * Preconditions:
 *      - y is between 0 and 1
 *      - pb and pr are between -0.5 and 0.5
 *      - denominator is greater than 0
 */
int YPbPr_calcR(float y, float pb, float pr, int denominator)
{
    /* assert preconditions */
    assert(denominator > 0);
    assert(y >= 0 && y <= 1);
    assert(pb >= -0.5 && pb <= 0.5);
    assert(pr >= -0.5 && pr <= 0.5);

    float r = (1.0 * y) + (0.0 * pb) + (1.402 * pr);
    r = bound(0.0, 1.0, r);

    int scaled = r * denominator;
    scaled = bound(0.0, (float)denominator, scaled);

    /* check return value */
    assert(scaled <= denominator && scaled >= 0);

    return scaled;
}


/*
 * function YPbPr_calcG
 * Parameters:
 *      - float y: brightness value
 *      - float pb: blue color difference signal
 *      - float pr: red color difference signal
 *      - int denominator: number to use to scale return value by
 * Returns: an int representing the green value based on the given YPbPr values
 * Does: calculate the value of the green component of a RGB pixel
 * Preconditions:
 *      - y is between 0 and 1
 *      - pb and pr are between -0.5 and 0.5
 *      - denominator is greater than 0
 */
int YPbPr_calcG(float y, float pb, float pr, int denominator)
{
    /* assert preconditions */
    assert(denominator > 0);
    assert(y >= 0 && y <= 1);
    assert(pb >= -0.5 && pb <= 0.5);
    assert(pr >= -0.5 && pr <= 0.5);

    float g = (1.0 * y) - (0.344136 * pb) - (0.714136 * pr);
    g = bound(0.0, 1.0, g);

    int scaled = g * denominator;
    scaled = bound(0.0, (float)denominator, scaled);

    /* check return value */
    assert(scaled <= denominator && scaled >= 0);

    return scaled;
}


/*
 * function YPbPr_calcB
 * Parameters:
 *      - float y: brightness value
 *      - float pb: blue color difference signal
 *      - float pr: red color difference signal
 *      - int denominator: number to use to scale return value by
 * Returns: an int representing the blue value based on the given YPbPr values
 * Does: calculate the value of the bleu component of a RGB pixel
 * Preconditions:
 *      - y is between 0 and 1
 *      - pb and pr are between -0.5 and 0.5
 *      - denominator is greater than 0
 */
int YPbPr_calcB(float y, float pb, float pr, int denominator)
{
    /* assert preconditions */
    assert(denominator > 0);
    assert(y >= 0 && y <= 1);
    assert(pb >= -0.5 && pb <= 0.5);
    assert(pr >= -0.5 && pr <= 0.5);

    float b = (1.0 * y) + (1.772 * pb) + (0.0 * pr);
    b = bound(0.0, 1.0, b);

    int scaled = b * denominator;
    scaled = bound(0.0, (float)denominator, scaled);

    /* check return value */
    assert(scaled <= denominator && scaled >= 0);

    return scaled;
}


/*
 * function YPbPr_newYPPPix
 * Parameters:
 *      - struct Pnm_rgb rgbPix for the pixel to convert
 *      - int denominator for the value the RGB pixel is scaled by
 * Returns: a YPbPr_pixel
 * Does: converts a pixel from RGB color space to YPbPr color space and
 *       returns the result
 * Preconditions:
 *      - denominator is greater than 0
 *      - rgbPix has been initalized
 */
YPbPr_pixel YPbPr_newYPPPix (struct Pnm_rgb rgbPix, int denominator)
{
    unsigned red = rgbPix.red;
    unsigned green = rgbPix.green;
    unsigned blue = rgbPix.blue;

    YPbPr_pixel new_pixel;
    new_pixel.y = YPbPr_calcY(red, green, blue, denominator);
    new_pixel.y = bound(0.0, 1.0, new_pixel.y);
    new_pixel.pb = YPbPr_calcPb(red, green, blue, denominator);
    new_pixel.pb = bound(-0.5, 0.5, new_pixel.pb);
    new_pixel.pr = YPbPr_calcPr(red, green, blue, denominator);
    new_pixel.pr = bound(-0.5, 0.5, new_pixel.pr);

    return new_pixel;
}


/*
 * function YPbPr_newRGBix
 * Parameters:
 *      - YPbPr_pixel yppPix for the pixel to convert
 *      - int denominator for the desired scale of the new RGB pixel
 * Returns: a struct Pnm_rgb
 * Does: converts the values in yppPix to RGB color space and stores them
 *       in the new pixel
 * Preconditions:
 *      - denominator is greater than 0
 *      - yppPix has been initalized
 */
struct Pnm_rgb YPbPr_newRGBpix (YPbPr_pixel yppPix, int denominator)
{
    struct Pnm_rgb new_pixel;

    float y = yppPix.y;
    float pb = yppPix.pb;
    float pr = yppPix.pr;

    new_pixel.red = YPbPr_calcR(y, pb, pr, denominator);
    new_pixel.red = bound(0.0, (float)denominator, new_pixel.red);
    new_pixel.green = YPbPr_calcG(y, pb, pr, denominator);
    new_pixel.green = bound(0.0, (float)denominator, new_pixel.green);
    new_pixel.blue = YPbPr_calcB(y, pb, pr, denominator);
    new_pixel.blue = bound(0.0, (float)denominator, new_pixel.blue);

    return new_pixel;
}

/*============================ APPLY FUNCS ==================================*/

/*
 * function YPbPr_newYPPApply
 * Parameters:
 *      - int col and row for the location of the pixel
 *      - A2Methods_T array2 for the 2D array of pixels
 *      - void * elem for a pointer to the pixel at (col, row)
 *      - void * cl for the closure argument
 *              - should be a struct newYPPcl containing a YPbPr_img image
 *                and an int denominator
 * Returns: nothing
 * Does: converts the given RGB pixel elem into a YPP pixel and stores it in
 *       the appropriate place in the YPbPr_img
 * Preconditions:
 *      - cl is non-null
 *      - array2 is non-null
 *      - elem is non-null
 */
void YPbPr_newYPPApply(int col, int row, A2Methods_UArray2 array2, void *elem,
    void *cl)
{
    /* check preconditions */
    assert(cl != NULL);
    assert(array2 != NULL);

    /* create new YPP pixel */
    YPbPr_pixel new_pix;

    /* fill in YPP values */
    Pnm_rgb rgb_pix = elem;
    unsigned red = rgb_pix -> red;
    unsigned green = rgb_pix -> green;
    unsigned blue = rgb_pix -> blue;

    struct newYPPcl * cl_large = cl;

    new_pix.y = YPbPr_calcY(red, green, blue, cl_large -> denominator);
    new_pix.y = bound(0.0, 1.0, new_pix.y);
    new_pix.pb = YPbPr_calcPb(red, green, blue, cl_large -> denominator);
    new_pix.pb = bound(-0.5, 0.5, new_pix.pb);
    new_pix.pr = YPbPr_calcPr(red, green, blue, cl_large -> denominator);
    new_pix.pr = bound(-0.5, 0.5, new_pix.pr);

    /* assign YPP pixel to correct place */
    YPbPr_pixel * final_dest = cl_large -> methods -> at(cl_large -> image ->
        pixels, col, row);
    *final_dest = new_pix;

    (void) array2;
}


/*
 * function YPbPr_newRGBApply
 * Parameters:
 *      - int col and row for the location of the pixel
 *      - A2Methods_T array2 for the 2D array of pixels
 *      - void * elem for a pointer to the pixel at (col, row)
 *      - void * cl for the closure argument
 *              - should be a struct newRGBcl containing a Pnm_ppm image
 *                and an int denominator
 * Returns: nothing
 * Does: converts the given YPP pixel elem into a RGB pixel and stores it in
 *       the appropriate place in the Pnm_ppm
 * Preconditions:
 *      - cl is non-null
 *      - array2 is non-null
 *      - elem is non-null
 */
void YPbPr_newRGBApply(int col, int row, A2Methods_UArray2 array2, void *elem,
    void *cl)
{
    /* check preconditions */
    assert(cl != NULL);
    assert(array2 != NULL);

    /* create new RGB pixel */
    struct Pnm_rgb new_pix;

    /* Fill in RGB values */
    YPbPr_pixel * ypp_pix = elem;
    float y = ypp_pix -> y;
    float pb = ypp_pix -> pb;
    float pr = ypp_pix -> pr;

    struct newRGBcl * cl_large = cl;

    new_pix.red = YPbPr_calcR(y, pb, pr, cl_large -> denominator);
    new_pix.red= bound(0.0, (float)cl_large -> denominator, new_pix.red);
    new_pix.green = YPbPr_calcG(y, pb, pr, cl_large -> denominator);
    new_pix.green = bound(0.0, (float)cl_large -> denominator, new_pix.green);
    new_pix.blue = YPbPr_calcB(y, pb, pr, cl_large -> denominator);
    new_pix.blue = bound(0.0, (float)cl_large -> denominator, new_pix.blue);

    /* Assign RGB pixel to correct place */
    Pnm_rgb final_dest = cl_large -> methods -> at(cl_large -> image -> pixels,
        col, row);

    final_dest -> red = new_pix.red;
    final_dest -> green = new_pix.green;
    final_dest -> blue = new_pix.blue;

    (void) array2;
}


/*
 * function bound
 * Paramters:
 *      - float lower for the lower bound
 *      - float upper for the upper bound
 *      - float num for the num to bound
 * Returns: num if it is bewteen upper and lower, lower if is less than lower
 *          higher if it is more than higher
 */
float bound(float lower, float upper, float num)
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


/*void YPbPr_print(YPbPr_img image)
{
    assert(image != NULL);

    FILE * out = stderr;

    for (int col = 0; col < image -> width; col++) {
        for (int row = 0; row < image -> height; row++) {
            YPbPr_pixel * to_print = image -> methods -> at(image -> pixels,
            col, row);
            fprintf(out, "y: %f\t pb: %f\t pr: %f\n", to_print -> y,
                       to_print -> pb, to_print -> pr);
        }
        fprintf(out, "\n");
    }
}*/

/*void print_vals(int col, int row, A2Methods_UArray2 array2, void *elem,
    void *cl)
{
    (void) cl;
    (void) col;
    (void) row;
    (void) array2;

     Pnm_rgb pix = elem;
     fprintf(stderr, "red: %d\n", pix-> red);
}*/

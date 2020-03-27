/*
 * Madeleine Street (mstree03) and Fergus Ferguson (mfergu05)
 * Comp 40 - Homework 4
 *
 * CodedImg.c
 *
 * Implementation for CodedImg. Uses bitpacking to create 32 bit codewords
 * representing each pixel in an image. Codewords are stored in a 2D array
 * and created using DCTBlocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "a2plain.h"
#include "a2methods.h"
#include "DCTransform.h"
#include "bitpack.h"
#include "CodedImg.h"

/******************************************************************************
 **                              DECLARATIONS                                **
 *****************************************************************************/

struct Coded_img {
    int height, width;
    A2Methods_UArray2 words;
    A2Methods_T methods;
};


void Coded_DCTtoCodedApply(int col, int row, A2Methods_UArray2 array2,
    void *elem, void *cl);
void Coded_CodedtoDCTApply(int col, int row, A2Methods_UArray2 array2,
    void *elem, void *cl);
void Coded_printCodewordApply(int col, int row, A2Methods_UArray2 array2,
    void *elem, void *cl);

uint64_t Coded_pack(DCTBlock block);
DCTBlock Coded_unpack(uint64_t codeword);


/* Constants used for packing codewords */
static const unsigned A_WIDTH = 9;
static const unsigned A_LSB = 23;
static const unsigned B_WIDTH = 5;
static const unsigned B_LSB = 18;
static const unsigned C_WIDTH = 5;
static const unsigned C_LSB = 13;
static const unsigned D_WIDTH = 5;
static const unsigned D_LSB = 8;
static const unsigned PB_WIDTH = 4;
static const unsigned PB_LSB = 4;
static const unsigned PR_WIDTH = 4;
static const unsigned PR_LSB = 0;

/******************************************************************************
 **                             NEW AND FREE                                 **
 *****************************************************************************/

/*
 * function Coded_new
 * Parameters:
 *     - DCT_img image - the DCT_image which will be converted to a Codewords
 *       representation
 * Returns: Coded_img - the converted image
 * Does: Converts a DCT_img representation of an image to a Coded_img struct.
 * This is done by bit packing the values in a DCT_block struct into int
 * codewords.
 * Preconditions: image is non-NULL
 */
Coded_img Coded_new(DCT_img image)
{
    assert(image != NULL);

    /* Allocate space */
    Coded_img new_image;
    new_image = malloc(sizeof(struct Coded_img));
    assert(new_image != NULL);

    /* Set up values */
    new_image -> methods = uarray2_methods_plain;
    new_image -> height = DCT_height(image);
    new_image -> width = DCT_width(image);
    new_image -> words = new_image -> methods -> new(new_image -> width,
                          new_image -> height, sizeof(uint64_t));
    assert(new_image -> words != NULL);

    /* Map function */
    A2Methods_mapfun *map = new_image -> methods -> map_row_major;

    /* TRANSFORM */
    map(new_image -> words, Coded_DCTtoCodedApply, image);

    return new_image;
}



/*
 * function Coded_newDCT
 * Parameters:
 *    - Coded_img image - bitpacked representation of an image
 * Returns: DCT_img - the converted image
 * Does: Converts an image from a bit packed representation (the Coded_img
 * struct) to a DCT_img representation
 * Preconditions: image is non-NULL
 */
DCT_img Coded_newDCT(Coded_img image)
{
    assert(image != NULL);

    /* Allocate space */
    DCT_img new_image;
    new_image = DCT_newDCTEmpty(image -> height, image -> width);
    assert(new_image != NULL);

    /* Map function */
    A2Methods_mapfun *map = image -> methods -> map_row_major;

    /* TRANSFORM */
    map(image -> words, Coded_CodedtoDCTApply, new_image);

    return new_image;
}


/*
 * function Coded_newEmpty
 * Parameters:
 *    - int height, width -- dimensions of the image
 * Returns: Coded_img - empty bitpacked representation of an image
 * Does: Creates and returns Coded_img struct with unitialized pixel values
 * Preconditions: None.
 */
Coded_img Coded_newEmpty(int height, int width)
{
    /* allocated and declare Coded_img */
    Coded_img new_image;
    new_image = malloc(sizeof(struct Coded_img));
    assert(new_image != NULL);

    new_image -> height = height;
    new_image -> width = width;

    new_image -> methods = uarray2_methods_plain;

    /* initialize word array in Coded_img */
    new_image -> words = new_image -> methods -> new(new_image -> width,
        new_image -> height, sizeof(uint64_t));

    return new_image;
}



/*
 * function Coded_free
 * Parameters:
 *      - Coded_img image to be freed
 * Returns: None
 * Preconditions:
 *      - image is not-NULL
 * Does: frees the coded image
 */
void Coded_free(Coded_img image)
{
    assert(image != NULL);

    image -> methods -> free(&(image->words));

    free(image);
}


/******************************* IO FUNCTIONS *******************************/

/*
 * function Coded_print
 * Parameters:
 *    - FILE * out - output stream
 *    - Coded_img image - bit packed image representation to be printed
 * Returns: None
 * Does: Prints out the bit packed image
 * Preconditions:
 *    - image is non-NULL
 *    - out is non-NULL
 */
void Coded_print(FILE * out, Coded_img image)
{
    assert(image != NULL && out != NULL);

    /* print header */
    int width = (image -> width) * 2;
    int height = (image -> height) * 2;
    fprintf(out, "COMP40 Compressed image format 2\n%u %u", width, height);
    fprintf(out, "\n");

    A2Methods_mapfun *map = image -> methods -> map_default;

    assert(image -> words);
    /* print out bit packed codewords */
    map(image -> words, Coded_printCodewordApply, out);

}




/*
 * function Coded_readin
 * Parameters:
 *    - File * in - input file
 * Returns: Coded_img - bitpacked image representation
 * Does: Creates a bitpacked file representation (Coded_img) from an input file
 * Preconditions:
 *    - in is non-NULL
 */
Coded_img Coded_readin(FILE * in)
{
    assert(in != NULL);

    /* get height and width from header */
    unsigned height, width;
    int read = fscanf(in, "COMP40 Compressed image format 2\n%u %u", &width,
                      &height);
    assert(read == 2);
    int c = getc(in);
    assert(c == '\n');

    /* create new empty image file to fill in */
    Coded_img new_image = Coded_newEmpty(height / 2, width /  2);

    /* fill in empty image file codewords */
    for (unsigned row = 0; row < height / 2; row++) {
        for (unsigned col = 0; col < width / 2; col++ ) {
            uint64_t * curr_word = new_image -> methods ->
                                   at(new_image -> words, col, row);
            *curr_word = 0;
            c = getc(in);
            assert(c != -1 && (row < height && col < width));
            *curr_word = Bitpack_newu(*curr_word, 8, 24, (uint64_t)c);

            c = getc(in);
            assert(c != -1 && (row < height && col < width));
            *curr_word = Bitpack_newu(*curr_word, 8, 16, (uint64_t)c);

            c = getc(in);
            assert(c != -1 && (row < height && col < width));
            *curr_word = Bitpack_newu(*curr_word, 8, 8, (uint64_t)c);

            c = getc(in);
            assert(c != -1 && (row < height && col < width));
            *curr_word = Bitpack_newu(*curr_word, 8, 0, (uint64_t)c);

        }
    }

    return new_image;
}


/*****************************************************************************
 **                                 HELPERS                                 **
 *****************************************************************************/

 /*
  * function Coded_pack
  * Parameters:
  *    - DCTBlock block - block representing a 2x2 pixel block after
  *      discrete cosign transformations have been applied.
  * Returns: uint64_t - bitpacked codeword
  * Does: Takes in a DCTBlock struct and converts it to a bitpacked integer
  * codeword.
  * Preconditions: block should be initialized
  */
uint64_t Coded_pack(DCTBlock block)
{
    uint64_t codeword = 0;
    /* insert a */
    assert(Bitpack_fitsu(block.a, A_WIDTH));
    codeword = Bitpack_newu(codeword, A_WIDTH, A_LSB, block.a);

    /* insert b */
    assert(Bitpack_fitss(block.b, B_WIDTH));
    codeword = Bitpack_news(codeword, B_WIDTH, B_LSB, block.b);

    /* insert c */
    assert(Bitpack_fitss(block.c, C_WIDTH));
    codeword = Bitpack_news(codeword, C_WIDTH, C_LSB, block.c);

    /* insert d */
    assert(Bitpack_fitss(block.d, D_WIDTH));
    codeword = Bitpack_news(codeword, D_WIDTH, D_LSB, block.d);

    /* insert Pb_avg */
    assert(Bitpack_fitsu((int64_t) block.Pb_avg, PB_WIDTH));
    codeword = Bitpack_newu(codeword, PB_WIDTH, PB_LSB, block.Pb_avg);

    /* insert Pr_avg */
    assert(Bitpack_fitsu((int64_t) block.Pr_avg, PR_WIDTH));
    codeword = Bitpack_newu(codeword, PR_WIDTH, PR_LSB, block.Pr_avg);

    return codeword;
}



/*
 * function Coded_unpack
 * Parameters:
 *    - uint64_t codeword
 * Returns: DCTBlock struct representation of the given codeword
 * Does: Creates a DCTBlock using a given bitpacked codeword representation
 * Preconditions: codeword expected to have been packed correctly
 */
DCTBlock Coded_unpack(uint64_t codeword)
{
    DCTBlock block;

    /* get a */
    block.a = Bitpack_getu(codeword, A_WIDTH, A_LSB);

    /* get b */
    block.b = Bitpack_gets(codeword, B_WIDTH, B_LSB);

    /* get c */
    block.c = Bitpack_gets(codeword, C_WIDTH, C_LSB);

    /* get d */
    block.d = Bitpack_gets(codeword, D_WIDTH, D_LSB);

    /* get Pb */
    block.Pb_avg = Bitpack_getu(codeword, PB_WIDTH, PB_LSB);

    /* get Pr */
    block.Pr_avg = Bitpack_getu(codeword, PR_WIDTH, PR_LSB);

    return block;
}

/****************************** APPLY FUNCTIONS ******************************/

/*
 * function Coded_DCTtoCodedApply
 * Parameters:
 *    - int col
 *    - int row
 *    - A2Methods_UArray2 array2 - array of codewords
 *    - void * elem - current element in the codeword array
 *    - void * cl - Coded_img struct being filled in
 * Returns: None
 * Does: apply function which is used to convert each DCTBlock in a given
 * DCT_img struct into a uint64_t and inserts it into a given
 * Coded_img struct.
 * Preconditions:
 *    - array2 and cl are non-NULL
 */
void Coded_DCTtoCodedApply(int col, int row, A2Methods_UArray2 array2,
    void *elem, void *cl)
{
    assert(array2 != NULL);
    assert(cl != NULL);

    DCT_img source = cl;
    DCTBlock * dctBlock = DCT_at(source, col, row);
    uint64_t codeword = Coded_pack(*dctBlock);

    /* insert  */
    uint64_t * dest = elem;
    *dest = codeword;
}


/*
 * function Coded_CodedtoDCTApply
 * Parameters:
 *    - int col
 *    - int row
 *    - A2Methods_UArray2 array2 - array of DCTBlock structs
 *    - void * elem - current element in the DCTBlock array
 *    - void * cl - DCT_img struct being filled in
 * Returns: None
 * Does: apply function which is used to convert each codeword in a given
 * Coded_img struct into a DCT_Block struct and inserts it into a given
 * DCT_img struct.
 * Preconditions:
 *    - array2 and cl are non-NULL
 */
void Coded_CodedtoDCTApply(int col, int row, A2Methods_UArray2 array2,
    void *elem, void *cl)
{
    assert(array2 != NULL);
    assert(cl != NULL);

    uint64_t * source = elem;
    DCT_img dest_image = cl;
    DCTBlock * dest = DCT_at(dest_image, col, row);
    *dest = Coded_unpack(*source);


}


/*
 * function Coded_printCodewordApply
 * Parameters:
 *    - int col
 *    - int row
 *    - A2Methods_UArray2 array2 - array of Coded_img structs
 *    - void * elem - current element in the codewords array
 *    - void * cl - output file
 * Returns: None
 * Does: Used to print all the bitpacked codewords in a given Coded_img struct
 * to the given output stream.
 * Preconditions:
 *    - array2 and cl are non-NULL
 */
void Coded_printCodewordApply(int col, int row, A2Methods_UArray2 array2,
    void *elem, void *cl)
{
    (void) col;
    (void) row;
    (void) array2;

    assert(cl != NULL);
    assert(elem != NULL);

    uint64_t * to_print = elem;
    FILE * out = cl;

    putc(Bitpack_getu(*to_print, 8, 24), out);
    putc(Bitpack_getu(*to_print, 8, 16), out);
    putc(Bitpack_getu(*to_print, 8, 8), out);
    putc(Bitpack_getu(*to_print, 8, 0), out);

}

/*
 * Madeleine Street (mstree03) and Fergus Ferguson (mfergu05)
 * COMP 40 Homework 4: Arith
 * DCTransform.h
 *
 * Interface for the DCTransform step of the compression
 */


#ifndef DCTRANSFORM_H
#define DCTRANSFORM_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include "a2plain.h"
#include "a2methods.h"
#include "PixBlock.h"

/* image struct */
typedef struct DCT_img * DCT_img;

/* each pixel in a DCT_img is a DCTBlock */
struct DCTBlock {
    uint64_t a;
    int64_t b, c, d;
    unsigned Pb_avg, Pr_avg;
};
typedef struct DCTBlock DCTBlock;

/* NEW AND FREE */
PixBlocked_img DCT_newPixBlockedImg(DCT_img image);
DCT_img DCT_newDCTImg(PixBlocked_img image);
DCT_img DCT_newDCTEmpty(int height, int width);
void DCT_free(DCT_img image);

/* GETTERS AND SETTERS */
DCTBlock * DCT_at(DCT_img image, int col, int row);
int DCT_height(DCT_img image);
int DCT_width(DCT_img image);



#endif

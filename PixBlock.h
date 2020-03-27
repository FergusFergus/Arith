/*
 * Madeleine Street (mstree03) and Fergus Ferguson (mfergu05)
 * COMP 40 Homework 4: Arith
 * PixBlock.h
 *
 * Interface for the PixBlock Steps of the transformation
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "YPbPr.h"
#include "a2plain.h"
#include "a2methods.h"

#ifndef PIXBLOCK_H
#define PIXBLOCK_H

/* Image struct */
typedef struct PixBlocked_img * PixBlocked_img;

/* Each pixel of an image is a pix_block */
typedef struct pix_block
{
    float Y1, Y2, Y3, Y4, Pb_avg, Pr_avg;
} pix_block;

/* NEW and FREE */
PixBlocked_img PixBlock_new(YPbPr_img image);
YPbPr_img PixBlock_newYPP(PixBlocked_img image);
PixBlocked_img PixBlock_newEmpty(int height, int width);
void PixBlock_free(PixBlocked_img image);

/* GETTERS AND SETTERS */
pix_block * PixBlock_at(PixBlocked_img image, int col, int row);
int PixBlock_height(PixBlocked_img image);
int PixBlock_width(PixBlocked_img image);

#endif

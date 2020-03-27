/*
 * Madeleine Street (mstree03) and Fergus Ferguson (mfergu05)
 * COMP 40 Homework 4: Arith
 * YPbPr.h
 *
 * Interface for the PixBlock Steps of the transformation
 */


#ifndef YPBPR_H
#define YPBPR_H

#include "pnm.h"
#include "uarray2.h"

typedef struct YPbPr_pixel {
    float y, pb, pr;
} YPbPr_pixel;

typedef struct YPbPr_img *YPbPr_img;

/***************************** INITIALIZERS ********************************/
YPbPr_img YPbPr_new(Pnm_ppm image);
YPbPr_img YPbPr_newEmpty(int height, int width);
Pnm_ppm YPbPr_newPnm(YPbPr_img image);

/******************************** GETTERS **********************************/
YPbPr_pixel YPbPr_getPixel(YPbPr_img image, int col, int row);
void YPbPr_setPixel(YPbPr_img image, YPbPr_pixel pix, int col, int row);

int YPbPr_getWidth(YPbPr_img image);
int YPbPr_getHeight(YPbPr_img image);

/******************************* DESTRUCTORS ********************************/
void YPbPr_free(YPbPr_img image);


/********************* FOR TESTING ONLY!!!!! REMOVE *********************/
YPbPr_img YPbPr_new2By2();
void YPbPr_print(YPbPr_img image);



#endif

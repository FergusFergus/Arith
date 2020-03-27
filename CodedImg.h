/*
 * Madeleine Street (mstree03) and Fergus Ferguson (mfergu05)
 * Comp 40 - Homework 4
 *
 * CodedImg.h
 *
 * Interface for CodedImg.
 */

#ifndef CODEDIMG_H
#define CODEDIMG_H

 #include <stdio.h>
 #include <stdlib.h>
 #include <assert.h>
 #include <stdbool.h>
 #include <stdint.h>
 #include "a2plain.h"
 #include "a2methods.h"
 #include "DCTransform.h"
 #include "bitpack.h"


typedef struct Coded_img *Coded_img;

/* NEW and FREE */
Coded_img Coded_new(DCT_img image);
DCT_img Coded_newDCT(Coded_img image);
Coded_img Coded_newEmpty(int height, int width);
void Coded_free(Coded_img image);

/* IO Functions */
void Coded_print(FILE * out, Coded_img image);
Coded_img Coded_readin(FILE * in);

#endif

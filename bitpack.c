/*
 * Madeleine Street (mstree03) and Fergus Ferguson (mfergu05)
 * Comp 40 - Homework 4
 *
 * bitpack.c
 *
 * Implementation for bitpack.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "except.h"
#include "bitpack.h"

/* HELPER DECLARATIONS */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
    uint64_t value);
uint64_t Bitpack_shiftL(uint64_t word, unsigned offset);
uint64_t Bitpack_shiftR(uint64_t word, unsigned offset);
uint64_t Bitpack_mask(unsigned width, unsigned lsb);
int64_t Bitpack_shiftRS(uint64_t word, unsigned offset);
int64_t Bitpack_shiftLS(uint64_t word, unsigned offset);

/*=============================== SETTERS ===================================*/

/* function Bitpack_fitsu
 * Parameters:
 *      - uint64 n: the word to check, where values are unsigned
 *      - unsigned width: for the length to try and fit
 * Returns: True if n can fit in width bits and false otherwise
 * Preconditions:
 * Does: Checks to see if the word n can be represented in width bits
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
    /* get the highest value an int can be if it is represented with width
       bits */
    double highest = pow(2, width) - 1;

    if (n <= highest) {
        return true;
    }
    else {
        return false;
    }
}

/* function Bitpack_fitss
 * Parameters:
 *      - int64_t n : the word to check
 *      - unsigned width: the length to try to fit
 * Returns: True is n can fit something of length width within it and false
 * otherwise.
 * Preconditions:
 * Does: Checks if a given integer n can fit in a bit representation of width
 * bits.
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
    /* number of ints that can be represented with width bits*/
    double range = pow(2, width);
    /* highest signed value that can be represented with width bits */
    double highest = (range / 2) - 1;
    /* lowest signed value that can be represented with width bits */
    double lowest = -(range / 2);

    if (n >= lowest && n <= highest) {
        return true;
    } else {
        return false;
    }
}



/*=============================== GETTERS ===================================*/

/* function Bitpack_getu
 * Parameters:
 *      - uint64_t word: the word to retrieve information from
 *      - unsigned width: the length of the value to get from the word
 *      - unsigned lsb: the location of the least signigficant bit in the word
 * Returns: A new 64 bit word containing only the data asked for
 * Preconditions:
 * Does: gets the stated bits from the word and returns them in a
 *       new 64 bit word
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
    assert(width <= 64);
    assert(lsb <= (64 - width));

    if (width == 0) {
        return 0;
    }

    uint64_t mask = Bitpack_mask(width, lsb);

    uint64_t result = word & mask;

    result = Bitpack_shiftR(result, lsb);

    return result;
}



/* function Bitpack_gets
 * Parameters:
 *      - uint64_t word: the word used to retrieve information
 *      - width: the length of the information being retrieved from the word
 *      - unsigned lsb: the location of the least significant bit in the word
 * Returns: A 64 bit integer containing only the information requested from the
 * given word.
 * Preconditions:
 * Does: Returns information from a given word of a given length with a given
 * index.
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
    assert(width <= 64);
    assert(lsb <= (64 - width));

    if (width == 0) {
        return 0;
    }

    uint64_t mask = Bitpack_mask(width, lsb);

    int64_t result = word & mask;


    unsigned offset = 64 - (lsb + width);
    result = Bitpack_shiftLS(result, offset);
    result = Bitpack_shiftRS(result, offset + lsb);

    return result;
}

/*============================= INITIALIZERS ================================*/

/* function Bitpack_newu
 * Parameters:
 *      - uint64_t word: represnting the value who's field will be changed
 *      - unsigned width: length in bits of the field to edit
 *      - unsigned lsb: location of the least significant bit of the field to
 *                       edit
 *      - uint64_t value: the value to insert into the given field of the word
 *
 * Returns: a copy of the updated word
 * Preconditions:
 *      - width is not greater than 64
 *      - value will fit in a space of length width
 *      - the field is placed within the word such that there is enough room
*         the entire field
 * Does: Sets a field in a word to a given unsigned value and returns a copy
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
    uint64_t value)
{
    assert(Bitpack_fitsu(value, width) == true);

    /* Clear out field */
    uint64_t  clear_mask = ~(Bitpack_mask(width, lsb));
    word = word & clear_mask;

    /* move value to correct place */
    value = Bitpack_shiftL(value, lsb);

    /* insert value */
    word = word | value;

    return word;
}



/* function Bitpack_news
 * Parameters:
 *      - uint64_t word: represnting the value who's field will be changed
 *      - unsigned width: length in bits of the field to edit
 *      - unsigned lsb: location of the least significant bit of the field to
 *                       edit
 *      - int64_t value: the signed value to insert into the given field
 *                       of the word
 *
 * Returns: a copy of the updated word
 * Preconditions:
 *      - width is not greater than 64
 *      - value will fit in a space of length width
 *      - the field is placed within the word such that there is enough room
*         the entire field
 * Does: Sets a field in a word to a given signed value and returns a copy
 */
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
    int64_t value)
{
    assert(Bitpack_fitss(value, width) == true);

    /* Clear out field */
    uint64_t  clear_mask = ~(Bitpack_mask(width, lsb));
    word = word & clear_mask;

    /* move value to correct place */
    value = Bitpack_shiftL(value, lsb);
    value = value & (~clear_mask);

    /* insert value */
    word = word | value;

    return word;
}

/*=============================== HELPERS ===================================*/

/* function Bitpack_mask
 * Parameters:
 *      - width: the width of the mask to create
 *      - lsb: the index of the least significant bit to start the mask from
 * Returns: an unsigned 64 bit int where all bits are 0 except for the bits
 *          from lsb to lsb + width which are ones
 * Preconditions:
 *      - width is less than or equal to 64
 *      - there must be enough space for width bits from lsb to the most
 *        significant bit
 * Does:
 */
uint64_t Bitpack_mask(unsigned width, unsigned lsb)
{
    assert(width <= 64);
    assert(lsb <= (64 - width));

    uint64_t mask = (uint64_t) ~0;

    mask = Bitpack_shiftR(mask, 64 - width);
    mask = Bitpack_shiftL(mask, lsb);

    return mask;
}

/* function Bitpack_shiftL
 * Parameters:
 *      - uint64_t word: the word to perform a shift on
 *      - unsigned offset: the amount to shift the given word
 * Returns: Returns a uint64_t represnting the shifted word
 * Preconditions: offset must be less than 64
 * Does: shifts the bits in word offest bits to the left
 */
uint64_t Bitpack_shiftL(uint64_t word, unsigned offset)
{
    assert(offset < 64);

    word = word << offset;

    return word;
}

/* function Bitpack_shiftR
 * Parameters:
 *      - uint64_t word: the word to perform a shift on
 *      - unsigned offset: the amount to shift the given word
 * Returns: Returns a uint64_t represnting the shifted word
 * Preconditions: offset must be less than 64
 * Does: shifts the bits in word offest bits to the right
 */
uint64_t Bitpack_shiftR(uint64_t word, unsigned offset)
{
    assert(offset < 64);

    word = word >> offset;

    return word;
}


/* function Bitpack_shiftLS
 * Parameters:
 *      - uint64_t word: the word to perform a shift on
 *      - unsigned offset: the amount to shift the given word
 * Returns: Returns an signed int64_t represnting the shifted word
 * Preconditions: offset must be less than 64
 * Does: shifts the bits in word offest bits to the left
 */
int64_t Bitpack_shiftLS(uint64_t word, unsigned offset)
{

    assert(offset < 64);

    int64_t result = word;
    result = result << offset;

    return result;
}

/* function Bitpack_shiftRS
 * Parameters:
 *      - uint64_t word: the word to perform a shift on
 *      - unsigned offset: the amount to shift the given word
 * Returns: Returns an signed int64_t represnting the shifted word
 * Preconditions: offset must be less than 64
 * Does: shifts the bits in word offest bits to the right
 */
int64_t Bitpack_shiftRS(uint64_t word, unsigned offset)
{
    assert(offset < 64);

    int64_t result = word;
    result = result >> offset;


    return result;
}



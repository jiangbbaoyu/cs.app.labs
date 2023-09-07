/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
    int zeroIfAllOne = ~(x & y);
    int oneIfContainsOne = ~(~x & ~y);
    int bitXor = oneIfContainsOne & zeroIfAllOne;
    return bitXor;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {

//  return 1<<31;
    return (~1+1)<<31;

}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
    //  use >>
//  int sign1 = x>>31 & 1;
//  int sign2 = (x+1) >> 31 & 1 ;
//  return !sign1 & sign2; //  x is positive number and x+1 is negative number

//  when x == 0111..11(Tmax) or x == 11111..1111 (-1),  ~(x+1) == x , and x^ ~(x+1) == 0
//  and when  x == 11111...1111, !(~1) == 1,  (x^(~(x+1)))|(!(~x)) = (0|1) == 1, and   !(x^(~(x+1)))|(!(~x))== 0
//  and when  x == 01111...1111, !(~1) == 0,  (x^(~(x+1)))|(!(~x)) = (0|0) == 0, and   !(x^(~(x+1)))|(!(~x))== 1
    return !((x^(~(x+1)))|(!(~x)));
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {

//    return 0xAAAAAAAA== (0xAAAAAAAA & x);

    int mask = 0xAA | 0xAA<<8; // Integer constants 0 through 255 (0xFF), inclusive
    mask = mask | mask << 16;

    return !((mask & x) ^ mask ); // (mask & x )== mask
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
    return (~x)+1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
    /**              *
     * 0x30:    0011 0000
     *          0011 0001
     *          0011 0010
     *          0011 0011
     *          0011 0100
     *          0011 0101
     *          0011 0110
     *          0011 0111  last four bit is zero
     *          ------------
     *          0011 1000  last four bit is one
     * 0x39:    0011 1001
     *
     * 1. the last two byte must be 0x0011  (!!((x>>4)^3)) == 0
     * 2  when the last four bit is zero, the last one,two, three bit can be one or zero ,both is ok
     * 3. when the last four bit is one, the last one bit can be one or zero ,both is ok. the last two,three must be zero.
     *    ((x>>3&x>>1)|(x>>3&x>>2))&1 is zero,  only if 1) the last four bit is zero,
     *                                                  2) the last four bit is one and the last two,three is zero
     */
    return !(  (!!((x>>4)^3)) | (((x>>3&x>>1)|(x>>3&x>>2)) & 1) );
//    return !(  (!!((x>>4)^3)) | (x>>3&x>>1)&1 | (x>>3&x>>2)&1) ;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
    // if x is zero , then mask is 0b000...000 ; if x is not zero , then mask is 0b1111...1111
    int mask  = !!(x)<<31>>31;
//    int mask = (~(!!x)) + 1;
    // if mask is 0b000...000 ,  mask & y == 0b000...000, ~mask&z == z, ( mask&y | ~mask&z ) ==z
    // if mask is 0b111...111 ,  mask & y == y, ~mask&z == 0b000...000, ( mask&y | ~mask&z ) ==y
    return ( (mask & y) | (~mask&z) );
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    // sign_ is one : when x<0 and y>= 0
    int sign_ = ((x&(~y)) >> 31) & 1;

    // equal_ is one when x == y
    int equal_ = !( x ^ y );

    // mark is 0b111...111 when x and y has same sign bit
    // mark is 0b000...000 when x and y has different sign bit
    int mark_ = ~( ( x ^ y ) >> 31 );

    /**
     * x leq y when :
     *  1. x<0 and y>= 0
     *  2. x == y;
     *  3. x,y has same sign bit and x+ (-y) < 0 , ( the bits representation of x+ (-y)  is 0b1xxxxx, sign bit is one )
     */
    return sign_ |equal_ | (((mark_)&(x+~y+1)>>31)&1);
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {

    /**
     *  Tmin  -x .... -y  0 y ...... x
     *        |        |____|        |
     *        |______________________|
     *          has opposite sign bit
     *
     * when x ==0 , ~x+1 ==0,  the sign bit is unchanged and is always zero
     * when x ==Tmin(0b1000...000) , ~x+1 ==Tmin(0b1000...000),  the sign bit is unchanged and is always one
     * when x !=0 && x!=Tmin, the sign bit of x and the sign bit of ~x+1 is different ,
     *                        and sign bit of x | sign bit of ~x+1 must be one
     */
    return ~((~x+1)| x) >>31 &1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */

int howManyBits(int x) {
    int n =0;
    /**
     * 1. for x = 0b11...110xx.. , the minimum bits to represent x is 0b10xx..,
     *    eg.  0b11 10 =-8+4+2=-2,  0b10 = -2, so 0b10 is the minimum bits pattern
     * 2. for x= 0b00...01xx...,  the minimum bits to represent x is 0b010xx.. instead of 0b010xx..
     *    eg. 0b00011 = 1+2 =3,  0b011 = 1+2 =3, but 0b11 = -2+1 =-1
     *
     * and x = x^ (x<<1)  operation
     *  1. let the last one of a series ones in the x's leading positions unchanged, and make other ones zero
     *     eg.   0b11 10x
     *         ^ 0b11 0xx
     *           --------
     *           0b00 1xx
     *  2. make the last zero of a series zeros in the x's leading positions one, and other zeros unchanged
     *     eg.   0b00 01x
     *         ^ 0b00 1xx
     *           --------
     *           0b00 1xx
     */
    x = x^ (x<<1);
    /**
     * by the conversion before,
     * to calculate the minimum number of bits required to represent x in 2's complement
     * we only need to `determine the position of the first leading one bit`, and the the minimum bits number depend on this position
     * eg:  x= 0b001111..101, x^ (x<<1) = x= 0b01xxxx..xxx the position of the first one bit is 31th, from left to right, and we need 31 bits to represent x
     *
     *  to locate the first one bit , we use binary search
     */
    // search 32~17 bits, if 32~17 bits contains one , search 32~25bits further, otherwise search 16~8 bits
    n += ((!!(x&((-1)<<(n+16)))) << 4);// +16
    // search 32~25bits, if 32~25 bits contains one , search 32~29bits further, otherwise search 28~25 bits
    // search 16~8bits, if 16~8 bits contains one , search 16~13bits further, otherwise search 8~5 bits
    n += ((!!(x&((-1)<<(n+8)))) << 3); // +8
    // search 32~29bits, if 32~29 bits contains one , search 32~31bits further, otherwise search 30~31 bits
    // search 28~25bits, if 28~25 bits contains one , search 28~27bits further, otherwise search 26~25 bits

    // search 16~13bits, if 16~13 bits contains one , search 16~15bits further, otherwise search 14~13 bits
    // search 8~5bits, if 8~5 bits contains one , search 8~7bits further, otherwise search 6~5 bits
    n += ((!!(x&((-1)<<(n+4)))) << 2); // +4

    // search 32~31bits, if 32~31 bits contains one , search 32bits further, otherwise search 31 bits
    // search 28~27bits, if 28~27 bits contains one , search 28bits further, otherwise search 27 bits

    // ...
    n += ((!!(x&((-1)<<(n+2)))) << 1); // +2

    // search 32bit
    // search 30bit
    // search 38bit
    // ...
    n += (!!(x&((-1)<<(n+1))));    // +1
    return n+1;
}

//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
    unsigned sign = uf >> 31;
    unsigned exp = (uf >> 23) & 0xff;
    unsigned frac = uf & 0x7fffff;

    if (exp == 0xFF) {
        return uf;
    }

    if (exp == 0) {
        /* Denormalized */
        frac <<= 1;
    } else if (exp == 0xFE) {
        /* twice to oo */
        exp = 0xFF;
        frac = 0;
    } else {
        /* Normalized */
        exp += 1;
    }

    return sign << 31 | exp << 23 | frac;
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
    unsigned sig = uf >> 31;
    unsigned exp = uf >> 23 & 0xFF;
    unsigned frac = uf & 0x7FFFFF;
    unsigned bias = 0x7F;

    int num;
    unsigned E;
    unsigned M;

    /*
     * consider positive numbers
     *
     * 0 00000000 00000000000000000000000
     *   ===>
     * 0 01111111 00000000000000000000000
     *   0 <= f < 1
     * get integer 0
     *
     * 0 01111111 00000000000000000000000
     *   ===>
     * 0 (01111111+31) 00000000000000000000000
     *   1 <= f < 2^31
     * integer round to 0
     *
     * 0 (01111111+31) 00000000000000000000000
     *   ===>
     * greater
     *   2^31 <= f < oo
     * return 0x80000000
     */
    if (exp >= 0 && exp < 0 + bias) {
        /* number less than 1 */
        num = 0;
    } else if (exp >= 31 + bias) { // E= exp-bias >=31
        /* to big, number overflow !! */
        /* or f < 0 and (int)f == INT_MIN */
        num = 0x80000000;
    } else {
        E = exp - bias;
        M = frac | 0x800000;  // M = 1 + frac
        if (E > 23) {  // 23<E<31
            /*this case is safe */
            num = M << (E - 23);  // threat M as an integer (already shift left 23 positions implicitly ) ,23<E<31 means num > M  and M will shift right E - 23 positions
        } else { // 1<=E <=23
            /* round may occur !! */
            num = M >> (23 - E);  // threat M as an integer (already shift left 23 positions implicitly ) ,1<=E <=23 means num <= M and M will shift left 23-E positions
        }
    }

    return sig ? -num : num;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
    unsigned INF = 0xff << 23;
    int e = 127 + x;
    if (e <= 0)
        return 0;
    if (e >= 255)
        return INF;
    return e << 23;
}

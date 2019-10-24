#ifndef _ZXSOFT_DIGEST_MD5_H_
#define _ZXSOFT_DIGEST_MD5_H_

typedef unsigned int  UINT4;

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* MD5 context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;


void MD5Init(MD5_CTX *context);                                        /* context */

void MD5Update(
		MD5_CTX *context,                                        /* context */
		unsigned char *input,                                /* input block */
		unsigned int inputLen);                     /* length of input block */

void MD5Final (
		unsigned char digest[16],                       /* message digest */
		MD5_CTX *context);                                       /* context */


#endif

/*
* Copyright Catena
* Creation date: 2016 04 04 16:46:09
* Version: 5.00 - p5.09
*
* Usage:
* Compile and link this C-file and declare in the source-file for required initialization transmissions the following:
*
* extern const size_t LutSize;
* extern const unsigned char * pLutBytes;
*
* extern const size_t PatchSize;
* extern const unsigned char * pPatchBytes;
*
* The source for required initialization transmissions should create the following set of I2C transmissions:
*
* [ w 1C 0000 ] script example: S C8 1C 00 00 P
* [ w 1C 0074 ] script example: S C8 1C 00 74 P
* [ w 1B <pPatchBytes> ] script example: S C8 1B F0 00 .... P
* PatchSize of pPatchBytes can be divided (on a 2 byte boundary) in separate transmissions all starting with command identifier 0x1B.
* [ w 1C 0000 ] script example: S C8 1C 00 00 P
* [ w 1C 0075 ] script example: S C8 1C 00 75 P
* [ w 1B <pLutBytes> ] script example: S C8 1B 80 17 .... P
* LutSize of pLutBytes can be divided (on a 2 byte boundary) in separate transmissions all starting with command identifier 0x1B.
* [ w 1C 0000 ] script example: S C8 1C 00 00 P
*
*/
#include <stdlib.h>
extern const size_t PatchSize;
extern const unsigned char *pPatchBytes;
static const unsigned char PatchByteValues[] =
{
0xF0, 0x00, 0x60, 0x33, 0xD0, 0x80, 0x20, 0x87, 0x60, 0x3D, 0xD0, 0x80, 0x90, 0x00, 0x60, 0x3F, 0xD0, 0x80, 0x22, 0x02, 0x00, 0x55, 0x60, 0x04,
0xF0, 0x00, 0x60, 0x51, 0xDF, 0x80, 0x20, 0x90, 0x60, 0x67, 0xD0, 0x80, 0x31, 0x40, 0x60, 0x6B, 0xD0, 0x80, 0x9E, 0xB9, 0x60, 0x74, 0xD0, 0x80,
0xF0, 0x00, 0x60, 0x76, 0xD0, 0x80, 0xF0, 0x00, 0x60, 0x7F, 0xD0, 0x80, 0x90, 0x01, 0x60, 0x84, 0xD0, 0x80, 0x91, 0x01, 0x60, 0x85, 0xD0, 0x80,
0xF0, 0x00, 0x60, 0x86, 0xD0, 0x80, 0xA2, 0x64, 0x70, 0x00, 0xF0, 0x00, 0x31, 0xA0, 0x60, 0xAE, 0xD0, 0x80, 0x20, 0x13, 0x62, 0xA5, 0xD0, 0x80,
0xF0, 0x00, 0x60, 0xE9, 0xD0, 0x80, 0xF0, 0x00, 0x60, 0xF7, 0xD2, 0x80, 0xF0, 0x00, 0x61, 0x02, 0xD0, 0x80, 0x4F, 0xD0, 0x61, 0x00, 0xD1, 0x80,
0x9E, 0x73, 0x61, 0x31, 0xD0, 0x80, 0xF0, 0x00, 0x61, 0x3C, 0xD0, 0x80, 0xF0, 0x00, 0x61, 0x44, 0xD0, 0x80, 0xF0, 0x00, 0x61, 0x59, 0xD0, 0x80,
0x2C, 0x87, 0x61, 0x65, 0xD0, 0x80, 0xF0, 0x00, 0x61, 0x69, 0xD0, 0x80, 0xF0, 0x00, 0x61, 0x6C, 0xD2, 0x80, 0xF0, 0x00, 0x70, 0x00, 0xF0, 0x00,
0xF0, 0x00, 0x61, 0x98, 0xD2, 0x80, 0x57, 0xF2, 0x61, 0x9A, 0xD5, 0x80, 0xA8, 0x80, 0x70, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x61, 0x9D, 0xD0, 0x80,
0xF0, 0x00, 0x61, 0xA4, 0xD2, 0x80, 0x4F, 0xA0, 0x61, 0x00, 0xD2, 0x80, 0x4F, 0xC0, 0x61, 0x00, 0xD2, 0x80, 0xF0, 0x00, 0x61, 0xB7, 0xD2, 0x80,
0x41, 0x20, 0x14, 0xF6, 0xD2, 0x80, 0xF0, 0x00, 0x62, 0x01, 0xD0, 0x80, 0xF0, 0x00, 0x62, 0x11, 0xD0, 0x80, 0xF0, 0x00, 0x62, 0x1A, 0xD0, 0x80,
0xF0, 0x00, 0x62, 0x2E, 0xD0, 0x80, 0xF0, 0x00, 0x62, 0x34, 0xD0, 0x80, 0xF0, 0x00, 0x62, 0x38, 0xD0, 0x80, 0xF0, 0x00, 0x62, 0x44, 0xD0, 0x80,
0xF0, 0x00, 0x62, 0x70, 0xD0, 0x80, 0xF0, 0x00, 0x62, 0x72, 0xD0, 0x80, 0x82, 0x09, 0x0D, 0xA2, 0x60, 0x0D, 0xF0, 0x00, 0x62, 0x84, 0xD2, 0x80,
0xF0, 0x00, 0x62, 0x84, 0xD2, 0x80, 0xF0, 0x00, 0x62, 0x83, 0xD5, 0x80, 0xF0, 0x00, 0x62, 0xA7, 0xD2, 0x80, 0x82, 0x00, 0x70, 0x00, 0xA0, 0x10,
0x82, 0x00, 0x70, 0x00, 0xA0, 0x2B, 0x82, 0x00, 0x70, 0x00, 0xA0, 0x69, 0x82, 0x00, 0x70, 0x00, 0xA0, 0xAA, 0x82, 0x00, 0x70, 0x00, 0xA2, 0x6D,
0x82, 0x00, 0x70, 0x00, 0xA1, 0x7A, 0x82, 0x00, 0x70, 0x00, 0xA1, 0x84, 0x82, 0x00, 0x70, 0x00, 0xA2, 0x0C, 0x82, 0x00, 0x70, 0x00, 0xA2, 0x69,
0xF0, 0x00, 0x27, 0xAA, 0xD0, 0x80, 0xF0, 0x00, 0x01, 0xA9, 0xD5, 0x80, 0x8C, 0x79, 0x00, 0x46, 0xD0, 0x80, 0xF0, 0x00, 0x70, 0x00, 0xD4, 0x09,
0xF0, 0x00, 0x00, 0x97, 0xD0, 0x80, 0x40, 0x11, 0x0E, 0x72, 0x60, 0x09, 0xD0, 0x7F, 0x70, 0x00, 0xF0, 0x00, 0xC5, 0xC9, 0x01, 0xB6, 0xD0, 0x80,
0x05, 0x44, 0x60, 0x08, 0xA2, 0x60, 0xF0, 0x00, 0x30, 0x80, 0xD0, 0x08, 0xF0, 0x00, 0x07, 0x69, 0x60, 0x08, 0xF0, 0x00, 0x05, 0x19, 0x60, 0x09,
0xF0, 0x00, 0x20, 0x00, 0xF0, 0x00, 0x26, 0x92, 0x54, 0xA3, 0x60, 0x01, 0xA2, 0x00, 0x70, 0x00, 0xF0, 0x00, 0xA2, 0x08, 0x70, 0x00, 0xF0, 0x00,
0xA2, 0x10, 0x70, 0x00, 0xF0, 0x00, 0xD8, 0x40, 0x70, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x34, 0x90, 0xF0, 0x00, 0xF0, 0x00, 0x35, 0x90, 0xF0, 0x00,
0xF0, 0x00, 0x36, 0x90, 0xD0, 0x08, 0x40, 0x15, 0x20, 0x53, 0xA2, 0x53, 0xA0, 0xE8, 0x58, 0x06, 0xA2, 0x52, 0xA0, 0x72, 0x20, 0x64, 0xF0, 0x00,
0xA8, 0x61, 0x70, 0x00, 0xF0, 0x00, 0xA1, 0x28, 0x70, 0x00, 0xF0, 0x00, 0xA0, 0xB2, 0x70, 0x00, 0xF0, 0x00, 0xA8, 0x62, 0x70, 0x00, 0xD0, 0x08,
0x05, 0x79, 0x60, 0x08, 0x82, 0x4D, 0xF0, 0x00, 0x10, 0x68, 0x60, 0x01, 0xF0, 0x00, 0x60, 0x03, 0x80, 0x02, 0xF0, 0x00, 0x08, 0x34, 0x60, 0x01,
0xF8, 0x00, 0x60, 0x03, 0x80, 0x00, 0x18, 0x31, 0x60, 0x08, 0xA2, 0x47, 0xF0, 0x00, 0x30, 0x01, 0xF0, 0x00, 0xF0, 0x00, 0x31, 0x03, 0xD0, 0x08,
0xF0, 0x00, 0x1D, 0x10, 0x60, 0x09, 0x40, 0x11, 0x7F, 0xFF, 0x60, 0x02, 0xF0, 0x00, 0x30, 0x10, 0xF0, 0x00, 0xF0, 0x00, 0x30, 0x92, 0xF0, 0x00,
0xF0, 0x00, 0x31, 0x11, 0xF0, 0x00, 0x1D, 0x0F, 0x60, 0x08, 0xA2, 0x40, 0xF0, 0x00, 0x70, 0x00, 0x82, 0x3E, 0x9A, 0x62, 0x1D, 0x0F, 0x60, 0x0A,
0x82, 0x92, 0x70, 0x00, 0x94, 0x01, 0x90, 0x8A, 0x00, 0x00, 0x60, 0x01, 0x30, 0x22, 0x08, 0x0C, 0xD0, 0x80, 0xF0, 0x00, 0x09, 0x44, 0xD2, 0x80,
0xF0, 0x00, 0x07, 0xFD, 0xD2, 0x80, 0xF0, 0x00, 0x08, 0x11, 0xD0, 0x80, 0xF0, 0x00, 0x18, 0x72, 0x60, 0x08, 0xF0, 0x00, 0x1D, 0x0F, 0x60, 0x09,
0xF0, 0x00, 0x2D, 0x80, 0xA2, 0x34, 0x9A, 0x02, 0x20, 0x11, 0xF0, 0x00, 0x90, 0x8A, 0x70, 0x00, 0xD0, 0x09, 0x90, 0x41, 0x70, 0x00, 0xD0, 0x08,
0x90, 0xC1, 0x08, 0xEC, 0xD1, 0x80, 0xF0, 0x00, 0x08, 0xDC, 0xD0, 0x80, 0xF0, 0x00, 0x1D, 0x0F, 0x60, 0x0A, 0xF0, 0x00, 0x70, 0x00, 0xF0, 0x00,
0x20, 0x23, 0x70, 0x00, 0xA2, 0x2C, 0x80, 0x59, 0x09, 0x49, 0xD0, 0x80, 0xF0, 0x00, 0x03, 0x13, 0x60, 0x06, 0xF0, 0x00, 0x70, 0x00, 0x80, 0x01,
0xF0, 0x00, 0xFC, 0xED, 0x60, 0x06, 0x1D, 0x10, 0x60, 0x0E, 0xA2, 0x27, 0xF0, 0x00, 0x30, 0x66, 0xD0, 0x08, 0x1D, 0x0F, 0x60, 0x0B, 0xA2, 0x25,
0xF0, 0x00, 0x20, 0x37, 0xA2, 0x24, 0x91, 0xC7, 0x21, 0x91, 0xF0, 0x00, 0x90, 0xC3, 0x0B, 0x0B, 0xD1, 0x80, 0x82, 0x49, 0x0B, 0x14, 0xD0, 0x80,
0x31, 0x91, 0x0B, 0x14, 0xD0, 0x80, 0x31, 0x91, 0x0B, 0x14, 0xD0, 0x80, 0x1D, 0x10, 0x60, 0x0D, 0xA2, 0x1E, 0xF0, 0x00, 0x21, 0x57, 0xA2, 0x1D,
0x91, 0xC7, 0x31, 0x50, 0xF0, 0x00, 0x90, 0x00, 0x70, 0x00, 0x94, 0x02, 0xF0, 0x00, 0x70, 0x00, 0x90, 0x01, 0xF0, 0x00, 0x70, 0x00, 0xAF, 0xEE,
0xF0, 0x00, 0x7F, 0xFF, 0x60, 0x04, 0x82, 0x00, 0x20, 0x57, 0xA2, 0x17, 0x91, 0xC7, 0x20, 0xD3, 0xA2, 0x16, 0x80, 0xFB, 0x70, 0x00, 0x90, 0x06,
0x82, 0xE2, 0x30, 0xD3, 0x98, 0x01, 0xF0, 0x00, 0x70, 0x00, 0x98, 0x04, 0x91, 0xC7, 0x30, 0x50, 0xF0, 0x00, 0x91, 0x00, 0x70, 0x00, 0xE1, 0xC0,
0x30, 0xD0, 0x70, 0x00, 0xF0, 0x00, 0x90, 0x03, 0x70, 0x00, 0xF0, 0x00, 0xA2, 0xED, 0x70, 0x00, 0xF0, 0x00, 0xA2, 0xF6, 0x31, 0x05, 0xF0, 0x00,
0xF0, 0x00, 0x0B, 0x22, 0xD0, 0x80, 0xF0, 0x00, 0x0B, 0xFD, 0xD2, 0x80, 0xF0, 0x00, 0x70, 0x00, 0x98, 0xB7, 0xF0, 0x00, 0x70, 0x00, 0x80, 0xB9,
0xF0, 0x00, 0x2D, 0xBB, 0xD0, 0x80, 0xF0, 0x00, 0x70, 0x00, 0x80, 0xE2, 0xF0, 0x00, 0x70, 0x00, 0x80, 0xE5, 0x83, 0xFF, 0x1D, 0x13, 0x60, 0x08,
0xF0, 0x00, 0x1D, 0x15, 0x60, 0x09, 0x10, 0x07, 0x00, 0x00, 0x60, 0x02, 0x10, 0x07, 0x01, 0xF4, 0x60, 0x03, 0x10, 0x12, 0x03, 0xE8, 0x60, 0x04,
0x10, 0x13, 0x00, 0x00, 0x60, 0x05, 0x10, 0x14, 0x00, 0x3C, 0x60, 0x06, 0x10, 0x15, 0x00, 0x00, 0x60, 0x07, 0x10, 0x16, 0x01, 0xF4, 0x60, 0x00,
0x10, 0x17, 0x03, 0xE8, 0x60, 0x01, 0x10, 0x10, 0x00, 0x00, 0x60, 0x00, 0x10, 0x11, 0x00, 0x3C, 0x60, 0x01, 0x10, 0x10, 0x70, 0x00, 0xF0, 0x00,
0x10, 0x11, 0x70, 0x00, 0xA1, 0xF8, 0xF0, 0x00, 0x70, 0x00, 0x80, 0x1B, 0x2F, 0xA0, 0x17, 0x8D, 0x60, 0x09, 0x20, 0x21, 0x1D, 0x13, 0x60, 0x06,
0xD2, 0x02, 0x00, 0x13, 0x4F, 0xF0, 0xC4, 0x8A, 0x70, 0x00, 0xF0, 0x00, 0x90, 0xC3, 0x40, 0x05, 0xF0, 0x00, 0x82, 0xD3, 0x00, 0x13, 0x90, 0x13,
0x90, 0x08, 0x70, 0x00, 0x97, 0xFD, 0xF0, 0x00, 0x2E, 0x24, 0x40, 0x15, 0x32, 0x20, 0x60, 0x9C, 0x60, 0x07, 0x32, 0xA5, 0x0C, 0xFA, 0xD2, 0x80,
0x90, 0x03, 0x0C, 0xA8, 0xD5, 0x80, 0xD0, 0x98, 0x17, 0x91, 0x60, 0x06, 0x80, 0x20, 0x0C, 0xFA, 0xD2, 0x80, 0x90, 0xC0, 0x0C, 0xCE, 0xD1, 0x80,
0xF0, 0x00, 0x0D, 0x92, 0x60, 0x09, 0xD0, 0x42, 0x17, 0x92, 0x60, 0x06, 0x20, 0x21, 0x00, 0x7F, 0x60, 0x03, 0x81, 0x96, 0x70, 0x00, 0xF0, 0x00,
0x82, 0x5B, 0x30, 0x16, 0x77, 0x0C, 0x82, 0xDB, 0x0C, 0x7F, 0xDD, 0x80, 0xC0, 0x51, 0x20, 0x19, 0xA1, 0xE2, 0x20, 0x12, 0x70, 0x00, 0xF0, 0x00,
0x20, 0x98, 0x70, 0x00, 0xF0, 0x00, 0x20, 0x99, 0x0D, 0x23, 0xD2, 0x80, 0xF0, 0x00, 0x0C, 0x7F, 0xD0, 0x80, 0x21, 0xA0, 0x0E, 0xD3, 0x60, 0x01,
0x32, 0xA5, 0x0C, 0x70, 0xD0, 0x80, 0xF0, 0x00, 0x05, 0x09, 0x60, 0x00, 0x07, 0xE9, 0x60, 0x08, 0xA1, 0xDB, 0xF0, 0x00, 0x00, 0x63, 0x60, 0x00,
0x07, 0xE8, 0x60, 0x08, 0x81, 0xD9, 0xF0, 0x00, 0x0D, 0x9A, 0x60, 0x0C, 0xF0, 0x00, 0x08, 0x2E, 0x60, 0x0F, 0xF0, 0x00, 0x20, 0x48, 0x40, 0x17,
0xF0, 0x00, 0x20, 0xC9, 0x58, 0x06, 0xF0, 0x00, 0x21, 0xCA, 0xF0, 0x00, 0xF0, 0x00, 0x22, 0x4B, 0xF0, 0x00, 0xF0, 0x00, 0x20, 0x11, 0xF0, 0x00,
0xF0, 0x00, 0x20, 0x00, 0xF0, 0x00, 0xA1, 0xC8, 0x21, 0x42, 0xF0, 0x00, 0xA1, 0x82, 0x20, 0x33, 0xF0, 0x00, 0xAE, 0x88, 0x20, 0x22, 0xF0, 0x00,
0xA1, 0xD8, 0x22, 0xC4, 0xF0, 0x00, 0xA1, 0x92, 0x60, 0xCD, 0x60, 0x07, 0xAF, 0x0B, 0x33, 0x40, 0xF0, 0x00, 0xF0, 0x00, 0x33, 0xC3, 0xF0, 0x00,
0xF0, 0x00, 0x30, 0x77, 0xD0, 0x08, 0x00, 0x6E, 0x60, 0x0C, 0xA1, 0xC7, 0xF0, 0x00, 0x21, 0x47, 0xA1, 0xC6, 0xC3, 0xC7, 0x70, 0x00, 0xD0, 0x08,
0xF0, 0x00, 0x14, 0xD7, 0xD0, 0x80, 0x44, 0xD0, 0x1D, 0x1F, 0x60, 0x08, 0xF0, 0x00, 0x45, 0xF2, 0x45, 0x51, 0x30, 0x00, 0x46, 0x93, 0xF0, 0x00,
0x30, 0x81, 0x47, 0xF4, 0xF0, 0x00, 0x31, 0x02, 0x05, 0x69, 0x60, 0x01, 0x31, 0x83, 0x70, 0x00, 0xF0, 0x00, 0x32, 0x04, 0x70, 0x00, 0xF0, 0x00,
0x32, 0x81, 0x70, 0x00, 0x80, 0x5D, 0xF0, 0x00, 0x4F, 0xA0, 0xA0, 0x16, 0xF0, 0x00, 0x4F, 0xB0, 0xA0, 0x15, 0xF0, 0x00, 0x4F, 0xC0, 0xA0, 0x14,
0xF0, 0x00, 0x4F, 0xD0, 0xA0, 0x13, 0xF0, 0x00, 0x4F, 0xE0, 0xA0, 0x12, 0xF0, 0x00, 0x4F, 0xF0, 0xA0, 0x11, 0xF0, 0x00, 0x40, 0x20, 0xA0, 0x10,
0xF0, 0x00, 0x40, 0x40, 0xA0, 0x0F, 0xF0, 0x00, 0x40, 0x50, 0xA0, 0x0E, 0xF0, 0x00, 0x40, 0x60, 0xA0, 0x0D, 0x40, 0x00, 0x15, 0x35, 0xD2, 0x80,
0x40, 0x10, 0x15, 0x35, 0xD2, 0x80, 0x40, 0x30, 0x15, 0x35, 0xD2, 0x80, 0xF0, 0x00, 0x15, 0x2D, 0xD0, 0x80, 0x9A, 0xD9, 0x15, 0x35, 0x60, 0x01,
0x60, 0xFB, 0x60, 0x01, 0xEE, 0x00, 0x9A, 0xDB, 0x70, 0x00, 0xE1, 0xC0, 0x80, 0x59, 0x70, 0x00, 0x81, 0xAA, 0x1D, 0x1F, 0x60, 0x09, 0x80, 0x05,
0x1D, 0x20, 0x60, 0x09, 0x80, 0x04, 0x1D, 0x21, 0x60, 0x09, 0x80, 0x03, 0x1D, 0x22, 0x60, 0x09, 0x80, 0x02, 0x1D, 0x23, 0x60, 0x09, 0x80, 0x01,
0x1D, 0x24, 0x60, 0x09, 0x80, 0x00, 0xF0, 0x00, 0x15, 0x4A, 0xD0, 0x80, 0x9F, 0xB9, 0x41, 0xA0, 0x40, 0xF3, 0x9F, 0xB1, 0x41, 0xA1, 0x90, 0x04,
0x00, 0x00, 0x60, 0x02, 0x90, 0x18, 0xF0, 0x00, 0x32, 0x20, 0xF0, 0x00, 0xF0, 0x00, 0x32, 0xA1, 0xF0, 0x00, 0xF0, 0x00, 0x34, 0x22, 0x80, 0x17,
0xD6, 0x21, 0x70, 0x00, 0xF0, 0x00, 0xC2, 0x59, 0x10, 0x00, 0x60, 0x03, 0x9E, 0x72, 0x41, 0xA0, 0xF0, 0x00, 0x9E, 0x6A, 0x40, 0xE0, 0xE2, 0x00,
0x9C, 0x6A, 0x41, 0x20, 0xE2, 0x00, 0xF0, 0x00, 0x70, 0x00, 0xF0, 0x00, 0x92, 0x50, 0x70, 0x00, 0xE1, 0xC0, 0xC3, 0x5B, 0x70, 0x00, 0xF0, 0x00,
0xD5, 0x22, 0x70, 0x00, 0x90, 0x06, 0x9E, 0xB3, 0x41, 0xA1, 0xF0, 0x00, 0x9E, 0xAB, 0x40, 0xE1, 0xE2, 0x00, 0x9C, 0xAB, 0x41, 0x21, 0xE2, 0x00,
0xF0, 0x00, 0x70, 0x00, 0xF0, 0x00, 0x92, 0x91, 0x70, 0x00, 0xE1, 0xC0, 0x00, 0x01, 0x60, 0x02, 0x8F, 0xEE, 0x9E, 0xB3, 0x41, 0xA1, 0xF0, 0x00,
0x9E, 0xAB, 0x41, 0x11, 0xE2, 0x00, 0x9C, 0xAB, 0x41, 0x21, 0xE2, 0x00, 0xF0, 0x00, 0x70, 0x00, 0xF0, 0x00, 0x92, 0x91, 0x70, 0x00, 0xE1, 0xC0,
0x00, 0x0B, 0x60, 0x02, 0x8F, 0xE8, 0xF0, 0x00, 0x40, 0xD1, 0x40, 0xD0, 0x00, 0x1F, 0x60, 0x02, 0x8F, 0xE6, 0x9F, 0xF9, 0x41, 0xA0, 0x40, 0xF3,
0xF0, 0x00, 0x41, 0xA1, 0x90, 0x01, 0x00, 0x06, 0x60, 0x02, 0x80, 0x0B, 0xC3, 0x19, 0x10, 0x00, 0x60, 0x03, 0x9E, 0x72, 0x41, 0xA0, 0xF0, 0x00,
0x9E, 0x6A, 0x40, 0xE0, 0xE2, 0x00, 0x9E, 0x62, 0x41, 0x20, 0xE2, 0x00, 0x9C, 0x6A, 0x41, 0x00, 0xE2, 0x00, 0xF0, 0x00, 0x70, 0x00, 0xF0, 0x00,
0x92, 0x50, 0x70, 0x00, 0xE1, 0xC0, 0xC3, 0x5B, 0x70, 0x00, 0xF0, 0x00, 0x41, 0xA1, 0x00, 0x17, 0x60, 0x02, 0x00, 0x37, 0x60, 0x02, 0xE6, 0x00,
0x90, 0x01, 0x70, 0x00, 0xE0, 0xC0, 0x44, 0x03, 0x20, 0xA7, 0xA1, 0x77, 0x91, 0xC4, 0x70, 0x00, 0xF0, 0x00, 0x41, 0xA1, 0x41, 0xA0, 0xE2, 0x40,
0xF0, 0x00, 0x15, 0x93, 0xD0, 0x80, 0xF0, 0x00, 0x70, 0x00, 0x98, 0x05, 0x9E, 0x6B, 0x70, 0x00, 0x90, 0x05, 0x9C, 0x43, 0x70, 0x00, 0x90, 0x05,
0x9C, 0x6B, 0x70, 0x00, 0x9C, 0x02, 0xF0, 0x00, 0x70, 0x00, 0x9C, 0x04, 0xF0, 0x00, 0x70, 0x00, 0x80, 0x00, 0x41, 0xA3, 0x70, 0x00, 0x80, 0x03,
0x40, 0x83, 0x70, 0x00, 0x80, 0x02, 0x41, 0x53, 0x70, 0x00, 0x80, 0x01, 0x94, 0xEB, 0x70, 0x00, 0x80, 0x00, 0x4F, 0xE0, 0x15, 0x32, 0xD0, 0x80,
0xF0, 0x00, 0x4F, 0xF0, 0xA0, 0x02, 0xD7, 0x12, 0x4F, 0xB0, 0xA0, 0x01, 0xF0, 0x00, 0x15, 0xC5, 0xD0, 0x80, 0xF0, 0x00, 0x40, 0xF3, 0xA1, 0x65,
0xC2, 0x9B, 0x70, 0x00, 0xF0, 0x00, 0x9C, 0xEB, 0x70, 0x00, 0xF0, 0x00, 0x82, 0xDB, 0x70, 0x00, 0xE1, 0x40, 0x94, 0xEB, 0x15, 0x32, 0xD0, 0x80,
0x4F, 0xB0, 0x70, 0x00, 0xAF, 0xBB, 0x4F, 0xF0, 0x70, 0x00, 0x8F, 0xBA, 0xF0, 0x00, 0x08, 0xCC, 0x60, 0x09, 0xF0, 0x00, 0x61, 0x4C, 0x60, 0x01,
0xF0, 0x00, 0x61, 0x4F, 0x60, 0x02, 0x35, 0x91, 0x70, 0x00, 0xF0, 0x00, 0x36, 0x12, 0x08, 0x7F, 0x60, 0x00, 0x08, 0xC3, 0x60, 0x08, 0x81, 0x5A,
0xF0, 0x00, 0x16, 0x95, 0xD2, 0x80, 0x05, 0x44, 0x60, 0x03, 0xF0, 0x00, 0x0E, 0x21, 0x60, 0x09, 0x8F, 0x91, 0xF0, 0x00, 0x16, 0x9B, 0xD2, 0x80,
0xF0, 0x00, 0x24, 0xB3, 0xF0, 0x00, 0x0E, 0x21, 0x60, 0x09, 0x8F, 0x8E, 0x00, 0x26, 0x60, 0x00, 0xA0, 0x05, 0x80, 0x12, 0x60, 0x03, 0xF0, 0x00,
0x0E, 0x20, 0x60, 0x09, 0x8F, 0x8B, 0x00, 0x30, 0x60, 0x00, 0xA0, 0x02, 0x80, 0x03, 0x60, 0x03, 0xF0, 0x00, 0x0E, 0x20, 0x60, 0x09, 0x8F, 0x88,
0xF0, 0x00, 0x16, 0xCB, 0xD0, 0x80, 0x0A, 0x15, 0x60, 0x08, 0xF0, 0x00, 0xF0, 0x00, 0x21, 0x5B, 0xD2, 0x80, 0xF0, 0x00, 0x1B, 0xCD, 0x60, 0x08,
0x22, 0x10, 0x70, 0x00, 0xF0, 0x00, 0x2C, 0x87, 0x70, 0x00, 0xF0, 0x00, 0x22, 0x91, 0x7F, 0xFF, 0x60, 0x06, 0x91, 0xC7, 0x2D, 0x0C, 0xF0, 0x00,
0x2E, 0x0A, 0x17, 0xCE, 0xD5, 0x80, 0xF0, 0x00, 0x33, 0x06, 0xF0, 0x00, 0xF0, 0x00, 0x33, 0x86, 0xF0, 0x00, 0xF0, 0x00, 0x34, 0x06, 0xF0, 0x00,
0x34, 0x86, 0x17, 0xCE, 0xD0, 0x80, 0xF0, 0x00, 0x32, 0x94, 0xF0, 0x00, 0x91, 0xC7, 0x70, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x17, 0xF7, 0xD5, 0x80,
0xF0, 0x00, 0x70, 0x00, 0xD0, 0x08, 0x9E, 0xB7, 0x39, 0x12, 0xE2, 0x00, 0x83, 0x8F, 0x39, 0x12, 0xE2, 0x00, 0xF0, 0x00, 0x18, 0x8F, 0xD0, 0x80,
0x82, 0x00, 0x03, 0xE0, 0x60, 0x08, 0x0C, 0x1D, 0x60, 0x09, 0xC0, 0x10, 0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00, 0x33, 0x10, 0x1A, 0xA2, 0xD0, 0x80,
0x82, 0x00, 0x70, 0x00, 0xF0, 0x00, 0x09, 0x5D, 0x60, 0x08, 0xA1, 0x33, 0xF0, 0x00, 0x34, 0x80, 0xF0, 0x00, 0xF0, 0x00, 0x34, 0x00, 0xD0, 0x08,
0x82, 0x00, 0x04, 0xA8, 0x60, 0x0A, 0xF0, 0x00, 0x06, 0x98, 0x60, 0x01, 0x09, 0xE2, 0x60, 0x08, 0xC0, 0x08, 0xF0, 0x00, 0x10, 0x21, 0xF0, 0x00,
0xF0, 0x00, 0x31, 0x80, 0xD0, 0x08, 0xF0, 0x00, 0x18, 0x93, 0xD2, 0x80, 0x08, 0x46, 0x60, 0x08, 0xA1, 0x2A, 0xF0, 0x00, 0x29, 0x00, 0xA1, 0x29,
0x9E, 0x30, 0x70, 0x00, 0xF0, 0x00, 0x09, 0x97, 0x60, 0x08, 0xD4, 0x09, 0x09, 0x6E, 0x60, 0x00, 0xA1, 0x26, 0xF0, 0x00, 0x30, 0x80, 0xD0, 0x08,
0x0A, 0x66, 0x60, 0x08, 0xF0, 0x00, 0x0A, 0x87, 0x60, 0x09, 0xA0, 0x0D, 0xF0, 0x00, 0x0F, 0x84, 0xD2, 0x80, 0x83, 0xFF, 0x1D, 0x3E, 0xD0, 0x80,
0x0A, 0x6D, 0x60, 0x08, 0xF0, 0x00, 0x0A, 0x87, 0x60, 0x09, 0xA0, 0x09, 0xF0, 0x00, 0x0F, 0x84, 0xD2, 0x80, 0x31, 0x90, 0x03, 0xE8, 0x60, 0x00,
0x32, 0x11, 0x0F, 0xBE, 0xD2, 0x80, 0x90, 0x07, 0x70, 0x00, 0xF0, 0x00, 0x80, 0x98, 0x0F, 0xBE, 0xD2, 0x80, 0x8B, 0xC7, 0x70, 0x00, 0xF0, 0x00,
0x90, 0x04, 0x31, 0x87, 0x40, 0x07, 0x90, 0x80, 0x0F, 0xBE, 0xD2, 0x80, 0x31, 0x00, 0x1D, 0x67, 0xD0, 0x80, 0x58, 0x07, 0x22, 0x25, 0xF0, 0x00,
0x91, 0x04, 0x03, 0x87, 0x60, 0x00, 0x7F, 0xFC, 0x60, 0x06, 0xD0, 0x09, 0x82, 0x2A, 0x0F, 0x9C, 0xD2, 0x80, 0x80, 0xB8, 0x27, 0x14, 0xA1, 0x11,
0x81, 0x39, 0x0F, 0xE7, 0xD2, 0x80, 0xA2, 0x76, 0x20, 0x21, 0xF0, 0x00, 0xF0, 0x00, 0x20, 0xA2, 0xF0, 0x00, 0xF0, 0x00, 0x21, 0x23, 0xD0, 0x08,
0xF7, 0xCF, 0x60, 0x03, 0xF0, 0x00, 0x28, 0x00, 0x60, 0x00, 0xD0, 0x08, 0xF0, 0x00, 0x70, 0x00, 0xC0, 0x08, 0xF0, 0x00, 0x10, 0x32, 0xF0, 0x00,
0xF0, 0x00, 0x70, 0x00, 0xD0, 0x08, 0x23, 0x3E, 0x60, 0x05, 0xA1, 0x07, 0x8E, 0x2D, 0x23, 0x43, 0x60, 0x06, 0x91, 0xC1, 0x22, 0xF1, 0xD5, 0x80,
0x8F, 0x86, 0x3E, 0x6B, 0x60, 0x03, 0x91, 0xC1, 0x22, 0xF1, 0xD5, 0x80, 0x80, 0x18, 0x70, 0x00, 0xF0, 0x00, 0x91, 0xC1, 0x22, 0xF1, 0xD0, 0x80,
0x0C, 0x78, 0x60, 0x08, 0xA0, 0x01, 0x40, 0x11, 0x40, 0x94, 0x80, 0xFF, 0x90, 0x00, 0xFF, 0xEF, 0x60, 0x06, 0xF0, 0x00, 0x23, 0xB8, 0xD1, 0x80,
0x41, 0x06, 0x23, 0xB6, 0xD0, 0x80, 0x00, 0x02, 0x60, 0x03, 0x80, 0x05, 0x00, 0x04, 0x60, 0x03, 0x80, 0x04, 0x00, 0x06, 0x60, 0x03, 0x80, 0x03,
0xF0, 0x00, 0x23, 0x18, 0xD0, 0x80, 0xF0, 0x00, 0x23, 0x18, 0xD0, 0x80, 0x00, 0x10, 0x60, 0x03, 0x80, 0x00, 0xF0, 0x00, 0x20, 0x84, 0xA0, 0xF5,
0xC2, 0xE3, 0x70, 0x00, 0xF0, 0x00, 0x90, 0x41, 0x23, 0x8C, 0xD1, 0x80, 0xF0, 0x00, 0x23, 0x77, 0xD0, 0x80, 0x40, 0xC0, 0x0C, 0x95, 0x60, 0x08,
0x40, 0x81, 0x0C, 0x92, 0x60, 0x09, 0xF0, 0x00, 0x30, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x30, 0x11, 0xD0, 0x08, 0x20, 0x02, 0x58, 0x01, 0x80, 0xED,
0xF0, 0x00, 0x1C, 0x8D, 0x60, 0x09, 0x61, 0xBD, 0x60, 0x01, 0xA0, 0xEB, 0x31, 0x11, 0x1C, 0x91, 0x60, 0x0A, 0x01, 0x8E, 0x60, 0x02, 0xA0, 0xE9,
0x30, 0xA2, 0x70, 0x00, 0xD0, 0x08, 0x40, 0x00, 0x23, 0x0D, 0xD0, 0x80, 0xF0, 0x00, 0x1C, 0xAF, 0x60, 0x0E, 0x62, 0x2C, 0x60, 0x06, 0xA0, 0xE5,
0x30, 0x66, 0x1C, 0xA2, 0x60, 0x0F, 0x62, 0x2A, 0x60, 0x07, 0xA0, 0xE3, 0x30, 0x77, 0x1C, 0xD8, 0x60, 0x08, 0xF0, 0x00, 0x61, 0xDB, 0x60, 0x00,
0xF0, 0x00, 0x61, 0xD9, 0x60, 0x01, 0x31, 0x80, 0x1C, 0xBA, 0x60, 0x09, 0x30, 0x81, 0x62, 0x0F, 0x60, 0x02, 0xF0, 0x00, 0x62, 0x0A, 0x60, 0x03,
0x33, 0x12, 0x62, 0x26, 0x60, 0x04, 0x32, 0x93, 0x1C, 0x94, 0x60, 0x0A, 0x35, 0x94, 0x61, 0xEA, 0x60, 0x05, 0xF0, 0x00, 0x61, 0xE8, 0x60, 0x06,
0x30, 0xA5, 0x61, 0xF0, 0x60, 0x07, 0x30, 0x26, 0x1C, 0xEA, 0x60, 0x0B, 0x34, 0xA7, 0x61, 0xE1, 0x60, 0x00, 0xF0, 0x00, 0x61, 0xE4, 0x60, 0x01,
0x31, 0xB0, 0x61, 0xE6, 0x60, 0x02, 0x32, 0x31, 0x1C, 0xD0, 0x60, 0x0C, 0x32, 0xB2, 0x61, 0xF2, 0x60, 0x03, 0xF0, 0x00, 0x61, 0xF9, 0x60, 0x04,
0x30, 0xC3, 0x61, 0xFD, 0x60, 0x05, 0x31, 0xC4, 0x61, 0xFB, 0x60, 0x06, 0x32, 0xC5, 0x70, 0x00, 0xF0, 0x00, 0x32, 0x46, 0x70, 0x00, 0x8F, 0xE0,
0xF0, 0x00, 0x28, 0x70, 0xD0, 0x80, 0x82, 0x00, 0x70, 0x00, 0xAE, 0x7E, 0xF0, 0x00, 0x29, 0x46, 0xD0, 0x80, 0xF0, 0x00, 0x31, 0xE4, 0xD2, 0x80,
0xF0, 0x00, 0x70, 0x00, 0xA0, 0x02, 0xF0, 0x00, 0x70, 0x00, 0xA0, 0xAA, 0xF0, 0x00, 0x29, 0x61, 0xD0, 0x80, 0xF0, 0x00, 0x70, 0x00, 0x92, 0x79,
0xF0, 0x00, 0x70, 0x00, 0x8E, 0x7A, 0xF0, 0x00, 0x70, 0x00, 0xAE, 0x64, 0xF0, 0x00, 0x23, 0xBD, 0xD2, 0x80, 0xF0, 0x00, 0x29, 0xB4, 0xD0, 0x80,
0xF0, 0x00, 0x23, 0xD7, 0xD2, 0x80, 0xF0, 0x00, 0x29, 0xBA, 0xD0, 0x80, 0xF0, 0x00, 0x23, 0xD5, 0xD2, 0x80, 0xF0, 0x00, 0x29, 0xBE, 0xD0, 0x80,
0xF0, 0x00, 0x70, 0x00, 0xAF, 0x90, 0xF0, 0x00, 0x29, 0xE7, 0xD0, 0x80, 0x40, 0x12, 0x28, 0x76, 0xD2, 0x80, 0xF0, 0x00, 0x29, 0xF4, 0xD1, 0x80,
0x00, 0x40, 0x60, 0x00, 0xAE, 0xF0, 0x00, 0x40, 0x60, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x2C, 0x7A, 0xD7, 0x80, 0xF0, 0x00, 0x11, 0x7C, 0xD0, 0x80,
0xF0, 0x00, 0x70, 0x00, 0xAF, 0x7F, 0xF0, 0x00, 0x2A, 0x35, 0xD0, 0x80, 0x0C, 0xB2, 0x60, 0x0D, 0xF0, 0x00, 0xF0, 0x00, 0x28, 0x6A, 0xD2, 0x80,
0xF0, 0x00, 0x19, 0x05, 0xD2, 0x80, 0xF0, 0x00, 0x0B, 0xE9, 0xD2, 0x80, 0xF0, 0x00, 0x0B, 0xF7, 0xD2, 0x80, 0xF0, 0x00, 0x01, 0xF1, 0xD2, 0x80,
0xF0, 0x00, 0x2A, 0x7B, 0xD0, 0x80, 0xF0, 0x00, 0x23, 0xBD, 0xD2, 0x80, 0xF0, 0x00, 0x2A, 0x8A, 0xD0, 0x80, 0xF0, 0x00, 0x18, 0x7C, 0xD2, 0x80,
0xF0, 0x00, 0x2A, 0x91, 0xD0, 0x80, 0xF0, 0x00, 0x0B, 0xF8, 0xD2, 0x80, 0xF0, 0x00, 0x0B, 0xEC, 0xD2, 0x80, 0xF0, 0x00, 0x01, 0xF5, 0xD2, 0x80,
0xF0, 0x00, 0x2A, 0x9B, 0xD0, 0x80, 0x62, 0x03, 0x60, 0x00, 0xF0, 0x00, 0x40, 0x21, 0x28, 0x89, 0xD0, 0x80, 0xF0, 0x00, 0x28, 0x92, 0xD2, 0x80,
0xF0, 0x00, 0x70, 0x00, 0xA0, 0x32, 0x62, 0x07, 0x60, 0x00, 0xF0, 0x00, 0x40, 0x21, 0x28, 0x89, 0xD0, 0x80, 0xF0, 0x00, 0x70, 0x00, 0xA0, 0x9B,
0xF0, 0x00, 0x11, 0x7C, 0xD2, 0x80, 0x2B, 0x03, 0x60, 0x00, 0x8F, 0xCE, 0xF0, 0x00, 0x2B, 0x0D, 0xD2, 0x80, 0x62, 0x0D, 0x60, 0x00, 0xF0, 0x00,
0x40, 0x51, 0x28, 0x89, 0xD0, 0x80, 0xF0, 0x00, 0x70, 0x00, 0xAF, 0x66, 0xF0, 0x00, 0x2B, 0x14, 0xD0, 0x80, 0xF0, 0x00, 0x70, 0x00, 0xAE, 0x89,
0xF0, 0x00, 0x2B, 0x19, 0xD0, 0x80, 0x62, 0x13, 0x60, 0x00, 0xF0, 0x00, 0x40, 0x21, 0x28, 0x89, 0xD0, 0x80, 0xF0, 0x00, 0x28, 0x90, 0xD2, 0x80,
0xF0, 0x00, 0x70, 0x00, 0xA0, 0x21, 0x62, 0x17, 0x60, 0x00, 0xF0, 0x00, 0x40, 0x21, 0x28, 0x89, 0xD0, 0x80, 0xF0, 0x00, 0x70, 0x00, 0xA0, 0x8B,
0xF0, 0x00, 0x11, 0x7C, 0xD2, 0x80, 0x2B, 0x21, 0x60, 0x00, 0x8F, 0xBE, 0xF0, 0x00, 0x19, 0x1F, 0xD2, 0x80, 0x40, 0x01, 0x26, 0xEE, 0xD2, 0x80,
0x40, 0x11, 0x26, 0xF0, 0xD2, 0x80, 0x40, 0x11, 0x26, 0xF2, 0xD2, 0x80, 0xF0, 0x00, 0x0B, 0xF1, 0xD2, 0x80, 0xF0, 0x00, 0x0B, 0xEC, 0xD2, 0x80,
0xF0, 0x00, 0x0C, 0x00, 0xD2, 0x80, 0x00, 0x80, 0x60, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x2C, 0x7B, 0xD2, 0x80, 0x40, 0x11, 0x0B, 0x90, 0xD7, 0x80,
0x2B, 0x4F, 0x60, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x2B, 0x4E, 0xD0, 0x80, 0x40, 0x21, 0x0B, 0x90, 0xD2, 0x80, 0xF0, 0x00, 0x1B, 0x32, 0xD2, 0x80,
0xF0, 0x00, 0x19, 0x21, 0xD2, 0x80, 0xF0, 0x00, 0x2B, 0x58, 0xD0, 0x80, 0xF0, 0x00, 0x70, 0x00, 0xAF, 0x4E, 0xF0, 0x00, 0x2B, 0x7A, 0xD0, 0x80,
0xF0, 0x00, 0x70, 0x00, 0xAF, 0x4C, 0xF0, 0x00, 0x2B, 0xE7, 0xD0, 0x80, 0xF0, 0x00, 0x28, 0x73, 0xD2, 0x80, 0x9C, 0x2F, 0x40, 0x01, 0xF0, 0x00,
0x9C, 0x0F, 0x0A, 0xA7, 0xD1, 0x80, 0x9A, 0x1F, 0x0A, 0xA7, 0xD1, 0x80, 0xF0, 0x00, 0x0A, 0xA7, 0xD1, 0x80, 0xF0, 0x00, 0x0A, 0x91, 0xD0, 0x80,
0x0C, 0xC4, 0x60, 0x08, 0xF0, 0x00, 0xF0, 0x00, 0x2C, 0x8A, 0xD0, 0x80, 0x40, 0x02, 0x40, 0x01, 0x80, 0x2B, 0x40, 0x12, 0x40, 0x01, 0x80, 0x2A,
0xF0, 0x00, 0x27, 0x2E, 0xD2, 0x80, 0x9E, 0x38, 0x70, 0x00, 0xF0, 0x00, 0x41, 0x31, 0x70, 0x00, 0x94, 0x04, 0xF0, 0x00, 0x70, 0x00, 0xAE, 0x32,
0x41, 0x31, 0x70, 0x00, 0x90, 0x02, 0xF0, 0x00, 0x70, 0x00, 0xAE, 0x3E, 0x43, 0xE1, 0x70, 0x00, 0x80, 0x00, 0xF0, 0x00, 0x62, 0x42, 0x60, 0x02,
0x40, 0x10, 0x2D, 0x5D, 0xD2, 0x80, 0xF0, 0x00, 0x2D, 0xB2, 0xD0, 0x80, 0xF0, 0x00, 0x26, 0x41, 0xD2, 0x80, 0xF0, 0x00, 0x2F, 0xC2, 0xD0, 0x80,
0xF0, 0x00, 0x70, 0x00, 0xAE, 0x35, 0xF0, 0x00, 0x40, 0x10, 0xF0, 0x00, 0xF0, 0x00, 0x32, 0x14, 0xD0, 0x80, 0xF0, 0x00, 0x0D, 0x15, 0x60, 0x08,
0xF0, 0x00, 0x17, 0x98, 0x60, 0x00, 0xF0, 0x00, 0x17, 0x98, 0x60, 0x01, 0x3F, 0x80, 0x0D, 0xAA, 0x60, 0x09, 0x30, 0x01, 0x56, 0xA5, 0x60, 0x02,
0xF0, 0x00, 0x5E, 0x86, 0x60, 0x03, 0x10, 0x12, 0x59, 0x89, 0x60, 0x04, 0x10, 0x13, 0x61, 0xAD, 0x60, 0x05, 0x10, 0x14, 0x5C, 0x6C, 0x60, 0x06,
0x10, 0x15, 0x64, 0xD3, 0x60, 0x07, 0x10, 0x16, 0x55, 0x2D, 0x60, 0x00, 0x10, 0x17, 0x5C, 0xEB, 0x60, 0x01, 0x10, 0x10, 0x58, 0x04, 0x60, 0x02,
0x10, 0x11, 0x60, 0x04, 0x60, 0x03, 0x10, 0x12, 0x5A, 0xDB, 0x60, 0x04, 0x10, 0x13, 0x63, 0x1D, 0x60, 0x05, 0x10, 0x14, 0x0D, 0xA2, 0x60, 0x0A,
0x10, 0x15, 0x02, 0xE1, 0x60, 0x06, 0xF0, 0x00, 0x43, 0x14, 0x60, 0x07, 0x10, 0x26, 0x04, 0x5B, 0x60, 0x00, 0x10, 0x27, 0x44, 0x95, 0x60, 0x01,
0x10, 0x20, 0x05, 0xD5, 0x60, 0x02, 0x10, 0x21, 0x46, 0x16, 0x60, 0x03, 0x10, 0x22, 0x06, 0xAE, 0x60, 0x04, 0x10, 0x23, 0x00, 0x00, 0x60, 0x05,
0x10, 0x24, 0x70, 0x00, 0xF0, 0x00, 0x10, 0x25, 0x70, 0x00, 0xD0, 0x08, 0x40, 0x05, 0x0D, 0x4F, 0x60, 0x0C, 0xF0, 0x00, 0x02, 0x09, 0x60, 0x03,
0x35, 0xC2, 0x08, 0xFB, 0x60, 0x06, 0x8F, 0x4D, 0x31, 0x40, 0xF0, 0x00, 0x8E, 0xC4, 0x20, 0x47, 0x90, 0x06, 0x81, 0x65, 0x70, 0x00, 0xF0, 0x00,
0x8F, 0x84, 0x40, 0x12, 0xF0, 0x00, 0x81, 0x65, 0x08, 0xFB, 0x60, 0x03, 0x8E, 0xA9, 0x70, 0x00, 0xF0, 0x00, 0x8E, 0xC2, 0x70, 0x00, 0xF0, 0x00,
0xC2, 0x51, 0x70, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x36, 0x41, 0xF0, 0x00, 0xF0, 0x00, 0x30, 0x45, 0xF0, 0x00, 0xF0, 0x00, 0x30, 0xC7, 0x80, 0x30,
0x82, 0x0A, 0x00, 0x61, 0x90, 0x09, 0xF0, 0x00, 0x30, 0xE3, 0xD0, 0x80, 0x17, 0xA9, 0x60, 0x08, 0xA0, 0x32, 0xF0, 0x00, 0x00, 0x02, 0xA0, 0x31,
0x90, 0x82, 0x70, 0x00, 0xF0, 0x00, 0x82, 0x8A, 0x70, 0x00, 0x90, 0x03, 0x90, 0x8A, 0x70, 0x00, 0x90, 0x01, 0xF0, 0x00, 0x70, 0x00, 0x8F, 0xFB,
0x82, 0xBF, 0x70, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x01, 0xB7, 0xD0, 0x80, 0x17, 0xA1, 0x60, 0x0E, 0xA0, 0x2A, 0x4F, 0xF6, 0x00, 0x61, 0xA0, 0x29,
0x90, 0x41, 0x70, 0x00, 0xF0, 0x00, 0x82, 0x0A, 0x00, 0x61, 0x90, 0x04, 0x91, 0x8E, 0x70, 0x00, 0x97, 0xFD, 0x37, 0x46, 0x70, 0x00, 0xF0, 0x00,
0xD7, 0xB1, 0x17, 0xA7, 0x60, 0x00, 0xF0, 0x00, 0x30, 0xF3, 0xD0, 0x80, 0xF0, 0x00, 0x30, 0xE7, 0xD0, 0x80, 0x21, 0x40, 0x31, 0x91, 0xD0, 0x80,
0xF0, 0x00, 0x2C, 0x86, 0xD2, 0x80, 0xF0, 0x00, 0x70, 0x00, 0xD4, 0x09, 0xF0, 0x00, 0x27, 0x2E, 0xD2, 0x80, 0x90, 0x00, 0x70, 0x00, 0xD0, 0x08,
0x0D, 0x4F, 0x60, 0x0C, 0xA0, 0x1C, 0x26, 0x42, 0x41, 0xD7, 0xA0, 0x1B, 0x90, 0x82, 0x70, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x60, 0x41, 0xD1, 0x80,
0xF0, 0x00, 0x31, 0xFE, 0xD2, 0x80, 0xF0, 0x00, 0x70, 0x00, 0xF0, 0x00, 0x90, 0x00, 0x0D, 0xAA, 0x60, 0x08, 0x0D, 0xB0, 0x60, 0x08, 0xE6, 0x00,
0xF0, 0x00, 0x30, 0x6F, 0xD2, 0x80, 0x83, 0xFF, 0x00, 0xFF, 0x60, 0x04, 0x7F, 0xFF, 0x60, 0x02, 0xCA, 0x86, 0xF0, 0x00, 0x00, 0x01, 0xA0, 0x11,
0xA2, 0x09, 0x00, 0x80, 0x60, 0x03, 0xC2, 0x61, 0x0D, 0xB7, 0x60, 0x0B, 0x82, 0x59, 0x0D, 0xB6, 0x60, 0x0A, 0x89, 0x09, 0x70, 0x00, 0xF0, 0x00,
0x8E, 0x55, 0x70, 0x00, 0xF0, 0x00, 0xD1, 0xCB, 0x40, 0x15, 0x90, 0x02, 0x90, 0x42, 0x30, 0x33, 0xF0, 0x00, 0x91, 0xC6, 0x30, 0x27, 0xF0, 0x00,
0x91, 0xCF, 0x70, 0x00, 0xF0, 0x00, 0xC3, 0xAF, 0x01, 0xB7, 0xD2, 0x80, 0xD7, 0xF3, 0x70, 0x00, 0xF0, 0x00, 0x96, 0xEF, 0x70, 0x00, 0x8D, 0xA1,
0x40, 0x10, 0x2F, 0xB7, 0xD2, 0x80, 0xF0, 0x00, 0x2E, 0x60, 0xD2, 0x80, 0xF0, 0x00, 0x31, 0xBE, 0xD0, 0x80, 0x0D, 0x4F, 0x60, 0x08, 0xA0, 0x01,
0x2F, 0x00, 0x31, 0xF7, 0xD0, 0x80, 0xF0, 0x00, 0x70, 0x00, 0xD0, 0x08, 0xF0, 0x00, 0x33, 0x57, 0xD0, 0x80, 0xF0, 0x00, 0x33, 0x95, 0xD2, 0x80,
0xF0, 0x00, 0x62, 0xA9, 0xD0, 0x80, 0xF0, 0x00, 0x17, 0x8D, 0x60, 0x08, 0xF0, 0x00, 0x40, 0x87, 0x60, 0x00, 0xF0, 0x00, 0x20, 0x2B, 0x60, 0x00,
0x10, 0x00, 0x20, 0x2C, 0x60, 0x00, 0x10, 0x00, 0x00, 0x00, 0x60, 0x00, 0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00,
0xF0, 0x00, 0x17, 0x91, 0x60, 0x08, 0xF0, 0x00, 0x0E, 0xEE, 0x60, 0x00, 0xF0, 0x00, 0x70, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00,
0xF0, 0x00, 0x17, 0x92, 0x60, 0x08, 0xF0, 0x00, 0x00, 0x00, 0x60, 0x00, 0xF0, 0x00, 0x0D, 0x92, 0x60, 0x00, 0x10, 0x00, 0x00, 0x05, 0x60, 0x00,
0x10, 0x00, 0x1D, 0x15, 0x60, 0x00, 0x10, 0x00, 0x00, 0x05, 0x60, 0x00, 0x10, 0x00, 0x1D, 0x1A, 0x60, 0x00, 0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00,
0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x17, 0x98, 0x60, 0x08, 0xF0, 0x00, 0x1E, 0x28, 0x60, 0x00, 0xF0, 0x00, 0xC0, 0x52, 0x60, 0x00,
0x10, 0x00, 0x1E, 0x2D, 0x60, 0x00, 0x10, 0x00, 0xC0, 0x52, 0x60, 0x00, 0x10, 0x00, 0x29, 0x18, 0x60, 0x00, 0x10, 0x00, 0xC0, 0x3A, 0x60, 0x00,
0x10, 0x00, 0x29, 0x18, 0x60, 0x00, 0x10, 0x00, 0x80, 0x38, 0x60, 0x00, 0x10, 0x00, 0x00, 0x00, 0x60, 0x00, 0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00,
0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x17, 0xA1, 0x60, 0x08, 0xF0, 0x00, 0x02, 0xA8, 0x60, 0x00, 0xF0, 0x00, 0x03, 0x5C, 0x60, 0x00,
0x10, 0x00, 0x06, 0x99, 0x60, 0x00, 0x10, 0x00, 0x10, 0x3B, 0x60, 0x00, 0x10, 0x00, 0x15, 0x06, 0x60, 0x00, 0x10, 0x00, 0x00, 0x00, 0x60, 0x00,
0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x17, 0xA7, 0x60, 0x08, 0xF0, 0x00, 0x55, 0x04, 0x60, 0x00,
0xF0, 0x00, 0x00, 0x03, 0x60, 0x00, 0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x17, 0xA9, 0x60, 0x08,
0xF0, 0x00, 0x00, 0x00, 0x60, 0x00, 0xF0, 0x00, 0x70, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x10, 0x00, 0xF0, 0x00, 0xF0, 0x00, 0x70, 0x00, 0xD0, 0x08
};
const size_t PatchSize = sizeof(PatchByteValues);
const unsigned char *pPatchBytes = &PatchByteValues[0];
extern const size_t LutSize;
extern const unsigned char *pLutBytes;
static unsigned char LutByteValues[] =
{
0x80, 0x17, 0x80, 0x45, 0x80, 0x96, 0x81, 0x5B,
0x82, 0xD6, 0x88, 0x0B, 0x88, 0x10, 0x88, 0xDB,
0x89, 0x48, 0x8B, 0x0A, 0x8B, 0x12, 0x8B, 0x13,
0x8B, 0x21, 0x8B, 0x26, 0x8C, 0x6F, 0x94, 0xD0,
0x95, 0x20, 0x95, 0x33, 0x95, 0x6F, 0x95, 0xA8,
0x95, 0xAC, 0x95, 0xC3, 0x95, 0xE1, 0x97, 0xC9,
0x97, 0xF6, 0x98, 0x8D, 0x99, 0x00, 0x9A, 0x00,
0x9E, 0x73, 0xA0, 0x12, 0xA1, 0x03, 0xA2, 0xF0,
0xA3, 0x0D, 0xA3, 0x4C, 0xA3, 0x52, 0xA3, 0xED,
0xA8, 0x31, 0xAB, 0x01, 0xAB, 0x1F, 0xAB, 0x4C,
0xAC, 0x76, 0xAC, 0x88, 0xAD, 0xB0, 0xAD, 0xB7,
0xB0, 0xE2, 0xB1, 0x01, 0xB1, 0x0B, 0xB1, 0x35,
0xB1, 0x3B, 0xB1, 0x97, 0xB3, 0x03
};
const size_t LutSize = sizeof(LutByteValues);
const unsigned char *pLutBytes = &LutByteValues[0];


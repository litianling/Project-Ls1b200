/*
 * asc-simple-8x16.inl
 *
 *  Created on: 2014-12-14
 *      Author: Bian
 */

/******************************************************************************
 * ASC simple font data - 8x16 - 0x20:x7F
 ******************************************************************************/

const unsigned char asc_simple_8x16 [] =
{

/* Character   (0x20):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,

/* Character ! (0x21):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |   **   |
   |  ****  |
   |  ****  |
   |  ****  |
   |  ****  |
   |   **   |
   |   **   |
   |        |
   |   **   |
   |   **   |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x18,
0x3C,
0x3C,
0x3C,
0x3C,
0x18,
0x18,
0x00,
0x18,
0x18,
0x00,
0x00,
0x00,
0x00,

/* Character " (0x22):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |  ** ** |
   |  ** ** |
   |  ** ** |
   |  ** ** |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x36,
0x36,
0x36,
0x36,
0x14,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,

/* Character # (0x23):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | ** **  |
   | ** **  |
   | ** **  |
   |******* |
   | ** **  |
   | ** **  |
   |******* |
   | ** **  |
   | ** **  |
   | ** **  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x6C,
0x6C,
0x6C,
0xFE,
0x6C,
0x6C,
0xFE,
0x6C,
0x6C,
0x6C,
0x00,
0x00,
0x00,
0x00,

/* Character $ (0x24):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |   **   |
   |   **   |
   | *****  |
   |**   ** |
   |** *    |
   | ****   |
   |  ****  |
   |   * ** |
   |**   ** |
   | *****  |
   |   **   |
   |   **   |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x18,
0x18,
0x7C,
0xC6,
0xC0,
0x78,
0x3C,
0x06,
0xC6,
0x7C,
0x18,
0x18,
0x00,
0x00,

/* Character % (0x25):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   | **   * |
   | **  ** |
   |    **  |
   |   **   |
   |  **    |
   | **  ** |
   |**   ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x62,
0x66,
0x0C,
0x18,
0x30,
0x66,
0xC6,
0x00,
0x00,
0x00,
0x00,

/* Character & (0x26):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |  ***   |
   | ** **  |
   |  ***   |
   |  **    |
   | *** ** |
   | ****** |
   |**  **  |
   |**  **  |
   |**  **  |
   | *** ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x38,
0x6C,
0x38,
0x30,
0x76,
0x7E,
0xCC,
0xCC,
0xCC,
0x76,
0x00,
0x00,
0x00,
0x00,

/* Character ' (0x27):
   ht=16, width=8
   +--------+
   |        |
   |    **  |
   |    **  |
   |    **  |
   |   **   |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x0C,
0x0C,
0x0C,
0x18,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,

/* Character ( (0x28):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |    **  |
   |   **   |
   |  **    |
   |  **    |
   |  **    |
   |  **    |
   |  **    |
   |  **    |
   |   **   |
   |    **  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x0C,
0x18,
0x30,
0x30,
0x30,
0x30,
0x30,
0x30,
0x18,
0x0C,
0x00,
0x00,
0x00,
0x00,

/* Character ) (0x29):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |  **    |
   |   **   |
   |    **  |
   |    **  |
   |    **  |
   |    **  |
   |    **  |
   |    **  |
   |   **   |
   |  **    |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x30,
0x18,
0x0C,
0x0C,
0x0C,
0x0C,
0x0C,
0x0C,
0x18,
0x30,
0x00,
0x00,
0x00,
0x00,

/* Character * (0x2A):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   | ** **  |
   |  ***   |
   |******* |
   |  ***   |
   | ** **  |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x6C,
0x38,
0xFE,
0x38,
0x6C,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,

/* Character  (0x2B):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |   **   |
   |   **   |
   | ****** |
   |   **   |
   |   **   |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x18,
0x18,
0x7E,
0x18,
0x18,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,

/* Character , (0x2C):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |    **  |
   |    **  |
   |    **  |
   |   **   |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x0C,
0x0C,
0x0C,
0x18,
0x00,
0x00,
0x00,

/* Character - (0x2D):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   | ****** |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0xFE,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,

/* Character . (0x2E):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |   **   |
   |   **   |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x18,
0x18,
0x00,
0x00,
0x00,
0x00,

/* Character / (0x2F):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |      * |
   |     ** |
   |    **  |
   |   **   |
   |  **    |
   | **     |
   | *      |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x02,
0x06,
0x0C,
0x18,
0x30,
0x60,
0xC0,
0x80,
0x00,
0x00,
0x00,
0x00,

/* Character 0 (0x30):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |**   ** |
   |**  *** |
   |** **** |
   |**** ** |
   |***  ** |
   |**   ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0xC6,
0xCE,
0xDE,
0xF6,
0xE6,
0xC6,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character 1 (0x31):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |   **   |
   | ****   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   | ****** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x18,
0x78,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0x7E,
0x00,
0x00,
0x00,
0x00,

/* Character 2 (0x32):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |**   ** |
   |     ** |
   |    **  |
   |   **   |
   |  **    |
   | **     |
   |**   ** |
   |******* |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0xC6,
0x06,
0x0C,
0x18,
0x30,
0x60,
0xC6,
0xFE,
0x00,
0x00,
0x00,
0x00,

/* Character 3 (0x33):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |     ** |
   |     ** |
   |  ****  |
   |     ** |
   |     ** |
   |     ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0x06,
0x06,
0x3C,
0x06,
0x06,
0x06,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character 4 (0x34):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |    **  |
   |   ***  |
   |  ****  |
   | ** **  |
   |**  **  |
   |**  **  |
   |******* |
   |    **  |
   |    **  |
   |   **** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x0C,
0x1C,
0x3C,
0x6C,
0xCC,
0xCC,
0xFE,
0x0C,
0x0C,
0x1E,
0x00,
0x00,
0x00,
0x00,

/* Character 5 (0x35):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |******* |
   |**      |
   |**      |
   |**      |
   |******  |
   |     ** |
   |     ** |
   |     ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xFE,
0xC0,
0xC0,
0xC0,
0xFC,
0x06,
0x06,
0x06,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character 6 (0x36):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |**      |
   |**      |
   |******  |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0xC0,
0xC0,
0xFC,
0xC6,
0xC6,
0xC6,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character 7 (0x37):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |******* |
   |**   ** |
   |     ** |
   |    **  |
   |   **   |
   |  **    |
   |  **    |
   |  **    |
   |  **    |
   |  **    |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xFE,
0xC6,
0x06,
0x0C,
0x18,
0x30,
0x30,
0x30,
0x30,
0x30,
0x00,
0x00,
0x00,
0x00,

/* Character 8 (0x38):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |**   ** |
   |**   ** |
   | *****  |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0xC6,
0xC6,
0x7C,
0xC6,
0xC6,
0xC6,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character 9 (0x39):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   | ****** |
   |     ** |
   |     ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0xC6,
0xC6,
0xC6,
0x7E,
0x06,
0x06,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character : (0x3A):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |    **  |
   |    **  |
   |        |
   |        |
   |    **  |
   |    **  |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x0C,
0x0C,
0x00,
0x00,
0x0C,
0x0C,
0x00,
0x00,
0x00,
0x00,
0x00,

/* Character ; (0x3B):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |    **  |
   |    **  |
   |        |
   |        |
   |    **  |
   |    **  |
   |    **  |
   |   **   |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x0C,
0x0C,
0x00,
0x00,
0x0C,
0x0C,
0x0C,
0x18,
0x00,
0x00,
0x00,

/* Character < (0x3C):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |    **  |
   |   **   |
   |  **    |
   | **     |
   |**      |
   | **     |
   |  **    |
   |   **   |
   |    **  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x0C,
0x18,
0x30,
0x60,
0xC0,
0x60,
0x30,
0x18,
0x0C,
0x00,
0x00,
0x00,
0x00,

/* Character = (0x3D):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |******* |
   |        |
   |******* |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0xFE,
0x00,
0xFE,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,

/* Character > (0x3E):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   | **     |
   |  **    |
   |   **   |
   |    **  |
   |     ** |
   |    **  |
   |   **   |
   |  **    |
   | **     |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x60,
0x30,
0x18,
0x0C,
0x06,
0x0C,
0x18,
0x30,
0x60,
0x00,
0x00,
0x00,
0x00,

/* Character ? (0x3F):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |**   ** |
   |    **  |
   |   **   |
   |   **   |
   |   **   |
   |        |
   |   **   |
   |   **   |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0xC6,
0x0C,
0x18,
0x18,
0x18,
0x00,
0x18,
0x18,
0x00,
0x00,
0x00,
0x00,

/* Character @ (0x40):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |**   ** |
   |**   ** |
   |** **** |
   |** **** |
   |** **** |
   |** ***  |
   |**      |
   | ****** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0xC6,
0xC6,
0xDE,
0xDE,
0xDE,
0xDC,
0xC0,
0x7E,
0x00,
0x00,
0x00,
0x00,

/* Character A (0x41):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |  ***   |
   | ** **  |
   |**   ** |
   |**   ** |
   |**   ** |
   |******* |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x38,
0x6C,
0xC6,
0xC6,
0xC6,
0xFE,
0xC6,
0xC6,
0xC6,
0xC6,
0x00,
0x00,
0x00,
0x00,

/* Character B (0x42):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |******  |
   | **  ** |
   | **  ** |
   | **  ** |
   | *****  |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   |******  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xFC,
0x66,
0x66,
0x66,
0x7C,
0x66,
0x66,
0x66,
0x66,
0xFC,
0x00,
0x00,
0x00,
0x00,

/* Character C (0x43):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |  ****  |
   | **  ** |
   |**    * |
   |**      |
   |**      |
   |**      |
   |**      |
   |**    * |
   | **  ** |
   |  ****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x3C,
0x66,
0xC2,
0xC0,
0xC0,
0xC0,
0xC0,
0xC2,
0x66,
0x3C,
0x00,
0x00,
0x00,
0x00,

/* Character D (0x44):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |*****   |
   | ** **  |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   | ** **  |
   |*****   |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xF8,
0x6C,
0x66,
0x66,
0x66,
0x66,
0x66,
0x66,
0x6C,
0xF8,
0x00,
0x00,
0x00,
0x00,

/* Character E (0x45):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |******* |
   | **  ** |
   | **     |
   | **  *  |
   | *****  |
   | **  *  |
   | **     |
   | **     |
   | **  ** |
   |******* |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xFE,
0x66,
0x60,
0x64,
0x7C,
0x64,
0x60,
0x60,
0x66,
0xFE,
0x00,
0x00,
0x00,
0x00,

/* Character F (0x46):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |******* |
   | **  ** |
   | **     |
   | **  *  |
   | *****  |
   | **  *  |
   | **     |
   | **     |
   | **     |
   |****    |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xFE,
0x66,
0x60,
0x64,
0x7C,
0x64,
0x60,
0x60,
0x60,
0xF0,
0x00,
0x00,
0x00,
0x00,

/* Character G (0x47):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |**   ** |
   |**      |
   |**      |
   |**      |
   |**  *** |
   |**   ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0xC6,
0xC0,
0xC0,
0xC0,
0xCE,
0xC6,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character H (0x48):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |******* |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xC6,
0xC6,
0xC6,
0xC6,
0xFE,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0x00,
0x00,
0x00,
0x00,

/* Character I (0x49):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |  ****  |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |  ****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x3C,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0x3C,
0x00,
0x00,
0x00,
0x00,

/* Character J (0x4A):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |  ****  |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |** **   |
   |** **   |
   | ***    |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x3C,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0xD8,
0xD8,
0x70,
0x00,
0x00,
0x00,
0x00,

/* Character K (0x4B):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |**   ** |
   |**   ** |
   |**  **  |
   |** **   |
   |****    |
   |****    |
   |** **   |
   |**  **  |
   |**   ** |
   |**   ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xC6,
0xC6,
0xCC,
0xD8,
0xF0,
0xF0,
0xD8,
0xCC,
0xC6,
0xC6,
0x00,
0x00,
0x00,
0x00,

/* Character L (0x4C):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |****    |
   | **     |
   | **     |
   | **     |
   | **     |
   | **     |
   | **     |
   | **   * |
   | **  ** |
   |******* |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xF0,
0x60,
0x60,
0x60,
0x60,
0x60,
0x60,
0x62,
0x66,
0xFE,
0x00,
0x00,
0x00,
0x00,

/* Character M (0x4D):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |**   ** |
   |**   ** |
   |*** *** |
   |*** *** |
   |******* |
   |** * ** |
   |** * ** |
   |** * ** |
   |**   ** |
   |**   ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xC6,
0xC6,
0xEE,
0xEE,
0xFE,
0xD6,
0xD6,
0xD6,
0xC6,
0xC6,
0x00,
0x00,
0x00,
0x00,

/* Character N (0x4E):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |**   ** |
   |**   ** |
   |***  ** |
   |***  ** |
   |**** ** |
   |** **** |
   |**  *** |
   |**  *** |
   |**   ** |
   |**   ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xC6,
0xC6,
0xE6,
0xE6,
0xF6,
0xDE,
0xCE,
0xCE,
0xC6,
0xC6,
0x00,
0x00,
0x00,
0x00,

/* Character O (0x4F):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character P (0x50):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |******  |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   | *****  |
   | **     |
   | **     |
   | **     |
   |****    |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xFC,
0x66,
0x66,
0x66,
0x66,
0x7C,
0x60,
0x60,
0x60,
0xF0,
0x00,
0x00,
0x00,
0x00,

/* Character Q (0x51):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |** * ** |
   |** * ** |
   | *****  |
   |     ** |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0xD6,
0xD6,
0x7C,
0x06,
0x00,
0x00,
0x00,

/* Character R (0x52):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |******  |
   | **  ** |
   | **  ** |
   | **  ** |
   | *****  |
   | ****   |
   | ** **  |
   | **  ** |
   | **  ** |
   |***  ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xFC,
0x66,
0x66,
0x66,
0x7C,
0x78,
0x6C,
0x66,
0x66,
0xE6,
0x00,
0x00,
0x00,
0x00,

/* Character S (0x53):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |**   ** |
   |**      |
   |**      |
   | ***    |
   |   ***  |
   |     ** |
   |     ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0xC6,
0xC0,
0xC0,
0x70,
0x1C,
0x06,
0x06,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character T (0x54):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | ****** |
   | * ** * |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |  ****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7E,
0x5A,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0x3C,
0x00,
0x00,
0x00,
0x00,

/* Character U (0x55):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character V (0x56):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   | ** **  |
   |  ***   |
   |   *    |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0x6C,
0x38,
0x10,
0x00,
0x00,
0x00,
0x00,

/* Character W (0x57):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |**   ** |
   |**   ** |
   |**   ** |
   |** * ** |
   |** * ** |
   |** * ** |
   |******* |
   |*** *** |
   |**   ** |
   |**   ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xC6,
0xC6,
0xC6,
0xD6,
0xD6,
0xD6,
0xFE,
0xEE,
0xC6,
0xC6,
0x00,
0x00,
0x00,
0x00,

/* Character X (0x58):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |**   ** |
   |**   ** |
   |**   ** |
   | ** **  |
   |  ***   |
   |  ***   |
   | ** **  |
   |**   ** |
   |**   ** |
   |**   ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xC6,
0xC6,
0xC6,
0x6C,
0x38,
0x38,
0x6C,
0xC6,
0xC6,
0xC6,
0x00,
0x00,
0x00,
0x00,

/* Character Y (0x59):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   |  ****  |
   |   **   |
   |   **   |
   |   **   |
   |  ****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x66,
0x66,
0x66,
0x66,
0x66,
0x3C,
0x18,
0x18,
0x18,
0x3C,
0x00,
0x00,
0x00,
0x00,

/* Character Z (0x5A):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |******* |
   |**   ** |
   |*    ** |
   |    **  |
   |   **   |
   |  **    |
   | **     |
   |**    * |
   |**   ** |
   |******* |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xFE,
0xC6,
0x86,
0x0C,
0x18,
0x30,
0x60,
0xC2,
0xC6,
0xFE,
0x00,
0x00,
0x00,
0x00,

/* Character [ (0x5B):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   | **     |
   | **     |
   | **     |
   | **     |
   | **     |
   | **     |
   | **     |
   | **     |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0x60,
0x60,
0x60,
0x60,
0x60,
0x60,
0x60,
0x60,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character \ (0x5C):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |*       |
   |**      |
   | **     |
   |  **    |
   |   **   |
   |    **  |
   |     ** |
   |      * |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x80,
0xC0,
0x60,
0x30,
0x18,
0x0C,
0x06,
0x02,
0x00,
0x00,
0x00,
0x00,

/* Character ] (0x5D):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *****  |
   |    **  |
   |    **  |
   |    **  |
   |    **  |
   |    **  |
   |    **  |
   |    **  |
   |    **  |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x7C,
0x0C,
0x0C,
0x0C,
0x0C,
0x0C,
0x0C,
0x0C,
0x0C,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character ^ (0x5E):
   ht=16, width=8
   +--------+
   |        |
   |   *    |
   |  ***   |
   | ** **  |
   |**   ** |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x10,
0x38,
0x6C,
0xC6,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,

/* Character _ (0x5F):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |********|
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0xFF,
0x00,
0x00,

/* Character ` (0x60):
   ht=16, width=8
   +--------+
   |        |
   |   **   |
   |   **   |
   |   **   |
   |    **  |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x18,
0x18,
0x18,
0x0C,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,

/* Character A (0x61):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   | ****   |
   |    **  |
   | *****  |
   |**  **  |
   |**  **  |
   |** ***  |
   | *** ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x78,
0x0C,
0x7C,
0xCC,
0xCC,
0xDC,
0x76,
0x00,
0x00,
0x00,
0x00,

/* Character B (0x62):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |***     |
   | **     |
   | **     |
   | *****  |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   |******  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xE0,
0x60,
0x60,
0x7C,
0x66,
0x66,
0x66,
0x66,
0x66,
0xFC,
0x00,
0x00,
0x00,
0x00,

/* Character C (0x63):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   | *****  |
   |**   ** |
   |**      |
   |**      |
   |**      |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x7C,
0xC6,
0xC0,
0xC0,
0xC0,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character D (0x64):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |   ***  |
   |    **  |
   |    **  |
   | *****  |
   |**  **  |
   |**  **  |
   |**  **  |
   |**  **  |
   |**  **  |
   | ****** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x1C,
0x0C,
0x0C,
0x7C,
0xCC,
0xCC,
0xCC,
0xCC,
0xCC,
0x7E,
0x00,
0x00,
0x00,
0x00,

/* Character E (0x65):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   | *****  |
   |**   ** |
   |**   ** |
   |******* |
   |**      |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x7C,
0xC6,
0xC6,
0xFE,
0xC0,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character F (0x66):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |   ***  |
   |  ** ** |
   |  **    |
   |  **    |
   |******  |
   |  **    |
   |  **    |
   |  **    |
   |  **    |
   | ****   |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x1C,
0x36,
0x30,
0x30,
0xFC,
0x30,
0x30,
0x30,
0x30,
0x78,
0x00,
0x00,
0x00,
0x00,

/* Character g (0x67):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   | *** ** |
   |**  *** |
   |**   ** |
   |**   ** |
   |**  *** |
   | *** ** |
   |     ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x76,
0xCE,
0xC6,
0xC6,
0xCE,
0x76,
0x06,
0xC6,
0x7C,
0x00,
0x00,

/* Character h (0x68):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |***     |
   | **     |
   | **     |
   | *****  |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   |***  ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xE0,
0x60,
0x60,
0x7C,
0x66,
0x66,
0x66,
0x66,
0x66,
0xE6,
0x00,
0x00,
0x00,
0x00,

/* Character i (0x69):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |   **   |
   |   **   |
   |        |
   |  ***   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |  ****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x18,
0x18,
0x00,
0x38,
0x18,
0x18,
0x18,
0x18,
0x18,
0x3C,
0x00,
0x00,
0x00,
0x00,

/* Character j (0x6A):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |    **  |
   |    **  |
   |        |
   |   ***  |
   |    **  |
   |    **  |
   |    **  |
   |    **  |
   |    **  |
   |**  **  |
   |**  **  |
   | ****   |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x0C,
0x0C,
0x00,
0x1C,
0x0C,
0x0C,
0x0C,
0x0C,
0x0C,
0xCC,
0xCC,
0x78,
0x00,
0x00,

/* Character k (0x6B):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |***     |
   | **     |
   | **     |
   | **  ** |
   | **  ** |
   | ** **  |
   | ****   |
   | ** **  |
   | **  ** |
   |***  ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0xE0,
0x60,
0x60,
0x66,
0x66,
0x6C,
0x78,
0x6C,
0x66,
0xE6,
0x00,
0x00,
0x00,
0x00,

/* Character l (0x6C):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   ***  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0x18,
0x1C,
0x00,
0x00,
0x00,
0x00,

/* Character m (0x6D):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   | ** **  |
   |******* |
   |** * ** |
   |** * ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x6C,
0xFE,
0xD6,
0xD6,
0xC6,
0xC6,
0xC6,
0x00,
0x00,
0x00,
0x00,

/* Character n (0x6E):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |** ***  |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0xDC,
0x66,
0x66,
0x66,
0x66,
0x66,
0x66,
0x00,
0x00,
0x00,
0x00,

/* Character o (0x6F):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   | *****  |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x7C,
0xC6,
0xC6,
0xC6,
0xC6,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character p (0x70):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |** ***  |
   | **  ** |
   | **  ** |
   | **  ** |
   | **  ** |
   | *****  |
   | **     |
   | **     |
   |****    |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0xDC,
0x66,
0x66,
0x66,
0x66,
0x7C,
0x60,
0x60,
0xF0,
0x00,
0x00,

/* Character q (0x71):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   | *** ** |
   |**  **  |
   |**  **  |
   |**  **  |
   |**  **  |
   | *****  |
   |    **  |
   |    **  |
   |   **** |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x76,
0xCC,
0xCC,
0xCC,
0xCC,
0x7C,
0x0C,
0x0C,
0x1E,
0x00,
0x00,

/* Character r (0x72):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |** ***  |
   | **  ** |
   | **     |
   | **     |
   | **     |
   | **     |
   |****    |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0xDC,
0x66,
0x60,
0x60,
0x60,
0x60,
0xF0,
0x00,
0x00,
0x00,
0x00,

/* Character s (0x73):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   | *****  |
   |**   ** |
   |**      |
   | *****  |
   |     ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x7C,
0xC6,
0xC0,
0x7C,
0x06,
0xC6,
0x7C,
0x00,
0x00,
0x00,
0x00,

/* Character t (0x74):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |  **    |
   |  **    |
   |  **    |
   |******  |
   |  **    |
   |  **    |
   |  **    |
   |  **    |
   |  ** ** |
   |   ***  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x30,
0x30,
0x30,
0xFC,
0x30,
0x30,
0x30,
0x30,
0x36,
0x1C,
0x00,
0x00,
0x00,
0x00,

/* Character u (0x75):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |**  **  |
   |**  **  |
   |**  **  |
   |**  **  |
   |**  **  |
   |**  **  |
   | *** ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0xCC,
0xCC,
0xCC,
0xCC,
0xCC,
0xCC,
0x76,
0x00,
0x00,
0x00,
0x00,

/* Character v (0x76):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   | ** **  |
   |  ***   |
   |   *    |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0xC6,
0xC6,
0xC6,
0xC6,
0x6C,
0x38,
0x10,
0x00,
0x00,
0x00,
0x00,

/* Character w (0x77):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |**   ** |
   |**   ** |
   |** * ** |
   |** * ** |
   |** * ** |
   |******* |
   | ** **  |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0xC6,
0xC6,
0xD6,
0xD6,
0xD6,
0xFE,
0x6C,
0x00,
0x00,
0x00,
0x00,

/* Character x (0x78):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |**   ** |
   |**   ** |
   | ** **  |
   |  ***   |
   | ** **  |
   |**   ** |
   |**   ** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0xC6,
0xC6,
0x6C,
0x38,
0x6C,
0xC6,
0xC6,
0x00,
0x00,
0x00,
0x00,

/* Character y (0x79):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |**   ** |
   |**   ** |
   |**   ** |
   |**   ** |
   |**  *** |
   | *** ** |
   |     ** |
   |**   ** |
   | *****  |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0xC6,
0xC6,
0xC6,
0xC6,
0xCE,
0x76,
0x06,
0xC6,
0x7C,
0x00,
0x00,

/* Character z (0x7A):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |******* |
   |*    ** |
   |    **  |
   |   **   |
   |  **    |
   | **   * |
   |******* |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0xFE,
0x86,
0x0C,
0x18,
0x30,
0x62,
0xFE,
0x00,
0x00,
0x00,
0x00,

/* Character { (0x7B):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |    *** |
   |   **   |
   |   **   |
   |   **   |
   | ***    |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |    *** |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x0E,
0x18,
0x18,
0x18,
0x70,
0x18,
0x18,
0x18,
0x18,
0x0E,
0x00,
0x00,
0x00,
0x00,

/* Character | (0x7C):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |        |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x18,
0x18,
0x18,
0x18,
0x00,
0x18,
0x18,
0x18,
0x18,
0x18,
0x00,
0x00,
0x00,
0x00,

/* Character } (0x7D):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | ***    |
   |   **   |
   |   **   |
   |   **   |
   |    *** |
   |   **   |
   |   **   |
   |   **   |
   |   **   |
   | ***    |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x70,
0x18,
0x18,
0x18,
0x0E,
0x18,
0x18,
0x18,
0x18,
0x70,
0x00,
0x00,
0x00,
0x00,

/* Character ~ (0x7E):
   ht=16, width=8
   +--------+
   |        |
   |        |
   | *** ** |
   |** ***  |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x76,
0xDC,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,
0x00,

/* Character DELTA (0x7F):
   ht=16, width=8
   +--------+
   |        |
   |        |
   |        |
   |        |
   |        |
   |   *    |
   |  ***   |
   |  ***   |
   | ** **  |
   | ** **  |
   |******* |
   |        |
   |        |
   |        |
   |        |
   |        |
   +--------+ */
0x00,
0x00,
0x00,
0x00,
0x00,
0x10,
0x38,
0x38,
0x6C,
0x6C,
0xFE,
0x00,
0x00,
0x00,
0x00,
0x00,
};



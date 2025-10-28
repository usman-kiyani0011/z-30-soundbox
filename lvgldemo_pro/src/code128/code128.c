/* code128.c - Handles Code 128 and derivatives */

/*
    libzint - the open source barcode library
    Copyright (C) 2008 Robin Stuart <robin@zint.org.uk>
    Bugfixes thanks to Christian Sakowski and BogDan Vatra

    This program is FREE software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

//#include "system/pub/mfmalloc.h"
#include "pub/common/misc/inc/mfmalloc.h"

#define TRUE 1
#define FALSE 0
#define SHIFTA 90
#define LATCHA 91
#define SHIFTB 92
#define LATCHB 93
#define SHIFTC 94
#define LATCHC 95
#define AORB 96
#define ABORC 97

#define DPDSET	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ*"

char list[2][170];

/* Code 128 tables checked against ISO/IEC 15417:2007 */

static const char * const C128Table[107] = {"212222", "222122", "222221", "121223", "121322", "131222", "122213",
	"122312", "132212", "221213", "221312", "231212", "112232", "122132", "122231", "113222",
	"123122", "123221", "223211", "221132", "221231", "213212", "223112", "312131", "311222",
	"321122", "321221", "312212", "322112", "322211", "212123", "212321", "232121", "111323",
	"131123", "131321", "112313", "132113", "132311", "211313", "231113", "231311", "112133",
	"112331", "132131", "113123", "113321", "133121", "313121", "211331", "231131", "213113",
	"213311", "213131", "311123", "311321", "331121", "312113", "312311", "332111", "314111",
	"221411", "431111", "111224", "111422", "121124", "121421", "141122", "141221", "112214",
	"112412", "122114", "122411", "142112", "142211", "241211", "221114", "413111", "241112",
	"134111", "111242", "121142", "121241", "114212", "124112", "124211", "411212", "421112",
	"421211", "212141", "214121", "412121", "111143", "111341", "131141", "114113", "114311",
	"411113", "411311", "113141", "114131", "311141", "411131", "211412", "211214", "211232",
	"2331112"};
/* Code 128 character encodation - Table 1 */

int parunmodd(uint8_t llyth)
{
	int modd = SHIFTB;

	if (llyth <= 31)
		modd = SHIFTA;
	else if ((llyth >= 48) && (llyth <= 57))
		modd = ABORC;
	else if (llyth <= 95)
		modd = AORB;
	else if (llyth <= 127)
		modd = SHIFTB;
	else if (llyth <= 159)
		modd = SHIFTA;
	else if (llyth <= 223)
		modd = AORB;

	return modd;
}

/**
 * bring together same type blocks
 */
void grwp(int *indexliste)
{
	int i,j;
	if (*indexliste <= 1)
		return;

	for (i = 1; i < *indexliste; i++) {
		if (list[1][i - 1] == list[1][i]) {
			/* bring together */
			list[0][i - 1] = list[0][i - 1] + list[0][i];

			/* decreace the list */
			for (j = i + 1; j < *(indexliste); j++) {
				list[0][j - 1] = list[0][j];
				list[1][j - 1] = list[1][j];
			}
			(*indexliste)--;
			i--;
		}
	}
}

/**
 * Implements rules from ISO 15417 Annex E
 */
void dxsmooth(int *indexliste)
{
	int current, length, last, next;
	int i;
	for (i = 0; i < *indexliste; i++) {
		current = list[1][i];
		length = list[0][i];

		if (i != 0)
			last = list[1][i - 1];
		else
			last = FALSE;

		if (i != *indexliste - 1)
			next = list[1][i + 1];
		else
			next = FALSE;

		if (i == 0) {
			/* first block */
			if (*indexliste == 1 && length == 2 && current == ABORC)
				/* Rule 1a */
				list[1][i] = LATCHC;

			if (current == ABORC) {
				if (length >= 4)
					/* Rule 1b */
					list[1][i] = LATCHC;
				else
					list[1][i] = current = AORB;
			}

			if (current == SHIFTA)
				/* Rule 1c */
				list[1][i] = LATCHA;
			if (current == AORB && next == SHIFTA)
				/* Rule 1c */
				list[1][i] = current = LATCHA;
			if (current == AORB)
				/* Rule 1d */
				list[1][i] = LATCHB;
		} else {
			if (current == ABORC && length >= 4)
				/* Rule 3 */
				list[1][i] = current = LATCHC;
			if (current == ABORC)
				list[1][i] = current = AORB;
			if (current == AORB && last == LATCHA)
				list[1][i] = current = LATCHA;
			if (current == AORB && last == LATCHB)
				list[1][i] = current = LATCHB;
			if (current == AORB && next == SHIFTA)
				list[1][i] = current = LATCHA;
			if (current == AORB && next == SHIFTB)
				list[1][i] = current = LATCHB;
			if (current == AORB)
				list[1][i] = current = LATCHB;
			if (current == SHIFTA && length > 1)
				/* Rule 4 */
				list[1][i] = current = LATCHA;
			if (current == SHIFTB && length > 1)
				/* Rule 5 */
				list[1][i] = current = LATCHB;
			if (current == SHIFTA && last == LATCHA)
				list[1][i] = current = LATCHA;
			if (current == SHIFTB && last == LATCHB)
				list[1][i] = current = LATCHB;
			if (current == SHIFTA && last == LATCHC)
				list[1][i] = current = LATCHA;
			if (current == SHIFTB && last == LATCHC)
				list[1][i] = current = LATCHB;
		} /* Rule 2 is implimented elsewhere, Rule 6 is implied */
	}
	grwp(indexliste);
}

/**
 * Translate Code 128 Set A characters into barcodes.
 * This set handles all control characters NULL to US.
 */
void c128_set_a(uint8_t source, char dest[], int values[], int *bar_chars)
{
	/* limit the range to 0-127 */
	source &= 127;

	if (source < 32)
		source += 64;
	else
		source -= 32;

	concat(dest, C128Table[source]);
	values[(*bar_chars)++] = source;
}

/**
 * Translate Code 128 Set B characters into barcodes.
 * This set handles all characters which are not part of long numbers and not
 * control characters.
 */
void c128_set_b(uint8_t source, char dest[], int values[], int *bar_chars)
{
	/* limit the range to 0-127 */
	source &= 127;
	source -= 32;

	concat(dest, C128Table[source]);
	values[(*bar_chars)++] = source;
}

/**
 * Translate Code 128 Set C characters into barcodes.
 * This set handles numbers in a compressed form.
 */
void c128_set_c(uint8_t source_a, uint8_t source_b, char dest[], int values[], int *bar_chars)
{
	int weight;

	weight = 10 * ctoi(source_a) + ctoi(source_b);
	concat(dest, C128Table[weight]);
	values[(*bar_chars)++] = weight;
}

/**
 * Handle Code 128 and NVE-18.
 */
int code_128(struct zint_symbol *symbol, uint8_t *source, int length)
{
	int i, j = 0;
	int values[170] = { 0 }, bar_characters, read, total_sum;
	int error_number, indexchaine, indexliste, sourcelen, f_state;
	char set[170] = { ' ' }, fset[170] = { ' ' }, mode, last_set, current_set = ' ';
	float glyph_count;
	char dest[1000];

	error_number = 0;
	strcpy(dest, "");

	sourcelen = length;

	bar_characters = 0;
	f_state = 0;

	if(sourcelen > 160) {
		/* This only blocks rediculously long input - the actual length of the
		   resulting barcode depends on the type of data, so this is trapped later */
		strcpy(symbol->errtxt, "Input too long");
		return ZERROR_TOO_LONG;
	}

	/* Detect extended ASCII characters */
	for(i = 0; i < sourcelen; i++) {
		if(source[i] >= 128)
			fset[i] = 'f';
	}
	fset[sourcelen] = '\0';

	/* Decide when to latch to extended mode - Annex E note 3 */
	j = 0;
	for(i = 0; i < sourcelen; i++) {
		if(fset[i] == 'f') {
			j++;
		} else {
			j = 0;
		}

		if(j >= 5) {
			int k;
			for(k = i; k > (i - 5); k--) {
				fset[k] = 'F';
			}
		}

		if((j >= 3) && (i == (sourcelen - 1))) {
			int k;
			for(k = i; k > (i - 3); k--) {
				fset[k] = 'F';
			}
		}
	}

	/* Decide if it is worth reverting to 646 encodation for a few
	   characters as described in 4.3.4.2 (d) */
	for (i = 1; i < sourcelen; i++) {
		if((fset[i - 1] == 'F') && (fset[i] == ' ')) {
			/* Detected a change from 8859-1 to 646 - count how long for */
			for (j = 0; (fset[i + j] == ' ') && ((i + j) < sourcelen); j++);
			if ((j < 5) || ((j < 3) && ((i + j) == (sourcelen - 1)))) {
				/* Uses the same figures recommended by Annex E note 3 */
				/* Change to shifting back rather than latching back */
				int k;
				for (k = 0; k < j; k++) {
					fset[i + k] = 'n';
				}
			}
		}
	}

	/* Decide on mode using same system as PDF417 and rules of ISO 15417 Annex E */
	indexliste = 0;
	indexchaine = 0;

	mode = parunmodd(source[indexchaine]);
	if((symbol->symbology == BARCODE_CODE128B) && (mode == ABORC)) {
		mode = AORB;
	}

	memset(list[0], 0, 170 * sizeof(list[0][0]));

	do {
		list[1][indexliste] = mode;
		while ((list[1][indexliste] == mode) && (indexchaine < sourcelen)) {
			list[0][indexliste]++;
			indexchaine++;
			mode = parunmodd(source[indexchaine]);
			if((symbol->symbology == BARCODE_CODE128B) && (mode == ABORC)) {
				mode = AORB;
			}
		}
		indexliste++;
	} while (indexchaine < sourcelen);

	dxsmooth(&indexliste);

	/* Resolve odd length LATCHC blocks */
	if((list[1][0] == LATCHC) && (list[0][0] & 1)) {
		/* Rule 2 */
		list[0][1]++;
		list[0][0]--;
		if(indexliste == 1) {
			list[0][1] = 1;
			list[1][1] = LATCHB;
			indexliste = 2;
		}
	}
	if (indexliste > 1) {
		for(i = 1; i < indexliste; i++) {
			if((list[1][i] == LATCHC) && (list[0][i] & 1)) {
				/* Rule 3b */
				list[0][i - 1]++;
				list[0][i]--;
			}
		}
	}

	/* Put set data into set[] */

	read = 0;
	for(i = 0; i < indexliste; i++) {
		for(j = 0; j < list[0][i]; j++) {
			switch(list[1][i]) {
			case SHIFTA:
				set[read] = 'a';
				break;
			case LATCHA:
				set[read] = 'A';
				break;
			case SHIFTB:
				set[read] = 'b';
				break;
			case LATCHB:
				set[read] = 'B';
				break;
			case LATCHC:
				set[read] = 'C';
				break;
			}
			read++;
		}
	}

	/* Adjust for strings which start with shift characters - make them latch instead */
	for (i = 0; set[i] == 'a'; i++)
		set[i] = 'A';

	for(i = 0; set[i] == 'b'; i++)
		set[i] = 'B';

	/* Now we can calculate how long the barcode is going to be - and stop it from
	   being too long */
	last_set = ' ';
	glyph_count = 0.0;
	for(i = 0; i < sourcelen; i++) {
		if((set[i] == 'a') || (set[i] == 'b')) {
			glyph_count = glyph_count + 1.0;
		}
		if((fset[i] == 'f') || (fset[i] == 'n')) {
			glyph_count = glyph_count + 1.0;
		}
		if(((set[i] == 'A') || (set[i] == 'B')) || (set[i] == 'C')) {
			if(set[i] != last_set) {
				last_set = set[i];
				glyph_count = glyph_count + 1.0;
			}
		}
		if(i == 0) {
			if(fset[i] == 'F') {
				glyph_count = glyph_count + 2.0;
			}
		} else {
			if((fset[i] == 'F') && (fset[i - 1] != 'F')) {
				glyph_count = glyph_count + 2.0;
			}
			if((fset[i] != 'F') && (fset[i - 1] == 'F')) {
				glyph_count = glyph_count + 2.0;
			}
		}

		if(set[i] == 'C') {
			glyph_count = glyph_count + 0.5;
		} else {
			glyph_count = glyph_count + 1.0;
		}
	}
	if(glyph_count > 80.0) {
		strcpy(symbol->errtxt, "Input too long");
		return ZERROR_TOO_LONG;
	}

	/* So now we know what start character to use - we can get on with it! */
	if(symbol->output_options & READER_INIT) {
		/* Reader Initialisation mode */
		switch(set[0]) {
			case 'A': /* Start A */
				concat(dest, C128Table[103]);
				values[0] = 103;
				current_set = 'A';
				concat(dest, C128Table[96]); /* FNC3 */
				values[1] = 96;
				bar_characters++;
				break;
			case 'B': /* Start B */
				concat(dest, C128Table[104]);
				values[0] = 104;
				current_set = 'B';
				concat(dest, C128Table[96]); /* FNC3 */
				values[1] = 96;
				bar_characters++;
				break;
			case 'C': /* Start C */
				concat(dest, C128Table[104]); /* Start B */
				values[0] = 105;
				concat(dest, C128Table[96]); /* FNC3 */
				values[1] = 96;
				concat(dest, C128Table[99]); /* Code C */
				values[2] = 99;
				bar_characters += 2;
				current_set = 'C';
				break;
		}
	} else {
		/* Normal mode */
		switch(set[0]) {
			case 'A': /* Start A */
				concat(dest, C128Table[103]);
				values[0] = 103;
				current_set = 'A';
				break;
			case 'B': /* Start B */
				concat(dest, C128Table[104]);
				values[0] = 104;
				current_set = 'B';
				break;
			case 'C': /* Start C */
				concat(dest, C128Table[105]);
				values[0] = 105;
				current_set = 'C';
				break;
		}
	}
	bar_characters++;
	last_set = set[0];

	if(fset[0] == 'F') {
		switch(current_set) {
			case 'A':
				concat(dest, C128Table[101]);
				concat(dest, C128Table[101]);
				values[bar_characters] = 101;
				values[bar_characters + 1] = 101;
				break;
			case 'B':
				concat(dest, C128Table[100]);
				concat(dest, C128Table[100]);
				values[bar_characters] = 100;
				values[bar_characters + 1] = 100;
				break;
		}
		bar_characters += 2;
		f_state = 1;
	}

	/* Encode the data */
	read = 0;
	do {

		if((read != 0) && (set[read] != current_set))
		{ /* Latch different code set */
			switch(set[read])
			{
				case 'A': concat(dest, C128Table[101]);
					values[bar_characters] = 101;
					bar_characters++;
					current_set = 'A';
					break;
				case 'B': concat(dest, C128Table[100]);
					values[bar_characters] = 100;
					bar_characters++;
					current_set = 'B';
					break;
				case 'C': concat(dest, C128Table[99]);
					values[bar_characters] = 99;
					bar_characters++;
					current_set = 'C';
					break;
			}
		}

		if(read != 0) {
			if((fset[read] == 'F') && (f_state == 0)) {
				/* Latch beginning of extended mode */
				switch(current_set) {
					case 'A':
						concat(dest, C128Table[101]);
						concat(dest, C128Table[101]);
						values[bar_characters] = 101;
						values[bar_characters + 1] = 101;
						break;
					case 'B':
						concat(dest, C128Table[100]);
						concat(dest, C128Table[100]);
						values[bar_characters] = 100;
						values[bar_characters + 1] = 100;
						break;
				}
				bar_characters += 2;
				f_state = 1;
			}
			if((fset[read] == ' ') && (f_state == 1)) {
				/* Latch end of extended mode */
				switch(current_set) {
					case 'A':
						concat(dest, C128Table[101]);
						concat(dest, C128Table[101]);
						values[bar_characters] = 101;
						values[bar_characters + 1] = 101;
						break;
					case 'B':
						concat(dest, C128Table[100]);
						concat(dest, C128Table[100]);
						values[bar_characters] = 100;
						values[bar_characters + 1] = 100;
						break;
				}
				bar_characters += 2;
				f_state = 0;
			}
		}

		if((fset[read] == 'f') || (fset[read] == 'n')) {
			/* Shift to or from extended mode */
			switch(current_set) {
				case 'A':
					concat(dest, C128Table[101]); /* FNC 4 */
					values[bar_characters] = 101;
					break;
				case 'B':
					concat(dest, C128Table[100]); /* FNC 4 */
					values[bar_characters] = 100;
					break;
			}
			bar_characters++;
		}

		if((set[read] == 'a') || (set[read] == 'b')) {
			/* Insert shift character */
			concat(dest, C128Table[98]);
			values[bar_characters] = 98;
			bar_characters++;
		}

		switch(set[read])
		{ /* Encode data characters */
			case 'a':
			case 'A': c128_set_a(source[read], dest, values, &bar_characters);
				read++;
				break;
			case 'b':
			case 'B': c128_set_b(source[read], dest, values, &bar_characters);
				read++;
				break;
			case 'C': c128_set_c(source[read], source[read + 1], dest, values, &bar_characters);
				read += 2;
				break;
		}

	} while (read < sourcelen);

	/* check digit calculation */
	total_sum = 0;
	/*for(i = 0; i < bar_characters; i++) {
		printf("%d\n", values[i]);
	}*/

	for(i = 0; i < bar_characters; i++) {
		if(i > 0)
			values[i] *= i;
		total_sum += values[i];
	}
	concat(dest, C128Table[total_sum % 103]);

	/* Stop character */
	concat(dest, C128Table[106]);
	expand(symbol, dest);
	return error_number;
}

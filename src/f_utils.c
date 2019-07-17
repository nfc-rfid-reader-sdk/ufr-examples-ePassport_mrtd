/*
 * f_utils.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "ini.h"
#include "f_utils.h"

void mrtd_fileread_write_image_to_file(const uint8_t *file_content, const int file_size, const char *filename) {
	FILE *out;
	char filetype;
	char filenamebuf[256];
	int baselength = 0;
	unsigned char start_sequence_jpeg[10] = { 0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46 };
	unsigned char start_sequence_jpeg2000[10] = { 0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20, 0x0d, 0x0a };
	unsigned char *start_sequence;
	int comparelen = 0;
	if (file_size > 84) {
		filetype = file_content[71]; // Bilo je [73] // 0x00: JPG, 0x01: JPEG2000
		if (strlen(filename) > 3 && filename[strlen(filename) - 4] == '.')
			baselength = strlen(filename) - 4;
		else
			baselength = strlen(filename);
		memcpy(filenamebuf, filename, baselength);
		if (filetype == 0x00) {
			memcpy(filenamebuf + baselength, ".jpg", 4);
			start_sequence = start_sequence_jpeg;
			comparelen = 2;
		} else {
			memcpy(filenamebuf + baselength, ".jp2", 4);
			start_sequence = start_sequence_jpeg2000;
			comparelen = 10;
		}
		filenamebuf[baselength + 4] = 0;

		int i, j;
		int offset = 0;
		char equal = 0;
		for (i = 0; i < 120; i++) {
			j = 0;
			if (file_content[i] == start_sequence[j]) {
				equal = 1;
				for (j = 1; j < comparelen; j++) {
					if (file_content[i + j] != start_sequence[j]) {
						equal = 0;
						break;
					}
				}
			}
			if (equal) {
				offset = i;
				break;
			}
		}
		if (!equal) {
			printf("Couldn't find start of image\n");
			return;
		}
		out = fopen(filenamebuf, "wb");
		printf("Saving image to %s...", filenamebuf);
		fwrite(file_content + offset, 1, file_size - offset, out);
		fclose(out);
		printf(" done\n");
		if (filetype == 0x01)
			printf(
					"\n(Note: .jp2 files are JPEG2000 images which can be opened\n with many different image viewers. If you are unable to\n open it, it can be converted to JPEG with GraphicsMagick:\n   gm convert image.jp2 image.jpg )\n\n");
	}
}

void mrtd_fileread_get_datagroup_name(const uint8_t dg, char *name) {
	switch (dg) {
	case (0x60):
		sprintf(name, "EF_COM");
		break;
	case (0x61):
		sprintf(name, "EF_DG1");
		break;
	case (0x75):
		sprintf(name, "EF_DG2");
		break;
	case (0x63):
		sprintf(name, "EF_DG3");
		break;
	case (0x76):
		sprintf(name, "EF_DG4");
		break;
	case (0x65):
		sprintf(name, "EF_DG5");
		break;
	case (0x66):
		sprintf(name, "EF_DG6");
		break;
	case (0x67):
		sprintf(name, "EF_DG7");
		break;
	case (0x68):
		sprintf(name, "EF_DG8");
		break;
	case (0x69):
		sprintf(name, "EF_DG9");
		break;
	case (0x6a):
		sprintf(name, "EF_DG10");
		break;
	case (0x6b):
		sprintf(name, "EF_DG11");
		break;
	case (0x6c):
		sprintf(name, "EF_DG12");
		break;
	case (0x6d):
		sprintf(name, "EF_DG13");
		break;
	case (0x6e):
		sprintf(name, "EF_DG14");
		break;
	case (0x6f):
		sprintf(name, "EF_DG15");
		break;
	case (0x70):
		sprintf(name, "EF_DG16");
		break;
	case (0x77):
		sprintf(name, "EF_SOD");
		break;
	default:
		sprintf(name, "not defined");
		break;
	}
	return;
}

void mrtd_fileread_decode_ef_com(const uint8_t *file_content, const int file_size, uint8_t *datagroups, int *numdatagroups) {
	int i;
	int currentlength;
	int interncounter;
	int totallength;
	char longer = 0;
	char currentkeys = 0;
	currentlength = 0;
	*numdatagroups = 0;
	for (i = 0; i < file_size; i++) {
		if (i == 1) {
			totallength = file_content[i];
			interncounter = 0;
		} else if (longer == 1) {
			longer = 2;
		} else if (longer == 2) {
			longer = 0;
			currentlength = file_content[i];
			interncounter = 0;
		} else if (interncounter == currentlength && file_content[i] == 0x5f) {
			currentkeys = 0;
			longer = 1;
		} else if (interncounter == currentlength) {
			longer = 2;
			if (file_content[i] == 0x5c)
				currentkeys = 1;
			else
				currentkeys = 0;
		} else {
			if (currentkeys) {
				datagroups[interncounter] = file_content[i];
				char buffer[30];
				mrtd_fileread_get_datagroup_name(datagroups[interncounter], buffer);
				printf("Found: %s\n", buffer);
				(*numdatagroups)++;
			}
			interncounter++;
		}
	}
	printf("\n");
}




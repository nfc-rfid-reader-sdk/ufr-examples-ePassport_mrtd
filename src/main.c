/*
 ============================================================================
 Name        : ePassport, MRTD uFR NFC reader example
 Author      : Digital Logic Ltd.
 Version     : 0.0-pre-alpha
 Copyright   : 2009-2019. Digital Logic Ltd.
 Description : "ePassport, MRTD uFR NFC reader example" in C, Ansi-style
 ============================================================================
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if __WIN32 || __WIN64
#	include <conio.h>
#	include <windows.h>
#elif linux || __linux__ || __APPLE__
#	define __USE_MISC
#	include <unistd.h>
#	include <termios.h>
#	undef __USE_MISC
#	include "conio_gnu.h"
#else
#	error "Unknown build platform."
#endif
#include <uFCoder.h>
#include "ini.h"
#include "hw_utils.h"
#include "p_utils.h"
//------------------------------------------------------------------------------
#define READER_RESET
//------------------------------------------------------------------------------
void usage(void);
void menu(char key);
UFR_STATUS NewCardInField(uint8_t sak, uint8_t *uid, uint8_t uid_size);
bool CheckMRZ(void);
void EnterMRZSubjacentRow(void);
void EnterMRZData(void);
void ReadCOM(void);
void ReadSOD(void );
void ReadSODSaveToFile(void );
void ReadDG1(void );
void ReadDG2SaveToFile(void );
void ReadFacialImageSaveToFile(void );
void ReadDGxSaveToFile(void );
//------------------------------------------------------------------------------
// Global vars:
bool mrz_proto_key_defined = false;
uint64_t send_sequence_cnt;
// Global arrays:
uint8_t ksenc[16];
uint8_t ksmac[16];
uint8_t mrz_proto_key[25];
//------------------------------------------------------------------------------
int main(void) {
	char key;
	bool card_in_field = false;
	uint8_t old_sak = 0, old_uid_size = 0, old_uid[10];
	uint8_t sak, uid_size, uid[10];
	UFR_STATUS status;

	usage();
	printf(" --------------------------------------------------\n");
	printf("     Please wait while opening uFR NFC reader.\n");
	printf(" --------------------------------------------------\n");

	status = ReaderOpen();
	if (status != UFR_OK) {
		printf(" Error while opening device, status is: 0x%08X\n", status);
		getchar();
		return EXIT_FAILURE;
	}

	if (!CheckDependencies()) {
		ReaderClose();
		getchar();
		return EXIT_FAILURE;
	}

#ifdef READER_RESET
	status = ReaderReset();
	if (status != UFR_OK) {
		printf(" Error while opening device, status is: 0x%08X\n", status);
		getchar();
		return EXIT_FAILURE;
	}
#endif

	printf(" --------------------------------------------------\n");
	printf("        uFR NFC reader successfully opened.\n");
	printf(" --------------------------------------------------\n");

#if linux || __linux__ || __APPLE__
	_initTermios(0);
#endif
	do {
		while (!_kbhit()) {
			status = GetCardIdEx(&sak, uid, &uid_size);
			switch (status) {
			case UFR_OK:
				if (card_in_field) {
					if (old_sak != sak || old_uid_size != uid_size
							|| memcmp(old_uid, uid, uid_size)) {
						old_sak = sak;
						old_uid_size = uid_size;
						memcpy(old_uid, uid, uid_size);
						NewCardInField(sak, uid, uid_size);
					}
				} else {
					old_sak = sak;
					old_uid_size = uid_size;
					memcpy(old_uid, uid, uid_size);
					NewCardInField(sak, uid, uid_size);
					card_in_field = true;
				}
				break;
			case UFR_NO_CARD:
				card_in_field = false;
				status = UFR_OK;
				break;
			default:
				ReaderClose();
				printf(" Fatal error while trying to read card, status is: %s\n", UFR_Status2String(status));
				getchar();
#if linux || __linux__ || __APPLE__
				_resetTermios();
				tcflush(0, TCIFLUSH); // Clear stdin to prevent characters appearing on prompt
#endif
				return EXIT_FAILURE;
			}
#if __WIN32 || __WIN64
			Sleep(100);
#else // if linux || __linux__ || __APPLE__
			usleep(300000);
#endif
		}

		key = _getch();
		menu(key);
	} while (key != '\x1b');

	ReaderClose();
#if linux || __linux__ || __APPLE__
	_resetTermios();
	tcflush(0, TCIFLUSH); // Clear stdin to prevent characters appearing on prompt
#endif
	return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------
void menu(char key) {

	switch (key) {
	case 'M':
	case 'm':
		EnterMRZSubjacentRow();
		break;
	case 'P':
	case 'p':
		EnterMRZData();
		break;
	case 'C':
	case 'c':
		ReadCOM();
		break;
	case 'S':
	case 's':
		ReadSOD();
		break;
	case 'F':
	case 'f':
		ReadSODSaveToFile();
		break;
	case '1':
		ReadDG1();
		break;
	case '2':
		ReadDG2SaveToFile();
		break;
	case 'I':
	case 'i':
		ReadFacialImageSaveToFile();
		break;
	case 'D':
	case 'd':
		ReadDGxSaveToFile();
		break;
	case '\x1b':
		// Exit
		break;
	default:
		usage();
		break;
	}
}
//------------------------------------------------------------------------------
void usage(void) {
	printf( " +------------------------------------------------+\n"
			" |     ePassport, MRTD uFR NFC reader example     |\n"
			" |                 version "APP_VERSION"                    |\n"
			" +------------------------------------------------+\n"
			"                              For exit, hit escape.\n");
	printf( " --------------------------------------------------\n");
	printf( "  'M' - Enter MRZ data (subjacent MRZ row) needed for authentication\n"
			"  'P' - Enter doc. number, date of birth and date of expiry needed for authentication\n"
			"  ------------------for every document you first have to choose M or P before reading\n\n"

			"  'C' - Read EF.COM (Common Data), FID = '01 1E'\n"
			"  'S' - Read EF.SOD (Document Security Object), FID = '01 1D'\n"
			"  'F' - Read EF.SOD and save it to the binary file\n\n"
			"  '1' - Read EF.DG1 and display the MRZ data\n"
			"  '2' - Read EF.DG2 and save it to the binary file\n\n"

			"  'I' - Read EF.DG2, extract the facial image and save it to file\n\n"

			"  'D' - Read any of the EF.DGx, x = {1..16} and save it to the binary file\n"
			"(Esc) - Quit example\n");
}
//------------------------------------------------------------------------------
UFR_STATUS NewCardInField(uint8_t sak, uint8_t *uid, uint8_t uid_size) {
	UFR_STATUS status;
	uint8_t dl_card_type;

	status = GetDlogicCardType(&dl_card_type);
	if (status != UFR_OK)
		return status;

	printf(" \a-------------------------------------------------------------------\n");
	printf(" Teg type: %s, sak = 0x%02X, uid[%d] = ", UFR_DLCardType2String(dl_card_type), sak, uid_size);
	print_hex_ln(uid, uid_size, ":");
	printf(" -------------------------------------------------------------------\n");

	return UFR_OK;
}
//------------------------------------------------------------------------------
bool CheckMRZ(void) {

	if (!mrz_proto_key_defined) {
		printf("You have not entered MRZ (machine readable zone) data necessary for MRTD authentication.\n"
				"Please press 'M' if you want to enter entire subjacent MRZ row\n"
				"or press 'P' if you want to enter doc. number, date of birth\n"
				"and date of expiry separately. Dates format have to be YYMMDD.\n");
		return false;
	}
	return true;
}
//------------------------------------------------------------------------------
void EnterMRZSubjacentRow(void) {
	UFR_STATUS status;
	char mrz[44 + 1]; // +1 for zero termination

	printf("You have chose to enter subjacent MRZ row located under\n"
			"'P<XXXSURNAME<<FIRSTNAME<<<<<<<<<<<<<<<<<<<<<':\n\n");
	printf("Enter subjacent MRZ row. Subjacent MRZ row have to be 44 characters long.\n");

	scanf("%44s", mrz);
	if (strlen(mrz) != 44) {
		printf("Error: subjacent MRZ row have to be 44 characters long.\n");
	}

	status = MRTD_MRZSubjacentToMRZProtoKey(mrz, mrz_proto_key);
	if (status != UFR_OK) {
		printf(" Wrong entered MRZ data, uFR status is: 0x%08X\n", status);
		return;
	}
	mrz_proto_key_defined = true;
}
//------------------------------------------------------------------------------
void EnterMRZData(void) {
	UFR_STATUS status;
	char doc_number[9 + 1]; // +1 for zero termination
	char date_of_birth[6 + 1]; // +1 for zero termination
	char date_of_expiry[6 + 1]; // +1 for zero termination

	printf("You have chose to enter doc. number, date of birth and date of expiry separately:\n\n");
	printf("Enter the document number. The document number should be 9 characters long.\n");
	scanf("%9s", doc_number);
	printf("Enter date of birth. Date format have to be YYMMDD.\n");
	scanf("%6s", date_of_birth);
	if (strlen(date_of_birth) != 6) {
		printf("Error: date format have to be YYMMDD.\n");
	}
	printf("Enter date of expiry. Date format have to be YYMMDD.\n");
	scanf("%6s", date_of_expiry);
	if (strlen(date_of_expiry) != 6) {
		printf("Error: date format have to be YYMMDD.\n");
	}

	status = MRTD_MRZDataToMRZProtoKey(doc_number, date_of_birth, date_of_expiry, mrz_proto_key);
	if (status != UFR_OK) {
		printf(" Wrong entered MRZ data, uFR status is: 0x%08X\n", status);
		return;
	}
	mrz_proto_key_defined = true;
}
//------------------------------------------------------------------------------
void ReadCOM(void) {
	UFR_STATUS status;
	uint8_t *file_content = NULL;
	uint32_t file_len;

	if (!CheckMRZ())
		return;

	status = SetISO14443_4_Mode();
	if (status != UFR_OK) {
		printf(" Error while switching into ISO 14443-4 mode, uFR status is: 0x%08X\n", status);
		return;
	}

	do { // <== try

		status = MRTDAppSelectAndAuthenticate(mrz_proto_key, ksenc, ksmac, &send_sequence_cnt);
		if (status != UFR_OK) {
			printf(" Error while switching into ISO 14443-4 mode, uFR status is: 0x%08X\n", status);
			return;
		}

		status = MRTDFileRead((const uint8_t *)"\x01\x1E", file_content, &file_len, ksenc, ksmac, &send_sequence_cnt);
		if (status != UFR_OK) {
			printf(" Error while switching into ISO 14443-4 mode, uFR status is: 0x%08X\n", status);
			return;
		}

	} while(0); // <== finally

	s_block_deselect(100);
}
//------------------------------------------------------------------------------
void ReadSOD(void ) {

}
//------------------------------------------------------------------------------
void ReadSODSaveToFile(void ) {

}
//------------------------------------------------------------------------------
void ReadDG1(void ) {

}
//------------------------------------------------------------------------------
void ReadDG2SaveToFile(void ) {

}
//------------------------------------------------------------------------------
void ReadFacialImageSaveToFile(void ) {

}
//------------------------------------------------------------------------------
void ReadDGxSaveToFile(void ) {

}
//------------------------------------------------------------------------------

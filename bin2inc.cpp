// bin2inc.cpp : Defines the entry point for the console application.
// No, don't modify and redistribute this, please. ;)

#include "stdafx.h"
#include <stdio.h>
#include <time.h>

int _tmain(int argc, _TCHAR* argv[])
{
	unsigned char buf[128];
	char szName[256];
	int nTotal=0;
	char szHeader[128]="const unsigned char %s[] = {\n";// top of list
	char szPreface[128]="";								// per line
	char szComment[128]="//";							// comment
	char szNum[128]="0x";								// hex indicator (lead only)
	char szFooter[128]="};\n";							// bottom of list
	char szTab[128]="\t";								// leading tab
	char szSize[128]="#define %s %d\n";					// name then size
	int  nWidth = 16;									// bytes per line
	bool lastComma=true;								// comma at end of line?
	bool emitIfdef = true;

	printf("bin2inc by Tursi - http://harmlesslion.com\n");

	if (argc < 4) {
		printf("%s infile outfile array_name [TI/Z80]\n", argv[0]);
		printf("C style is used unless you specify an alternate format:\n");
		printf("  TI  - TI 9900 Editor/Assembler style formatting\n");
		printf("  Z80 - SDCC Assembler style formatting\n");
		return 1;
	}

	FILE *fin, *fout;

	fin=fopen(argv[1], "rb");
	if (NULL == fin) {
		printf("Failed to open %s\n", argv[1]);
		return 2;
	}

	fout=fopen(argv[2], "w");
	if (NULL == fout) {
		fclose(fin);
		printf("Failed to open %s\n", argv[2]);
		return 3;
	}

	if (argc>4) {
		if (stricmp(argv[4], "TI") == 0) {
			strcpy(szHeader, "%s\n");			// top of list
			strcpy(szPreface, "BYTE");			// per line
			strcpy(szComment, "*");				// comment
			strcpy(szNum, ">");					// hex indicator (lead only)
			strcpy(szFooter,"\n");				// bottom of list
			strcpy(szTab, "  ");				// leading tab
			strcpy(szSize, "%s EQU %d\n");		// name then size
			nWidth = 8;							// bytes per line
			lastComma=false;					// comma at end of line?
			emitIfdef = false;
		} else if (stricmp(argv[4], "Z80") == 0) {
			strcpy(szHeader, "%s:\n");			// top of list
			strcpy(szPreface, ".db");			// per line
			strcpy(szComment, ";");				// comment
			strcpy(szNum, "#0x");				// hex indicator (lead only)
			strcpy(szFooter,"\n");				// bottom of list
			strcpy(szTab, "\t");				// leading tab
			strcpy(szSize, "%s = %d\n");		// name then size
			nWidth = 8;							// bytes per line
			lastComma=false;					// comma at end of line?
			emitIfdef = false;
		} else {
			printf("Unrecognized machine '%s'\n", argv[4]);
			return 10;
		}
	}

	char date[128];
	struct tm tim;
	time_t now;
	now = time(NULL);
	tim = *(localtime(&now));
	strftime(date,128,"%b %d, %Y",&tim);

	fprintf(fout, "%s\n%s Data file %s - %s\n%s\n\n", szComment, szComment, argv[1],date, szComment);

	if (emitIfdef) fprintf(fout, "#ifndef BIN2INC_HEADER_ONLY\n");

	fprintf(fout, szHeader, argv[3]);

	int nPos = 0;

	int lastbyte;
	fseek(fin, 0, SEEK_END);
	lastbyte = ftell(fin)-1;
	fseek(fin, 0, SEEK_SET);

	while (!feof(fin)) {
		int cnt;

		memset(buf, 0, nWidth);
		cnt=(int)fread(buf, 1, nWidth, fin);
		if (cnt > 0) {
			// if we got any bytes
			fprintf(fout, "%s%s ", szTab, szPreface);
			for (int idx=0; idx<cnt; idx++) {
				fprintf(fout, "%s%02X", szNum, buf[idx]);
				if (idx<cnt-1) {
					fprintf(fout, ",");
				} else if ((lastComma)&&(nPos+idx<lastbyte)) {
					fprintf(fout, ",");
				}
			}
			// pad the last comment if needed
			for (int idx=0; idx<(nWidth-cnt)*(strlen(szNum)+3); idx++) {
				fprintf(fout, " ");
			}
			// output the ASCII dump
			fprintf(fout, "%s%s %08X ", szTab, szComment, nPos);
			for (int idx=0; idx<cnt; idx++) {
				if (isprint(buf[idx])) {
					fprintf(fout, "%c", buf[idx]);
				} else {
					fprintf(fout, ".");
				}
			}
			fprintf(fout, " //\n");
			nTotal+=cnt;
			nPos+=cnt;
		} else {
			break;
		}
	}

	fprintf(fout, szFooter);

	if (emitIfdef) {
		fprintf(fout, "#else\nextern const unsigned char %s[];\n", argv[3]);
	}

	_snprintf(szName, 256, "SIZE_OF_%s", argv[3]);
	szName[255]='\0';
	_strupr(szName);
	fprintf(fout, "\n%s Size of data in above array\n", szComment);
	fprintf(fout, szSize, szName, nTotal);

	if (emitIfdef) {
		fprintf(fout, "#endif\n");
	}

	fclose(fin);
	fclose(fout);

	printf("Converted %d bytes", nTotal);
	if (argc > 4) {
		printf(" for %s", argv[4]);
	}
	printf(".\n");
	printf("** Done **\n");

	return 0;
}

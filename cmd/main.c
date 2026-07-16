#include "pch.h"

bool ispe32(IMAGE_DOS_HEADER buffer, char* argv[]);


int main(int argc, char* argv[]) {
	FILE* fh = NULL;

	if (argc != 2) {
		VTableError_File(TYPE_ERROR_FILE_NOT_FOUND,
			"INSUFFICENT ARGUMENTS",
			"NOT FILE THE PAST AN ARGUMENT");
		return EXIT_FAILURE;
	}

	fh = fopen(argv[1], "rb");
	if (fh == NULL) {
		VTableError_File(TYPE_ERROR_FILE_NOT_FOUND,
			"NOT POSSIBLE TO OPEN THE FILE",
			argv[1]);
		return EXIT_FAILURE;
	}

	IMAGE_DOS_HEADER buffer;

	if(fread(&buffer, 32, sizeof(IMAGE_DOS_HEADER),1, fh)!= 1){
		VTableError_File(TYPE_ERROR_FILE_INVALID,
			"NOT POSSIBLE TO READ 32 BYTES FROM THE FILE",
			argv[1]);
		return EXIT_FAILURE;
	}

	fclose(fh);
	return EXIT_SUCCESS;
}


bool ispe32(IMAGE_DOS_HEADER buffer, char* argv[]) {
	//"PE\0\0" é IMAGE_NT_SIGNATURE (0x00004550)
	if (buffer.e_magic != IMAGE_DOS_SIGNATURE) {
		VTableError_File(TYPE_ERROR_FILE_INVALID,
			"FILE IS NOT A VALID DOS EXECUTABLE",
			argv[1]);
		return EXIT_FAILURE;
	}
	puts("FILE IS A VALID DOS EXECUTABLE");
	return EXIT_SUCCESS;
}
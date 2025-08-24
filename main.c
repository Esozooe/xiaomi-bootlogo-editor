#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

struct offset_size
{
	unsigned int offset;
	unsigned int size;
}__attribute__((packed));

struct header
{
	char magic[8];
	struct offset_size off_sz[(0x1000 - 8) / sizeof(struct offset_size)];
}__attribute__((packed));

int unpack(char *file_path)
{
	FILE *fd = fopen(file_path, "rb");
	if (fd == NULL) {
		perror("Open input image error");
		return -1;
	}
	// Allocate 4KB memory for header located at 0x4000
	struct header *logo_header = malloc(sizeof(struct header));
	if (logo_header == NULL) {
		perror("Error");
		return -1;
	}
	puts("Verifying logo.img");
	if (fseek(fd, 0x4000, SEEK_SET)) {
		perror("Error seeking header offset, image is too small");
		return -1;
	}
	if (fread(logo_header, 1, 0x1000, fd) != 0x1000) {
		puts("Error: corrupted file header");
		return -1;
	}

	if (memcmp(logo_header->magic, "LOGO!!!!", 8)) {
		puts("Error: invalid magic, image not supported");
		return -1;
	}

	system("mkdir unpacked");
	char *buffer = malloc(4096);
	int i;
	if (buffer == NULL) {
		perror("Error");
		return -1;
	}
	for (i = 0; i < (0x1000 - 8) / sizeof(struct offset_size); i++) {
		// Stop when there is no more bmp file inside
		if (!logo_header->off_sz[i].offset) {
			break;
		}
		// Create output bmp file
		char fname[32];
		snprintf(fname, 32, "./unpacked/logo%d.bmp", i + 1);
		FILE *out = fopen(fname, "wb+");
		if (out == NULL) {
			perror("Error creating output file");
			return -1;
		}
		// Copy current bmp file to new file
		printf("Extracting logo number %d\n", i + 1);
		// Offset and size in header are shifted right by 12
		unsigned int remain = logo_header->off_sz[i].size << 12;
		if (fseek(fd, logo_header->off_sz[i].offset << 12, SEEK_SET)) {
			perror("Error");
			return -1;
		}
		while (remain) {
			unsigned int cpy_len = remain > 4096 ? 4096 : remain;
			if (fread(buffer, 1, cpy_len, fd) != cpy_len) {
				puts("Error: logo.img ends unexpectedly, corrupted image");
				return -1;
			}
			if (fwrite(buffer, 1, cpy_len, out) != cpy_len) {
				puts("Error: write output file failed");
				return -1;
			}
			remain -= cpy_len;
		}
		if (fclose(out)) {
			perror("Error");
			return -1;
		}
	}
	// Throw error if no bmp file is found
	if (!i) {
		puts("No image found in input file");
		return -1;
	}
	if (fclose(fd)) {
		perror("Error");
		return -1;
	}
	free(buffer);
	free(logo_header);
	return 0;
}

int repack(char *output_path)
{
	FILE *fd = fopen(output_path, "wb");
	if (fd == NULL) {
		perror("Error creating output file");
		return -1;
	}
	// Allocate 4KB memory for constructing new header
	struct header *logo_header = malloc(sizeof(struct header));
	if (logo_header == NULL) {
		perror("Error");
		return -1;
	}
	memset(logo_header, 0, sizeof(struct header));
	memcpy(logo_header->magic, "LOGO!!!!", 8);
	// Offset and size in header are shifted right by 12
	logo_header->off_sz[0].offset = 0x5000 >> 12;

	int i;
	char *buffer = malloc(4096);
	for (i = 0; i < (0x1000 - 8) / sizeof(struct offset_size); i++) {
		char fname[32];
		snprintf(fname, 32, "./unpacked/logo%d.bmp", i + 1);
		FILE *input = fopen(fname, "rb");
		if (input == NULL) {
			// If read current bmp file failed, clear current offset in header which was set at last loop
			logo_header->off_sz[i].offset = 0;
			break;
		}
		if (fseek(input, 0, SEEK_END)) {
			perror("Error");
			return -1;
		}
		// Align file size to 0x1000
		logo_header->off_sz[i].size = (ftell(input) + 0xfff) >> 12;
		if (i < (0x1000 - 8) / sizeof(struct offset_size) - 1) {
			logo_header->off_sz[i + 1].offset = logo_header->off_sz[i].size + logo_header->off_sz[i].offset;
		}
		// Write current bmp file to the right offset
		if (fseek(input, 0, SEEK_SET)) {
			perror("Error");
			return -1;
		}
		if (fseek(fd, logo_header->off_sz[i].offset << 12, SEEK_SET)) {
			perror("Error");
			return -1;
		}
		printf("Packing file %s\n", fname);
		size_t fread_ret;
		do {
			fread_ret = fread(buffer, 1, 4096, input);
			if (fwrite(buffer, 1, fread_ret, fd) != fread_ret) {
				puts("Error writing output file");
				return -1;
			}
		} while (fread_ret == 4096);
		if (fclose(input)) {
			perror("Error");
			return -1;
		}
	}
	// if errno is not no such file or directory, throw error
	// if the first bmp file is not found, also throw error
	if (errno == ENOENT) {
		if (!i) {
			perror("Nothing to be packed in unpacked folder");
			return -1;
		}
	} else {
		perror("open input file error");
		return -1;
	}

	free(buffer);
	if (fseek(fd, 0x4000, SEEK_SET)) {
		perror("Error");
		return -1;
	}
	if (fwrite(logo_header, 1, sizeof(struct header), fd) != sizeof(struct header)) {
		perror("Error writing output file");
		return -1;
	}
	if (fclose(fd)) {
		perror("Error");
		return -1;
	}
	free(logo_header);
	return 0;
}

void phelp()
{
	printf(
		"\n--Logo unpack/repack tool for Xiaomi/Redmi Qualcomm devices--\n"
		"\n"
		"If you have MIUI13 or above installed, try this at your own risk\n"
		"Tested on: dipper, sagit\n"
		"\n"
		"Usage:   logotool {unpack|repack} input_output_file\n"
		"\n"
		"Example: logotool unpack logo.img\n"
		"         logotool repack logo_repacked.img\n"
	);
	return;
}

int main(int argc, char *argv[])
{
	if (argc > 2) {
		if (!strcmp(argv[1], "unpack")) {
			if (unpack(argv[2])) {
				return -1;
			}
		} else if (!strcmp(argv[1], "repack")) {
			if (repack(argv[2])) {
				return -1;
			}
		} else {
			phelp();
		}
	} else {
		phelp();
	}
	return 0;
}

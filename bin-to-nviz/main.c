// bin-to-nviz - a program that converts arbitrary binary files to .nviz visual files
// made by Nikola Whallon (https://github.com/nikolawhallon/nviz-project, nikola.whallon@gmail.com)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_COL 200
#define MAX_ROW 100
#define CH_BYTS 2

int main(int argc, char * argv[])
{
	// check for the right number of arguments
	if (argc != 7)
	{
		fprintf(stderr, "ERROR - wrong number of arguments\n");
		fprintf(stderr, "usage: %s in_file_path out_file_path columns rows frames_per_second seconds\n", argv[0]);
		return 1;
	}

	// make the bin file path
	char bin_file_path[256];
	sprintf(bin_file_path, argv[1]);

	// make the nviz file path
	char nviz_file_path[256];
	sprintf(nviz_file_path, argv[2]);

	// input columns, rows, frames_per_second, and seconds
	uint8_t col = atoi(argv[3]);
	uint8_t row = atoi(argv[4]);
	uint8_t fps = atoi(argv[5]);
	uint16_t sec = atoi(argv[6]);

	// declare and open the bin_file
	FILE * bin_file = fopen(bin_file_path, "rb");

	// check that the file could be opened
	if (bin_file == NULL)
	{
		fprintf(stderr, "ERROR - could not open %s\n", bin_file_path);
		return 1;
	}

	// calculate the file size
	fseek(bin_file, 0, SEEK_END);
	uint32_t file_size = ftell(bin_file);
	fseek(bin_file, 0, SEEK_SET);

	// a buffer for a frame
	char frame[CH_BYTS * (MAX_COL * MAX_ROW)];
	memset(frame, 48, CH_BYTS * (MAX_COL * MAX_ROW));

	// declare and open nviz_file
	FILE * nviz_file = fopen(nviz_file_path, "wb");

	// write out the video info
	fwrite(&col, 1, 1, nviz_file);
	fwrite(&row, 1, 1, nviz_file);
	fwrite(&fps, 1, 1, nviz_file);
	fwrite(&sec, 2, 1, nviz_file);

	// let user know data writing has begun
	printf("wrting data...\n");
	printf("\n");

	int i;
	for (i = 0; i < fps * sec; i++)
	{
		if (fread(frame, CH_BYTS * (MAX_COL * MAX_ROW), 1, bin_file) != EOF)
		{
			fwrite(frame, CH_BYTS * (MAX_COL * MAX_ROW), 1, nviz_file);
		}
		else
		{
			printf("the binary file was not big enough to create a %d second long nviz file\n", sec);
			printf("truncating at %f seconds\n", (float) i / (float) sec);
			break;
		}
	}

	// close nviz_file
	fclose(nviz_file);

	// close bin_file
	fclose(bin_file);

	// print nviz info
	printf("nviz file info\n");
	printf("\n");
	printf("col\t\t\t\t%d\n", col);
	printf("row\t\t\t\t%d\n", row);
	printf("fps\t\t\t\t%d\n", fps);
	printf("fno\t\t\t\t%d\n", i);
	printf("\n");

	return 0;
}

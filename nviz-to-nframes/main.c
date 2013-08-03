// nviz-to-nframes - a simple program that converts .nviz video/visual files to .nframe ascii art files
// made by Nikola Whallon (https://github.com/nikolawhallon/nviz-project, nikola.whallon@gmail.com)

#include <stdio.h>
#include <stdint.h>

#define NVIZ_HD 5
#define CH_BYTS 2
#define MAX_COL 250
#define MAX_ROW 75

//----------------------------------------------------				// GLOBAL VARIABLES

// nviz
char g_nviz_file_path[256];
int g_nviz_col;
int g_nviz_row;
int g_nviz_fps;
int g_nviz_sec;

// nframe
char g_nframe_files_base_path[256];
char g_frame[CH_BYTS * (MAX_COL * MAX_ROW)];
int g_nframe_col;
int g_nframe_row;

//----------------------------------------------------				// FUNCTIONS

int init_nviz()
{
	// declare and open the nviz file
	FILE * nviz_file = fopen(g_nviz_file_path, "rb");

	// check that the file could be opened
	if (nviz_file == NULL)
	{
		return 1;
	}

	// calculate the file size
	fseek(nviz_file, 0, SEEK_END);
	int32_t file_size = ftell(nviz_file);
	fseek(nviz_file, 0, SEEK_SET);

	// check that the file contains the nviz info
	if (file_size < NVIZ_HD)
	{
		fclose(nviz_file);
		return 1;
	}

	// input the nviz info
	fread(&g_nviz_col, 1, 1, nviz_file);
	fread(&g_nviz_row, 1, 1, nviz_file);
	fread(&g_nviz_fps, 1, 1, nviz_file);
	fread(&g_nviz_sec, 2, 1, nviz_file);

	// check that the file contains the nviz data
	if (file_size < NVIZ_HD + CH_BYTS * (g_nviz_col * g_nviz_row) * g_nviz_fps * g_nviz_sec)
	{
		fclose(nviz_file);
		return 1;
	}

	// close the nviz file
	fclose(nviz_file);
}

int main(int argc, char * argv[])
{
	// command line input
	if (argc != 3)
	{
		fprintf(stderr, "ERROR - wrong number of arguments\n");
		fprintf(stderr, "usage: %s in_file_path out_files_base_path\n", argv[0]);
		return 1;
	}

	sprintf(g_nviz_file_path, argv[1]);
	sprintf(g_nframe_files_base_path, argv[2]);

	if (init_nviz())
	{
		fprintf(stderr, "ERROR - unable to open %s\n", g_nviz_file_path);
		return 1;
	}

	int32_t f;
	for (f = 0; f < g_nviz_fps * g_nviz_sec; f++)
	{
		FILE * nviz_file = fopen(g_nviz_file_path, "rb");

		fseek(nviz_file, NVIZ_HD + CH_BYTS * (g_nviz_col * g_nviz_row) * f, SEEK_SET);
		fread(g_frame, 1, CH_BYTS * (g_nviz_col * g_nviz_row), nviz_file);

		fclose(nviz_file);

		char nframe_file_path[256];
		sprintf(nframe_file_path, "%s%d.nframe", g_nframe_files_base_path, f);

		FILE * nframe_file = fopen(nframe_file_path, "wb");

		fwrite(&g_nviz_col, 1, 1, nframe_file);
		fwrite(&g_nviz_row, 1, 1, nframe_file);
		fwrite(&g_frame, 1, CH_BYTS * (g_nviz_col * g_nviz_row), nframe_file);

		fclose(nframe_file);
	}

	return 0;
}

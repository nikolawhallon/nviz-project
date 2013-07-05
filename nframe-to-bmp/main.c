// nframe-to-bmp - a simple program that converts .nframe ascii art files to .bmp image files
// made by Nikola Whallon (https://github.com/nikolawhallon/nviz-project, nikola.whallon@gmail.com)

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAX_COL 200
#define MAX_ROW 100
#define CH_BYTS 2

#define CURSOR_W 8
#define CURSOR_H 16

typedef struct {
	uint8_t pxl[CURSOR_W * CURSOR_H];
} character;

//----------------------------------------------------				// GLOBAL VARIABLES

// defines the pixels of characters
character g_character_array[94];

// nframe
char g_nframe_file_path[256];
char g_frame[CH_BYTS * (MAX_COL * MAX_ROW)];
uint32_t g_nframe_col;
uint32_t g_nframe_row;

// bmp
char g_bmp_file_path[256];
int32_t g_px_w;
int32_t g_px_h;

//----------------------------------------------------				// FUNCTIONS

// initialize the pixel definitions of 94 ascii characters
void init_characters()
{
	// set the character array pixels to 0
	for (int i = 0; i < 94; i++)
	{
		for (int j = 0; j < CURSOR_W * CURSOR_H; j++)
		{
			g_character_array[i].pxl[j] = 0;
		}
	}

	// TO-DO - more elegant initialization
	g_character_array[1].pxl[27] = 1;
	g_character_array[1].pxl[28] = 1;
	g_character_array[1].pxl[35] = 1;
	g_character_array[1].pxl[36] = 1;
	g_character_array[1].pxl[43] = 1;
	g_character_array[1].pxl[44] = 1;
	g_character_array[1].pxl[51] = 1;
	g_character_array[1].pxl[52] = 1;
	g_character_array[1].pxl[59] = 1;
	g_character_array[1].pxl[60] = 1;
	g_character_array[1].pxl[67] = 1;
	g_character_array[1].pxl[68] = 1;

	g_character_array[1].pxl[83] = 1;
	g_character_array[1].pxl[84] = 1;
	g_character_array[1].pxl[91] = 1;
	g_character_array[1].pxl[92] = 1;

	// TO-DO - initialize all other characters as well
}

// initialize frame
int init_frame()
{
	memset(g_frame, 48, CH_BYTS * (MAX_COL * MAX_ROW));

	// declare and open the nframe file
	FILE * nframe_file = fopen(g_nframe_file_path, "rb");

	// check that the file could be opened
	if (nframe_file == NULL)
	{
		return 1;
	}

        // calculate the file size
	fseek(nframe_file, 0, SEEK_END);
	int32_t file_size = ftell(nframe_file);
	fseek(nframe_file, 0, SEEK_SET);

	// check that the file contains the nframe info
	if (file_size < 4 * 2)
	{
		fclose(nframe_file);
		return 1;
	}

	// input the nframe info
	fread(&g_nframe_col, 4, 1, nframe_file);
	fread(&g_nframe_row, 4, 1, nframe_file);

	// check that the file contains the nframe data
	if (file_size < 4 * 2 + CH_BYTS * (g_nframe_col * g_nframe_row))
	{
		fclose(nframe_file);
		return 1;
	}

	// input the nframe data
	fread(&g_frame, 1, CH_BYTS * (g_nframe_col * g_nframe_row), nframe_file);

	// close the nframe file
	fclose(nframe_file);

	return 0;
}

// main
int main(int argc, char * argv[])
{
	// command line input
	if (argc != 3)
	{
		fprintf(stderr, "ERROR - wrong number of arguments\n");
		fprintf(stderr, "usage: %s in_file_path out_file_path\n", argv[0]);

		return 1;
	}

	sprintf(g_nframe_file_path, argv[1]);
	sprintf(g_bmp_file_path, argv[2]);

	// characters
	init_characters();

	// nframe
	if (init_frame())
	{
		fprintf(stderr, "ERROR - could not open %s\n", g_nframe_file_path);

		return 1;
	}

	// bmp
	g_px_w = g_nframe_col * CURSOR_W;
	g_px_h = g_nframe_row * CURSOR_H;

	// bitmap file header
	uint16_t BM = 0x4d42;
	uint32_t file_size = 54 + g_px_w * g_px_h * 3;
	uint16_t reserve1 = 8;
	uint16_t reserve2 = 8;
	uint32_t pixel_array_start = 54;

	// bitmap information header
	uint32_t bm_info_size = 40;
	int32_t pixel_wd = g_px_w;
	int32_t pixel_ht = g_px_h;
	uint16_t color_planes = 1;
	uint16_t bits_per_pixel = 24;
	uint32_t compression = 0;
	uint32_t image_size = g_px_w * g_px_h * 3;
	int32_t hor_res = g_px_w;
	int32_t ver_res = g_px_h;
	uint32_t palette_colors = 0;
	uint32_t important_colors = 0;

	// declare and open the bmp file
	FILE * bmp_file = fopen(g_bmp_file_path, "wb");

	// write out the bitmap file header
	fwrite(&BM, 2, 1, bmp_file);
	fwrite(&file_size, 4, 1, bmp_file);
	fwrite(&reserve1, 2, 1, bmp_file);
	fwrite(&reserve2, 2, 1, bmp_file);
	fwrite(&pixel_array_start, 4, 1, bmp_file);

	// write out the bitmap information header
	fwrite(&bm_info_size, 4, 1, bmp_file);
	fwrite(&pixel_wd, 4, 1, bmp_file);
	fwrite(&pixel_ht, 4, 1, bmp_file);
	fwrite(&color_planes, 2, 1, bmp_file);
	fwrite(&bits_per_pixel, 2, 1, bmp_file);
	fwrite(&compression, 4, 1, bmp_file);
	fwrite(&image_size, 4, 1, bmp_file);
	fwrite(&hor_res, 4, 1, bmp_file);
	fwrite(&ver_res, 4, 1, bmp_file);
	fwrite(&palette_colors, 4, 1, bmp_file);
	fwrite(&important_colors, 4, 1, bmp_file);

	// bmp pixel array
	char clr;
	char chr;

	uint8_t red;
	uint8_t green;
	uint8_t blue;

	for (int r = 0; r < g_nframe_row; r++)
	{
		for (int h = 0; h < CURSOR_H; h++)
		{
			for (int c = 0; c < g_nframe_col; c++)
			{
				for (int w = 0; w < CURSOR_W; w++)
				{
					int index = CH_BYTS * ((g_nframe_row - r - 1) * g_nframe_col + c);

					clr = g_frame[index];
					chr = g_frame[index + 1];

					if (g_character_array[chr - 32].pxl[(CURSOR_H - h - 1) * CURSOR_W + w])
					{
						switch (clr)
						{
							case 1:
								red = 0;
								green = 0;
								blue = 255;
								break;
							case 2:
								red = 0;
								green = 255;
								blue = 0;
								break;
							case 3:
								red = 0;
								green = 255;
								blue = 255;
								break;
							case 4:
								red = 255;
								green = 0;
								blue = 0;
								break;
							case 5:
								red = 255;
								green = 0;
								blue = 255;
								break;
							case 6:
								red = 255;
								green = 255;
								blue = 0;
								break;
							case 7:
								red = 255;
								green = 255;
								blue = 255;
								break;
							default:
								red = 0;
								green = 0;
								blue = 0;
								break;
						}

						fwrite(&blue, 1, 1, bmp_file);
						fwrite(&green, 1, 1, bmp_file);
						fwrite(&red, 1, 1, bmp_file);
					}
					else
					{
						red = 0;
						green = 0;
						blue = 0;

						fwrite(&blue, 1, 1, bmp_file);
						fwrite(&green, 1, 1, bmp_file);
						fwrite(&red, 1, 1, bmp_file);
					}
				}
			}
		}
	}

	// close the bmp file
	fclose(bmp_file);

	return 0;
}

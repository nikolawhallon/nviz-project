// wav-to-nviz - a program that converts .wav audio files to .nviz visual files of the waveform of the audio
// made by Nikola Whallon (https://github.com/nikolawhallon/nviz-project, nikola.whallon@gmail.com)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_COL 200
#define MAX_ROW 100
#define CH_BYTS 2

int main(int argc, char * argv[])
{
	// seed random
	srand(time(NULL));

	// check for the right number of arguments
	if (argc != 7)
	{
		fprintf(stderr, "ERROR - wrong number of arguments\n");
		fprintf(stderr, "usage: %s in_file_path out_file_path columns rows frames_per_second color\n", argv[0]);
		return 1;
	}

	// make the wav file path
	char wav_file_path[256];
	sprintf(wav_file_path, argv[1]);

	// make the nviz file path
	char nviz_file_path[256];
	sprintf(nviz_file_path, argv[2]);

	// input columns, rows, frames_per_second, and color
	uint8_t col = atoi(argv[3]);
	uint8_t row = atoi(argv[4]);
	uint8_t fps = atoi(argv[5]);
	char color = atoi(argv[6]);

	// declare and open the wav_file
	FILE * wav_file = fopen(wav_file_path, "rb");

	// check that the file could be opened
	if (wav_file == NULL)
	{
		fprintf(stderr, "ERROR - could not open %s\n", wav_file_path);
		return 1;
	}

	// calculate the file size
	fseek(wav_file, 0, SEEK_END);
	uint32_t file_size = ftell(wav_file);
	fseek(wav_file, 0, SEEK_SET);

	// check that wav_file contains minimum bytes for RIFF, fmt , and data
	if (file_size < 44)
	{
		fprintf(stderr, "ERROR - %s does not contain enough bytes for RIFF, fmt , and data\n", wav_file_path);
		return 1;
	}

	// RIFF
	uint32_t ChunkID;
	uint32_t ChunkSize;
	uint32_t Format;

	// read RIFF
	fread(&ChunkID, 4, 1, wav_file);
	fread(&ChunkSize, 4, 1, wav_file);
	fread(&Format, 4, 1, wav_file);

	// check RIFF
	if (ChunkID != 0x46464952)
	{
		fprintf(stderr, "ERROR - RIFF FOURCC\n");
		return 1;
	}

	if (Format != 0x45564157)
	{
		fprintf(stderr, "ERROR - WAVE\n");
		return 1;
	}

	// print RIFF chunk info
	printf("\n");
	printf("RIFF\n");
	printf("\n");
	printf("ChunkID\t\t\t\t0x%08x\t\t%d\n", ChunkID, ChunkID);
	printf("ChunkSize\t\t\t0x%08x\t\t%d\n", ChunkSize, ChunkSize);
	printf("Format\t\t\t\t0x%08x\t\t%d\n", Format, Format);
	printf("\n");

	// fmt 
	uint32_t Subchunk1ID;
	uint32_t Subchunk1Size;
	uint16_t AudioFormat;
	uint16_t NumChannels;
	uint32_t SampleRate;
	uint32_t ByteRate;
	uint16_t BlockAlign;
	uint16_t BitsPerSample;

	// read fmt 
	fread(&Subchunk1ID, 4, 1, wav_file);
	fread(&Subchunk1Size, 4, 1, wav_file);
	fread(&AudioFormat, 2, 1, wav_file);
	fread(&NumChannels, 2, 1, wav_file);
	fread(&SampleRate, 4, 1, wav_file);
	fread(&ByteRate, 4, 1, wav_file);
	fread(&BlockAlign, 2, 1, wav_file);
	fread(&BitsPerSample, 2, 1, wav_file);

	// check fmt 
	if (Subchunk1ID != 0x20746d66)
	{
		fprintf(stderr, "ERROR - fmt  FOURCC\n");
		return 1;
	}

	// print fmt  chunk info
	printf("fmt \n");
	printf("\n");
	printf("Subchunk1ID\t\t\t0x%08x\t\t%d\n", Subchunk1ID, Subchunk1ID);
	printf("Subchunk1Size\t\t\t0x%08x\t\t%d\n", Subchunk1Size, Subchunk1Size);
	printf("AudioFormat\t\t\t0x%8x\t\t%d\n", AudioFormat, AudioFormat);
	printf("NumChannels\t\t\t0x%8x\t\t%d\n", NumChannels, NumChannels);
	printf("SampleRate\t\t\t0x%08x\t\t%d\n", SampleRate, SampleRate);
	printf("ByteRate\t\t\t0x%08x\t\t%d\n", ByteRate, ByteRate);
	printf("BlockAlign\t\t\t0x%8x\t\t%d\n", BlockAlign, BlockAlign);
	printf("BitsPerSample\t\t\t0x%8x\t\t%d\n", BitsPerSample, BitsPerSample);
	printf("\n");

	// data
	uint32_t Subchunk2ID;
	uint32_t Subchunk2Size;

	// optional chunk
	uint32_t optional_chunk_id;
	uint32_t optional_chunk_size;

	// skip optional chunks to data
	while(1)
	{
		if (fread(&optional_chunk_id, 4, 1, wav_file) != 1)
		{
			if (feof(wav_file))
			{
				fprintf(stderr, "ERROR - end of file reached and no data chunk found\n");
				return 1;
			}
			else
			{
				perror("fread ERROR");
				return 1;
			}
		}

		if (optional_chunk_id == 0x61746164)
		{
			Subchunk2ID = optional_chunk_id;
			fread(&Subchunk2Size, 4, 1, wav_file);
			break;
		}
		else if (optional_chunk_id != 0x00000000)
		{
			fread(&optional_chunk_size, 4, 1, wav_file);

			printf("optional_chunk_id\t\t0x%08x\t\t%d\n", optional_chunk_id, optional_chunk_id);
			printf("optional_chunk_size\t\t0x%08x\t\t%d\n", optional_chunk_size, optional_chunk_size);
			printf("\n");

			fseek(wav_file, optional_chunk_size, SEEK_CUR);
		}
	}

	// check that the audio information can be stored in the following bytes
	if (Subchunk2Size % (NumChannels * BitsPerSample / 8) != 0)
	{
		fprintf(stderr, "ERROR - could not parse %s\n", wav_file_path);
		fprintf(stderr, "        the number of bytes indicated by Subchunk2Size does not match the number of bytes indicated by NumChannels and BitsPerSample\n");
		return 1;
	}

	// check that the file contains enough audio words
	if (file_size - ftell(wav_file) < Subchunk2Size)	// *using ftell() here is a hack*
	{
		fprintf(stderr, "ERROR - could not parse %s\n", wav_file_path);
		fprintf(stderr, "        the number of bytes indicated by Subchunk2Size does not fit in the file\n");
		return 1;
	}

	// check that the file is 44.1 kHz / 16 bit
	if (SampleRate != 44100 || BitsPerSample != 16)
	{
		fprintf(stderr, "ERROR - this program only works with 44100 Hz / 16 bit .wav files\n");
		fprintf(stderr, "        this file is %d Hz / %d bit\n", SampleRate, BitsPerSample);
		return 1;
	}

	// print data chunk info
	printf("data\n");
	printf("\n");
	printf("Subchunk2ID\t\t\t0x%08x\t\t%d\n", Subchunk2ID, Subchunk2ID);
	printf("Subchunk2Size\t\t\t0x%08x\t\t%d\n", Subchunk2Size, Subchunk2Size);
	printf("\n");

	// left and right sample values
	int16_t l = 0;
	int16_t r = 0;

	// variables for calculations/write out
	float lpercent = 0;
	float rpercent = 0;

	int lamp = 0;
	int ramp = 0;

	// nviz info
	// col										// supplied by user, declared/defined above
	// row										// supplied by user, declared/defined above
	// fps										// supplied by user, declared/defined above
	int16_t sec = ((Subchunk2Size / (BitsPerSample / 8)) / 2) / SampleRate;		// integer number of seconds
	int32_t fno = fps * sec;							// maximum number of frames that fps goes evenly into
	int32_t frame_count = 0;							// used to truncate remaining frames

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

	// let user know data conversion has begun
	printf("converting audio data...\n");
	printf("\n");

	// input the audio data
	for (uint32_t s = 0; s < (Subchunk2Size / (BitsPerSample / 8)) / 2; s++)
	{
		// input left and right sample values
		fread(&l, 2, 1, wav_file);
		fread(&r, 2, 1, wav_file);

		// adjust for two's compliment
		if ((uint16_t) l > 32767)
		{
			l = (uint16_t) l - 65536;
		}

		if ((uint16_t) r > 32767)
		{
			r = (uint16_t) r - 65536;
		}

		// write out the waveform
		if (s % (SampleRate / fps) == 0)
		{
			// this truncates the frames from the last partial second
			if (frame_count < fno)
			{
				// shift each row up
				for (int rw = 0; rw < row - 1; rw++)
				{
					for (int cl = 0; cl < col; cl++)
					{
						frame[2 * (cl + col * rw)]     = frame[2 * (cl + col * (rw + 1))];
						frame[2 * (cl + col * rw) + 1] = frame[2 * (cl + col * (rw + 1)) + 1];
						if (frame[2 * (cl + col * rw) + 1] != ' ')
						{
							frame[2 * (cl + col * rw) + 1] = 32 + 94 * (float) rand() / (float) RAND_MAX;
						}
					}
				}

				// calculate the left amplitude
				lpercent = (float) sqrt(l * l) / 32768.0;
				lamp = (int) (col / 2 * lpercent);

				// calculate the right amplitude
				rpercent = (float) sqrt(r * r) / 32768.0;
				ramp = (int) (col / 2 * rpercent);

				// add new sample info on the bottom row
				// left channel
				for (int la = 0; la < col / 2 - lamp; la++)
				{
					int cl = la;
					frame[2 * (cl + col * (row - 1))]     = 0;
					frame[2 * (cl + col * (row - 1)) + 1] = ' ';
				}
				for (int la = 0; la < lamp; la++)
				{
					int cl = (col / 2 - lamp) + la;
					frame[2 * (cl + col * (row - 1))] = color;
					frame[2 * (cl + col * (row - 1)) + 1] = 32 + 94 * (float) rand() / (float) RAND_MAX;
				}
				// right channel
				for (int ra = 0; ra < ramp; ra++)
				{
					int cl = (col / 2) + ra;
					frame[2 * (cl + col * (row - 1))] = color;
					frame[2 * (cl + col * (row - 1)) + 1] = 32 + 94 * (float) rand() / (float) RAND_MAX;
				}
				for (int ra = 0; ra < col / 2 - ramp; ra++)
				{
					int cl = (col / 2 + ramp) + ra;
					frame[2 * (cl + col * (row - 1))]     = 0;
					frame[2 * (cl + col * (row - 1)) + 1] = ' ';
				}

				// write out the new frame
				fwrite(frame, 1, CH_BYTS * col * row, nviz_file);

				frame_count++;
			}
		}
	}

	// close nviz_file
	fclose(nviz_file);

	// close wave_file
	fclose(wav_file);

	// print nviz info
	printf("nviz file info\n");
	printf("\n");
	printf("col\t\t\t\t%d\n", col);
	printf("row\t\t\t\t%d\n", row);
	printf("fps\t\t\t\t%d\n", fps);
	printf("fno\t\t\t\t%d\n", fno);
	printf("\n");

	return 0;
}

// nframe-viewer - a simple ncurses program that views .nframe ascii art files
// made by Nikola Whallon (https://github.com/nikolawhallon/nviz-project, nikola.whallon@gmail.com)

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ncurses.h>

#define MAX_COL 200
#define MAX_ROW 100
#define CH_BYTS	2

//----------------------------------------------------				// GLOBAL VARIABLES

// control
int g_ch;
int g_running = 1;				// bool

// ncurses
int g_col, g_row;
int g_color_mode;				// bool

// nframe
char g_nframe_file_path[256];
char g_frame[CH_BYTS * (MAX_COL * MAX_ROW)];
uint32_t g_nframe_col;
uint32_t g_nframe_row;

// panels
int g_hide_panel = 0;				// bool

//----------------------------------------------------				// FUNCTIONS

// initialize color pairs
void init_color_pairs()
{
	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_CYAN, COLOR_BLACK);
	init_pair(4, COLOR_RED, COLOR_BLACK);
	init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(6, COLOR_YELLOW, COLOR_BLACK);
	init_pair(7, COLOR_WHITE, COLOR_BLACK);
}

// initialize ncurses
void init_ncurses()
{
	initscr();					// init the ncurses screen
	nonl();						// faster cursor motion, detect return key
	cbreak();					// receives ^C and like signals
	noecho();					// do not echo user input
	keypad(stdscr, TRUE);				// enable user input
	leaveok(stdscr, TRUE);				// reduced cursor motion
	curs_set(0);					// no cursor

	if (has_colors())
	{
		g_color_mode = 1;

		start_color();				// init ncurses color variables

		if (COLORS >= 8 && COLOR_PAIRS >= 8)
		{
			init_color_pairs();
		}
		else
		{
			g_color_mode = 0;
		}
	}
	else
	{
		g_color_mode = 0;
	}

	getmaxyx(stdscr, g_row, g_col);			// get the col x row of the terminal
}

// deinitialize ncurses
void deinit_ncurses()
{
	endwin();
}

// toggle_panel
void toggle_panel()
{
	g_hide_panel ^= 1;
}

// render frame
void render_frame()
{
	for (int r = 0; r < g_nframe_row; r++)
	{
		for (int c = 0; c < g_nframe_col; c++)
		{
			char chr;
			char clr;

			int32_t index = CH_BYTS * (r * g_nframe_col + c);

			clr = g_frame[index];
			chr = g_frame[index + 1];

			if (g_color_mode)
			{
				attron(COLOR_PAIR(clr));
			}

			mvaddch(r, c, chr);

			if (g_color_mode)
			{
				attroff(COLOR_PAIR(clr));
			}
		}
	}

	if (!g_hide_panel)
	{
		for (int i = 0; i < g_col; i++)
		{
			mvaddch(g_row - 4, i, '-');
		}

		// frame info
		mvprintw(g_row - 3, 0, "col x row = %d x %d", g_nframe_col, g_nframe_row);
		mvprintw(g_row - 1, 0, "file = %s", g_nframe_file_path);

		// general controls
		mvprintw(g_row - 3, g_col - 18, "q = quit nframe");
		mvprintw(g_row - 2, g_col - 18, "p = toggle panel");
	}
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
int main (int argc, char * argv[])
{
	// command line input
	if (argc < 2)
	{
		fprintf(stderr, "wrong number of arguments\n");
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		return 1;
	}

	sprintf(g_nframe_file_path, argv[1]);

	init_ncurses();

	if (init_frame())
	{
		clear();

		deinit_ncurses();

		fprintf(stderr, "could not open %s\n", g_nframe_file_path);

		return 1;
	}

	while(g_running)
	{
		// render
		render_frame();
		refresh();

		// input
		g_ch = getch();

		switch(g_ch)
		{
			case 'q':
				g_running = 0;
				break;
			case 'p':
				toggle_panel();
				break;
		}

		clear();
	}

	deinit_ncurses();

	return 0;
}

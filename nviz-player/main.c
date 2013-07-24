// nviz-player - a simple ncurses program that plays .nviz video/visualization files
// made by Nikola Whallon (https://github.com/nikolawhallon/nviz-project, nikola.whallon@gmail.com)

#include <stdio.h>
#include <stdint.h>
#include <ncurses.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>

#define NVIZ_HD 5
#define CH_BYTS	2
#define MAX_COL 250
#define MAX_ROW 75
#define MAX_FPS 96

typedef struct {
	int t_running;
	char * t_frame_pool;
	char * t_nviz_file_path;
	int t_nviz_col;
	int t_nviz_row;
	int t_nviz_fps;
	int t_nviz_sec;
	sem_t * t_switch_sem;
	sem_t * t_read_sem;
	int32_t t_read_frame_index;
} thread_info;

//----------------------------------------------------				// GLOBAL VARIABLES

// control
int g_ch;
int g_running = 1;

// nviz control
int g_paused;
int g_looping;
int g_just_reset;
int32_t g_render_frame_index;
int32_t g_rewind_fast_forward_rate;

// ncurses
int g_col, g_row;
int g_color_mode;

// frame pool
char g_frame_pool_0[CH_BYTS * (MAX_COL * MAX_ROW) * MAX_FPS];
char g_frame_pool_1[CH_BYTS * (MAX_COL * MAX_ROW) * MAX_FPS];
int g_frame_pool_rendering;
int g_frame_pool_reading;

// nviz
char g_nviz_file_path[256];
int g_nviz_col;
int g_nviz_row;
int g_nviz_fps;
int g_nviz_sec;

// panels
int g_hide_panel = 0;
int g_info_control_panel = 0;

// semaphore
sem_t * g_switch_sem;
sem_t * g_read_sem;

// thread
pthread_attr_t g_read_thread_attr;
pthread_t g_read_thread_id;
thread_info g_thread_info;

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
	nodelay(stdscr, TRUE);				// no waiting for user input
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

	clear();
}

// deinitialize ncurses
void deinit_ncurses()
{
	clear();

	endwin();
}

// toggle_panel
void toggle_panel()
{
	g_hide_panel ^= 1;

	clear();
}

// set_info_control_panel
void set_info_control_panel(int info_or_control)
{
	g_info_control_panel = info_or_control;

	clear();
}

// switch frame pools
void switch_frame_pools(int32_t _read_frame_index)
{
	sem_wait(g_switch_sem);

	g_frame_pool_rendering ^= 1;
	g_frame_pool_reading ^= 1;

	if (g_frame_pool_reading == 0)
	{
		g_thread_info.t_frame_pool = g_frame_pool_0;
	}
	else
	{
		g_thread_info.t_frame_pool = g_frame_pool_1;
	}

	g_thread_info.t_read_frame_index = _read_frame_index;

	sem_post(g_read_sem);
}

// read frames thread
static void * read_frames(void * param)
{
	thread_info * ti = (thread_info *) param;

	while (ti->t_running)
	{
		sem_wait(ti->t_read_sem);

		FILE * nviz_file = fopen(ti->t_nviz_file_path, "rb");

		fseek(nviz_file, NVIZ_HD + CH_BYTS * (ti->t_nviz_col * ti->t_nviz_row) * ti->t_read_frame_index, SEEK_SET);
		fread(ti->t_frame_pool, 1, CH_BYTS * (ti->t_nviz_col * ti->t_nviz_row) * ti->t_nviz_fps, nviz_file);

		fclose(nviz_file);

		sem_post(ti->t_switch_sem);
	}
}

// reset to render frame index
void reset_to_render_frame_index()
{
	switch_frame_pools(g_nviz_fps * (g_render_frame_index / g_nviz_fps));

	if (g_nviz_fps * (g_render_frame_index / g_nviz_fps) + g_nviz_fps < g_nviz_fps * g_nviz_sec)
	{
		switch_frame_pools(g_nviz_fps * (g_render_frame_index / g_nviz_fps) + g_nviz_fps);
	}
	else
	{
		switch_frame_pools(0);
	}

	g_just_reset = 1;
}

// initialize nviz
int init_nviz()
{
	// frame pool
	g_frame_pool_rendering = 0;
	g_frame_pool_reading = 1;

	// nviz control
	g_paused = 1;
	g_looping = 1;
	g_render_frame_index = 0;
	g_rewind_fast_forward_rate = 1;

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

	// semaphore
	g_switch_sem = sem_open("/switchsem", O_CREAT, S_IRUSR | S_IWUSR, 1);
	g_read_sem = sem_open("/readsem", O_CREAT, S_IRUSR | S_IWUSR, 0);

	// thread
	g_thread_info.t_running = 1;
	g_thread_info.t_frame_pool = g_frame_pool_1;
	g_thread_info.t_nviz_file_path = g_nviz_file_path;
	g_thread_info.t_nviz_col = g_nviz_col;
	g_thread_info.t_nviz_row = g_nviz_row;
	g_thread_info.t_nviz_fps = g_nviz_fps;
	g_thread_info.t_nviz_sec = g_nviz_sec;
	g_thread_info.t_switch_sem = g_switch_sem;
	g_thread_info.t_read_sem = g_read_sem;
	g_thread_info.t_read_frame_index = 0;

	pthread_attr_init(&g_read_thread_attr);
	pthread_attr_setstacksize(&g_read_thread_attr, 0x10000000);
	pthread_create(&g_read_thread_id, &g_read_thread_attr, &read_frames, &g_thread_info);

	// read in the first frame pool
	reset_to_render_frame_index();

	return 0;
}

// deinitialize
void deinit_nviz()
{
	g_thread_info.t_running = 0;
	sem_post(g_read_sem);
	pthread_join(g_read_thread_id, NULL);

	sem_close(g_switch_sem);
	sem_unlink("/switchsem");

	sem_close(g_read_sem);
	sem_unlink("/readsem");
}

// start/stop
void start_stop()
{
	g_paused ^= 1;

	reset_to_render_frame_index();
}

// toggle looping
void toggle_looping()
{
	g_looping ^= 1;
}

// increase the rewind/fast forward rate
void up_rewind_fast_forward_rate()
{
	if (g_rewind_fast_forward_rate == 1)
	{
		g_rewind_fast_forward_rate = g_nviz_fps;
	}
	else if (g_rewind_fast_forward_rate != g_nviz_fps * g_nviz_sec - g_nviz_fps)
	{
		g_rewind_fast_forward_rate += g_nviz_fps;
	}
}

// decrease the rewind/fast forward rate
void down_rewind_fast_forward_rate()
{
	if (g_rewind_fast_forward_rate == g_nviz_fps)
	{
		g_rewind_fast_forward_rate = 1;
	}
	else if (g_rewind_fast_forward_rate != 1)
	{
		g_rewind_fast_forward_rate -= g_nviz_fps;
	}
}

// rewind
void rewind_nviz()
{
	int32_t previous_frame_index = g_render_frame_index;

	if (g_render_frame_index - g_rewind_fast_forward_rate >= 0)
	{
		g_render_frame_index -= g_rewind_fast_forward_rate;
	}
	else
	{
		g_render_frame_index = 0;
	}

	if (g_nviz_fps * (g_render_frame_index / g_nviz_fps) != g_nviz_fps * (previous_frame_index / g_nviz_fps))
	{
		reset_to_render_frame_index();
	}
}

// fast forward
void fast_forward_nviz()
{
	int32_t previous_frame_index = g_render_frame_index;

	if (g_render_frame_index + g_rewind_fast_forward_rate < g_nviz_fps * g_nviz_sec)
	{
		g_render_frame_index += g_rewind_fast_forward_rate;
	}
	else
	{
		g_render_frame_index = g_nviz_fps * g_nviz_sec - 1;
	}

	if (g_nviz_fps * (g_render_frame_index / g_nviz_fps) != g_nviz_fps * (previous_frame_index / g_nviz_fps))
	{
		reset_to_render_frame_index();
	}
}

// render
void render()
{
	int c;
	int r;
	for (r = 0; r < g_nviz_row; r++)
	{
		for (c = 0; c < g_nviz_col; c++)
		{
			char chr;
			char clr;

			int32_t index = CH_BYTS * ((g_nviz_col * g_nviz_row) * (g_render_frame_index % g_nviz_fps) + g_nviz_col * r + c);

			if (g_frame_pool_rendering == 1)
			{
				clr = g_frame_pool_1[index];
				chr = g_frame_pool_1[index + 1];
			}
			else
			{
				clr = g_frame_pool_0[index];
				chr = g_frame_pool_0[index + 1];
			}

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
		int i;
		for (i = 0; i < g_col; i++)
		{
			mvaddch(g_row - 7, i, '-');
		}

		// video info
		if (g_info_control_panel == 0)
		{
			mvprintw(g_row - 6, 0, "col x row = %d x %d", g_nviz_col, g_nviz_row);
			mvprintw(g_row - 5, 0, "fps = %d", g_nviz_fps);
			mvprintw(g_row - 4, 0, "seconds = %d / %d\t", g_render_frame_index / g_nviz_fps, g_nviz_sec);
			mvprintw(g_row - 3, 0, "frames = %d / %d\t", g_render_frame_index, g_nviz_fps * g_nviz_sec - 1);
			mvprintw(g_row - 1, 0, "file = %s", g_nviz_file_path);
		}

		// video controls
		if (g_info_control_panel == 1)
		{
			mvprintw(g_row - 6, 0, "s = start/stop");
			mvprintw(g_row - 5, 0, "l = toggle looping");
			mvprintw(g_row - 4, 0, "r = rewind");
			mvprintw(g_row - 3, 0, "f = fast forward");
			mvprintw(g_row - 2, 0, "u = increase rewind/fast forward rate");
			mvprintw(g_row - 1, 0, "d = decrease rewind/fast forward rate");
		}

		mvprintw(g_row - 6, g_col / 2 - 12, "started = %d", !g_paused);
		mvprintw(g_row - 5, g_col / 2 - 12, "looping = %d", g_looping);
		mvprintw(g_row - 4, g_col / 2 - 12, "rewind/fast forward rate = %d", g_rewind_fast_forward_rate);

		// general controls
		mvprintw(g_row - 6, g_col - 18, "q = quit nviz");
		mvprintw(g_row - 5, g_col - 18, "i = info panel");
		mvprintw(g_row - 4, g_col - 18, "c = control panel");
		mvprintw(g_row - 3, g_col - 18, "p = toggle panel");
	}
}

// main
int main (int argc, char * argv[])
{
	// command line input
	if (argc != 2)
	{
		fprintf(stderr, "ERROR - wrong number of arguments\n");
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		return 1;
	}

	sprintf(g_nviz_file_path, argv[1]);

	// initialize ncurses
	init_ncurses();

	// initialize nviz
	if (init_nviz())
	{
		deinit_ncurses();
		fprintf(stderr, "ERROR - could not open %s\n", g_nviz_file_path);
		return 1;
	}

	while (g_running)
	{
		// input
		g_ch = getch();

		switch (g_ch)
		{
			case 'q':
				g_running = 0;
				break;
			case 'p':
				toggle_panel();
				break;
			case 'i':
				set_info_control_panel(0);
				break;
			case 'c':
				set_info_control_panel(1);
				break;
			case 's':
				start_stop();
				break;
			case 'l':
				toggle_looping();
				break;
			case 'u':
				up_rewind_fast_forward_rate();
				break;
			case 'd':
				down_rewind_fast_forward_rate();
				break;
			case 'r':
				rewind_nviz();
				break;
			case 'f':
				fast_forward_nviz();
				break;
		}

		// update
		if (!g_paused)
		{
			if (!g_just_reset)
			{
				if (g_render_frame_index + 1 > g_nviz_fps * g_nviz_sec)
				{
					g_render_frame_index = 0;
				}

				if (g_render_frame_index % g_nviz_fps == 0)
				{
					if (g_render_frame_index + g_nviz_fps == g_nviz_fps * g_nviz_sec)
					{
						switch_frame_pools(0);
					}
					else
					{
						switch_frame_pools(g_render_frame_index + g_nviz_fps);
					}
				}
			}
			else
			{
				g_just_reset = 0;
			}

			if (!g_looping && g_render_frame_index == g_nviz_fps * g_nviz_sec - 1)
			{
				g_paused = 1;
			}
		}

		// render
		render();
		refresh();
		napms(1000 / g_nviz_fps);

		// next frame
		if (!g_paused)
		{
			g_render_frame_index++;
		}
	}

	// deinitialize nviz
	deinit_nviz();

	// deinitialize ncurses
	deinit_ncurses();

	return 0;
}

#include <stdio.h>

#define GREY_ON_BLACK 0x07

/** Set cursor position via ANSI escape code */
static int fprintf_setpos(FILE *f, int r, int c)
{
    return fprintf(f, "\033[%d;%dH", r + 1, c + 1);
}

/** Set print color (foreground) via ANSI escape code */
static int fprintf_setcolor(FILE *f, int fg)
{
    return fprintf(f, "\033[38;5;%dm", fg);
}

#define ED_CUR_TO_END   0
#define ED_CUR_TO_START 1
#define ED_SCREEN       2

/** Erase display via ANSI escape code */
static int fprintf_erasedisplay(FILE *f, int erasemode)
{
    return fprintf(f, "\033[%dJ", erasemode);
}

static int fprintf_reset(FILE *f)
{
    int res = 0;
    res += fprintf_setcolor(f, GREY_ON_BLACK);
    res += fprintf_erasedisplay(f, ED_CUR_TO_END);
    return res;
}

int screen_rows = 25;
int screen_cols = 80;

/* clang-format off */
static const char *plane_art[] =
{
    "     ___       _  ",
    " | __\\_\\_o____/_| ",
    " <[___\\_\\_-----<  ",
    " |  o'            ",
    0
};

static const char *help_text[] =
{
    "plane switches: ",
    "   -s N    set slowdown    busy wait for 2^N loops per frame ",
    "   -c N    set color       use N as color byte ",
    "   -a N    set altitude    fly at row N from bottom ",
    0
};
/* clang-format on */

static int atoi(const char *a)
{
    /* Skip whitespace. */
    while (*a == ' ') a++;

    /* Add digits. */
    int i = 0;
    for (; *a && '0' <= *a && *a <= '9'; a++) i = i * 10 + (*a - '0');
    return i;
}

static void draw_char(int r, int c, char ch, unsigned char color)
{
    if (0 <= r && r < screen_rows && 0 <= c && c < screen_cols) {
        fprintf_setpos(stdout, r, c);
        fprintf_setcolor(stdout, color);
        fprintf(stdout, "%c", ch);
    }
}

static int draw_art(const char *art[], int r, int c, unsigned char color)
{
    int width = 0;
    for (int artrow = 0; art[artrow]; artrow++) {
        const char *rowchars = art[artrow];
        for (int artcol = 0; rowchars[artcol]; artcol++) {
            draw_char(r + artrow, c + artcol, rowchars[artcol], color);
            if (artcol > width) width = artcol;
        }
    }
    return width;
}

static void delay_loop(int slowdown)
{
    unsigned delay_loops = 1 << slowdown;
    for (unsigned i = 0; i < delay_loops; i++) {
        /* Do nothing, but trick the compiler into thinking that we are doing
         * something. If the compiler can tell that this loop is useless then
         * it will simply remove it during optimization. */
        asm volatile("");
    }
}

static void
fly(const char *art[], int altitude, unsigned char color, int slowdown)
{
    int r     = screen_rows - altitude;
    int width = 0;
    for (int c = screen_cols; c >= -width; c--) {
        width = draw_art(art, r, c, color);
        delay_loop(slowdown);
    }

    fprintf_reset(stdout);
}

int main(int argc, char *argv[])
{
    const char  **plane    = plane_art;
    int           altitude = 22;
    unsigned char color    = GREY_ON_BLACK;
    int           slowdown = 24;
    int           helpmode = 0;

    /* Process command line arguments. */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case 'a': altitude = atoi(argv[i + 1]), i++; break;
            case 'c': color = atoi(argv[i + 1]), i++; break;
            case 's': slowdown = atoi(argv[i + 1]), i++; break;
            case 'e': plane = help_text; break;
            case 'h':
            default: helpmode = 1;
            }
        } else helpmode = 1;
    }

    /* Display help text if requested. */
    if (helpmode) {
        draw_art(help_text, screen_rows - altitude, 0, color);
        fprintf_reset(stdout);
        return 0;
    }

    /* Fly plane. */
    fly(plane, altitude, color, slowdown);
    return 0;
}

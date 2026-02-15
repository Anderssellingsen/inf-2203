union colorchar {
    short bits;
    struct {
        unsigned char ch;
        unsigned char color;
    };
};

#define GREY_ON_BLACK 0x07

union colorchar *screen      = (void *) 0xb8000;
int              screen_rows = 25;
int              screen_cols = 80;

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
        union colorchar *out = screen + r * screen_cols + c;
        *out                 = (union colorchar){.ch = ch, .color = color};
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
}

int _start(int argc, char *argv[])
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
        return 0;
    }

    /* Fly plane. */
    fly(plane, altitude, color, slowdown);
    return 0;
}

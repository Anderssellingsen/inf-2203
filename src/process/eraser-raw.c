#include <stddef.h> // Get size_t
#include <stdint.h> // Get uintptr_t

union colorchar {
    short bits;
    struct {
        unsigned char ch;
        unsigned char color;
    };
};

#define GREY_ON_BLACK 0x07
#define DARK_RED      0x04

union colorchar *screen      = (void *) 0xb8000;
int              screen_rows = 25;
int              screen_cols = 80;

/* clang-format off */
static const char *help_text[] =
{
    "eraser args: ",
    "   -s N    set slowdown    busy wait for 2^N loops per frame ",
    "   N:M     erase M bytes starting at address N ",
    "",
    "example: ",
    "   # Erase 0x4000 bytes starting at 5 MiB ",
    "   eraser-raw -s 22 0x500000:0x4000 ",
    "",
    "tip: ",
    "   # Use the readelf tool to see what addresses to target ",
    "   readelf -l out/i386-elf/process/eraser ",
    0
};
/* clang-format on */

static int isxdigit(int c)
{
    return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F')
           || ('a' <= c && c <= 'f');
}

static int atoi(const char *a)
{
    unsigned base = 10;

    /* Skip whitespace. */
    while (*a == ' ') a++;

    /* Check for "0x" prefix */
    if (a[0] == '0' && (a[1] == 'x' || a[1] == 'X')) base = 16, a += 2;

    /* Add digits. */
    int i = 0;
    for (; *a && isxdigit(*a); a++) {
        if ('0' <= *a && *a <= '9') i = i * base + (*a - '0');
        if ('a' <= *a && *a <= 'f') i = i * base + (*a - 'a' + 0xa);
        if ('A' <= *a && *a <= 'F') i = i * base + (*a - 'A' + 0xa);
    }
    return i;
}

static void parse_mn(const char *a, uintptr_t *m, size_t *n)
{
    if (m) *m = atoi(a);
    if (n) {
        while (*a && *a != ':') a++;
        if (*a == ':') a++;
        *n = atoi(a);
    }
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
erase(uintptr_t addr_min,
      uintptr_t addr_max,
      uintptr_t addr,
      size_t    size,
      int       slowdown)
{
    unsigned char   *byte, *endbyte = (void *) (addr + size);
    size_t           i;
    union colorchar *cc;

    size_t addrs_per_cell =
            (addr_max - addr_min) / (screen_rows * screen_cols);

    /* Scan memory area and print map of what will be erased. */
    byte = (void *) addr;
    cc   = screen + ((addr - addr_min) / addrs_per_cell);
    for (; byte < endbyte; cc++) {

        int found_data = 0;
        for (i = 0; byte < endbyte && i < addrs_per_cell; i++, byte++)
            if (*byte) found_data = 1;

        char indicator = found_data ? 'x' : '_';
        *cc = (union colorchar){.ch = indicator, .color = GREY_ON_BLACK};
        delay_loop(slowdown);
    }

    /* Erase it. */
    byte = (void *) addr;
    cc   = screen + ((addr - addr_min) / addrs_per_cell);
    for (; byte < endbyte; cc++) {

        int found_data = 0;
        for (i = 0; byte < endbyte && i < addrs_per_cell; i++, byte++)
            if (*byte) found_data = 1, *byte = 0;

        char indicator = found_data ? 'x' : '_';
        *cc            = (union colorchar){.ch = indicator, .color = DARK_RED};
        delay_loop(slowdown);
    }
}

int _start(int argc, char *argv[])
{
    int       slowdown = 20;
    int       helpmode = 0;
    uintptr_t addr_min = UINTPTR_MAX;
    uintptr_t addr_max = 0;

    /* Process command line. */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case 's': slowdown = atoi(argv[i + 1]), i++; break;
            case 'h':
            default: helpmode = 1;
            }
        } else {
            uintptr_t addr;
            size_t    size;
            parse_mn(argv[i], &addr, &size);
            if (addr < addr_min) addr_min = addr;
            if (addr + size > addr_max) addr_max = addr + size;
        };
    }

    /* Display help text if requested. */
    if (argc == 1 || helpmode) {
        draw_art(help_text, 3, 0, GREY_ON_BLACK);
        return 0;
    }

    /* Process positional command line arguments and do erasing. */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            uintptr_t addr;
            size_t    size;
            parse_mn(argv[i], &addr, &size);
            erase(addr_min, addr_max, addr, size, slowdown);
        }
    }
    return 0;
}

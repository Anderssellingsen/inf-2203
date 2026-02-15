#include <drivers/devices.h>
#include <drivers/fileformat/ascii.h>
#include <drivers/log.h>
#include <drivers/vfs.h>

#include <core/ctype.h>
#include <core/errno.h>
#include <core/macros.h>
#include <core/string.h>

#define IBUFSZ 256

struct tty {
    struct file portdev; ///< File for wrapped port device: serial or screen
    unsigned    flags;

    unsigned initialized : 1; ///< Has TTY been initialized?
    unsigned ibuf_eol    : 1; ///< Has input reached the end of a line?
    unsigned ibuf_eof    : 1; ///< Has nput reached an EOF (or CTRL-D)?

    size_t ilen;
    char   ibuf[IBUFSZ];
};

#define TTY_CT 2
static struct tty ttys[TTY_CT] = {};

#define SP_NONE    0x0000
#define SP_ENDLINE 0x0001
#define SP_ISINPUT 0x0002

struct specialchar {
    char     ch;
    char    *special_echo;
    unsigned flags;
    void (*onrecv)(struct tty *tty);
};

#define ISCOOKED(TTY) (TTY->flags & TTY_COOKED)

static int tty_open_dev(struct file *file, unsigned min)
{
    if (min > TTY_CT) return -ENODEV;

    /* Use static struct as file's device data. */
    struct tty *tty     = &ttys[min];
    file->f_driver_data = tty;

    /* If already initialzed, we're done. */
    if (tty->initialized) return 0;

    /*
     * Open inner port device based on minor number:
     *
     *  - 0 -> console (screen + keyboard)
     *  - 1 and up -> serial port 1 and up
     */
    int portres;
    if (min == 0) {
        portres = -ENODEV;
        TODO();
        log_result(portres, "init tty %d on console\n", min);
    } else {
        portres = file_open_dev(&tty->portdev, MAKEDEV(MAJ_SERIAL, min));
        log_result(portres, "init tty %d on serial %d\n", min, min);
    }
    if (portres < 0) return portres;

    /* Continue initialization. */
    tty->ilen        = 0;
    tty->initialized = 1;
    return 0;
}

/** @name Echo */
///@{

static void echoc(struct tty *tty, char ch)
{
    if (!(tty->flags & TTY_ECHO)) return;

    /* If ECHOCTL is not set, or if it is set and the character is printable,
     * print it verbatime. */
    if (!(tty->flags & TTY_ECHOCTL) || isprint(ch) || strchr("\n\r\t", ch)) {
        file_write(&tty->portdev, &ch, 1);
        return;
    }

    /* Print representation of control chars. */
    if (0x00 <= ch && ch < 0x1f) {
        /* Control chars from 0x00 to 0x1f have a caret notation
         * where the letter after the caret is the control code + 0x40.
         *  - 0x00 NULL -> ^@
         *  - 0x01 SOH  -> ^A
         *  - 0x02 STX  -> ^B
         *  - ...and so on. */
        char ctlbuf[2] = {'^', ch + 0x40};
        file_write(&tty->portdev, ctlbuf, 2);

    } else if (ch == 0x7f) {
        /* 0x7f (delete) has its own caret notation: "^?". */
        file_write(&tty->portdev, "^?", 2);

    } else {
        /* For other characters, print hex notation, e.g. "\xff" */
        file_printf(&tty->portdev, "\\x%02hhx", ch);
    }
    return;
}

static inline void echos(struct tty *tty, char *str)
{
    if (tty->flags & TTY_ECHO) file_write(&tty->portdev, str, strlen(str));
}

///@}

/** @name Input and special-character handling */
///@{

static int add_to_inbuf(struct tty *tty, char ch)
{
    if (tty->ilen == IBUFSZ) return -ENOBUFS;
    tty->ibuf[tty->ilen++] = ch;
    echoc(tty, ch);
    return 0;
}

static void backspace(struct tty *tty)
{
    if (tty->ilen == 0) return;
    tty->ilen--;
    echos(tty, "\b \b");
}

static void clearline(struct tty *tty)
{
    for (int n = tty->ilen; n; n--) backspace(tty);
}

static void on_eof(struct tty *tty)
{
    tty->ibuf_eol = 1;
    if (tty->ilen == 0) tty->ibuf_eof = 1;
}

static const struct specialchar SPECIALCHARS[] = {
        {'\n', NULL, SP_ENDLINE | SP_ISINPUT, NULL},
        {CTRL_D, "^D\n", SP_ENDLINE, on_eof},
        {'\b', NULL, SP_NONE, backspace},   // Backspace (^H)
        {'\x7f', NULL, SP_NONE, backspace}, // Delete (^?) as backspace
        {CTRL_U, NULL, SP_NONE, clearline},
};

static int tty_inchar(struct tty *tty, char ch)
{
    /* If not in "cooked" mode, simply add to buffer. */
    if (!ISCOOKED(tty)) return add_to_inbuf(tty, ch);

    /* If the current line has not been read yet, refuse new input. */
    if (tty->ibuf_eol) return -ENOBUFS;

    /* Check for special characters. */
    for (size_t i = 0; i < ARRAY_SIZE(SPECIALCHARS); i++) {
        const struct specialchar *spch = &SPECIALCHARS[i];
        if (ch != spch->ch) continue;

        /* If special char matches. */
        if (spch->flags & SP_ENDLINE) tty->ibuf_eol = 1;
        if (spch->special_echo) echos(tty, spch->special_echo);
        if (spch->onrecv) spch->onrecv(tty);
        if (spch->flags & SP_ISINPUT) return add_to_inbuf(tty, ch);
        return 0;
    }

    /* Not special. */
    return add_to_inbuf(tty, ch);
}

///@}

static ssize_t tty_read(struct file *f, void *dst, size_t count, loff_t *off)
{
    UNUSED(off);
    struct tty *tty = f->f_driver_data;

    /* Read characters from port device into buffer. */
    char *cdst    = dst;
    int   portres = -EAGAIN;
    while (tty->ilen < IBUFSZ && !tty->ibuf_eol) {
        /* Read char. */
        char ch;
        portres = file_read(&tty->portdev, &ch, 1);
        if (portres <= 0) break;

        /* Add to buffer. */
        int res = tty_inchar(tty, ch);
        if (res < 0) return res;
    }

    /* If there is no data in buffer, is it an EOF? Or just no new data? */
    if (tty->ilen == 0) {
        if (portres == 0) return 0;           // EOF on input port.
        if (ISCOOKED(tty) && tty->ibuf_eof) { // EOF character (^D).
            tty->ibuf_eof = tty->ibuf_eol = 0;
            return 0;
        }
        return -EAGAIN;
    }

    /* If waiting for rest of line, continue waiting. */
    if (ISCOOKED(tty) && !tty->ibuf_eol) return -EAGAIN;

    /* If there are characters to yield, yield them. */
    size_t retct = MIN(tty->ilen, count);
    memcpy(cdst, tty->ibuf, retct);

    /* If the whole line wasn't yielded,
     * move the remaining chars to the front of the line buffer. */
    memmove(tty->ibuf, tty->ibuf + retct, tty->ilen - retct);
    tty->ilen -= retct;
    if (tty->ilen == 0) tty->ibuf_eol = 0;
    return retct;
}

static ssize_t
tty_write(struct file *f, const void *src, size_t count, loff_t *off)
{
    UNUSED(off);
    struct tty *tty = f->f_driver_data;
    return file_write(&tty->portdev, src, count);
}

static int tty_ioctl(struct file *f, unsigned cmd, uintptr_t arg)
{
    struct tty *tty = f->f_driver_data;
    switch (cmd) {
    case TTY_GETFLAGS: *(unsigned *) arg = tty->flags; return 0;
    case TTY_SETFLAGS: tty->flags = arg; return 0;
    default: return -EINVAL;
    }
}

static struct file_operations tty_ops = {
        .name     = "tty",
        .open_dev = tty_open_dev,
        .read     = tty_read,
        .write    = tty_write,
        .ioctl    = tty_ioctl,
};

int init_driver_tty(void) { return chrdev_register(MAJ_TTY, &tty_ops); }

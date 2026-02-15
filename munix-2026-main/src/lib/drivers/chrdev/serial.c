/**
 * @file
 *
 * Driver for serial port UART
 *
 * @see
 *  - <https://wiki.osdev.org/Serial_Ports>
 */
#include <cpu.h>

#include <drivers/devices.h>
#include <drivers/vfs.h>

#include <core/errno.h>
#include <core/macros.h>
#include <core/types.h>

#include <stdarg.h>
#include <stddef.h>

static const ioport_t PORT_NOS[] = {0x3f8, 0x2f8};

/* I/O port offsets for serial port registers */

#define POFF_DATA      0
#define POFF_INTENABLE 1
#define POFF_INTID     2
#define POFF_LINECTL   3
#define POFF_MODEMCTL  4
#define POFF_LINESTAT  5
#define POFF_MODEMSTAT 6
#define POFF_SCRATCH   7

#define IE_NONE      0        ///< Disable interrupts
#define IE_RDA       (1 << 0) ///< Received Data Available
#define IE_THRE      (1 << 1) ///< Transmitter Holding Register Empty
#define IE_LINESTAT  (1 << 2) ///< Line Status
#define IE_MODEMSTAT (1 << 3) ///< Modem Status

#define LC_DB     0b00000011 ///< Data Bits
#define LC_DB5    0b00000000 ///< Data: 5 bits per character
#define LC_DB6    0b00000001 ///< Data: 6 bits per character
#define LC_DB7    0b00000010 ///< Data: 7 bits per character
#define LC_DB8    0b00000011 ///< Data: 8 bits per character
#define LC_STOP2  0b00000100 ///< Stop bits: Use 1.5 or 2 stop bits
#define LC_PARITY 0b00111000 ///< Parity Bits
#define LC_PODD   0b00001000 ///< Parity: Odd
#define LC_PEVEN  0b00011000 ///< Parity: Even
#define LC_MARK   0b00101000 ///< Parity: Mark (always one)
#define LC_PSPACE 0b00111000 ///< Parity: Space (always zero)
#define LC_BREAK  0b01000000 ///< Break Enable
#define LC_DLAB   0b10000000 ///< Divisor Latch Access Bit

#define LS_DR   (1 << 0) ///< Data Ready
#define LS_OE   (1 << 1) ///< Overflow Error
#define LS_PE   (1 << 2) ///< Parity Error
#define LS_FE   (1 << 3) ///< Framing Error
#define LS_BI   (1 << 4) ///< Break Indicator
#define LS_THRE (1 << 5) ///< Transmit Holding Register Empty
#define LS_TEMT (1 << 6) ///< Transmitter Empty
#define LS_IE   (1 << 7) ///< Impending Error

#define MC_DTR  (1 << 0) ///< Data Terminal Ready pin
#define MC_RTS  (1 << 1) ///< Request to Send pin
#define MC_OUT1 (1 << 2) ///< OUT1 pin, unused in PC implementations
#define MC_OUT2 (1 << 3) ///< OUT2 pin, used for IRQ enable in PCs
#define MC_IRQ  (1 << 3) ///< OUT2 pin, used for IRQ enable in PCs
#define MC_LOOP (1 << 4) ///< Loopback feature

struct serial {
    ioport_t port;
    unsigned flags;
};

static struct serial serials[ARRAY_SIZE(PORT_NOS)] = {};

static int serial_open_dev(struct file *file, unsigned min)
{
    /* Use device minor number as com number. */
    unsigned com_no = min;
    if (com_no < 1 || ARRAY_SIZE(PORT_NOS) < com_no) return -ENODEV;

    /* Use static struct as file's device data. */
    struct serial *s    = &serials[com_no - 1];
    file->f_driver_data = s;

    /* If already initialzed, we're done. */
    if (s->port) return 0;

    /* Initialize port. */
    s->port = PORT_NOS[com_no - 1];

    /* Do a test using the loopback feature */
    uint8_t testchar = 0x0a;
    outb(MC_RTS | MC_LOOP, s->port + POFF_MODEMCTL);
    outb(testchar, s->port);
    if (inb(s->port) != testchar) return -EIO;

    /* Return to regular operation */
    outb(MC_DTR | MC_RTS | MC_OUT1 | MC_OUT2, s->port + POFF_MODEMCTL);
    return 0;
}

static inline int check_linestat(struct serial *s, unsigned bits)
{
    return inb(s->port + POFF_LINESTAT) & bits;
}

static int serial_readch(struct serial *s)
{
    if (!check_linestat(s, LS_DR)) return -EAGAIN;
    return inb(s->port);
}

static int serial_writech(struct serial *s, char ch)
{
    while (!check_linestat(s, LS_THRE)) // Wait for send ready
        ;
    outb(ch, s->port);
    return ch;
}

static int ifilter(struct serial *s, char ch)
{
    if (s->flags & SRL_ICRNL && ch == '\r') return '\n';
    return ch;
}

/** Maximum number of filtered chars that can come from one char of output. */
#define OFILTER_MAX 2

static char *ofilter(struct serial *s, char *outbuf, char ch)
{
    if (s->flags & SRL_OCRNL && ch == '\n') *outbuf++ = '\r';
    *outbuf++ = ch;
    return outbuf;
}

static ssize_t
serial_read(struct file *f, void *dst, size_t count, loff_t *off)
{
    UNUSED(off);
    struct serial *s = f->f_driver_data;

    unsigned char *bdst = dst;
    for (size_t n = 0; n < count; n++, bdst++) {
        /* Read char from hardware. */
        int ch = serial_readch(s);
        if (ch == -EAGAIN && n) return n; // Out of data for now.
        if (ch < 0) return ch;            // Report other error.

        /* Filter input. */
        *bdst = ifilter(s, ch);
    }
    return count;
}

static ssize_t
serial_write(struct file *f, const void *src, size_t count, loff_t *off)
{
    UNUSED(off);
    struct serial *s = f->f_driver_data;

    const unsigned char *bsrc = src;
    for (size_t n = 0; n < count; n++, bsrc++) {
        /* Filter output. */
        char  outbuf[OFILTER_MAX];
        char *end = ofilter(s, outbuf, *bsrc);

        /* Write to hardware. */
        for (char *out = outbuf; out < end; out++) {
            int res = serial_writech(s, *out);
            if (res < 0) return res;
        }
    }
    return count;
}

static int serial_ioctl(struct file *f, unsigned cmd, uintptr_t arg)
{
    struct serial *s = f->f_driver_data;
    switch (cmd) {
    case SRL_GETFLAGS: *(unsigned *) arg = s->flags; return 0;
    case SRL_SETFLAGS: s->flags = arg; return 0;
    default: return -EINVAL;
    }
}

static const struct file_operations serial_ops = {
        .name     = "serial",
        .open_dev = serial_open_dev,
        .read     = serial_read,
        .write    = serial_write,
        .ioctl    = serial_ioctl,
};

int init_driver_serial(void)
{
    return chrdev_register(MAJ_SERIAL, &serial_ops);
}

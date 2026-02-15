typedef unsigned char uint8_t;

typedef short ioport_t;

static inline uint8_t inb(ioport_t port)
{
    uint8_t ret;
    asm inline volatile("in	%[port],	%[val]\n"
                        : [val] "=a"(ret)   /* A register */
                        : [port] "Nd"(port) /* D register or immediate byte */
    );
    return ret;
}

static inline void outb(uint8_t val, ioport_t port)
{
    asm inline volatile("out	%[val],	%[port]\n"
                        :
                        : [val] "a"(val),   /* A register */
                          [port] "Nd"(port) /* D register or immediate byte */
    );
}

static const ioport_t com1 = 0x3f8; ///< I/O port address for serial port
#define POFF_LINESTAT 5             ///< Line status register offset
#define LS_THRE       (1 << 5)      ///< Transmit Holding Register Empty

static void serial_writech(ioport_t port, char ch)
{
    while (!(inb(port + POFF_LINESTAT) & LS_THRE)) // Wait for send ready
        ;
    outb(ch, port);
}

static void serial_writestr(ioport_t port, const char *s)
{
    for (; *s; s++) serial_writech(port, *s);
}

int _start(int argc, char *argv[])
{
    serial_writestr(com1, "Hello, world!\r\n");

    char *progname = argc ? argv[0] : "hello";
    serial_writestr(com1, "This is the ");
    serial_writestr(com1, progname);
    serial_writestr(com1, " program speaking!\r\n");
    return 0;
}

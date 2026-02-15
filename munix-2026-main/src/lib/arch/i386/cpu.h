#ifndef CPU_X86_H
#define CPU_X86_H

#include <core/compiler.h>
#include <core/inttypes.h>

#include <stddef.h>
#include <stdint.h>

#if __i386__
typedef uint32_t ureg_t; ///< Int type for general-purpose register (32-bit)
#elif __x86_64__
typedef uint64_t ureg_t; ///< Int type for general-purpose register (64-bit)
#endif

typedef uint16_t ioport_t; ///< I/O port number

#define PRIdREG "zd"     ///< ureg format snippet: signed decimal
#define PRIuREG "zu"     ///< ureg format snippet: unsigned decimal
#define PRIxREG "zx"     ///< ureg format snippet: hex
#define FMT_REG "%#10zx" ///< ureg convenient format

#define PRIdPORT "hd"    ///< ioport format snippet: signed decimal
#define PRIuPORT "hu"    ///< ioport format snippet: unsigned decimal
#define PRIxPORT "hx"    ///< ioport format snippet: hex
#define FMT_PORT "%#4hx" ///< ioport convenient printf format

/** Read input from an I/O port */
#define IO_IN(VAL, PORT) \
    asm inline volatile( \
            "in	%[port],	%[val]\n" \
            : [val] "=a"(VAL)   /* A register */ \
            : [port] "Nd"(PORT) /* D register or immediate byte */ \
    )

/** Send output to an I/O port */
#define IO_OUT(VAL, PORT) \
    asm inline volatile( \
            "out	%[val],	%[port]\n" \
            : \
            : [val] "a"(VAL),   /* A register */ \
              [port] "Nd"(PORT) /* D register or immediate byte */ \
    )

static inline uint8_t inb(ioport_t port)
{
    uint8_t ret;
    IO_IN(ret, port);
    return ret;
}

static inline uint16_t inw(ioport_t port)
{
    uint16_t ret;
    IO_IN(ret, port);
    return ret;
}

static inline uint32_t inl(ioport_t port)
{
    uint32_t ret;
    IO_IN(ret, port);
    return ret;
}

static inline void outb(uint8_t val, ioport_t port) { IO_OUT(val, port); }
static inline void outw(uint16_t val, ioport_t port) { IO_OUT(val, port); }
static inline void outl(uint32_t val, ioport_t port) { IO_OUT(val, port); }

static inline void cpu_halt(void) { asm inline volatile("hlt"); }

#endif /* CPU_X86_H */

#ifndef INTTYPES_H
#define INTTYPES_H

#include <core/compiler.h>

#include <stdint.h>

#define PRIb32 __PRI32_PREFIX "b"
#define PRIo32 __PRI32_PREFIX "o"
#define PRIu32 __PRI32_PREFIX "u"
#define PRIx32 __PRI32_PREFIX "x"
#define PRIX32 __PRI32_PREFIX "X"

#define PRIb64 __PRI64_PREFIX "b"
#define PRIo64 __PRI64_PREFIX "o"
#define PRIu64 __PRI64_PREFIX "u"
#define PRIx64 __PRI64_PREFIX "x"
#define PRIX64 __PRI64_PREFIX "X"

#define PRIbPTR __PRIPTR_PREFIX "b"
#define PRIoPTR __PRIPTR_PREFIX "o"
#define PRIuPTR __PRIPTR_PREFIX "u"
#define PRIxPTR __PRIPTR_PREFIX "x"
#define PRIXPTR __PRIPTR_PREFIX "X"

#endif /* INTTYPES_H */

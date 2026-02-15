#!/bin/bash

INFILE="$1"
VERBOSE="TRUE"

function log {
    if [[ "$VERBOSE" == "TRUE" ]]; then
        >&2 echo "$0:" "$@"
    fi
}

# Massage syntax of attribute macros so that they don't confuse Doxygen
function filter_attrs() {
    # Skip filtering on the compile.h file itself, so Doxygen can document it.
    if [ "$(basename $INFILE)" == "compiler.h" ]; then
        log "skipping ATTR_ filter in $INFILE"
        cat
        return
    fi

    # Regex 1: Convert function-like attribute macros to simple names
    #
    #       ATTR_PRINTFLIKE(2, 3)   // <-- Doxygen thinks this is another fn
    #       int sprintf(char *s, const char *format, ...);
    #
    #       ATTR_PRINTFLIKE
    #       int sprintf(char *s, const char *format, ...);

    # Regex 2: Fully remove attributes that come after structs
    #
    #       struct tss {
    #           uint16_t debug_trap;
    #           uint16_t iomap_base;
    #       } ATTR_PACKED;          // <-- Doxygen thinks this is a variable
    #
    #       struct tss {
    #           uint16_t debug_trap;
    #           uint16_t iomap_base;
    #       };
    sed -E 's/(ATTR_\w+)\([^)]*\)/\1/g' \
        | sed -E 's/(\})\s*ATTR_[^;]*(;)/\1\2/'
}

function filter_link_warning() {
    # Remove LINK_WARNING declarations that will be mistaken for functions.
    sed -E 's/^LINK_WARNING\([^)]*\)\s*;//g'
}

function filter_metacode() {
    # Special filter for working with code in the staff repository.
    # Students can ignore it.

    # If the special "buildas" header exists, use 'unifdef' to process
    # the input with defines from this header.

    BUILDAS_HEADER="_metacode/buildas.h"
    if ! which unifdef &> /dev/null || ! [ -f "$BUILDAS_HEADER" ]; then
        # If unifdef is not installed or the buildas header is missing,
        # just pass the input through with cat.
        cat
        return
    fi

    # unifdef flags
    #   -b  Replace lines with blanks, to preserve line numbering.
    #   -t  Do not attempt to parse as C.
    if [[ "$INFILE" =~ \.[ch]$ ]]
    then flags="-b"
    else flags="-b -t"
    fi
    unifdef $flags -f "$BUILDAS_HEADER"
    [[ $? -eq 2 ]] && log "unifdef error: while processing $INFILE"
}

log "filtering $INFILE"

cat "$INFILE" \
    | filter_metacode \
    | filter_attrs \
    | filter_link_warning \

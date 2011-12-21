/*
 * This file is generated from directiv.dat
 * by directiv.pl; do not edit.
 */

#include "compiler.h"
#include <string.h>
#include "nasm.h"
#include "hashtbl.h"
#include "directiv.h"

const char * const directives[26] = 
{
    NULL,
    NULL,
    "absolute",
    "bits",
    "common",
    "cpu",
    "debug",
    "default",
    "extern",
    "float",
    "global",
    "list",
    "section",
    "segment",
    "warning",
    "sectalign",
    "export",
    "group",
    "import",
    "library",
    "map",
    "module",
    "org",
    "osabi",
    "safeseh",
    "uppercase"
};

enum directives find_directive(const char *token)
{
#define UNUSED 16383
    static const int16_t hash1[32] = {
        0,
        UNUSED,
        UNUSED,
        UNUSED,
        UNUSED,
        0,
        0,
        10,
        0,
        10,
        UNUSED,
        UNUSED,
        UNUSED,
        0,
        0,
        UNUSED,
        UNUSED,
        UNUSED,
        UNUSED,
        0,
        UNUSED,
        23,
        17,
        12,
        -15,
        14,
        UNUSED,
        UNUSED,
        21,
        -18,
        14,
        3,
    };
    static const int16_t hash2[32] = {
        0,
        UNUSED,
        UNUSED,
        UNUSED,
        0,
        UNUSED,
        6,
        0,
        20,
        2,
        0,
        29,
        0,
        0,
        3,
        UNUSED,
        UNUSED,
        UNUSED,
        8,
        22,
        9,
        1,
        UNUSED,
        UNUSED,
        UNUSED,
        UNUSED,
        1,
        5,
        UNUSED,
        UNUSED,
        -4,
        UNUSED,
    };
    uint32_t k1, k2;
    uint64_t crc;
    uint16_t ix;

    crc = crc64i(UINT64_C(0x076259c3e291c26c), token);
    k1 = (uint32_t)crc;
    k2 = (uint32_t)(crc >> 32);

    ix = hash1[k1 & 0x1f] + hash2[k2 & 0x1f];
    if (ix >= 24)
        return D_unknown;

    ix += 2;
    if (nasm_stricmp(token, directives[ix]))
        return D_unknown;

    return ix;
}

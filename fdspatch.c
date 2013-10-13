#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HDR_SIZE 16
#define DSK_SIZE 65500
#define LDR_SIZE 15
#define IDT_SIZE 5

static const size_t bufsize = HDR_SIZE + DSK_SIZE * 2;

typedef unsigned char byte_t;

static const byte_t header_1d [ HDR_SIZE ] = {
    0x46, 0x44, 0x53, 0x1a, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const byte_t header_2d [ HDR_SIZE ] = {
    0x46, 0x44, 0x53, 0x1a, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const byte_t leader [ LDR_SIZE ] = {
    0x01, 0x2a, 0x4e, 0x49, 0x4e, 0x54, 0x45, 0x4e,
    0x44, 0x4f, 0x2d, 0x48, 0x56, 0x43, 0x2a };

typedef enum
{
    ACTION_PRINT,
    ACTION_CONVERT
} action_t;

typedef struct
{
    int num_dsk;
    int has_hdr;
    int vld_hdr;
    int vld_ldr;
    int vendor;
    char ident [ IDT_SIZE ];
    int version;
} class_t;

typedef struct
{
    action_t action;
} flags_t;

static struct option longopts[] =
{
    { "print",    no_argument, NULL, 'p' },
    { "convert",  no_argument, NULL, 'c' },
    { NULL,       0,           NULL, 0   }
};

void usage ( )
{
    fprintf ( stderr,
        "\nUsage: fdspatch [options] file\n"
        "Options:\n"
        "  --print,-p     Display information about the FDS file. This is\n"
        "                 the default action.\n"
        "  --convert, -c  Convert (patch) the FDS file if required, by\n"
        "                 adding the FDSLoader header. Files that already\n"
        "                 have this header, or that are considered invalid,\n"
        "                 will be left unchanged.\n" );
}

int get_options ( int argc, char** argv, flags_t* flags )
{
    int ch;
    flags->action = ACTION_PRINT;
    while ( ( ch = getopt_long ( argc, argv, "pc", longopts, NULL ) ) != -1 )
        switch ( ch )
        {
            case 'p':
                flags->action = ACTION_PRINT;
                break;
            case 'c':
                flags->action = ACTION_CONVERT;
                break;
            default:
                usage ( );
                return 1;
        }

    if ( optind + 1 != argc )
    {
        fprintf (
            stderr, "fdspatch: must provide one and only one file name\n" );
        usage ( );
        return 1;
    }

    return 0;
}

byte_t* alloc_buffer ( )
{
    byte_t* buf;
    if ( ! ( buf = ( byte_t* ) calloc ( bufsize, 1 ) ) )
        fprintf (
            stderr, "Failed to allocate disk buffer, less than %zd bytes "
                    "available\n", bufsize );
    return buf;
}

FILE* open_input_file ( const char * fn )
{
    FILE* fh;
    if ( ( fh = fopen ( fn, "rb" ) ) )
        return fh;
    fprintf ( stderr, "Failed to open input file: %s\n", fn );
    return 0;
}

size_t load_file ( byte_t* buf, FILE* fh )
{
    size_t len;
    byte_t lbuf;
    len = fread ( buf, 1, bufsize, fh );
    fclose ( fh );
    if ( len < bufsize )
        return len;
    if ( fread ( &lbuf, 1, 1, fh ) )
    {
        fprintf (
            stderr, "File too long for buffer, valid FDS images must be %zd"
                    "or fewer\nbytes in length\n", bufsize );
        return 0;
    }
    return len;
}

int classify_image ( class_t* cls, const byte_t* buf, size_t len )
{
    int idx, offset;

    switch ( len )
    {
        case DSK_SIZE:
            cls->num_dsk = 1;
            cls->has_hdr = 0;
            break;
        case DSK_SIZE * 2:
            cls->num_dsk = 2;
            cls->has_hdr = 0;
            break;
        case HDR_SIZE + DSK_SIZE:
            cls->num_dsk = 1;
            cls->has_hdr = 1;
            break;
        case HDR_SIZE + DSK_SIZE * 2:
            cls->num_dsk = 2;
            cls->has_hdr = 2;
            break;
        default:
            fprintf (
                stderr, "Image has unrecognised size, should be a multiple "
                        "of %d\nbytes with an optional %d byte header;"
                        " actual size is\n%zd bytes\n",
                        DSK_SIZE, HDR_SIZE, len );
            return 1;
    }

    cls->vld_hdr = 0;
    if ( cls->has_hdr )
    {
        const byte_t* header = cls->num_dsk == 2 ? header_2d : header_1d;
        if ( ! memcmp ( header, buf, HDR_SIZE ) )
            cls->vld_hdr = 1;
    }

    offset = cls->has_hdr ? offset = HDR_SIZE : 0;
    cls->vld_ldr = memcmp ( leader, buf + offset, LDR_SIZE ) == 0;

    offset += LDR_SIZE;
    cls->vendor = ( int ) buf [ offset ];

    memcpy ( cls->ident, buf + offset + 1, IDT_SIZE );
    cls->ident [ IDT_SIZE - 1 ] = 0;
    cls->version = ( int ) buf [ offset + IDT_SIZE ];
    return 0;
}

const char * bool_to_str ( int val )
{
    return val ? "yes" : "no";
}

int action_print ( const class_t* cls )
{
    printf ( "Disk count: %d\n"
             "FDS header expected: %s\n",
             cls->num_dsk,
             bool_to_str ( cls->has_hdr ) );
    if ( cls->has_hdr )
        printf ( "FDS header valid: %s\n", bool_to_str ( cls->vld_hdr ) );
    printf ( "Image header valid: %s\n", bool_to_str ( cls->vld_ldr ) );
    if ( cls->vld_ldr )
        printf ( "Game vendor id: %d\n"
                 "Game ident: \"%s\"\n"
                 "Game version: %d\n",
                 cls->vendor,
                 cls->ident,
                 cls->version );
    return 0;
}

int main ( int argc, char** argv )
{
    FILE* fh;
    byte_t* buf;
    size_t flen;
    class_t cls;
    flags_t flags;
    if ( get_options ( argc, argv, &flags ) )
        return 1;
    if ( ! ( buf = alloc_buffer ( ) ) )
        return 1;
    if ( ! ( fh = open_input_file ( argv [ argc - 1 ] ) ) )
        return 1;
    if ( ! ( flen = load_file ( buf, fh ) ) )
        return 1;
    if ( classify_image ( &cls, buf, flen ) )
        return 1;

    if ( flags.action == ACTION_CONVERT )
    {
    }
    else
        return action_print ( &cls );
}


#ifndef _VARDEF_H_
#define _VARDEF_H_

/* 
 * sizeof (int) == 4;
 * sizeof (short) == 2;
 * sizeof (char) == 1;
 */

/* for windows data type compatible */
#define _(text) text
#define  TEXT(text)	text

typedef int     LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned char BYTE;
typedef char    TCHAR;

typedef int     INT32;
typedef short   INT16;
typedef char    INT8;
typedef unsigned int UNIT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

typedef int BOOL;

#define TRUE   1
#define FALSE  0


typedef struct {
    const char     *program_name;
    const char     *package;
    const char     *version;
    const char     *authors;
    INT32           baudrate;
    char           *device;
    INT32           debug;
    INT32           echo;
    INT32           port;
    INT32           databit;
    char           *stopbit;
    char            parity;
} Arguments;

extern Arguments args;


/* some useful macros */
#ifndef MAX
# define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
# define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#endif                          /*vardef.h */

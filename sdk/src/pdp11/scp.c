/* Simulator control program

   Copyright (c) 1993-1998,
   Robert M Supnik, Digital Equipment Corporation
   Commercial use prohibited

   20-Aug-98	RMS	Added radix commands
   05-Jun-98	RMS	Fixed bug in ^D handling for UNIX
   10-Apr-98	RMS	Added switches to all commands
   26-Oct-97	RMS	Added search capability
   25-Jan-97	RMS	Revised data types
   23-Jan-97	RMS 	Added bi-endian I/O
   06-Sep-96	RMS	Fixed bug in variable length IEXAMINE
   16-Jun-96	RMS	Changed interface to parse/print_sym
   06-Apr-96	RMS	Added error checking in reset all
   07-Jan-96	RMS	Added register buffers in save/restore
   11-Dec-95	RMS	Fixed ordering bug in save/restore
   22-May-95	RMS	Added symbolic input
   13-Apr-95	RMS	Added symbolic printouts
*/

#ifdef SANOS
#include <os.h>
#endif

#define SCP	1					/* defining module */
#include "sim_defs.h"
#include <limits.h>
#include <signal.h>
#include <ctype.h>
#define EX_D	0					/* deposit */
#define EX_E	1					/* examine */
#define EX_I	2					/* interactive */
#define SCH_OR	0					/* search logicals */
#define SCH_AND	1
#define SCH_XOR	2
#define SCH_E	0					/* search booleans */
#define SCH_N	1
#define SCH_G	2
#define SCH_L	3
#define SCH_EE	4
#define SCH_NE	5
#define SCH_GE	6
#define SCH_LE	7
#define SWHIDE	(1u << 26)				/* enable hiding */
#define RU_RUN	0					/* run */
#define RU_GO	1					/* go */
#define RU_STEP 2					/* step */
#define RU_CONT 3					/* continue */
#define RU_BOOT 4					/* boot */
#define UPDATE_SIM_TIME(x) sim_time = sim_time + (x - sim_interval); \
	x = sim_interval

extern char sim_name[];
extern DEVICE *sim_devices[];
extern REG *sim_PC;
extern char *sim_stop_messages[];
extern t_stat sim_instr (void);
extern t_stat sim_load (FILE *ptr);
extern int32 sim_emax;
extern t_stat print_sym (t_addr addr, t_value *val, UNIT *uptr, int32 sw);
extern t_stat parse_sym (char *cptr, t_addr addr, UNIT *uptr, t_value *val,
	int32 sw);
extern t_stat ttinit (void);
extern t_stat ttrunstate (void);
extern t_stat ttcmdstate (void);
extern t_stat ttclose (void);
UNIT *sim_clock_queue = NULL;
int32 sim_interval = 0;
int32 sim_switches = 0;
static double sim_time;
static int32 noqueue_time;
volatile int32 stop_cpu = 0;
t_value *sim_eval = NULL;
int32 sim_end = 1;				/* 1 = little, 0 = big */
unsigned char sim_flip[FLIP_SIZE];

#define print_val(a,b,c,d) fprint_val (stdout, (a), (b), (c), (d))
#define SZ_D(dp) (size_map[((dp) -> dwidth + CHAR_BIT - 1) / CHAR_BIT])
#define SZ_R(rp) \
	(size_map[((rp) -> width + (rp) -> offset + CHAR_BIT - 1) / CHAR_BIT])
#define GET_SWITCHES(cp,gb) \
	for (sim_switches = 0; *cp == '-'; ) { \
		int32 lsw; \
		cp = get_glyph (cp, gb, 0); \
		if ((lsw = get_switches (gb)) <= 0) return SCPE_ARG; \
		sim_switches = sim_switches | lsw;  }
#define GET_RADIX(val,dft) \
	if (sim_switches & SWMASK ('O')) val = 8; \
	else if (sim_switches & SWMASK ('D')) val = 10; \
	else if (sim_switches & SWMASK ('H')) val = 16; \
	else val = dft;

int32 get_switches (char *cptr);
t_value get_rval (REG *rptr, int idx);
void put_rval (REG *rptr, int idx, t_value val, t_value mask);
t_stat get_aval (t_addr addr, DEVICE *dptr, UNIT *uptr);
t_stat fprint_val (FILE *stream, t_value val, int rdx, int wid, int fmt);
char *read_line (char *ptr, int size, FILE *stream);
DEVICE *find_device (char *ptr, int32 *iptr);
DEVICE *find_dev_from_unit (UNIT *uptr);
REG *find_reg (char *ptr, char **optr, DEVICE *dptr);
t_stat detach_all (int start_device);
t_stat ex_reg (t_value val, int flag, REG *rptr);
t_stat dep_reg (int flag, char *cptr, REG *rptr);
t_stat ex_addr (int flag, t_addr addr, DEVICE *dptr, UNIT *uptr);
t_stat dep_addr (int flag, char *cptr, t_addr addr, DEVICE *dptr,
	UNIT *uptr, int dfltinc);
SCHTAB *get_search (char *cptr, DEVICE *dptr, SCHTAB *schptr);
int test_search (t_value val, SCHTAB *schptr);
t_stat step_svc (UNIT *ptr);

UNIT step_unit = { UDATA (&step_svc, 0, 0)  };
const char *scp_error_messages[] = {
	"Address space exceeded",
	"Unit not attached",
	"I/O error",
	"Checksum error",
	"Format error",
	"Unit not attachable",
	"File open error",
	"Memory exhausted",
	"Invalid argument",
	"Step expired",
	"Unknown command",
	"Read only argument",
	"Command not completed",
	"Simulation stopped",
	"Goodbye",
	"Console input I/O error",
	"Console output I/O error",
	"End of file",
	"Relocation error",
	"No settable parameters",
	"Unit already attached"  };

const size_t size_map[] = { sizeof (int8),
	sizeof (int8), sizeof (int16), sizeof (int32), sizeof (int32)
#if defined (int64)
	, sizeof (int64), sizeof (int64), sizeof (int64), sizeof (int64)
#endif
};

const t_value width_mask[] = { 0,
	0x1, 0x3, 0x7, 0xF,
	0x1F, 0x3F, 0x7F, 0xFF,
	0x1FF, 0x3FF, 0x7FF, 0xFFF,
	0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
	0x1FFFF, 0x3FFFF, 0x7FFFF, 0xFFFFF,
	0x1FFFFF, 0x3FFFFF, 0x7FFFFF, 0xFFFFFF,
	0x1FFFFFF, 0x3FFFFFF, 0x7FFFFFF, 0xFFFFFFF,
	0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
#if defined (int64)
	, 0x1FFFFFFFF, 0x3FFFFFFFF, 0x7FFFFFFFF, 0xFFFFFFFFF,
	0x1FFFFFFFFF, 0x3FFFFFFFFF, 0x7FFFFFFFFF, 0xFFFFFFFFFF,
	0x1FFFFFFFFFF, 0x3FFFFFFFFFF, 0x7FFFFFFFFFF, 0xFFFFFFFFFFF,
	0x1FFFFFFFFFFF, 0x3FFFFFFFFFFF, 0x7FFFFFFFFFFF, 0xFFFFFFFFFFFF,
	0x1FFFFFFFFFFFF, 0x3FFFFFFFFFFFF, 0x7FFFFFFFFFFFF, 0xFFFFFFFFFFFFF,
	0x1FFFFFFFFFFFFF, 0x3FFFFFFFFFFFFF, 0x7FFFFFFFFFFFFF, 0xFFFFFFFFFFFFFF,
	0x1FFFFFFFFFFFFFF, 0x3FFFFFFFFFFFFFF,
		0x7FFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFF,
	0x1FFFFFFFFFFFFFFF, 0x3FFFFFFFFFFFFFFF,
		 0x7FFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF
#endif
};

int main (int argc, char *argv[])
{
char cbuf[CBUFSIZE], gbuf[CBUFSIZE], *cptr;
int32 i, stat;
FILE *fpin;
union {int32 i; char c[sizeof (int32)]; } end_test;
t_stat reset_cmd (int flag, char *ptr);
t_stat exdep_cmd (int flag, char *ptr);
t_stat load_cmd (int flag, char *ptr);
t_stat run_cmd (int flag, char *ptr);
t_stat attach_cmd (int flag, char *ptr);
t_stat detach_cmd (int flag, char *ptr);
t_stat save_cmd (int flag, char *ptr);
t_stat restore_cmd (int flag, char *ptr);
t_stat exit_cmd (int flag, char *ptr);
t_stat set_cmd (int flag, char *ptr);
t_stat show_cmd (int flag, char *ptr);
t_stat add_cmd (int flag, char *ptr);
t_stat remove_cmd (int flag, char *ptr);
t_stat help_cmd (int flag, char *ptr);

static CTAB cmd_table[] = {
	{ "RESET", &reset_cmd, 0 },
	{ "EXAMINE", &exdep_cmd, EX_E },
	{ "IEXAMINE", &exdep_cmd, EX_E+EX_I },
	{ "DEPOSIT", &exdep_cmd, EX_D },
	{ "IDEPOSIT", &exdep_cmd, EX_D+EX_I },
	{ "RUN", &run_cmd, RU_RUN },
	{ "GO", &run_cmd, RU_GO }, 
	{ "STEP", &run_cmd, RU_STEP },
	{ "CONT", &run_cmd, RU_CONT },
	{ "BOOT", &run_cmd, RU_BOOT },
	{ "ATTACH", &attach_cmd, 0 },
	{ "DETACH", &detach_cmd, 0 },
	{ "SAVE", &save_cmd, 0 },
	{ "RESTORE", &restore_cmd, 0 },
	{ "GET", &restore_cmd, 0 },
	{ "LOAD", &load_cmd, 0 },
	{ "EXIT", &exit_cmd, 0 },
	{ "QUIT", &exit_cmd, 0 },
	{ "BYE", &exit_cmd, 0 },
	{ "SET", &set_cmd, 0 },
	{ "SHOW", &show_cmd, 0 },
	{ "ADD", &add_cmd, 0 },
	{ "REMOVE", &remove_cmd, 0 },
	{ "HELP", &help_cmd, 0 },
	{ NULL, NULL, 0 }  };

/* Main command loop */

/* printf ("%s simulator V2.3d\n", sim_name); */
end_test.i = 1;					/* test endian-ness */
sim_end = end_test.c[0];
if (sim_emax <= 0) sim_emax = 1;
if ((sim_eval = calloc (sim_emax, sizeof (t_value))) == NULL) {
	printf ("Unable to allocate examine buffer\n");
	return 0;  };
if ((stat = ttinit ()) != SCPE_OK) {
	printf ("Fatal terminal initialization error\n%s\n",
		scp_error_messages[stat - SCPE_BASE]);
	return 0;  }
stop_cpu = 0;
sim_interval = 0;
sim_time = 0;
noqueue_time = 0;
sim_clock_queue = NULL;
if ((stat = reset_all (0)) != SCPE_OK) {
	printf ("Fatal simulator initialization error\n%s\n",
		scp_error_messages[stat - SCPE_BASE]);
	return 0;  }

if ((argc > 1) && (argv[1] != NULL) &&
    ((fpin = fopen (argv[1], "rt")) != NULL)) {		/* command file? */
	do {	cptr = read_line (cbuf, CBUFSIZE, fpin);
		if (cptr == NULL) break;		/* exit on eof */
		if (*cptr == 0) continue;		/* ignore blank */
		cptr = get_glyph (cptr, gbuf, 0);	/* get command glyph */
		for (i = 0; cmd_table[i].name != NULL; i++) {
		    if (MATCH_CMD (gbuf, cmd_table[i].name) == 0) {
			stat = cmd_table[i].action (cmd_table[i].arg, cptr);
			break;  }  }
		if (stat >= SCPE_BASE)
			printf ("%s\n", scp_error_messages[stat - SCPE_BASE]);
	} while (stat != SCPE_EXIT);  }			/* end if cmd file */

do {	printf ("sim> ");				/* prompt */
	cptr = read_line (cbuf, CBUFSIZE, stdin);	/* read command line */
	stat = SCPE_UNK;
	if (cptr == NULL) continue;			/* ignore EOF */
	if (*cptr == 0) continue;			/* ignore blank */
	cptr = get_glyph (cptr, gbuf, 0);		/* get command glyph */
	for (i = 0; cmd_table[i].name != NULL; i++) {
		if (MATCH_CMD (gbuf, cmd_table[i].name) == 0) {
			stat = cmd_table[i].action (cmd_table[i].arg, cptr);
			break;  }  }
	if (stat >= SCPE_BASE)
		printf ("%s\n", scp_error_messages[stat - SCPE_BASE]);
} while (stat != SCPE_EXIT);

detach_all (0);
ttclose ();
return 0;
}

/* Exit command */

t_stat exit_cmd (int flag, char *cptr)
{
return SCPE_EXIT;
}

/* Help command */

t_stat help_cmd (int flag, char *cptr)
{
printf ("r{eset} {ALL|<device>}   reset simulator\n");
printf ("e{xamine} <list>         examine memory or registers\n");
printf ("ie{xamine} <list>        interactive examine memory or registers\n");
printf ("d{eposit} <list> <val>   deposit in memory or registers\n");
printf ("id{eposit} <list>        interactive deposit in memory or registers\n");
printf ("l{oad} <file>            load binary file\n");
printf ("ru{n} {new PC}           reset and start simulation\n");
printf ("go {new PC}              start simulation\n");
printf ("c{ont}                   continue simulation\n");
printf ("s{tep} {n}               simulate n instructions\n");
printf ("b{oot} <device>|<unit>   bootstrap device\n");
printf ("at{tach} <unit> <file>   attach file to simulated unit\n");
printf ("det{ach} <unit>          detach file from simulated unit\n");
printf ("sa{ve} <file>            save simulator to file\n");
printf ("rest{ore}|ge{t} <file>   restore simulator from file\n");
printf ("exi{t}|q{uit}|by{e}      exit from simulation\n");
printf ("set <unit> <val>         set unit parameter\n");
printf ("show <device>            show device parameters\n");
printf ("sh{ow} c{onfiguration}   show configuration\n");
printf ("sh{ow} q{ueue}           show event queue\n");
printf ("sh{ow} t{ime}            show simulated time\n");
printf ("ad{d} <unit>             add unit to configuration\n");
printf ("rem{ove} <unit>          remove unit from configuration\n");
printf ("h{elp}                   type this message\n");
return SCPE_OK;
}

/* Set command */

t_stat set_cmd (int flag, char *cptr)
{
int32 i, unitno;
t_stat r;
char gbuf[CBUFSIZE];
DEVICE *dptr;
UNIT *uptr;
MTAB *mptr;
t_stat set_radix (DEVICE *dptr, int flag);
static CTAB set_table[] = {
	{ "OCTAL", &set_radix, 8 },
	{ "DECIMAL", &set_radix, 10 },
	{ "HEX", &set_radix, 16 },
	{ NULL, NULL, 0 }  };

GET_SWITCHES (cptr, gbuf);				/* test for switches */
cptr = get_glyph (cptr, gbuf, 0);			/* get next glyph */
dptr = find_device (gbuf, &unitno);			/* find device */
if ((dptr == NULL) || (*cptr == 0)) return SCPE_ARG;	/* argument? */
cptr = get_glyph (cptr, gbuf, 0);			/* get glyph */
if (*cptr != 0) return SCPE_ARG;			/* now eol? */
uptr = dptr -> units + unitno;
if (uptr -> flags & UNIT_DIS) return SCPE_ARG;		/* disabled? */
for (i = 0; set_table[i].name != NULL; i++) {		/* check globals */
	if (MATCH_CMD (gbuf, set_table[i].name) == 0)
		return set_table[i].action (dptr, set_table[i].arg);  }
if (dptr -> modifiers == NULL) return SCPE_NOPARAM;	/* any modifiers? */
for (mptr = dptr -> modifiers; mptr -> mask != 0; mptr++) {
	if ((mptr -> mstring != NULL) &&
	    (MATCH_CMD (gbuf, mptr -> mstring) == 0)) {
		if ((mptr -> valid != NULL) &&
		   ((r = mptr -> valid (uptr, mptr -> match)) != SCPE_OK))
			return r;			/* invalid? */
		uptr -> flags = (uptr -> flags & ~(mptr -> mask)) |
			(mptr -> match & mptr -> mask);	/* set new value */
		return SCPE_OK;  }  }
return SCPE_ARG;					/* no match */
}

/* Set radix routine */

t_stat set_radix (DEVICE *dptr, int flag)
{
dptr -> dradix = flag & 017;
return SCPE_OK;
}

/* Show command */

t_stat show_cmd (int flag, char *cptr)
{
int32 i;
char gbuf[CBUFSIZE];
DEVICE *dptr;
t_stat show_config (int flag);
t_stat show_queue (int flag);
t_stat show_time (int flag);
t_stat show_device (DEVICE *dptr);
static CTAB show_table[] = {
	{ "CONFIGURATION", &show_config, 0 },
	{ "QUEUE", &show_queue, 0 },
	{ "TIME", &show_time, 0 },
	{ NULL, NULL, 0 }  };

GET_SWITCHES (cptr, gbuf);				/* test for switches */
cptr = get_glyph (cptr, gbuf, 0);			/* get next glyph */
if (*cptr != 0) return SCPE_ARG;			/* now eol? */
for (i = 0; show_table[i].name != NULL; i++) {		/* find command */
	if (MATCH_CMD (gbuf, show_table[i].name) == 0)
		return show_table[i].action (show_table[i].arg);  }
dptr = find_device (gbuf, NULL);			/* find device */
if (dptr == NULL) return SCPE_ARG;
return show_device (dptr);
}

/* Show processors */

t_stat show_device (DEVICE *dptr)
{
int32 j, ucnt;
UNIT *uptr;
MTAB *mptr;

printf ("%s", dptr -> name);
for (j = ucnt = 0; j < dptr -> numunits; j++) {
	uptr = (dptr -> units) + j;
	if (!(uptr -> flags & UNIT_DIS)) ucnt++;  }
if (ucnt == 0) printf (", all units disabled\n");
if (ucnt > 1) printf (", %d units\n", ucnt);
for (j = 0; j < dptr -> numunits; j++) {
	uptr = (dptr -> units) + j;
	if (uptr -> flags & UNIT_DIS) continue;
	if (ucnt > 1) printf ("  unit %d", j);
	if (uptr -> flags & UNIT_FIX)
		printf (", %dK%s",
		uptr -> capac / ((uptr -> flags & UNIT_BINK)? 1024: 1000),
		((dptr -> dwidth / dptr -> aincr) > 8)? "W": "B");
	if (uptr -> flags & UNIT_ATT)
		printf (", attached to %s", uptr -> filename);
	else if (uptr -> flags & UNIT_ATTABLE) printf (", not attached");
	if (dptr -> modifiers != NULL) {
		for (mptr = dptr -> modifiers; mptr -> mask != 0; mptr++) {
			if ((mptr -> pstring != NULL) &&
			   ((uptr -> flags & mptr -> mask) == mptr -> match))
				printf (", %s", mptr -> pstring);  }  }
	printf ("\n");  }
return SCPE_OK;
}

t_stat show_config (int flag)
{
int32 i;
DEVICE *dptr;

printf ("%s simulator configuration\n\n", sim_name);
for (i = 0; (dptr = sim_devices[i]) != NULL; i++) show_device (dptr);
return SCPE_OK;
}

t_stat show_queue (int flag)
{
DEVICE *dptr;
UNIT *uptr;
int32 accum;

if (sim_clock_queue == NULL) {
	printf ("%s event queue empty, time = %-16.0f\n", sim_name, sim_time);
	return SCPE_OK;  }
printf ("%s event queue status, time = %-16.0f\n", sim_name, sim_time);
accum = 0;
for (uptr = sim_clock_queue; uptr != NULL; uptr = uptr -> next) {
	if (uptr == &step_unit) printf ("  Step timer");
	else if ((dptr = find_dev_from_unit (uptr)) != NULL) {
		printf ("  %s", dptr -> name);
		if (dptr -> numunits > 1) printf (" unit %d",
			uptr - dptr -> units);  }
	else printf ("  Unknown");
	printf (" at %d\n", accum + uptr -> time);
	accum = accum + uptr -> time;  }
return SCPE_OK;
}

t_stat show_time (int flag)
{
printf ("Time:	%-16.0f\n", sim_time);
return SCPE_OK;
}

/* Add and remove commands and routines

   ad[d]		add unit to configuration
   rem[ove]		remove unit from configuration
*/

t_stat add_cmd (int flag, char *cptr)
{
int32 unitno;
char gbuf[CBUFSIZE];
DEVICE *dptr;
UNIT *uptr;

GET_SWITCHES (cptr, gbuf);				/* test for switches */
cptr = get_glyph (cptr, gbuf, 0);			/* get next glyph */
dptr = find_device (gbuf, &unitno);			/* locate device */
if ((dptr == NULL) || (*cptr != 0)) return SCPE_ARG;	/* found it? */
uptr = dptr -> units + unitno;				/* locate unit */
if ((uptr -> flags & UNIT_DISABLE) && (uptr -> flags & UNIT_DIS)) {
	uptr -> flags = uptr -> flags & ~UNIT_DIS;	/* enable it */
	return SCPE_OK;  }
return SCPE_ARG;					/* not valid */
}

t_stat remove_cmd (int flag, char *cptr)
{
int32 unitno;
char gbuf[CBUFSIZE];
DEVICE *dptr;
UNIT *uptr;

GET_SWITCHES (cptr, gbuf);				/* test for switches */
cptr = get_glyph (cptr, gbuf, 0);			/* get next glyph */
dptr = find_device (gbuf, &unitno);			/* locate device */
if ((dptr == NULL) || (*cptr != 0)) return SCPE_ARG;	/* found it? */
uptr = dptr -> units + unitno;				/* locate unit */
if ((uptr -> flags & UNIT_DISABLE) && !(uptr -> flags & UNIT_DIS) &&
   !(uptr -> flags & UNIT_ATT) && !sim_is_active (uptr)) {
	uptr -> flags = uptr -> flags | UNIT_DIS;	/* disable it */
	return SCPE_OK;  }
return SCPE_ARG;					/* not valid */
}

/* Reset command and routines

   re[set]		reset all devices
   re[set] all		reset all devices
   re[set] device	reset specific device
*/

t_stat reset_cmd (int flag, char *cptr)
{
char gbuf[CBUFSIZE];
DEVICE *dptr;

GET_SWITCHES (cptr, gbuf);				/* test for switches */
if (*cptr == 0) return (reset_all (0));			/* reset(cr) */
cptr = get_glyph (cptr, gbuf, 0);			/* get next glyph */
if (*cptr != 0) return SCPE_ARG;			/* now (cr)? */
if (strcmp (gbuf, "ALL") == 0) return (reset_all (0));
if ((dptr = find_device (gbuf, NULL)) == NULL) return SCPE_ARG;
if (dptr -> reset != NULL) return dptr -> reset (dptr);
else return SCPE_OK;
}

/* Reset devices start..end

   Inputs:
	start	=	number of starting device
   Outputs:
	status	=	error status
*/

t_stat reset_all (int start)
{
DEVICE *dptr;
int32 i;
t_stat reason;

if ((start < 0) || (start > 1)) return SCPE_ARG;
for (i = start; (dptr = sim_devices[i]) != NULL; i++) {
	if (dptr -> reset != NULL) {
		reason = dptr -> reset (dptr);
		if (reason != SCPE_OK) return reason;  }  }
return SCPE_OK;
}

/* Load command

   lo[ad] filename		load specified file
*/

t_stat load_cmd (int flag, char *cptr)
{
char gbuf[CBUFSIZE];
FILE *loadfile;
t_stat reason;

GET_SWITCHES (cptr, gbuf);				/* test for switches */
if (*cptr == 0) return SCPE_ARG;
loadfile = fopen (cptr, "rb");
if (loadfile == NULL) return SCPE_OPENERR;
reason = sim_load (loadfile);
fclose (loadfile);
return reason;
}

/* Attach command

   at[tach] unit file	attach specified unit to file
*/

t_stat attach_cmd (int flag, char *cptr)
{
char gbuf[CBUFSIZE];
int32 unitno;
DEVICE *dptr;
UNIT *uptr;

GET_SWITCHES (cptr, gbuf);				/* test for switches */
if (*cptr == 0) return SCPE_ARG;
cptr = get_glyph (cptr, gbuf, 0);			/* get next glyph */
if (*cptr == 0) return SCPE_ARG;
if ((dptr = find_device (gbuf, &unitno)) == NULL) return SCPE_ARG;
uptr = (dptr -> units) + unitno;
if (dptr -> attach != NULL) return dptr -> attach (uptr, cptr);
return attach_unit (uptr, cptr);
}

t_stat attach_unit (UNIT *uptr, char *cptr)
{
DEVICE *dptr;
t_stat reason;

if (uptr -> flags & UNIT_DIS) return SCPE_ARG;		/* disabled? */
if (!(uptr -> flags & UNIT_ATTABLE)) return SCPE_NOATT;	/* not attachable? */
if ((dptr = find_dev_from_unit (uptr)) == NULL) return SCPE_NOATT;
if (uptr -> flags & UNIT_ATT) {				/* already attached? */
	reason = detach_unit (uptr);
	if (reason != SCPE_OK) return reason;  }
uptr -> filename = calloc (CBUFSIZE, sizeof (char));
if (uptr -> filename == NULL) return SCPE_MEM;
strncpy (uptr -> filename, cptr, CBUFSIZE);
uptr -> fileref = fopen (cptr, "rb+");
if (uptr -> fileref == NULL) {
	uptr -> fileref = fopen (cptr, "wb+");
	if (uptr -> fileref == NULL) return SCPE_OPENERR;
	printf ("%s: creating new file\n", dptr -> name);  }
if (uptr -> flags & UNIT_BUFABLE) {
	if ((uptr -> filebuf = calloc (uptr -> capac, SZ_D (dptr))) != NULL) {
		printf ("%s: buffering file in memory\n", dptr -> name);
		uptr -> hwmark = fxread (uptr -> filebuf, SZ_D (dptr),
					uptr -> capac, uptr -> fileref);
		uptr -> flags = uptr -> flags | UNIT_BUF;  }
	else if (uptr -> flags & UNIT_MUSTBUF) return SCPE_MEM;  }
uptr -> flags = uptr -> flags | UNIT_ATT;
uptr -> pos = 0;
return SCPE_OK;
}

/* Detach command

   det[ach] all		detach all units
   det[ach] unit	detach specified unit
*/

t_stat detach_cmd (int flag, char *cptr)
{
char gbuf[CBUFSIZE];
int32 unitno;
DEVICE *dptr;
UNIT *uptr;

GET_SWITCHES (cptr, gbuf);				/* test for switches */
if (*cptr == 0) return SCPE_ARG;
cptr = get_glyph (cptr, gbuf, 0);			/* get next glyph */
if (*cptr != 0) return SCPE_ARG;
if (strcmp (gbuf, "ALL") == 0) return (detach_all (0));
if ((dptr = find_device (gbuf, &unitno)) == NULL) return SCPE_ARG;
uptr = (dptr -> units) + unitno;
if (!(uptr -> flags & UNIT_ATTABLE)) return SCPE_NOATT;
if (dptr -> detach != NULL) return dptr -> detach (uptr);
return detach_unit (uptr);
}

/* Detach devices start..end

   Inputs:
	start	=	number of starting device
   Outputs:
	status	=	error status
*/

t_stat detach_all (int start)
{
int32 i, j;
t_stat reason;
DEVICE *dptr;
UNIT *uptr;

if ((start < 0) || (start > 1)) return SCPE_ARG;
for (i = start; (dptr = sim_devices[i]) != NULL; i++) {
	for (j = 0; j < dptr -> numunits; j++) {
		uptr = (dptr -> units) + j;
		if (dptr -> detach != NULL) reason = dptr -> detach (uptr);
		else reason = detach_unit (uptr);
		if (reason != SCPE_OK) return reason;  }  }
return SCPE_OK;
}

t_stat detach_unit (UNIT *uptr)
{
DEVICE *dptr;

if (uptr == NULL) return SCPE_ARG;
if (!(uptr -> flags & UNIT_ATT)) return SCPE_OK;
if ((dptr = find_dev_from_unit (uptr)) == NULL) return SCPE_OK;
uptr -> flags = uptr -> flags & ~UNIT_ATT;
if (uptr -> flags & UNIT_BUF) {
	printf ("%s: writing buffer to file\n", dptr -> name);
	uptr -> flags = uptr -> flags & ~UNIT_BUF;
	rewind (uptr -> fileref);
	fxwrite (uptr -> filebuf, SZ_D (dptr), uptr -> hwmark, uptr -> fileref);
	if (ferror (uptr -> fileref)) perror ("I/O error");
	free (uptr -> filebuf);
	uptr -> filebuf = NULL;  }
free (uptr -> filename);
uptr -> filename = NULL;
return (fclose (uptr -> fileref) == EOF)? SCPE_IOERR: SCPE_OK;
}

/* Save command

   sa[ve] filename		save state to specified file
*/

t_stat save_cmd (int flag, char *cptr)
{
char gbuf[CBUFSIZE];
FILE *sfile;
int32 i, j, t, zerocnt;
t_addr k, high;
t_value val;
t_stat reason;
DEVICE *dptr;
UNIT *uptr;
REG *rptr;

#define WRITE_I(xx) fxwrite (&(xx), sizeof (xx), 1, sfile)

GET_SWITCHES (cptr, gbuf);				/* test for switches */
if (*cptr == 0) return SCPE_ARG;
if ((sfile = fopen (cptr, "wb")) == NULL) return SCPE_OPENERR;
fputs (sim_name, sfile);				/* sim name */
fputc ('\n', sfile);
WRITE_I (sim_time);					/* sim time */

for (i = 0; (dptr = sim_devices[i]) != NULL; i++) {	/* loop thru devices */
	fputs (dptr -> name, sfile);			/* device name */
	fputc ('\n', sfile);
	for (j = 0; j < dptr -> numunits; j++) {
		uptr = (dptr -> units) + j;
		t = sim_is_active (uptr);
		WRITE_I (j);				/* unit number */
		WRITE_I (t);				/* activation time */
		WRITE_I (uptr -> u3);			/* unit specific */
		WRITE_I (uptr -> u4);
		if (uptr -> flags & UNIT_ATT) fputs (uptr -> filename, sfile);
		fputc ('\n', sfile);
		if (((uptr -> flags & (UNIT_FIX + UNIT_ATTABLE)) == UNIT_FIX) &&
		     (dptr -> examine != NULL) &&
		    ((high = uptr -> capac) != 0)) {	/* memory-like unit? */
			WRITE_I (high);			/* memory limit */
			zerocnt = 0;
			for (k = 0; k < high; k = k + (dptr -> aincr)) {
				reason = dptr -> examine (&val, k, uptr, 0);
				if (reason != SCPE_OK) return reason;
				if (val == 0) zerocnt = zerocnt - 1;
				else {	if (zerocnt) WRITE_I (zerocnt);
					zerocnt = 0;
					WRITE_I (val);  }  }
			if (zerocnt) WRITE_I (zerocnt);  }
		else {	k = 0;				/* no memory */
			WRITE_I (k);  }  }
	j = -1;
	WRITE_I (j);					/* end units */
	for (rptr = dptr -> registers;			/* loop thru regs */
		(rptr != NULL) && (rptr -> name != NULL); rptr++) {
		fputs (rptr -> name, sfile);		/* name */
		fputc ('\n', sfile);
		for (j = 0; j < rptr -> depth; j++) {	/* loop thru values */
			val = get_rval (rptr, j);	/* get value */
			WRITE_I (val);  }  }		/* store */
	fputc ('\n', sfile);  }				/* end registers */
fputc ('\n', sfile);					/* end devices */
reason = (ferror (sfile))? SCPE_IOERR: SCPE_OK;		/* error during save? */
fclose (sfile);
return reason;
}

/* Restore command

   re[store] filename		restore state from specified file
*/

t_stat restore_cmd (int flag, char *cptr)
{
char buf[CBUFSIZE];
FILE *rfile;
int32 j, data, unitno, time;
t_addr k, high;
t_value val, mask;
t_stat reason;
DEVICE *dptr;
UNIT *uptr;
REG *rptr;

#define READ_S(xx) if (read_line ((xx), CBUFSIZE, rfile) == NULL) \
	{ fclose (rfile); return SCPE_IOERR;  }
#define READ_I(xx) if (fxread (&xx, sizeof (xx), 1, rfile) <= 0) \
	{ fclose (rfile); return SCPE_IOERR;  }

GET_SWITCHES (cptr, buf);				/* test for switches */
if (*cptr == 0) return SCPE_ARG;
if ((rfile = fopen (cptr, "rb")) == NULL) return SCPE_OPENERR;
READ_S (buf);						/* sim name */
if (strcmp (buf, sim_name)) {
	printf ("Wrong system type: %s\n", buf);
	fclose (rfile);
	return SCPE_OK;  }
READ_I (sim_time);					/* sim time */

for ( ;; ) {						/* device loop */
	READ_S (buf);					/* read device name */
	if (buf[0] == 0) break;				/* last? */
	if ((dptr = find_device (buf, NULL)) == NULL) {
		printf ("Invalid device name: %s\n", buf);
		fclose (rfile);
		return SCPE_INCOMP;  }
	for ( ;; ) {					/* unit loop */
		READ_I (unitno);			/* unit number */
		if (unitno < 0) break;
		if (unitno >= dptr -> numunits) {
			printf ("Invalid unit number %s%d\n", dptr -> name,
				unitno);
			fclose (rfile);
			return SCPE_INCOMP;  }
		READ_I (time);				/* event time */
		uptr = (dptr -> units) + unitno;
		sim_cancel (uptr);
		if (time > 0) sim_activate (uptr, time - 1);
		READ_I (uptr -> u3);			/* device specific */
		READ_I (uptr -> u4);
		READ_S (buf);				/* attached file */
		if (buf[0] != 0) {
			uptr -> flags = uptr -> flags & ~UNIT_DIS;
			reason = attach_unit (uptr, buf);
			if (reason != SCPE_OK) return reason;  }
		READ_I (high);				/* memory capacity */
		if ((high > 0) &&			/* validate if > 0 */
		   (((uptr -> flags & (UNIT_FIX + UNIT_ATTABLE)) != UNIT_FIX) ||
		     (high > uptr -> capac) || (dptr -> deposit == NULL))) {
			printf ("Invalid memory bound: %u\n", high);
			fclose (rfile);
			return SCPE_INCOMP;  }
		for (k = 0; k < high; k = k + (dptr -> aincr)) {
			READ_I (data);
			if (data < 0) {
				for (j = data + 1; j < 0; j++) {
					reason = dptr -> deposit (0, k, uptr, 0);
					if (reason != SCPE_OK) return reason;
					k = k + (dptr -> aincr);  }
				data = 0;  }
			reason = dptr -> deposit (data, k, uptr, 0);
			if (reason != SCPE_OK) return reason;  }
		}					/* end unit loop */
	for ( ;; ) {					/* register loop */
		READ_S (buf);				/* read reg name */
		if (buf[0] == 0) break;			/* last? */
		if ((rptr = find_reg (buf, NULL, dptr)) == NULL) {
			printf ("Invalid register name: %s\n", buf);
			fclose (rfile);
			return SCPE_INCOMP;  }
		mask = width_mask[rptr -> width];
		for (j = 0; j < rptr -> depth; j++) {	/* loop thru values */
			READ_I (val);			/* read value */
			if (val > mask)
				printf ("Invalid register value: %s\n", buf);
			else put_rval (rptr, j, val, mask);  }  }
	}						/* end device loop */
fclose (rfile);
return SCPE_OK;
}

/* Run, go, cont, step commands

   ru[n] [new PC]	reset and start simulation
   go [new PC]		start simulation
   co[nt]		start simulation
   s[tep] [step limit]	start simulation for 'limit' instructions
   b[oot] device	bootstrap from device and start simulation
*/

t_stat run_cmd (int flag, char *cptr)
{
char gbuf[CBUFSIZE];
int32 i, j, step, unitno;
t_stat r;
t_addr addr, k;
DEVICE *dptr;
UNIT *uptr;
void int_handler (int signal);

GET_SWITCHES (cptr, gbuf);				/* test for switches */
step = 0;
if (((flag == RU_RUN) || (flag == RU_GO)) && (*cptr != 0)) {	/* run or go */
	cptr = get_glyph (cptr, gbuf, 0);		/* get next glyph */
	if ((r = dep_reg (0, gbuf, sim_PC)) != SCPE_OK) return r;  }

if (flag == RU_STEP) {					/* step */
	if (*cptr == 0) step = 1;
	else {	cptr = get_glyph (cptr, gbuf, 0);
		step = get_uint (gbuf, 10, INT_MAX, &r);
		if ((r != SCPE_OK) || (step == 0)) return SCPE_ARG;  }  }

if (flag == RU_BOOT) {					/* boot */
	if (*cptr == 0) return SCPE_ARG;
	cptr = get_glyph (cptr, gbuf, 0);		/* get next glyph */
	if ((dptr = find_device (gbuf, &unitno)) == NULL) return SCPE_ARG;
	if (dptr -> boot == NULL) return SCPE_ARG;
	uptr = dptr -> units + unitno;
	if (uptr -> flags & UNIT_DIS) return SCPE_ARG;	/* disabled? */
	if (!(uptr -> flags & UNIT_ATTABLE)) return SCPE_NOATT;
	if (!(uptr -> flags & UNIT_ATT)) return SCPE_UNATT;
	if ((r = dptr -> boot (unitno)) != SCPE_OK) return r;  }

if (*cptr != 0) return SCPE_ARG;

if ((flag == RU_RUN) || (flag == RU_BOOT)) {		/* run or boot */
	sim_interval = 0;				/* reset queue */
	sim_time = 0;
	noqueue_time = 0;
	sim_clock_queue = NULL;
	if ((r = reset_all (0)) != SCPE_OK) return r;  }
for (i = 1; (dptr = sim_devices[i]) != NULL; i++) {
	for (j = 0; j < dptr -> numunits; j++) {
		uptr = (dptr -> units) + j;
		if ((uptr -> flags & (UNIT_ATT + UNIT_SEQ)) ==
		    (UNIT_ATT + UNIT_SEQ))
			fseek (uptr -> fileref, uptr -> pos, SEEK_SET);  }  }
stop_cpu = 0;
if ((int) signal (SIGINT, int_handler) == -1) {		/* set WRU */
	printf ("Simulator interrupt handler setup failed\n");
	return SCPE_OK;  }
if (ttrunstate () != SCPE_OK) {				/* set console */
	ttcmdstate ();
	printf ("Simulator terminal setup failed\n");
	return SCPE_OK;  }
if (step) sim_activate (&step_unit, step);		/* set step timer */
r = sim_instr();

ttcmdstate ();						/* restore console */
signal (SIGINT, SIG_DFL);				/* cancel WRU */
sim_cancel (&step_unit);				/* cancel step timer */
if (sim_clock_queue != NULL) {				/* update sim time */
	UPDATE_SIM_TIME (sim_clock_queue -> time);  }
else {	UPDATE_SIM_TIME (noqueue_time);  }
#ifdef VMS
printf ("\n");
#endif
if (r >= SCPE_BASE) printf ("\n%s, %s: ", scp_error_messages[r - SCPE_BASE],
	sim_PC -> name);
else printf ("\n%s, %s: ", sim_stop_messages[r], sim_PC -> name);
print_val (addr = get_rval (sim_PC, 0), sim_PC -> radix,
	sim_PC -> width, sim_PC -> flags & REG_FMT);
if (((dptr = sim_devices[0]) != NULL) && (dptr -> examine != NULL)) {
	for (i = 0; i < sim_emax; i++) sim_eval[i] = 0;
	for (i = 0, k = addr; i < sim_emax; i++, k = k + dptr -> aincr) {
		if (r = dptr -> examine (&sim_eval[i], k, dptr -> units,
			SWMASK ('V')) != SCPE_OK) break;  }
	if ((r == SCPE_OK) || (i > 0)) {
		printf (" (");
		if (print_sym (addr, sim_eval, NULL, SWMASK('M')) > 0)
			print_val (sim_eval[0], dptr -> dradix,
				dptr -> dwidth, PV_RZRO);
		printf (")");  }  }
printf ("\n");
return SCPE_OK;
}

/* Run time routines */

/* Unit service for step timeout, originally scheduled by STEP n command

   Return step timeout SCP code, will cause simulation to stop
*/

t_stat step_svc (UNIT *uptr)
{
return SCPE_STEP;
}

/* Signal handler for ^C signal

   Set stop simulation flag
*/

void int_handler (int sig)
{
stop_cpu = 1;
return;
}

/* Examine/deposit commands

   ex[amine] [modifiers] list		examine
   de[posit] [modifiers] list val	deposit
   ie[xamine] [modifiers] list		interactive examine
   id[eposit] [modifiers] list		interactive deposit

   modifiers
	-letter(s)			switches
	devname'n			device name and unit number
	[{&|^}value]{=|==|!|!=|>|>=|<|<=} value	search specification

   list					list of addresses and registers
	addr[:addr|-addr]		address range
	ALL				all addresses
	register[:register|-register]	register range
	STATE				all registers
*/

t_stat exdep_cmd (int flag, char *cptr)
{
char gbuf[CBUFSIZE], *gptr, *tptr;
int32 unitno, t;
t_addr low, high;
t_stat reason;
DEVICE *dptr, *tdptr;
UNIT *uptr;
REG *lowr, *highr;
SCHTAB stab, *schptr;
t_stat exdep_addr_loop (SCHTAB *schptr, int flag, char *ptr,
	t_addr low, t_addr high, DEVICE *dptr, UNIT *uptr);
t_stat exdep_reg_loop (SCHTAB *schptr, int flag, char *ptr,
	REG *lptr, REG *hptr);

if (*cptr == 0) return SCPE_ARG;			/* err if no args */
sim_switches = 0;					/* no switches */
schptr = NULL;						/* no search */
stab.logic = SCH_OR;					/* default search params */
stab.bool = SCH_GE;
stab.mask = stab.comp = 0;
dptr = sim_devices[0];					/* default device, unit */
unitno = 0;
for (;;) {						/* loop through modifiers */
	if (*cptr == 0) return SCPE_ARG;		/* error if no arguments */
	cptr = get_glyph (cptr, gbuf, 0);
	if ((t = get_switches (gbuf)) != 0) {		/* try for switches */
		if (t < 0) return SCPE_ARG;		/* err if bad switch */
		sim_switches = sim_switches | t;  }	/* or in new switches */
	else if (get_search (gbuf, dptr, &stab) != NULL) {	/* try for search */
		schptr = &stab;  }			/* set search */
	else if ((tdptr = find_device (gbuf, &t)) != NULL) {	/* try for unit */
		dptr = tdptr;				/* set as default */
		unitno = t;  }
	else break;  }				/* not recognized, break out */
if ((*cptr == 0) == (flag == 0)) return SCPE_ARG;	/* eol if needed? */

gptr = gbuf;
uptr = (dptr -> units) + unitno;
while (*gptr != 0) {
	errno = 0;
	low = strtoul (gptr, &tptr, dptr -> aradix);
	if ((errno == 0) && (gptr != tptr)) {
		high = low;
		if ((*tptr == '-') || (*tptr == ':')) {
			gptr = tptr + 1;
			errno = 0;
			high = strtoul (gptr, &tptr, dptr -> aradix);
			if (errno || (gptr == tptr)) return SCPE_ARG;  }
		if (*tptr == ',') tptr++;
		else if (*tptr != 0) return SCPE_ARG;
		reason = exdep_addr_loop (schptr, flag, cptr, low, high,
			dptr, uptr);
		if (reason != SCPE_OK) return reason; }

	else if (strncmp (gptr, "ALL", strlen ("ALL")) == 0) {
		tptr = gptr + strlen ("ALL");
		if (*tptr == ',') tptr++;
		else if (*tptr != 0) return SCPE_ARG;
		if ((uptr -> capac == 0) | (flag == EX_E)) return SCPE_ARG;
		high = (uptr -> capac) - (dptr -> aincr);
		reason = exdep_addr_loop (schptr, flag, cptr, 0, high,
			dptr, uptr);
		if (reason != SCPE_OK) return reason; }

	else if (strncmp (gptr, "STATE", strlen ("STATE")) == 0) {
		tptr = gptr + strlen ("STATE");
		if (*tptr == ',') tptr++;
		else if (*tptr != 0) return SCPE_ARG;
		if ((lowr = dptr -> registers) == NULL) return SCPE_ARG;
		for (highr = lowr; highr -> name != NULL; highr++) ;
		sim_switches = sim_switches | SWHIDE;
		reason = exdep_reg_loop (schptr, flag, cptr, lowr, --highr);
		if (reason != SCPE_OK) return reason; }

	else {	lowr = find_reg (gptr, &tptr, dptr);
		if (lowr == NULL) return SCPE_ARG;
		highr = lowr;
		if ((*tptr == '-') || (*tptr == ':')) {
			highr = find_reg (tptr + 1, &tptr, dptr);
			if (highr == NULL) return SCPE_ARG;  }
		if (*tptr == ',') tptr++;
		else if (*tptr != 0) return SCPE_ARG;
		reason = exdep_reg_loop (schptr, flag, cptr, lowr, highr);
		if (reason != SCPE_OK) return reason; }

	gptr = tptr;  }					/* end while */
return SCPE_OK;
}

/* Loop controllers for examine/deposit

   exdep_reg_loop	examine/deposit range of registers
   exdep_addr_loop	examine/deposit range of addresses
*/

t_stat exdep_reg_loop (SCHTAB *schptr, int flag, char *cptr, 
	REG *lowr, REG *highr)
{
t_stat reason;
t_value val;
REG *rptr;

if ((lowr == NULL) || (highr == NULL)) return SCPE_ARG;
if (lowr > highr) return SCPE_ARG;
for (rptr = lowr; rptr <= highr; rptr++) {
	if ((sim_switches & SWHIDE) && (rptr -> flags & REG_HIDDEN)) continue;
	val = get_rval (rptr, 0);
	if (schptr && !test_search (val, schptr)) continue;
	if (flag != EX_D) {
		reason = ex_reg (val, flag, rptr);
		if (reason != SCPE_OK) return reason; }
	if (flag != EX_E) {
		reason = dep_reg (flag, cptr, rptr);
		if (reason != SCPE_OK) return reason;  }  }
return SCPE_OK;
}

t_stat exdep_addr_loop (SCHTAB *schptr, int flag, char *cptr,
	t_addr low, t_addr high, DEVICE *dptr, UNIT *uptr)
{
int32 i;
t_addr mask;
t_stat reason;

if (uptr -> flags & UNIT_DIS) return SCPE_ARG;		/* disabled? */
reason = 0;
mask = (1u << dptr -> awidth) - 1;
if ((low < 0) || (low > mask) || (high < 0) || (high > mask) ||
    (low > high)) return SCPE_ARG;
for (i = low; i <= high; i = i + (dptr -> aincr)) {
	reason = get_aval (i, dptr, uptr);		/* get data */
	if (reason != SCPE_OK) return reason;		/* return if error */
	if (schptr && !test_search (sim_eval[0], schptr)) continue;
	if (flag != EX_D) {
		reason = ex_addr (flag, i, dptr, uptr);
		if (reason > SCPE_OK) return reason;  }
	if (flag != EX_E) {
		reason = dep_addr (flag, cptr, i, dptr, uptr, reason);
		if (reason > SCPE_OK) return reason;  }
	if (reason < SCPE_OK) i = i - (reason * dptr -> aincr);  }
return SCPE_OK;
}

/* Examine register routine

   Inputs:
	val	=	current register value
	flag	=	type of ex/mod command (ex, iex, idep)
	rptr	=	pointer to register descriptor
   Outputs:
	return	=	error status
*/

t_stat ex_reg (t_value val, int flag, REG *rptr)
{
int32 rdx;

if (rptr == NULL) return SCPE_ARG;
printf ("%s:	", rptr -> name);
if (!(flag & EX_E)) return SCPE_OK;
GET_RADIX (rdx, rptr -> radix);
print_val (val, rdx, rptr -> width, rptr -> flags & REG_FMT);
if (flag & EX_I) printf ("	");
else printf ("\n");
return SCPE_OK;
}

/* Get register value

   Inputs:
	rptr	=	pointer to register descriptor
	idx	=	index (SAVE register buffers only)
   Outputs:
	return	=	register value
*/

t_value get_rval (REG *rptr, int idx)
{
size_t sz;
t_value val;

sz = SZ_R (rptr);
if ((rptr -> depth > 1) && (sz == sizeof (int8)))
	val = *(((unsigned int8 *) rptr -> loc) + idx);
else if ((rptr -> depth > 1) && (sz == sizeof (int16)))
	val = *(((unsigned int16 *) rptr -> loc) + idx);
#if !defined (int64)
else val = *(((unsigned int32 *) rptr -> loc) + idx);
#else
else if (sz <= sizeof (int32))
	 val = *(((unsigned int32 *) rptr -> loc) + idx);
else val = *(((unsigned int64 *) rptr -> loc) + idx);
#endif
val = (val >> rptr -> offset) & width_mask[rptr -> width];
return val;
}

/* Deposit register routine

   Inputs:
	flag	=	type of deposit (normal/interactive)
	cptr	=	pointer to input string
	rptr	=	pointer to register descriptor
   Outputs:
	return	=	error status
*/

t_stat dep_reg (int flag, char *cptr, REG *rptr)
{
t_stat r;
t_value val, mask;
int32 rdx;
char gbuf[CBUFSIZE];

if ((cptr == NULL) || (rptr == NULL)) return SCPE_ARG;
if (rptr -> flags & REG_RO) return SCPE_RO;
if (flag & EX_I) {
	cptr = read_line (gbuf, CBUFSIZE, stdin);
	if (cptr == NULL) return 1;			/* force exit */
	if (*cptr == 0) return SCPE_OK;	 }		/* success */
errno = 0;
mask = width_mask[rptr -> width];
GET_RADIX (rdx, rptr -> radix);
val = get_uint (cptr, rdx, mask, &r);
if (r != SCPE_OK) return SCPE_ARG;
if ((rptr -> flags & REG_NZ) && (val == 0)) return SCPE_ARG;
put_rval (rptr, 0, val, mask);
return SCPE_OK;
}

/* Put register value

   Inputs:
	rptr	=	pointer to register descriptor
	idx	=	index (RESTORE reg buffers only)
	val	=	new value
	mask	=	mask
   Outputs:
	none
*/

void put_rval (REG *rptr, int idx, t_value val, t_value mask)
{
size_t sz;

#define PUT_RVAL(sz,rp,id,val,msk) \
	*(((unsigned sz *) rp -> loc) + id) = \
		(*(((unsigned sz *) rp -> loc) + id) & \
		~((msk) << (rp) -> offset)) | ((val) << (rp) -> offset)

sz = SZ_R (rptr);
if ((rptr -> depth > 1) && (sz == sizeof (int8)))
	PUT_RVAL (int8, rptr, idx, val, mask);
else if ((rptr -> depth > 1) && (sz == sizeof (int16)))
	PUT_RVAL (int16, rptr, idx, val, mask);
#if !defined (int64)
else PUT_RVAL (int32, rptr, idx, val, mask);
#else
if (sz <= sizeof (int32)) PUT_RVAL (int32, rptr, idx, val, mask);
else PUT_RVAL (int64, rptr, idx, val, mask);
#endif
return;
}

/* Examine address routine

   Inputs: (sim_eval is an implicit argument)
	flag	=	type of ex/mod command (ex, iex, idep)
	addr	=	address to examine
	dptr	=	pointer to device
	uptr	=	pointer to unit
   Outputs:
	return	=	if >= 0, error status
			if < 0, number of extra words retired
*/

t_stat ex_addr (int flag, t_addr addr, DEVICE *dptr, UNIT *uptr)
{
t_stat reason;
int32 rdx;

print_val (addr, dptr -> aradix, dptr -> awidth, PV_LEFT);
printf (":	");
if (!(flag & EX_E)) return SCPE_OK;

GET_RADIX (rdx, dptr -> dradix);
if ((reason = print_sym (addr, sim_eval, uptr, sim_switches)) > 0)
    reason = print_val (sim_eval[0], rdx, dptr -> dwidth, PV_RZRO);
if (flag & EX_I) printf ("	");
else printf ("\n");
return reason;
}

/* Get address routine

   Inputs:
	flag	=	type of ex/mod command (ex, iex, idep)
	addr	=	address to examine
	dptr	=	pointer to device
	uptr	=	pointer to unit
   Outputs: (sim_eval is an implicit output)
	return	=	error status
*/

t_stat get_aval (t_addr addr, DEVICE *dptr, UNIT *uptr)
{
int32 i;
t_value mask;
t_addr j, loc;
t_stat reason;
size_t sz;

if ((dptr == NULL) || (uptr == NULL)) return SCPE_ARG;
mask = width_mask[dptr -> dwidth];
for (i = 0; i < sim_emax; i++) sim_eval[i] = 0;
for (i = 0, j = addr; i < sim_emax; i++, j = j + dptr -> aincr) {
	if (dptr -> examine != NULL) {
		reason = dptr -> examine (&sim_eval[i], j, uptr, sim_switches);
		if (reason != SCPE_OK) break;  }
	else {	if (!(uptr -> flags & UNIT_ATT)) return SCPE_UNATT;
		if ((uptr -> flags & UNIT_FIX) && (j >= uptr -> capac)) {
			reason = SCPE_NXM;
			break;  }
		sz = SZ_D (dptr);
		loc = j / dptr -> aincr;
		if (uptr -> flags & UNIT_BUF) {
			if (sz == sizeof (int8)) sim_eval[i] =
				*(((unsigned int8 *) uptr -> filebuf) + loc);
			else if (sz == sizeof (int16)) sim_eval[i] =
				*(((unsigned int16 *) uptr -> filebuf) + loc);
#if !defined (int64)
		    	else sim_eval[i] =
				*(((unsigned int32 *) uptr -> filebuf) + loc);
#else
		    	else if (sz == sizeof (int32)) sim_eval[i] =
				*(((unsigned int32 *) uptr -> filebuf) + loc);
			else sim_eval[i] =
				*(((unsigned int64 *) uptr -> filebuf) + loc);
#endif
		}
		else {	fseek (uptr -> fileref, sz * loc, SEEK_SET);
			fxread (&sim_eval[i], sz, 1, uptr -> fileref);
			if ((feof (uptr -> fileref)) &&
			   !(uptr -> flags & UNIT_FIX)) {
				reason = SCPE_EOF;
				break;  }
		 	else if (ferror (uptr -> fileref)) {
				clearerr (uptr -> fileref);
				reason = SCPE_IOERR;
				break;  }  }  }
	sim_eval[i] = sim_eval[i] & mask;  }
if ((reason != SCPE_OK) && (i == 0)) return reason;
return SCPE_OK;
}

/* Deposit address routine

   Inputs:
	flag	=	type of deposit (normal/interactive)
	cptr	=	pointer to input string
	addr	=	address to examine
	dptr	=	pointer to device
	uptr	=	pointer to unit
	dfltinc	=	value to return on cr input
   Outputs:
	return	=	if >= 0, error status
			if < 0, number of extra words retired
*/

t_stat dep_addr (int flag, char *cptr, t_addr addr, DEVICE *dptr,
	UNIT *uptr, int dfltinc)
{
int32 i, count, rdx;
t_addr j, loc;
t_stat r, reason;
t_value mask;
size_t sz;
char gbuf[CBUFSIZE];

if (dptr == NULL) return SCPE_ARG;
if (flag & EX_I) {
	cptr = read_line (gbuf, CBUFSIZE, stdin);
	if (cptr == NULL) return 1;			/* force exit */
	if (*cptr == 0) return dfltinc;	 }		/* success */
mask = width_mask[dptr -> dwidth];

GET_RADIX (rdx, dptr -> dradix);
if ((reason = parse_sym (cptr, addr, uptr, sim_eval, sim_switches)) > 0) {
	sim_eval[0] = get_uint (cptr, rdx, mask, &reason);
	if (reason != SCPE_OK) return reason;  }
count = 1 - reason;

for (i = 0, j = addr; i < count; i++, j = j + dptr -> aincr) {
	sim_eval[i] = sim_eval[i] & mask;
	if (dptr -> deposit != NULL) {
		r = dptr -> deposit (sim_eval[i], j, uptr, sim_switches);
		if (r != SCPE_OK) return r;  }
	else {	if (!(uptr -> flags & UNIT_ATT)) return SCPE_UNATT;
		if ((uptr -> flags & UNIT_FIX) && (j >= uptr -> capac))
			return SCPE_NXM;
		sz = SZ_D (dptr);
		loc = j / dptr -> aincr;
		if (uptr -> flags & UNIT_BUF) {
			if (sz == sizeof (int8)) *(((unsigned int8 *)
				uptr -> filebuf) + loc) = sim_eval[i];
			else if (sz == sizeof (int16)) *(((unsigned int16 *)
				uptr -> filebuf) + loc) = sim_eval[i];
#if !defined (int64)
			else *(((unsigned int32 *) uptr -> filebuf) + loc) =
				sim_eval[i];
#else
			else if (sz == sizeof (int32)) *(((unsigned int32 *)
				uptr -> filebuf) + loc) = sim_eval[i];
			else *(((unsigned int64 *) uptr -> filebuf) + loc) =
				sim_eval[i];
#endif
			if (loc >= uptr -> hwmark) uptr -> hwmark = loc + 1;  }
		else {	fseek (uptr -> fileref, sz * loc, SEEK_SET);
			fxwrite (sim_eval, sz, 1, uptr -> fileref);
			if (ferror (uptr -> fileref)) {
				clearerr (uptr -> fileref);
				return SCPE_IOERR;  }  }  }  }
return reason;
}

/* String processing routines

   read_line		read line

   Inputs:
	cptr	=	pointer to buffer
	size	=	maximum size
	stream	=	pointer to input stream
   Outputs:
	optr	=	pointer to first non-blank character
			NULL if EOF
*/

char *read_line (char *cptr, int size, FILE *stream)
{
char *tptr;

#ifdef SANOS
if (stream == stdin)
{
  extern int readline(char *buf, int size);
  if (readline(cptr, size) < 0) return NULL;
}
else
{
  cptr = fgets(cptr, size, stream);
  if (cptr == NULL) 
  {
    clearerr(stream);
    return NULL;  
  }
}
#else
cptr = fgets (cptr, size, stream);			/* get cmd line */
if (cptr == NULL) {
	clearerr (stream);				/* clear error */
	return NULL;  }					/* ignore EOF */
#endif
for (tptr = cptr; tptr < (cptr + size); tptr++)		/* remove cr */
	if (*tptr == '\n') *tptr = 0; 
while (isspace (*cptr)) cptr++;				/* absorb spaces */
return cptr;
}

/* get_glyph		get next glyph

   Inputs:
	iptr	=	pointer to input string
	optr	=	pointer to output string
	mchar	=	optional end of glyph character
   Outputs
	result	=	pointer to next character in input string
*/

char *get_glyph (char *iptr, char *optr, char mchar)
{
while ((isspace (*iptr) == 0) && (*iptr != 0) && (*iptr != mchar)) {
	if (islower (*iptr)) *optr = toupper (*iptr);
	else *optr = *iptr;
	iptr++; optr++;  }
*optr = 0;
if (mchar && (*iptr == mchar)) iptr++;			/* skip terminator */
while (isspace (*iptr)) iptr++;				/* absorb spaces */
return iptr;
}

/* get_yn		yes/no question

   Inputs:
	cptr	=	pointer to question
	deflt	=	default answer
   Outputs:
	result	=	true if yes, false if no
*/

t_stat get_yn (char *ques, t_stat deflt)
{
char cbuf[CBUFSIZE], *cptr;

printf ("%s ", ques);
cptr = read_line (cbuf, CBUFSIZE, stdin);
if ((cptr == NULL) || (*cptr == 0)) return deflt;
if ((*cptr == 'Y') || (*cptr == 'y')) return TRUE;
return FALSE;
}

/* get_uint		unsigned number

   Inputs:
	cptr	=	pointer to input string
	radix	=	input radix
	max	=	maximum acceptable value
	*status	=	pointer to error status
   Outputs:
	val	=	value
*/

t_value get_uint (char *cptr, int radix, t_value max, t_stat *status)
{
t_value val;
char *tptr;

errno = 0;
val = strtoul (cptr, &tptr, radix);
if (errno || (cptr == tptr) || (val > max) || (*tptr != 0)) *status = SCPE_ARG;
else *status = SCPE_OK;
return val;
}

/* Find_device		find device matching input string

   Inputs:
	cptr	=	pointer to input string
	iptr	=	pointer to unit number (can be null)
   Outputs:
	result	=	pointer to device
	*iptr	=	unit number, if valid
*/

DEVICE *find_device (char *cptr, int32 *iptr)
{
int32 i, lenn, unitno;
t_stat r;
DEVICE *dptr;

for (i = 0; (dptr = sim_devices[i]) != NULL; i++) {
	lenn = strlen (dptr -> name);
	if (strncmp (cptr, dptr -> name, lenn) != 0) continue;
	cptr = cptr + lenn;
	if (*cptr == 0) unitno = 0;
	else {	unitno = get_uint (cptr, 10, dptr -> numunits - 1, &r);
		if (r != SCPE_OK) return NULL;  }
	if (iptr != NULL) *iptr = unitno;
	return sim_devices[i];  }	
return NULL;
}

/* Find_dev_from_unit	find device for unit

   Inputs:
	uptr	=	pointer to unit
   Outputs:
	result	=	pointer to device
*/

DEVICE *find_dev_from_unit (UNIT *uptr)
{
DEVICE *dptr;
int32 i, j;

for (i = 0; (dptr = sim_devices[i]) != NULL; i++) {
	for (j = 0; j < dptr -> numunits; j++) {
		if (uptr == (dptr -> units + j)) return dptr;  }  }
return NULL;
}

/* find_reg		find register matching input string

   Inputs:
	cptr	=	pointer to input string
	optr	=	pointer to output pointer (can be null)
	dptr	=	pointer to device
   Outputs:
	result	=	pointer to register, NULL if error
	*optr	=	pointer to next character in input string
*/

REG *find_reg (char *cptr, char **optr, DEVICE *dptr)
{
char *tptr;
REG *rptr;

if ((cptr == NULL) || (dptr == NULL)) return NULL;
if (dptr -> registers == NULL) return NULL;
tptr = cptr;
do { tptr++; } while (isalnum (*tptr) || (*tptr == '_'));
for (rptr = dptr -> registers; rptr -> name != NULL; rptr++) {
	if (strncmp (cptr, rptr -> name, tptr - cptr) == 0) {
		if (optr != NULL) *optr = tptr;
		return rptr;  }  }
return NULL;
}

/* get_switches		get switches from input string

   Inputs:
	cptr	=	pointer to input string
   Outputs:
	sw	=	switch bit mask
			0 if no switches, -1 if error
*/

int32 get_switches (char *cptr)
{
int32 sw;

if (*cptr != '-') return 0;
sw = 0;
for (cptr++; (isspace (*cptr) == 0) && (*cptr != 0); cptr++) {
	if (isalpha (*cptr) == 0) return -1;
	sw = sw | SWMASK (*cptr);  }
return sw;
}

/* Get search specification

   Inputs:
	cptr	=	pointer to input string
	dptr	=	pointer to device
	schptr =	pointer to search table
   Outputs:
	return =	NULL if error
			schptr if valid search specification
*/

SCHTAB *get_search (char *cptr, DEVICE *dptr, SCHTAB *schptr)
{
int c, logop, cmpop;
t_value logval, cmpval;
char *sptr, *tptr;
const char logstr[] = "|&^", cmpstr[] = "=!><";

if (*cptr == 0) return NULL;				/* check for clause */
for (logop = cmpop = -1; c = *cptr++; ) {		/* loop thru clauses */
	if (sptr = strchr (logstr, c)) {		/* check for mask */
		logop = sptr - logstr;
		errno = 0;
		logval = strtoul (cptr, &tptr, dptr -> dradix);
		if (errno || (cptr == tptr)) return NULL;
		cptr = tptr;  }
	else if (sptr = strchr (cmpstr, c)) {	/* check for bool */
		cmpop = sptr - cmpstr;
		if (*cptr == '=') {
			cmpop = cmpop + strlen (cmpstr);
			cptr++;  }
		errno = 0;
		cmpval = strtoul (cptr, &tptr, dptr -> dradix);
		if (errno || (cptr == tptr)) return NULL;
		cptr = tptr;  }
	else return NULL;  }				/* end while */
if (logop >= 0) {
	schptr -> logic = logop;
	schptr -> mask = logval;  }
if (cmpop >= 0) {
	schptr -> bool = cmpop;
	schptr -> comp = cmpval;  }
return schptr;
}

/* Test value against search specification

   Inputs:
	val	=	value to test
	schptr =	pointer to search table
   Outputs:
	return =	1 if value passes search criteria, 0 if not
*/

int test_search (t_value val, SCHTAB *schptr)
{
if (schptr == NULL) return 0;
switch (schptr -> logic) {				/* case on logical */
case SCH_OR:
	val = val | schptr -> mask;
	break;
case SCH_AND:
	val = val & schptr -> mask;
	break;
case SCH_XOR:
	val = val ^ schptr -> mask;
	break;  }
switch (schptr -> bool) {				/* case on comparison */
case SCH_E: case SCH_EE:
	return (val == schptr -> comp);
case SCH_N: case SCH_NE:
	return (val != schptr -> comp);
case SCH_G:
	return (val > schptr -> comp);
case SCH_GE:
	return (val >= schptr -> comp);
case SCH_L:
	return (val < schptr -> comp);
case SCH_LE:
	return (val <= schptr -> comp);  }
return 0;
}

/* General radix printing routine

   Inputs:
	stream	=	stream designator
	val	=	value to print
	radix	=	radix to print
	width	=	width to print
	format	=	leading zeroes format
   Outputs:
	status	=	error status
*/

t_stat fprint_val (FILE *stream, t_value val, int radix,
	int width, int format)
{
#define MAX_WIDTH ((int) (CHAR_BIT * sizeof (t_value)))
t_value mask, digit;
int32 d, ndigits;
double fptest;
char dbuf[MAX_WIDTH + 1];

for (d = 0; d < MAX_WIDTH; d++) dbuf[d] = (format == PV_RZRO)? '0': ' ';
dbuf[MAX_WIDTH] = 0;
d = MAX_WIDTH;
do {	d = d - 1;
	digit = val % (unsigned) radix;
	val = val / (unsigned) radix;
	dbuf[d] = (digit <= 9)? '0' + digit: 'A' + (digit - 10);
   } while ((d > 0) && (val != 0));

if (format != PV_LEFT) {
	mask = width_mask[width];
	fptest = (double) radix;
	ndigits = 1;
	while (fptest < (double) mask) {
		fptest = fptest * (double) radix;
		ndigits = ndigits + 1; }
	if ((MAX_WIDTH - ndigits) < d) d = MAX_WIDTH - ndigits;  }
if (fputs (&dbuf[d], stream) == EOF) return SCPE_IOERR;
return SCPE_OK;
}

/* Event queue routines

	sim_activate		add entry to event queue
	sim_cancel		remove entry from event queue
	sim_process_event	process entries on event queue
	sim_is_active		see if entry is on event queue
	sim_atime		return absolute time for an entry
	sim_gtime		return global time

   Asynchronous events are set up by queueing a unit data structure
   to the event queue with a timeout (in simulator units, relative
   to the current time).  Each simulator 'times' these events by
   counting down interval counter sim_interval.  When this reaches
   zero the simulator calls sim_process_event to process the event
   and to see if further events need to be processed, or sim_interval
   reset to count the next one.

   The event queue is maintained in clock order; entry timeouts are
   RELATIVE to the time in the previous entry.

   Sim_process_event - process event

   Inputs:
	none
   Outputs:
	reason		reason code returned by any event processor,
			or 0 (SCPE_OK) if no exceptions
*/

t_stat sim_process_event (void)
{
UNIT *uptr;
t_stat reason;

if (stop_cpu) return SCPE_STOP;				/* stop CPU? */
if (sim_clock_queue == NULL) {				/* queue empty? */
	UPDATE_SIM_TIME (noqueue_time);			/* update sim time */
	sim_interval = noqueue_time = NOQUEUE_WAIT;	/* flag queue empty */
	return SCPE_OK;  }
UPDATE_SIM_TIME (sim_clock_queue -> time);		/* update sim time */
do {	uptr = sim_clock_queue;				/* get first */
	sim_clock_queue = uptr -> next;			/* remove first */
	uptr -> next = NULL;				/* hygiene */
	uptr -> time = 0;
	if (sim_clock_queue != NULL) sim_interval = sim_clock_queue -> time;
	else sim_interval = noqueue_time = NOQUEUE_WAIT;
	if (uptr -> action != NULL) reason = uptr -> action (uptr);
	else reason = SCPE_OK;
   } while ((reason == SCPE_OK) && (sim_interval == 0));

/* Empty queue forces sim_interval != 0 */

return reason;
}

/* Activate (queue) event

   Inputs:
	uptr	=	pointer to unit
	event_time =	relative timeout
   Outputs:
	reason	=	result (SCPE_OK if ok)
*/

t_stat sim_activate (UNIT *uptr, int32 event_time)
{
UNIT *cptr, *prvptr;
int32 accum;

if (event_time < 0) return SCPE_ARG;
if (sim_is_active (uptr)) return SCPE_OK;		/* already active? */
if (sim_clock_queue == NULL) { UPDATE_SIM_TIME (noqueue_time);  }
else  {	UPDATE_SIM_TIME (sim_clock_queue -> time);  }	/* update sim time */

prvptr = NULL;
accum = 0;
for (cptr = sim_clock_queue; cptr != NULL; cptr = cptr -> next) {
	if (event_time < accum + cptr -> time) break;
	accum = accum + cptr -> time;
	prvptr = cptr;  }
if (prvptr == NULL) {					/* insert at head */
	cptr = uptr -> next = sim_clock_queue;
	sim_clock_queue = uptr;  }
else {	cptr = uptr -> next = prvptr -> next;		/* insert at prvptr */
	prvptr -> next = uptr;  }
uptr -> time = event_time - accum;
if (cptr != NULL) cptr -> time = cptr -> time - uptr -> time;
sim_interval = sim_clock_queue -> time;
return SCPE_OK;
}

/* Cancel (dequeue) event

   Inputs:
	uptr	=	pointer to unit
   Outputs:
	reason	=	result (SCPE_OK if ok)

*/

t_stat sim_cancel (UNIT *uptr)
{
UNIT *cptr, *nptr;

if (sim_clock_queue == NULL) return SCPE_OK;
UPDATE_SIM_TIME (sim_clock_queue -> time);		/* update sim time */
nptr = NULL;
if (sim_clock_queue == uptr) nptr = sim_clock_queue = uptr -> next;
else {	for (cptr = sim_clock_queue; cptr != NULL; cptr = cptr -> next) {
		if (cptr -> next == uptr) {
			nptr = cptr -> next = uptr -> next;
			break;  }  }  }			/* end queue scan */
if (nptr != NULL) nptr -> time = nptr -> time + uptr -> time;
uptr -> next = NULL;					/* hygiene */
uptr -> time = 0;
if (sim_clock_queue != NULL) sim_interval = sim_clock_queue -> time;
else sim_interval = noqueue_time = NOQUEUE_WAIT;
return SCPE_OK;
}

/* Test for entry in queue, return activation time

   Inputs:
	uptr	=	pointer to unit
   Outputs:
	result =	absolute activation time + 1, 0 if inactive
*/

int32 sim_is_active (UNIT *uptr)
{
UNIT *cptr;
int32 accum;

accum = 0;
for (cptr = sim_clock_queue; cptr != NULL; cptr = cptr -> next) {
	accum = accum + cptr -> time;
	if (cptr == uptr) return accum + 1;  }
return 0;
}

/* Return global time

   Inputs: none
   Outputs:
	time	=	global time
*/

double sim_gtime (void)
{
if (sim_clock_queue == NULL) { UPDATE_SIM_TIME (noqueue_time);  }
else  {	UPDATE_SIM_TIME (sim_clock_queue -> time);  }
return sim_time;
}

/* Endian independent binary I/O package

   For consistency, all binary data read and written by the simulator
   is stored in little endian data order.  That is, in a multi-byte
   data item, the bytes are written out right to left, low order byte
   to high order byte.  On a big endian host, data is read and written
   from high byte to low byte.  Consequently, data written on a little
   endian system must be byte reversed to be usable on a big endian
   system, and vice versa.

   These routines are analogs of the standard C runtime routines
   fread and fwrite.  If the host is little endian, or the data items
   are size char, then the calls are passed directly to fread or
   fwrite.  Otherwise, these routines perform the necessary byte swaps
   using an intermediate buffer.
*/

size_t fxread (void *bptr, size_t size, size_t count, FILE *fptr)
{
size_t c, j, nelem, nbuf, lcnt, total;
int32 i, k;
unsigned char *sptr, *dptr;

if (sim_end || (size == sizeof (char)))
	return fread (bptr, size, count, fptr);
if ((size == 0) || (count == 0)) return 0;
nelem = FLIP_SIZE / size;				/* elements in buffer */
nbuf = count / nelem;					/* number buffers */
lcnt = count % nelem;					/* count in last buf */
if (lcnt) nbuf = nbuf + 1;
else lcnt = nelem;
total = 0;
for (i = nbuf; i > 0; i--) {
	c = fread (sim_flip, size, (i == 1? lcnt: nelem), fptr);
	if (c == 0) return total;
	total = total + c;
	for (j = 0, sptr = sim_flip, dptr = bptr; j < c; j++) {
		for (k = size - 1; k >= 0; k--) *(dptr + k) = *sptr++;
		dptr = dptr + size;  }  }
return total;
}

size_t fxwrite (void *bptr, size_t size, size_t count, FILE *fptr)
{
size_t c, j, nelem, nbuf, lcnt, total;
int32 i, k;
unsigned char *sptr, *dptr;

if (sim_end || (size == sizeof (char)))
	return fwrite (bptr, size, count, fptr);
if ((size == 0) || (count == 0)) return 0;
nelem = FLIP_SIZE / size;				/* elements in buffer */
nbuf = count / nelem;					/* number buffers */
lcnt = count % nelem;					/* count in last buf */
if (lcnt) nbuf = nbuf + 1;
else lcnt = nelem;
total = 0;
for (i = nbuf; i > 0; i--) {
	c = (i == 1)? lcnt: nelem;
	for (j = 0, sptr = bptr, dptr = sim_flip; j < c; j++) {
		for (k = size - 1; k >= 0; k--)
			*(dptr + k) = *sptr++;
		dptr = dptr + size;  }
	c = fwrite (sim_flip, size, c, fptr);
	if (c == 0) return total;
	total = total + c;  }
return total;
}


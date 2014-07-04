/*	Copyright (c) 1989 by Jean-Christophe Collet	*/
/*	Copyright (c) 1990 by M. Stephenson		*/
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the main function for the parser
 * and some useful functions needed by yacc
 */

#include "config.h"
#include "dlb.h"

#define MAX_ERRORS	25

extern int  NDECL (yyparse);
extern int line_number;
const char *fname = "(stdin)";
int fatal_error = 0;

int  FDECL (main, (int,char **));
void FDECL (yyerror, (const char *));
void FDECL (yywarning, (const char *));
int  NDECL (yywrap);
void FDECL (init_yyin, (FILE *));
void FDECL (init_yyout, (FILE *));

#define Fprintf (void)fprintf

int
main(argc, argv)
int argc;
char **argv;
{
	char	infile[64], outfile[64], basename[64];
	FILE	*fin, *fout;
	int	i, len;
	boolean errors_encountered = FALSE;

	Strcpy(infile, "(stdin)");
	fin = stdin;
	Strcpy(outfile, "(stdout)");
	fout = stdout;

	if (argc == 1) {	/* Read standard input */
	    init_yyin(fin);
	    init_yyout(fout);
	    (void) yyparse();
	    if (fatal_error > 0)
		errors_encountered = TRUE;
	} else {		/* Otherwise every argument is a filename */
	    for(i=1; i<argc; i++) {
		fname = strcpy(infile, argv[i]);
		/* the input file had better be a .pdf file */
		len = strlen(fname) - 4;	/* length excluding suffix */
		if (len < 0 || strncmp(".pdf", fname + len, 4)) {
		    Fprintf(stderr,
			    "Error - file name \"%s\" in wrong format.\n",
			    fname);
		    errors_encountered = TRUE;
		    continue;
		}

		/* build output file name */
		/* Use the whole name - strip off the last 3 or 4 chars. */

		(void) strncpy(basename, infile, len);
		basename[len] = '\0';

		outfile[0] = '\0';
#ifdef PREFIX
		(void) strcat(outfile, PREFIX);
#endif
		(void) strcat(outfile, basename);

		fin = freopen(infile, "r", stdin);
		if (!fin) {
		    Fprintf(stderr, "Can't open %s for input.\n", infile);
		    perror(infile);
		    errors_encountered = TRUE;
		    continue;
		}
		fout = freopen(outfile, WRBMODE, stdout);
		if (!fout) {
		    Fprintf(stderr, "Can't open %s for output.\n", outfile);
		    perror(outfile);
		    errors_encountered = TRUE;
		    continue;
		}
		init_yyin(fin);
		init_yyout(fout);
		(void) yyparse();
		line_number = 1;
		if (fatal_error > 0) {
			errors_encountered = TRUE;
			fatal_error = 0;
		}
	    }
	}
	if (fout && fclose(fout) < 0) {
	    Fprintf(stderr, "Can't finish output file.");
	    perror(outfile);
	    errors_encountered = TRUE;
	}
	exit(errors_encountered ? EXIT_FAILURE : EXIT_SUCCESS);
	/*NOTREACHED*/
	return 0;
}

/*
 * Each time the parser detects an error, it uses this function.
 * Here we take count of the errors. To continue farther than
 * MAX_ERRORS wouldn't be reasonable.
 */

void yyerror(s)
const char *s;
{
	(void) fprintf(stderr,"%s : line %d : %s\n",fname,line_number, s);
	if (++fatal_error > MAX_ERRORS) {
		(void) fprintf(stderr,"Too many errors, good bye!\n");
		exit(EXIT_FAILURE);
	}
}

/*
 * Just display a warning (that is : a non fatal error)
 */

void yywarning(s)
const char *s;
{
	(void) fprintf(stderr,"%s : line %d : WARNING : %s\n",fname,line_number,s);
}

int yywrap()
{
       return 1;
}

/*dgn_main.c*/

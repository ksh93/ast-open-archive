/* gmake MAM additions.
   #include of this file appended to config.h for edit convenience.  */

#ifndef _MAM_H
#define _MAM_H		1

#define MAM		1

#if _PACKAGE_ast

#include <ast_std.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#ifndef __GNU_LIBRARY__
#define __GNU_LIBRARY__	1
#endif

#ifndef __alloca
#define __alloca	alloca
#endif

#ifndef index
#define index		strchr
#endif

#ifndef rindex
#define rindex		strrchr
#endif

#ifndef HAVE_GLOB_H
#define HAVE_GLOB_H	1
#endif

#ifndef HAVE_STRCOLL
#define HAVE_STRCOLL	1
#endif

#ifndef HAVE_UNISTD_H
#define HAVE_UNISTD_H	1
#endif

#ifndef STDC_HEADERS
#define STDC_HEADERS	1
#endif

#ifndef PATH_REGULAR
#define PATH_REGULAR	010
#endif

extern char	*pathpath();

#endif /* _PACKAGE_ast */

#include <stdio.h>

struct mam_state			/* mam state			*/
{
	unsigned char	dynamic;	/* dynamic mam			*/
	unsigned char	regress;	/* regression mam		*/
	unsigned char	state;		/* static mam			*/

	int		parent;		/* mam parent label		*/
	int		pwdlen;		/* strlen (mam.pwd)		*/
	int		rootlen;	/* strlen (mam.root)		*/

	char*		label;		/* instruction label		*/
	char*		options;	/* option string		*/
	char*		pwd;		/* file names relative to this	*/
	char*		root;		/* mam names relative to root	*/
	char*		type;		/* mam type string		*/

	FILE*		out;		/* output stream pointer	*/
};

extern int force_flag;
extern int never_flag;

extern struct mam_state mam;

extern char *mamname ();
extern char *pathcanon ();

#endif /* _MAM_H */

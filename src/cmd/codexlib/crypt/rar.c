#pragma prototyped

/*
 * crypt-rar decoder/encoder wrapper
 */

#include "rar.h"

static int
rar_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	int		version;
	char*		e;

	if (!args[2] || !args[3] || strcmp(args[2], "rar") || (version = strtol(args[3], &e, 10)) <= 0 || *e)
		return -1;
	if (version <= 13)
		p->meth = &crypt_rar_13;
	else if (version <= 15)
		p->meth = &crypt_rar_15;
	else
		p->meth = &crypt_rar_20;
	return (*p->meth->openf)(p, args, flags);
}

Codexmeth_t	crypt_rar =
{
	"crypt-rar",
	"\brar\b encryption. The first option is the \brar\b implementation"
	" version. Versions { 13 15 20 } are supported.",
	"[+(version)?crypt-rar-13 2003-12-29]"
	"[+(author)?Eugene Roshal]"
	"[+(copyright)?Copyright (c) 1999-2003 Eugene Roshal]"
	"[+(license)?GPL]",
	CODEX_DECODE|CODEX_ENCODE|CODEX_CRYPT,
	0,
	0,
	rar_open,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	crypt_rar_next
};

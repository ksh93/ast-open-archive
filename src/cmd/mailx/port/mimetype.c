#pragma prototyped

/*
 * mime/mailcap to magic support
 */

#include "mimelib.h"

/*
 * close magic handle
 * done this way so that magic is only pulled in
 * if mimetype() is used
 */

static void
drop(Mime_t* mp)
{
	if (mp->magic)
	{
		magicclose(mp->magic);
		mp->magic = 0;
	}
}

/*
 * return mime type for file
 */

char*
mimetype(Mime_t* mp, Sfio_t* fp, const char* file, struct stat* st)
{
	if (mp->disc->flags & MIME_NOMAGIC)
		return 0;
	if (!mp->magic)
	{
		mp->magicd.version = MAGIC_VERSION;
		mp->magicd.flags = MAGIC_MIME;
		mp->magicd.errorf = mp->disc->errorf;
		if (!(mp->magic = magicopen(&mp->magicd)))
		{
			mp->disc->flags |= MIME_NOMAGIC;
			return 0;
		}
		mp->freef = drop;
		magicload(mp->magic, NiL, 0);
	}
	return magictype(mp->magic, fp, file, st);
}

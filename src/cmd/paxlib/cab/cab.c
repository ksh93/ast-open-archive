/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2004 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * pax cabinet format
 * based on GPL code from cabextract
 *
 * cabextract 0.6 - a program to extract Microsoft Cabinet files
 * Copyright (c) 2000-2002 Stuart Caie <kyzer@4u.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * CAB files are 'cabinets'. 'Folders' store compressed data, and may span
 * several cabinets. 'Files' live as data inside a folder when
 * uncompressed. EOR checksums are used instead of CRCs. Four compression
 * formats are known - NONE, MSZIP, QUANTUM and LZX. NONE is obviously
 * uncompressed data. MSZIP is simply PKZIP's deflate/inflate algorithims
 * with 'CK' as a signature instead of 'PK'. QUANTUM is an LZ77 +
 * arithmetic coding method. LZX is a much loved LZH based archiver in the
 * Amiga world, the algorithm taken (bought?) by Microsoft and tweaked for
 * Intel code.
 */

#include <paxlib.h>
#include <codex.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sfstr.h>
#include <tm.h>
#include <vmalloc.h>

#define CAB_MAGIC		"MSCF"
#define CAB_VERSION		0x0103

typedef unsigned _ast_int1_t  UBYTE; /* 8 bits exactly    */
typedef unsigned _ast_int2_t  UWORD; /* 16 bits (or more) */
typedef unsigned _ast_int4_t  ULONG; /* 32 bits (or more) */
typedef   signed _ast_int4_t   LONG; /* 32 bits (or more) */

/* endian-neutral reading of little-endian data */
#define getulong(a)  ((ULONG)((((a)[3])<<24)|(((a)[2])<<16)|(((a)[1])<<8)|((a)[0])))
#define getuword(a)  ((UWORD)((((a)[1])<<8)|((a)[0])))

/* maximum number of cabinets any one folder can be split across */
#define CAB_SPLITMAX (10)

struct Ar_s; typedef struct Ar_s Ar_t;
struct Cabinet_s; typedef struct Cabinet_s Cabinet_t;
struct Folder_s; typedef struct Folder_s Folder_t;
struct File_s; typedef struct File_s File_t;

struct Cabinet_s
{
  Cabinet_t *next;                     /* for making a list of cabinets  */
  char  *filename;                     /* input name of cabinet          */
  Sfio_t  *fh;                         /* open file handle or NULL       */
  Sfoff_t filelen;                     /* length of cabinet file         */
  Sfoff_t blocks_off;                  /* offset to data blocks in file  */
  Cabinet_t *prevcab, *nextcab;        /* multipart cabinet chains       */
  char *prevname, *nextname;           /* and their filenames            */
  char *previnfo, *nextinfo;           /* and their visible names        */
  Folder_t *folders;                   /* first folder in this cabinet   */
  File_t *files;                       /* first file in this cabinet     */
  UBYTE block_resv;                    /* reserved space in datablocks   */
  UBYTE flags;                         /* header flags                   */
};

struct Folder_s
{
  Folder_t *next;
  Cabinet_t *cab[CAB_SPLITMAX];        /* cabinet(s) this folder spans   */
  Sfoff_t offset[CAB_SPLITMAX];        /* offset to data blocks          */
  UWORD comp_type;                     /* compression format/window size */
  ULONG comp_size;                     /* compressed size of folder      */
  UBYTE num_splits;                    /* number of split blocks + 1     */
  UWORD num_blocks;                    /* total number of blocks         */
  File_t *contfile;                    /* the first split file           */
};

struct File_s
{
  File_t *next;                        /* next file in sequence          */
  Folder_t *folder;                    /* folder that contains this file */
  char *filename;                      /* output name of file            */
  Sfio_t *fh;                          /* open file handle or NULL       */
  ULONG length;                        /* uncompressed length of file    */
  ULONG offset;                        /* uncompressed offset in folder  */
  UWORD index;                         /* magic index number of folder   */
  UWORD time, date, attr;              /* MS-DOS time/date/attributes    */
};

/* structure offsets */

#define CAB_HDR_Signature         (0x00)
#define CAB_HDR_CabinetSize       (0x08)
#define CAB_HDR_FileOffset        (0x10)
#define CAB_HDR_MinorVersion      (0x18)
#define CAB_HDR_MajorVersion      (0x19)
#define CAB_HDR_NumFolders        (0x1A)
#define CAB_HDR_NumFiles          (0x1C)
#define CAB_HDR_Flags             (0x1E)
#define CAB_HDR_SetID             (0x20)
#define CAB_HDR_CabinetIndex      (0x22)
#define CAB_HDR_SIZEOF            (0x24)

#define CAB_EXT_HeaderReserved    (0x00)
#define CAB_EXT_FolderReserved    (0x02)
#define CAB_EXT_DataReserved      (0x03)
#define CAB_EXT_SIZEOF            (0x04)

#define CAB_DIR_DataOffset        (0x00)
#define CAB_DIR_NumBlocks         (0x04)
#define CAB_DIR_CompType          (0x06)
#define CAB_DIR_SIZEOF            (0x08)

#define CAB_FIL_UncompressedSize  (0x00)
#define CAB_FIL_FolderOffset      (0x04)
#define CAB_FIL_FolderIndex       (0x08)
#define CAB_FIL_Date              (0x0A)
#define CAB_FIL_Time              (0x0C)
#define CAB_FIL_Attribs           (0x0E)
#define CAB_FIL_SIZEOF            (0x10)

#define CAB_DAT_CheckSum          (0x00)
#define CAB_DAT_CompressedSize    (0x04)
#define CAB_DAT_UncompressedSize  (0x06)
#define CAB_DAT_SIZEOF            (0x08)

/* flags */

#define CAB_HDR_PREV_CABINET             (0x0001)
#define CAB_HDR_NEXT_CABINET             (0x0002)
#define CAB_HDR_RESERVE_PRESENT          (0x0004)

#define CAB_DIR_COMPTYPE_MASK            (0x000f)
#define CAB_DIR_COMPTYPE_NONE            (0x0000)
#define CAB_DIR_COMPTYPE_MSZIP           (0x0001)
#define CAB_DIR_COMPTYPE_QUANTUM         (0x0002)
#define CAB_DIR_COMPTYPE_LZX             (0x0003)

#define CAB_FIL_CONTINUED_FROM_PREV      (0xFFFD)
#define CAB_FIL_CONTINUED_TO_NEXT        (0xFFFE)
#define CAB_FIL_CONTINUED_PREV_AND_NEXT  (0xFFFF)

#define CAB_FIL_A_RDONLY                 (0x01)
#define CAB_FIL_A_HIDDEN                 (0x02)
#define CAB_FIL_A_SYSTEM                 (0x04)
#define CAB_FIL_A_ARCH                   (0x20)
#define CAB_FIL_A_EXEC                   (0x40)
#define CAB_FIL_A_NAME_IS_UTF            (0x80)

#define CAB_ATTR_READONLY		0x0001
#define CAB_ATTR_HIDDEN			0x0002
#define CAB_ATTR_SYSTEM			0x0004
#define CAB_ATTR_VOLUME			0x0008
#define CAB_ATTR_DIRECTORY		0x0010
#define CAB_ATTR_ARCHIVE		0x0020
#define CAB_ATTR_EXECUTE		0x0040
#define CAB_ATTR_UTF			0x0080

/* CAB data blocks are <= 32768 bytes in uncompressed form. Uncompressed
 * blocks have zero growth. MSZIP guarantees that it won't grow above
 * uncompressed size by more than 12 bytes. LZX guarantees it won't grow
 * more than 6144 bytes.
 */
#define CAB_BLOCKMAX (32768)
#define CAB_INPUTMAX (CAB_BLOCKMAX+6144)

struct Ar_s
{
  Sfdisc_t disc;	  /* data block sfio part discipline       */
  Sfio_t *io;             /* data block sfio part stream           */
  UBYTE *ip;              /* current inbuf position                */
  UBYTE *ie;              /* 1 past last inbuf char                */
  Codexdisc_t codexdisc;  /* codex(3) discipline                   */

  Pax_t *pax;
  Paxarchive_t *ap;
  Vmalloc_t *vm;          /* vmalloc region                        */

  File_t *file;		  /* next file to extract		   */
  Folder_t *current;      /* current folder we're extracting from  */
  ULONG offset;           /* uncompressed offset within folder     */
  UWORD split;            /* at which split in current folder?     */
  UBYTE inbuf[CAB_INPUTMAX+2]; /* +2 for lzx bitbuffer overflows!  */
  UBYTE buf[CAB_BLOCKMAX];

  UWORD skip;
  UWORD sync;
  int   eof;
};

/* all the file IO is abstracted into these routines:
 * cabinet_(open|close|read|seek|skip|getoffset)
 */

static void
cabinet_close(Ar_t *ar, Cabinet_t *cab)
{
  if (cab->fh) {
    sfclose(cab->fh);
  }
  cab->fh = 0;
}

static void
cabinet_seek(Ar_t *ar, Cabinet_t *cab, Sfoff_t offset)
{
  if (sfseek(cab->fh, offset, SEEK_SET) < 0) {
    (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: seek error", cab->filename);
  }
}

static void
cabinet_skip(Ar_t *ar, Cabinet_t *cab, Sfoff_t distance)
{
  if (sfseek(cab->fh, distance, SEEK_CUR) < 0) {
    (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: seek error", cab->filename);
  }
}

static Sfoff_t
cabinet_tell(Ar_t *ar, Cabinet_t *cab)
{
  return sftell(cab->fh);
}

/* read data from a cabinet, returns success */
static int
cabinet_read(Ar_t *ar, Cabinet_t *cab, UBYTE *buf, size_t length)
{
  size_t avail = (size_t) (cab->filelen - cabinet_tell(ar, cab));
  if (length > avail) {
    if (!avail)
      return 0;
    (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: cabinet truncated [length=%I*u avail=%I*u]", cab->filename, sizeof(length), length, sizeof(avail), avail);
    length = avail;
  }
  if (sfread(cab->fh, (void *)buf, length) != length) {
    (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: read error", cab->filename);
    return 0;
  }
  return 1;
}

/* try to open a cabinet file, returns success */
static int
cabinet_open(Ar_t *ar, Cabinet_t *cab)
{
  char *name = cab->filename;
  Sfio_t *fh;

  /* note: this is now case sensitive */
  if (!(fh = sfopen(NiL, name, "rb"))) {
    (*ar->pax->errorf)(NiL, ar->pax, ERROR_SYSTEM|2, "%s: cannot open", name);
    return 0;
  }

  /* seek to end of file */
  if (sfseek(fh, 0, SEEK_END) < 0) {
    (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: seek error", name);
    sfclose(fh);
    return 0;
  }

  /* get length of file */
  cab->filelen = sftell(fh);

  /* return to the start of the file */
  if (sfseek(fh, 0, SEEK_SET) != 0) {
    (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: seek error", name);
    sfclose(fh);
    return 0;
  }

  cab->fh = fh;
  return 1;
}

/* allocate and read an aribitrarily long string from the cabinet */
static char*
cabinet_read_string(Ar_t *ar, Cabinet_t *cab)
{
  Sfoff_t len=256, base = cabinet_tell(ar, cab), maxlen = cab->filelen - base;
  int ok = 0, i;
  UBYTE *buf = 0;
  do {
    if (len > maxlen) len = maxlen;
    if (!(buf = realloc(buf, (size_t) len))) break;
    if (!cabinet_read(ar, cab, buf, (size_t) len)) break;

    /* search for a null terminator in what we've just read */
    for (i=0; i < len; i++) {
      if (!buf[i]) {ok=1; break;}
    }

    if (!ok) {
      if (len == maxlen) {
        (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: cabinet truncated", cab->filename);
        break;
      }
      len += 256;
      cabinet_seek(ar, cab, base);
    }
  } while (!ok);

  if (!ok) {
    if (buf) free(buf);
    else paxnospace(ar->pax);
    return 0;
  }

  /* otherwise, set the stream to just after the string and return */
  cabinet_seek(ar, cab, base + ((Sfoff_t) strlen((char *) buf)) + 1);
  return (char *) buf;
}

/* reads the header and all folder and file entries in this cabinet */
static int
cabinet_read_entries(Ar_t *ar, Cabinet_t *cab)
{
  int num_folders, num_files, header_resv, folder_resv = 0, i;
  Folder_t *fol, *linkfol = 0;
  File_t *file, *linkfile = 0;
  Sfoff_t base_offset;
  UBYTE buf[64];

  /* read in the CFHEADER */
  base_offset = cabinet_tell(ar, cab);
  if (!cabinet_read(ar, cab, buf, CAB_HDR_SIZEOF)) {
    return 0;
  }
  
  /* check basic MSCF signature */
  if (getulong(buf+CAB_HDR_Signature) != 0x4643534d) {
    (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: not a cabinet file", cab->filename);
    return 0;
  }

  /* get the number of folders */
  num_folders = getuword(buf+CAB_HDR_NumFolders);
  if (num_folders == 0) {
    return 0;
  }

  /* get the number of files */
  num_files = getuword(buf+CAB_HDR_NumFiles);
  if (num_files == 0) {
    return 0;
  }

  /* just check the header revision */
  if ((buf[CAB_HDR_MajorVersion] > 1) ||
      (buf[CAB_HDR_MajorVersion] == 1 && buf[CAB_HDR_MinorVersion] > 3))
  {
    (*ar->pax->errorf)(NiL, ar->pax, 1, "%s: %d.%d: version 1.3 or older supported", buf[CAB_HDR_MajorVersion], buf[CAB_HDR_MinorVersion]);
  }

  /* read the reserved-sizes part of header, if present */
  cab->flags = getuword(buf+CAB_HDR_Flags);
  if (cab->flags & CAB_HDR_RESERVE_PRESENT) {
    if (!cabinet_read(ar, cab, buf, CAB_EXT_SIZEOF)) return 0;
    header_resv     = getuword(buf+CAB_EXT_HeaderReserved);
    folder_resv     = buf[CAB_EXT_FolderReserved];
    cab->block_resv = buf[CAB_EXT_DataReserved];

    if (header_resv > 60000) {
      (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: header reserved space > 60000", cab->filename);
    }

    /* skip the reserved header */
    if (header_resv) sfseek(cab->fh, (Sfoff_t) header_resv, SEEK_CUR);
  }

  if (cab->flags & CAB_HDR_PREV_CABINET) {
    cab->prevname = cabinet_read_string(ar, cab);
    if (!cab->prevname) return 0;
    cab->previnfo = cabinet_read_string(ar, cab);
  }

  if (cab->flags & CAB_HDR_NEXT_CABINET) {
    cab->nextname = cabinet_read_string(ar, cab);
    if (!cab->nextname) return 0;
    cab->nextinfo = cabinet_read_string(ar, cab);
  }

  /* read folders */
  for (i = 0; i < num_folders; i++) {
    if (!cabinet_read(ar, cab, buf, CAB_DIR_SIZEOF)) return 0;
    if (folder_resv) cabinet_skip(ar, cab, folder_resv);

    fol = (Folder_t *) calloc(1, sizeof(Folder_t));
    if (!fol) { paxnospace(ar->pax); return 0; }

    fol->cab[0]     = cab;
    fol->offset[0]  = base_offset + (Sfoff_t) getulong(buf+CAB_DIR_DataOffset);
    fol->num_blocks = getuword(buf+CAB_DIR_NumBlocks);
    fol->comp_type  = getuword(buf+CAB_DIR_CompType);

    if (!linkfol) cab->folders = fol; else linkfol->next = fol;
    linkfol = fol;
  }

  /* read files */
  for (i = 0; i < num_files; i++) {
    if (!cabinet_read(ar, cab, buf, CAB_FIL_SIZEOF)) return 0;
    file = (File_t *) calloc(1, sizeof(File_t));
    if (!file) { paxnospace(ar->pax); return 0; }
      
    file->length   = getulong(buf+CAB_FIL_UncompressedSize);
    file->offset   = getulong(buf+CAB_FIL_FolderOffset);
    file->index    = getuword(buf+CAB_FIL_FolderIndex);
    file->time     = getuword(buf+CAB_FIL_Time);
    file->date     = getuword(buf+CAB_FIL_Date);
    file->attr     = getuword(buf+CAB_FIL_Attribs);
    file->filename = cabinet_read_string(ar, cab);
    if (!file->filename) return 0;
    if (!linkfile) cab->files = file; else linkfile->next = file;
    linkfile = file;
  }
  return 1;
}

/* this does the tricky job of running through every file in the cabinet,
 * including spanning cabinets, and working out which file is in which
 * folder in which cabinet. It also throws out the duplicate file entries
 * that appear in spanning cabinets. There is memory leakage here because
 * those entries are not freed. See the XAD CAB client for an
 * implementation of this that correctly frees the discarded file entries.
 */
static File_t*
process_files(Ar_t *ar, Cabinet_t *basecab)
{
  Cabinet_t *cab;
  File_t *outfi = 0, *linkfi = 0, *nextfi, *fi, *cfi;
  Folder_t *fol, *firstfol, *lastfol = 0, *predfol;
  int i, mergeok;

  for (cab = basecab; cab; cab = cab->nextcab) {
    /* firstfol = first folder in this cabinet */
    /* lastfol  = last folder in this cabinet */
    /* predfol  = last folder in previous cabinet (or NULL if first cabinet) */
    predfol = lastfol;
    firstfol = cab->folders;
    for (lastfol = firstfol; lastfol->next;) lastfol = lastfol->next;
    mergeok = 1;

    for (fi = cab->files; fi; fi = nextfi) {
      i = fi->index;
      nextfi = fi->next;

      if (i < CAB_FIL_CONTINUED_FROM_PREV) {
        for (fol = firstfol; fol && i--; ) fol = fol->next;
        fi->folder = fol; /* NULL if an invalid folder index */
      }
      else {
        /* folder merging */
        if (i == CAB_FIL_CONTINUED_TO_NEXT
        ||  i == CAB_FIL_CONTINUED_PREV_AND_NEXT) {
          if (cab->nextcab && !lastfol->contfile) lastfol->contfile = fi;
        }

        if (i == CAB_FIL_CONTINUED_FROM_PREV
        ||  i == CAB_FIL_CONTINUED_PREV_AND_NEXT) {
          /* these files are to be continued in yet another
           * cabinet, don't merge them in just yet */
          if (i == CAB_FIL_CONTINUED_PREV_AND_NEXT) mergeok = 0;

          /* only merge once per cabinet */
          if (predfol) {
            if ((cfi = predfol->contfile)
            && (cfi->offset == fi->offset)
            && (cfi->length == fi->length)
            && (strcmp(cfi->filename, fi->filename) == 0)
            && (predfol->comp_type == firstfol->comp_type)) {
              /* increase the number of splits */
              if ((i = ++(predfol->num_splits)) > CAB_SPLITMAX) {
                mergeok = 0;
                (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: internal error, increase CAB_SPLITMAX", basecab->filename);
              }
              else {
                /* copy information across from the merged folder */
                predfol->offset[i] = firstfol->offset[0];
                predfol->cab[i]    = firstfol->cab[0];
                predfol->next      = firstfol->next;
                predfol->contfile  = firstfol->contfile;

                if (firstfol == lastfol) lastfol = predfol;
                firstfol = predfol;
                predfol = 0; /* don't merge again within this cabinet */
              }
            }
            else {
              /* if the folders won't merge, don't add their files */
              mergeok = 0;
            }
          }

          if (mergeok) fi->folder = firstfol;
        }
      }

      if (fi->folder) {
        if (linkfi) linkfi->next = fi; else outfi = fi;
        linkfi = fi;
      }
    } /* for (fi= .. */
  } /* for (cab= ...*/

  return outfi;
}

/* validates and reads file entries from a cabinet at offset [offset] in
 * file [name]. Returns a cabinet structure if successful, or NULL
 * otherwise.
 */
static Cabinet_t*
load_cab_offset(Ar_t *ar, char *name, Sfoff_t offset)
{
  Cabinet_t *cab = (Cabinet_t *) calloc(1, sizeof(Cabinet_t));
  int ok;
  if (!cab) return 0;

  cab->filename = name;
  if ((ok = cabinet_open(ar, cab))) {
    cabinet_seek(ar, cab, offset);
    ok = cabinet_read_entries(ar, cab);
    cabinet_close(ar, cab);
  }

  if (ok) return cab;
  free(cab);
  return 0;
}

/* Searches a file for embedded cabinets (also succeeds on just normal
 * cabinet files). The first result of this search will be returned, and
 * the remaining results will be chained to it via the cab->next structure
 * member.
 */
#define SEARCH_SIZE (32*1024)

static Cabinet_t*
find_cabs_in_file(Ar_t *ar, char *name)
{
  Cabinet_t *cab, *cab2, *firstcab = 0, *linkcab = 0;
  UBYTE search_buf[SEARCH_SIZE];
  UBYTE *pstart = &search_buf[0], *pend, *p;
  ULONG offset, caboff, cablen, foffset, filelen;
  size_t length;
  int state = 0, found = 0, ok = 0;

  /* open the file and search for cabinet headers */
  if ((cab = (Cabinet_t *) calloc(1, sizeof(Cabinet_t)))) {
    cab->filename = name;
    if (cabinet_open(ar, cab)) {
      filelen = (ULONG) cab->filelen;
      for (offset = 0; offset < filelen; offset += length) {
	/* search length is either the full length of the search buffer,
	 * or the amount of data remaining to the end of the file,
	 * whichever is less.
	 */
	length = filelen - offset;
	if (length > SEARCH_SIZE) length = SEARCH_SIZE;

	/* fill the search buffer with data from disk */
	if (!cabinet_read(ar, cab, search_buf, length)) break;

	/* read through the entire buffer. */
	p = pstart;
	pend = &search_buf[length];
	while (p < pend) {
	  switch (state) {
	  /* starting state */
	  case 0:
	    /* we spend most of our time in this while loop, looking for
	     * a leading 'M' of the 'MSCF' signature
	     */
	    while (*p++ != 0x4D && p < pend);
	    if (p < pend) state = 1; /* if we found tht 'M', advance state */
	    break;

	  /* verify that the next 3 bytes are 'S', 'C' and 'F' */
	  case 1: state = (*p++ == 0x53) ? 2 : 0; break;
	  case 2: state = (*p++ == 0x43) ? 3 : 0; break;
	  case 3: state = (*p++ == 0x46) ? 4 : 0; break;

	  /* we don't care about bytes 4-7 */
	  /* bytes 8-11 are the overall length of the cabinet */
	  case 8:  cablen  = *p++;       state++; break;
	  case 9:  cablen |= *p++ << 8;  state++; break;
	  case 10: cablen |= *p++ << 16; state++; break;
	  case 11: cablen |= *p++ << 24; state++; break;

	  /* we don't care about bytes 12-15 */
	  /* bytes 16-19 are the offset within the cabinet of the filedata */
	  case 16: foffset  = *p++;       state++; break;
	  case 17: foffset |= *p++ << 8;  state++; break;
	  case 18: foffset |= *p++ << 16; state++; break;
	  case 19: foffset |= *p++ << 24;
	    /* now we have recieved 20 bytes of potential cab header. */
	    /* work out the offset in the file of this potential cabinet */
	    caboff = offset + (p-pstart) - 20;

	    /* check that the files offset is less than the alleged length
	     * of the cabinet, and that the offset + the alleged length are
	     * 'roughly' within the end of overall file length
	     */
	    if ((foffset < cablen) &&
		((caboff + foffset) < (filelen + 32)) &&
		((caboff + cablen) < (filelen + 32)) )
	    {
	      /* found a potential result - try loading it */
	      found++;
	      cab2 = load_cab_offset(ar, name, (Sfoff_t) caboff);
	      if (cab2) {
		/* success */
		ok++;

		/* cause the search to restart after this cab's data. */
		offset = caboff + cablen;
		if (offset < cab->filelen) cabinet_seek(ar, cab, offset);
		length = 0;
		p = pend;

		/* link the cab into the list */
		if (linkcab == 0) firstcab = cab2;
		else linkcab->next = cab2;
		linkcab = cab2;
	      }
	    }
	    state = 0;
	    break;
	  default:
	    p++, state++; break;
	  }
	}
      }
      cabinet_close(ar, cab);
    }
    free(cab);
  }

  /* if there were cabinets that were found but are not ok, point this out */
  if (found > ok) {
    (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: found %d bad cabinet%s", name, found-ok, (found-ok) == 1 ? "" : "s");
  }

  /* if no cabinets were found, let the user know */
  if (!firstcab) {
    (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: not a cabinet file", name);
  }
  return firstcab;
}

/* UTF translates two-byte unicode characters into 1, 2 or 3 bytes.
 * %000000000xxxxxxx -> %0xxxxxxx
 * %00000xxxxxyyyyyy -> %110xxxxx %10yyyyyy
 * %xxxxyyyyyyzzzzzz -> %1110xxxx %10yyyyyy %10zzzzzz
 *
 * Therefore, the inverse is as follows:
 * First char:
 *  0x00 - 0x7F = one byte char
 *  0x80 - 0xBF = invalid
 *  0xC0 - 0xDF = 2 byte char (next char only 0x80-0xBF is valid)
 *  0xE0 - 0xEF = 3 byte char (next 2 chars only 0x80-0xBF is valid)
 *  0xF0 - 0xFF = invalid
 */

/* translate UTF -> ASCII */
static int
convertUTF(UBYTE *in)
{
  UBYTE c, *out = in, *end = in + strlen((char *) in) + 1;
  ULONG x;

  do {
    /* read unicode character */
    if ((c = *in++) < 0x80) x = c;
    else {
      if (c < 0xC0) return 0;
      else if (c < 0xE0) {
        x = (c & 0x1F) << 6;
        if ((c = *in++) < 0x80 || c > 0xBF) return 0; else x |= (c & 0x3F);
      }
      else if (c < 0xF0) {
        x = (c & 0xF) << 12;
        if ((c = *in++) < 0x80 || c > 0xBF) return 0; else x |= (c & 0x3F)<<6;
        if ((c = *in++) < 0x80 || c > 0xBF) return 0; else x |= (c & 0x3F);
      }
      else return 0;
    }

    /* terrible unicode -> ASCII conversion */
    if (x > 127) x = '_';

    if (in > end) return 0; /* just in case */
  } while ((*out++ = (UBYTE) x));
  return 1;
}

static ULONG
checksum(UBYTE *data, UWORD bytes, ULONG csum)
{
  int len;
  ULONG ul = 0;

  for (len = bytes >> 2; len--; data += 4) {
    csum ^= ((data[0]) | (data[1]<<8) | (data[2]<<16) | (data[3]<<24));
  }

  switch (bytes & 3) {
  case 3: ul |= *data++ << 16;
  case 2: ul |= *data++ <<  8;
  case 1: ul |= *data;
  }
  csum ^= ul;

  return csum;
}

static Sfio_t*
cabpart(Ar_t* ar, Paxfile_t* f, File_t *fi)
{
	Folder_t*	fp = fi->folder;
	Folder_t*	op = ar->current;
	size_t		n;
	ssize_t		m;

	/*
	 * is a change of folder needed?
	 * do we need to reset the current folder?
	 */

	if (!fp)
		return 0;
	if (fp != op || fi->offset < ar->offset)
	{
		UWORD	type = fp->comp_type;
		int	nt = type & CAB_DIR_COMPTYPE_MASK;
		int	ot = (op && nt != CAB_DIR_COMPTYPE_LZX && nt != CAB_DIR_COMPTYPE_QUANTUM) ? (op->comp_type & CAB_DIR_COMPTYPE_MASK) : -1;
		char	method[256];

		n = ar->sync;
		if (nt != ot)
		{
			if (op)
				codexpop(ar->io, NiL, 0);
			ar->skip = ar->sync = 0;
			switch (nt)
			{
			case CAB_DIR_COMPTYPE_NONE:
				sfsprintf(method, sizeof(method), "copy");
				break;
			case CAB_DIR_COMPTYPE_MSZIP:
				sfsprintf(method, sizeof(method), "zip-deflate");
				ar->skip = 2;
				break;
			case CAB_DIR_COMPTYPE_QUANTUM:
				sfsprintf(method, sizeof(method), "quantum-%s-32k", fmtnum(1L << ((type >> 8) & 0x1f), 0));
				ar->sync = 1;
				break;
			case CAB_DIR_COMPTYPE_LZX:
				sfsprintf(method, sizeof(method), "lzx-%s-32k", fmtnum(1L << ((type >> 8) & 0x1f), 0));
				ar->sync = 1;
				break;
			default:
				(*ar->pax->errorf)(NiL, ar->pax, 2, "%s: %s: %04x: unknown compression method", ar->ap->name, f ? f->name : "SKIP", nt);
				return 0;
			}
		}
		if (!op || op->cab[ar->split] != fp->cab[0])
		{
			if (op)
				cabinet_close(ar, op->cab[ar->split]);
			if (!cabinet_open(ar, fp->cab[0]))
				return 0;
			n = 1;
		}
		if (n)
		{
			cabinet_seek(ar, fp->cab[0], fp->offset[0]);
			ar->ip = ar->ie = ar->inbuf;
		}
		ar->current = fp;
		ar->offset = 0;
		ar->split = 0;
		if (nt != ot)
		{
			if (error_info.trace)
				(*ar->pax->errorf)(NiL, ar->pax, -1, "archive=%s file=%s method=%s", ar->ap->name, f->name, method);
			if (codex(ar->io, NiL, method, CODEX_DECODE, &ar->codexdisc, NiL) < 0)
				return 0;
		}
	}
	if (fi->offset > ar->offset)
	{
		/*
		 * skip (but still decompress) these bytes
		 */

		n = fi->offset - ar->offset;
		do
		{
			if ((m = n) > sizeof(ar->buf))
				m = sizeof(ar->buf);
			if ((m = sfread(ar->io, ar->buf, m)) <= 0)
			{
				(*ar->pax->errorf)(NiL, ar->pax, 2, "%s: %s: cabinet data skip error", ar->ap->name, f->name);
				return 0;
			}
		} while (n -= m);
		ar->offset = fi->offset;
	}
	return ar->io;
}

/* tries to find *cabname, from the directory path of origcab, correcting the
 * case of *cabname if necessary, If found, writes back to *cabname.
 */
static void
find_cabinet_file(Ar_t *ar, char **cabname, char *origcab)
{
  char *tail, *cab, *name, *nextpart;
  struct dirent *entry;
  struct stat st_buf;
  int found = 0, len;
  DIR *dir;

  /* ensure we have a cabinet name at all */
  if (!(name = *cabname)) return;

  /* find if there's a directory path in the origcab */
  tail = origcab ? strrchr(origcab, '/') : (char*)0;

  if ((cab = (char *) malloc((tail ? tail-origcab : 1) + strlen(name) + 2))) {
    /* add the directory path from the original cabinet name */
    if (tail) {
      memcpy(cab, origcab, tail-origcab);
      cab[tail-origcab] = '\0';
    }
    else {
      /* default directory path of '.' */
      cab[0] = '.';
      cab[1] = '\0';
    }

    do {
      /* we don't want null cabinet filenames */
      if (name[0] == '\0') break;

      /* if there is a directory component in the cabinet name,
       * look for that alone first
       */
      nextpart = strchr(name, '\\');
      if (nextpart) *nextpart = '\0';

      /* try accessing the component with its current name (case-sensitive) */
      len = strlen(cab); strcat(cab, "/"); strcat(cab, name);
      found = (stat(cab, &st_buf) == 0) && 
	nextpart ? S_ISDIR(st_buf.st_mode) : S_ISREG(st_buf.st_mode);

      /* if the component was not found, look for it in the current dir */
      if (!found) {
	cab[len] = '\0';
	if ((dir = opendir(cab))) {
	  while ((entry = readdir(dir))) {
	    if (strcasecmp(name, entry->d_name) == 0) {
	      strcat(cab, "/"); strcat(cab, entry->d_name); found = 1;
	    }
	  }
	  closedir(dir);
	}
      }
      
      /* restore the real name and skip to the next directory component
       * or actual cabinet name
       */
      if (nextpart) *nextpart = '\\', name = &nextpart[1];

      /* while there is another directory component, and while we
       * successfully found the current component
       */
    } while (nextpart && found);


    /* if we found the cabinet, change the next cabinet's name.
     * otherwise, pretend nothing happened
     */
    if (found) {
      free((void *) *cabname);
      *cabname = cab;
    }
    else {
      free((void *) cab);
    }
  }
}

/* process_cabinet() is called by main() for every file listed on the
 * command line. It will find every cabinet file in that file, and will
 * search for every chained cabinet attached to those cabinets, then it
 * will either extract or list the cabinet(s). Returns 0 for success or 1
 * for failure (unlike most cabextract functions).
 */
static int
process_cabinet(Ar_t* ar, char *cabname, char *dir)
{
  Cabinet_t *basecab, *cab, *cab1, *cab2;

  /* load the file requested */
  basecab = find_cabs_in_file(ar, cabname);
  if (!basecab) return 1;
  /* iterate over all cabinets found in that file */
  for (cab = basecab; cab; cab=cab->next) {
    /* bi-directionally load any spanning cabinets -- backwards */
    for (cab1 = cab; cab1->flags & CAB_HDR_PREV_CABINET; cab1 = cab1->prevcab) {
      (*ar->pax->errorf)(NiL, ar->pax, 1, "%s: extends back to %s (%s)", cabname, cab1->prevname, cab1->previnfo);
      find_cabinet_file(ar, &(cab1->prevname), cabname);
      if (!(cab1->prevcab = load_cab_offset(ar, cab1->prevname, 0))) {
        (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: %s: cannot read previous cabinet", cabname, cab1->prevname);
        break;
      }
      cab1->prevcab->nextcab = cab1;
    }
    /* bi-directionally load any spanning cabinets -- forwards */
    for (cab2 = cab; cab2->flags & CAB_HDR_NEXT_CABINET; cab2 = cab2->nextcab) {
      (*ar->pax->errorf)(NiL, ar->pax, 1, "%s: extends ahead to %s (%s)", cabname, cab2->nextname, cab2->nextinfo);
      find_cabinet_file(ar, &(cab2->nextname), cabname);
      if (!(cab2->nextcab = load_cab_offset(ar, cab2->nextname, 0))) {
        (*ar->pax->errorf)(NiL, ar->pax, 2, "%s: %s: cannot read next cabinet", cabname, cab2->nextname);
        break;
      }
      cab2->nextcab->prevcab = cab2;
    }
    ar->file = process_files(ar, cab1);
    ar->current = 0;
  }
  return 0;
}

static ssize_t
partread(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register Ar_t*	ar = (Ar_t*)disc;
	Cabinet_t*	cab = ar->current->cab[ar->split];
	size_t		r = 0;
	UBYTE*		b = (UBYTE*)buf;
	ssize_t		m;
	ULONG		sum;
	ULONG		chk;
	ULONG		len;
	UBYTE		hdr[CAB_DAT_SIZEOF];
#if 0
static ULONG t;
#endif

	for (;;)
	{
		if (m = ar->ie - ar->ip)
		{
			if (m > n)
				m = n;
			memcpy(b, ar->ip, m);
			b += m;
			ar->ip += m;
			r += m;
			if (!(n -= m))
				break;
		}
		if (ar->eof || !cabinet_read(ar, cab, hdr, sizeof(hdr)))
			return r ? r : 0;
		cabinet_skip(ar, cab, cab->block_resv);
		m = getuword(hdr+CAB_DAT_CompressedSize);
		if (m > sizeof(ar->inbuf))
		{
    			(*ar->pax->errorf)(NiL, ar->pax, 2, "%s: corrupt block: size %lu > %lu", cab->filename, m, sizeof(ar->inbuf));
			return -1;
		}
		if (!cabinet_read(ar, cab, ar->inbuf, m))
			return r ? r : -1;
		len = getuword(hdr+CAB_DAT_UncompressedSize);
		sum = getulong(hdr+CAB_DAT_CheckSum);
		if (sum && sum != (chk = checksum(hdr+4, 4, checksum(ar->inbuf, m, 0))))
		{
			if (len == CAB_BLOCKMAX)
			{
    				(*ar->pax->errorf)(NiL, ar->pax, 2, "%s: block checksum mismatch: %08x expecting %08x", cab->filename, chk, sum);
				return -1;
			}
			ar->eof = 1;
			break;
		}
		ar->ip = ar->inbuf + ar->skip;
		ar->ie = ar->inbuf + m;
		ar->ie[0] = ar->ie[1] = 0;
		if (!len)
		{
			/*
			 * split block
			 */

			ar->ip = ar->ie;
			cabinet_close(ar, cab);
			if (!(cab = ar->current->cab[++ar->split]))
			{
				ar->eof = 1;
				return -1;
			}
			if (!cabinet_open(ar, cab))
				return -1;
			cabinet_seek(ar, cab, ar->current->offset[ar->split]);
		}
		else if (!sum && len < CAB_BLOCKMAX)
			ar->eof = 1;
	}
	return r;
}

static int
cab_getprologue(Pax_t* pax, Paxformat_t* fp, register Paxarchive_t* ap, Paxfile_t* f, unsigned char* buf, size_t size)
{
	register Ar_t*		ar;
	Vmalloc_t*		vm;
	int			fd;

	if (size < CAB_HDR_SIZEOF)
		return 0;
	if (getulong(buf+CAB_HDR_Signature) != 0x4643534d)
		return 0;
	if ((buf[CAB_HDR_MajorVersion] > 1) || (buf[CAB_HDR_MajorVersion] == 1 && buf[CAB_HDR_MinorVersion] > 3))
		(*pax->errorf)(NiL, pax, 3, "%s: %s format version %d.%d may be incompatible", ap->name, fp->name, buf[CAB_HDR_MajorVersion], buf[CAB_HDR_MinorVersion]);
	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)))
	{
		paxnospace(pax);
		return -1;
	}
	if (!(ar = vmnewof(vm, 0, Ar_t, 1, 0)))
	{
		paxnospace(pax);
		vmclose(vm);
		return -1;
	}
	ar->vm = vm;
	ar->pax = pax;
	ar->ap = ap;
	ap->data = ar;
	codexinit(&ar->codexdisc, pax->errorf);
	if (!process_cabinet(ar, ap->name, "."))
	{
		paxseek(pax, ap, 0, SEEK_END, 1);
		if (!(ar->io = sfstropen()))
		{
			paxnospace(ar->pax);
			vmclose(vm);
			return -1;
		}
		ar->io->_flags &= ~(SF_READ|SF_WRITE|SF_STRING);
		ar->io->_flags |= SF_READ;
		fd = open("/dev/null", O_RDWR);
		ar->io->_file = fd;
		sfsetbuf(ar->io, ar->io, CAB_BLOCKMAX);
		ar->disc.readf = partread;
		if (sfdisc(ar->io, &ar->disc) != &ar->disc)
		{
			sfclose(ar->io);
			paxnospace(ar->pax);
			vmclose(vm);
			return -1;
		}
		return 1;
	}
 bad:
	vmclose(vm);
	(*pax->errorf)(NiL, pax, 3, "%s: %s version %d.%d format header corrupted", ap->name, fp->name, buf[CAB_HDR_MajorVersion], buf[CAB_HDR_MinorVersion]);
	return -1;
}

static int
cab_done(Pax_t* pax, register Paxarchive_t* ap)
{
	Ar_t*	ar = (Ar_t*)ap->data;

	if (ar)
	{
		if (ar->io)
			sfclose(ar->io);
		if (ar->vm)
			vmclose(ar->vm);
		ap->data = 0;
	}
	return 0;
}

static int
cab_getheader(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f)
{
	register Ar_t*		ar = (Ar_t*)ap->data;
	register File_t*	file;
	ULONG			t;
	Tm_t			tm;

	if (!(file = ar->file))
		return 0;
	f->linkpath = 0;
	f->name = file->filename;
	if (file->attr & CAB_FIL_A_NAME_IS_UTF)
		convertUTF((UBYTE*)f->name);
	f->st->st_dev = 0;
	f->st->st_ino = 0;
	f->st->st_mode = X_IFREG|X_IRUSR|X_IRGRP|X_IROTH;
	if (!(file->attr & CAB_ATTR_READONLY))
		f->st->st_mode |= X_IWUSR;
	if ((file->attr & CAB_ATTR_EXECUTE) || strmatch(f->name, "*.([Cc][Oo][Mm]|[Bb][Aa][Tt]|[Ee][Xx][Ee]|[BbCcKk][Ss][Hh]|[Pp][Ll]|[Ss][Hh])"))
		f->st->st_mode |= X_IXUSR|X_IXGRP|X_IXOTH;
	f->st->st_uid = pax->uid;
	f->st->st_gid = pax->gid;
	f->st->st_nlink = 1;
	IDEVICE(f->st, 0);
	t = (file->date << 16) | file->time;
	memset(&tm, 0, sizeof(tm));
	tm.tm_year = ((t>>25)&0377) + 80;
	tm.tm_mon = ((t>>21)&017) - 1;
	tm.tm_mday = ((t>>16)&037);
	tm.tm_hour = ((t>>11)&037);
	tm.tm_min = ((t>>5)&077);
	tm.tm_sec = ((t<<1)&037);
	f->st->st_mtime = f->st->st_ctime = f->st->st_atime = tmtime(&tm, TM_LOCALZONE);
	f->st->st_size = f->uncompressed = file->length;
	return 1;
}

static int
cab_getdata(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f, int fd)
{
	register Ar_t*		ar = (Ar_t*)ap->data;
	register File_t*	file;
	Sfio_t*			sp;
	size_t			n;
	ssize_t			m;

	file = ar->file;
	ar->file = file->next;
	if (fd < 0 || !f->st->st_size)
		return 1;
	if (!(sp = cabpart(ar, f, file)))
	{
		ar->file = 0;
		return -1;
	}
	n = f->uncompressed;
	while (n)
	{
		if ((m = n) > sizeof(ar->buf))
			m = sizeof(ar->buf);
		if ((m = sfread(sp, ar->buf, m)) <= 0)
		{
			if (m)
				(*pax->errorf)(NiL, pax, 2, "%s: %s: read error", ap->name, f->name);
			else
				(*pax->errorf)(NiL, pax, 2, "%s: %s: unexpected EOF", ap->name, f->name);
			return -1;
		}
		if (paxdata(pax, ap, f, fd, ar->buf, m))
			return -1;
		n -= m;
	}
	ar->offset += f->uncompressed;
	return 1;
}

/*
 * pax cab format
 */

Paxformat_t	pax_cab_format =
{
	"cab",
	"cabinet",
	"MS cabinet file",
	0,
	PAX_ARCHIVE|PAX_DOS|PAX_NOHARDLINKS|PAX_IN,
	PAX_DEFBUFFER,
	PAX_DEFBLOCKS,
	0,
	PAXNEXT(pax_cab_next),
	0,
	cab_done,
	cab_getprologue,
	cab_getheader,
	cab_getdata,
};

PAXLIB(&pax_cab_format)

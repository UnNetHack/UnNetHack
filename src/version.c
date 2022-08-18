/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "date.h"
/*
 * All the references to the contents of patchlevel.h have been moved
 * into makedefs....
 */
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif

/* #define BETA_INFO "" */  /* "[ beta n]" */

/* fill buffer with short version (so caller can avoid including date.h) */
char *
version_string(char *buf)
{
    return strcpy(buf, VERSION_STRING);
}

/* fill and return the given buffer with the long nethack version string */
char *
getversionstring(char *buf)
{
    Strcpy(buf, VERSION_ID);
#if defined(BETA) && defined(BETA_INFO)
    Sprintf(eos(buf), " %s", BETA_INFO);
#endif
#if defined(RUNTIME_PORT_ID)
    append_port_id(buf);
#endif
    return buf;
}

/** the 'V' command */
int
doversion(void)
{
    char buf[BUFSZ];

    pline("%s", getversionstring(buf));
    return 0;
}

/** the '#version' command; also a choice for '?' */
int
doextversion(void)
{
    display_file_area(NH_OPTIONS_USED_AREA, OPTIONS_USED, TRUE);
    return 0;
}

#ifdef MICRO
boolean
comp_times(filetime)
long filetime;
{
    return((boolean)(filetime < BUILD_TIME));
}
#endif

boolean
check_version(struct version_info *version_data, const char *filename, boolean complain)
{
    if (
#ifdef VERSION_COMPATIBILITY
        version_data->incarnation < VERSION_COMPATIBILITY ||
        version_data->incarnation > VERSION_NUMBER
#else
        version_data->incarnation != VERSION_NUMBER
#endif
        ) {
        if (complain)
            pline("Version mismatch for file \"%s\".", filename);
        return FALSE;
    } else if (
#ifndef IGNORED_FEATURES
        version_data->feature_set != VERSION_FEATURES ||
#else
        (version_data->feature_set & ~IGNORED_FEATURES) !=
        (VERSION_FEATURES & ~IGNORED_FEATURES) ||
#endif
        version_data->entity_count != VERSION_SANITY1 ||
        version_data->struct_sizes != VERSION_SANITY2) {
        if (complain)
            pline("Configuration incompatibility for file \"%s\".",
                  filename);
        return FALSE;
    }
    return TRUE;
}

/* this used to be based on file date and somewhat OS-dependant,
   but now examines the initial part of the file's contents */
boolean
uptodate(int fd, const char *name)
{
    int rlen;
    struct version_info vers_info;
    boolean verbose = name ? TRUE : FALSE;

    rlen = read(fd, (genericptr_t) &vers_info, sizeof vers_info);
    minit();        /* ZEROCOMP */
    if (rlen == 0) {
        if (verbose) {
            pline("File \"%s\" is empty?", name);
            wait_synch();
        }
        return FALSE;
    }
    if (!check_version(&vers_info, name, verbose)) {
        if (verbose) wait_synch();
        return FALSE;
    }
    return TRUE;
}

void
store_version(int fd)
{
    static const struct version_info version_data = {
        VERSION_NUMBER, VERSION_FEATURES,
        VERSION_SANITY1, VERSION_SANITY2
    };

    bufoff(fd);
    /* bwrite() before bufon() uses plain write() */
    bwrite(fd, (genericptr_t)&version_data, (unsigned)(sizeof version_data));
    bufon(fd);
    return;
}

#ifdef AMIGA
const char amiga_version_string[] = AMIGA_VERSION_STRING;
#endif

unsigned long
get_feature_notice_ver(char *str)
{
    char buf[BUFSZ];
    int ver_maj, ver_min, patch;
    char *istr[3];
    int j = 0;

    if (!str) return 0L;
    str = strcpy(buf, str);
    istr[j] = str;
    while (*str) {
        if (*str == '.') {
            *str++ = '\0';
            j++;
            istr[j] = str;
            if (j == 2) break;
        } else if (index("0123456789", *str) != 0) {
            str++;
        } else
            return 0L;
    }
    if (j != 2) return 0L;
    ver_maj = atoi(istr[0]);
    ver_min = atoi(istr[1]);
    patch = atoi(istr[2]);
    return FEATURE_NOTICE_VER(ver_maj, ver_min, patch);
    /* macro from hack.h */
}

unsigned long
get_current_feature_ver(void)
{
    return FEATURE_NOTICE_VER(VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
}

/*version.c*/

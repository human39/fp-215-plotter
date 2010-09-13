#include <stdio.h>
#include <string.h>
#include "getopt.h"

int optind=1;           /* index of which argument is next      */
char *optarg=NULL;      /* pointer to argument of current option */
static char *letP=NULL; /* remember next option char's location */
static char SW='/';     /* DOS switch character:  '/' */

/*      getopt.c
 *
 * Copyright (c) 1986,87,88 by Borland International Inc.
 * All Rights Reserved.
 *
 * Parse the command line options, System V style.
 */
int getopt(int argc, char *argv[], char *optionS)
{
   unsigned char ch;
   char *optP;

   /* More arguments? */
   if (optind < argc) {

      /* Start processing a new argument? */
      if (letP == NULL) {

         /* Out of arguments or next argument not a switch? */
         if ((letP = argv[optind]) == NULL || *(letP++) != SW) goto gopEOF;

         /* Two switches in a row? */
         if (*letP == SW) { optind++; goto gopEOF;       }
      }

      /* SW without an option? */
      if ( (ch = *(letP++)) == 0) { optind++; goto gopEOF; }

      /* illegal option ':'  or unknown option? */
      if (ch == ':' || (optP = strchr(optionS, ch)) == NULL) goto gopError;

      /* Is option an argLetter?      */
      if (*(++optP) == ':') {
         optind++;

         /* Is there white space between the argLetter and the arg? */
         if (*letP == 0) {

            /* Out of arguments? */
            if (optind >= argc) goto gopError;

            letP = argv[optind++];
         }
         optarg = letP;
         letP = NULL;
      }
      else {  /* Option is just a switch */

         /* Out of options for this argument? */
         if (*letP == 0) {
            optind++;
            letP = NULL;
         }
         optarg = NULL;
      }
      return ch;
   }

gopEOF:
   optarg = letP = NULL;
   return EOF;

gopError:
   optind++;
   optarg = letP = NULL;
   return ':';
}


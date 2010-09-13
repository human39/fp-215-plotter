#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#include "rsplot.h"
#include "getopt.h"

#define VERSION "0.23"

#define MAXLLEN 81
#define MAXCLEN 3
#define ADOFS 100
#define SCALE 4
#define MAXX0 2699
#define MAXY0 1859
#define MAXX1 2979
#define MAXY1 2159

enum { PU, PD, PA, SP };
typedef struct STRUCTCMD { char *cmd; int narg; } CMD;

const CMD vldcmd[]={ { "PU", 0 }, { "PD", 0 }, { "PA", 2 }, { "SP", 1 }, { NULL, 0 } };

int verbose=0, quiet=0;

void usage(int status);

int main(int argc, char *argv[])
{
   FILE *fpi, *fpo=NULL;
   int ffo[8]={ 0 };
   char line[MAXLLEN], cmd[MAXCLEN];
   char fnami[MAXPATH], fnamo[MAXPATH];
   char fdrv[MAXDRIVE], fdir[MAXDIR], ffil[MAXFILE], fext[MAXEXT];
   int penmd='M', pennm=0, pen=0, form=0, adapt=0, delfi=0;
   unsigned long nvect=0;
   int maxx=0, maxy=0;
   int argx, argy, i, j, st;

   while ((i=getopt(argc, argv, "fp:adqv?"))!=EOF)
      switch (i) {
      case '?':
         usage(0);
         break;
      case 'v':
         verbose=!0;
      case 'q':
         quiet=!verbose;
         break;
      case 'd':
         delfi=!0;
         break;
      case 'a':
         adapt=!0;
         break;
      case 'p':
         pen=!0;
         pennm=(atoi(optarg)-1)&7;
         break;
      case 'f':
         form=!0;
         break;
      default:
         fprintf(stderr, "\nIllegal option\n");
         usage(1);
         break;
      }

   /* reset argument vector and count */
   argc-= optind;
   argv = &argv[optind];

   if (argc<1) {
      fprintf(stderr, "\nCommand parameters not found\n");
      usage(1);
   }

   if (fnsplit(argv[0], fdrv, fdir, ffil, fext)&EXTENSION)
      fnmerge(fnami, fdrv, fdir, ffil, fext);
   else
      fnmerge(fnami, fdrv, fdir, ffil, ".PLT");

   if (argc>1)
      fnsplit(argv[1], fdrv, fdir, ffil, fext);

   fnmerge(fnamo, fdrv, fdir, ffil, ".PLn");

   if (!quiet)
      printf("Converting HPGL file %s to RSGL file(s) %s\n\n", fnami, fnamo);

   if ((fpi=fopen(fnami, "rb"))==NULL) {
      fprintf(stderr, "Error opening %s\n", fnami);
      exit(1);
   }

   if (verbose) {
      printf("Input file: %s\n", fnami);
      printf("Output file(s): %s\n", fnamo);
      printf("Input file will%s be deleted\n", (delfi ? "" : " not"));
      if (adapt)
         printf("Plot will be offset %d.%02d\" for adapter\n",
         ADOFS/254, (ADOFS-ADOFS/254*254)*100/254);
      else
         printf("Plot will not use adapter\n");
      if (pen)
         printf("Plot will use pen %c\n", (char)('1'+pennm));
      else
         printf("Plot will use all pens\n");
      printf("Plot will use form %d (%d.%02d\" x %d.%02d\")\n", form,
         (form ? MAXX1+1 : MAXX0+1)/254, ((form ? MAXX1+1 : MAXX0+1)-(form ? MAXX1+1 : MAXX0+1)/254*254)*100/254,
         (form ? MAXY1+1 : MAXY0+1)/254, ((form ? MAXY1+1 : MAXY0+1)-(form ? MAXY1+1 : MAXY0+1)/254*254)*100/254);
      printf("\n");
   }

   while ((i=fscanf(fpi, " %80[^;\n]%*c", line))!=EOF)
      if (i>0) {
         if ((i=sscanf(line, "%2[^-+0-9]", cmd))>0) {
            for (j=0; vldcmd[j].cmd!=NULL; j++)
               if (strcmpi(vldcmd[j].cmd, cmd)==0) break;
            if (vldcmd[j].cmd!=NULL) {
               i=strlen(vldcmd[j].cmd);
               switch (vldcmd[j].narg) {
                  case 0: st=0; break;
                  case 1: st=sscanf(&line[i], " %d", &argx); break;
                  case 2: st=sscanf(&line[i], " %d , %d", &argx, &argy); break;
                  default: st=-1; break;
               }
               switch (j) {
                  case PU: penmd='M'; break;
                  case PD: penmd='D'; break;
                  case PA: if (fpo==NULL) {
                              fnamo[strlen(fnamo)-1]=(char)('1'+pennm);
                              if ((fpo=fopen(fnamo, (ffo[pennm] ? "ab" : "wb")))==NULL) {
                                 fprintf(stderr, "Error (re)opening %s\n", fnamo);
                                 fclose(fpi);
                                 exit(1);
                              }
                              if (!ffo[pennm]) {
                                 if (!quiet) printf("Creating file: %s\n", fnamo);
                                 fprintf(fpo, "F%d\rI%d,0\rL0\r", form, (adapt ? ADOFS : 0));
                                 ffo[pennm]=!0;
                              }
                           }
                           argx/=SCALE; argy/=SCALE;
                           fprintf(fpo, "%c%d,%d\r", penmd, argx, argy);
                           if (argx>maxx) maxx=argx;
                           if (argy>maxy) maxy=argy;
                           nvect++;
                           break;
                  case SP: if (!pen)
                              if (st>=1)
                                 if (pennm!=(argx-1)&7) {
                                    pennm=(argx-1)&7;
                                    if (fpo!=NULL)
                                       fclose(fpo);
                                    fpo=NULL;
                                 }
                           penmd='M';
                           break;
               }
            }
            else
               if (verbose) printf("Unsupported HPGL command: %s\n", line);
         }
      }
      else
         fscanf(fpi, "%*c");

   fclose(fpi);

   if (fpo!=NULL) {
      fprintf(fpo, "H\r");
      fclose(fpo);
      i=pennm;
   }
   else
      i=-1;
   for (pennm=0; pennm<8; pennm++)
      if (pennm!=i)
         if (ffo[pennm]) {
            fnamo[strlen(fnamo)-1]=(char)('1'+pennm);
            if ((fpo=fopen(fnamo, "ab"))==NULL) {
               fprintf(stderr, "Error reopening %s\n", fnamo);
               exit(1);
            }
            fprintf(fpo, "H\r");
            fclose(fpo);
         }

   if (delfi)
      if (!remove(fnami))
         if (!quiet) printf("\nInput file %s deleted\n", fnami);

   if (!quiet) {
      printf("\n%lu vectors processed\n", nvect);
      printf("%d.%02d\" x %d.%02d\" plot size\n",
         (maxx+1)/254, ((maxx+1)-(maxx+1)/254*254)*100/254,
         (maxy+1)/254, ((maxy+1)-(maxy+1)/254*254)*100/254);
      if (maxx+(adapt ? ADOFS : 0)>(form ? MAXX1 : MAXX0) ||
          maxy>(form ? MAXY1 : MAXY0))
         printf("Maximum plot size exceeded!\n");
   }

   return 0;
}

/*
 * usage
 */
void usage(int status)
{
   printf("\nVersion %s usage:  rsplot [options] infile[.PLT] [outfile]\n\n", VERSION);
   printf("Options            (default)\n");
   printf("----------------------------\n");
   printf("/f     Form        (form0)\n");
   printf("/p #   Pen (1-8)   (allpens)\n");
   printf("/a     Adapt       (noadapt)\n");
   printf("/d     Delete      (nodelete)\n");
   printf("/q     Quiet       (noquiet)\n");
   printf("/v     Verbose     (noverbose)\n");
   printf("/?     Usage       (nousage)\n");
   exit(status);
}

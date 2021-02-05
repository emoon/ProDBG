
/*
 *     xDMS -  Portable DMS archive unpacker - Public Domain
 *     Written by     Andre Rodrigues de la Rocha  <adlroc@usa.net>
 *
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>

#include "cdata.h"
#include "pfile.h"
#include "crc_csum.h"
#include "xdmsconfig.h"

#ifdef UNDER_DOS
#include <io.h>
#include <fcntl.h>
#endif


#define FNAME_MAXC 512

/*
static void Usage(void);
static int strcmpnc(char *, char *);
static void strcpymax(char *, char *, int);
static void strcatmax(char *, char *, int);
static void ErrMsg(USHORT, char *, char *);
*/

#if 0
int main(int argc, char **argv){
    USHORT i, cmd=0, opt=0, ret, PCRC=0, pwd=0;
    int ext;
    char iname[FNAME_MAXC+1], oname[FNAME_MAXC+1], cmdstr[FNAME_MAXC+20], *inm, *onm, *p, *q, *destdir=NULL;
    char tname[FNAME_MAXC];


    if (argc < 3) {
        Usage();
        exit(EXIT_FAILURE);
    }

    /*  proccess options in the command line  */
    for (i=1; (i<argc) && (argv[i][0] == '-'); i++){
        if (strlen(argv[i])>2) {
            fprintf(stderr,"Unknown option !\n");
            Usage();
            exit(EXIT_FAILURE);
        }
        switch (tolower(argv[i][1])) {
        case 'f':
            OverrideErrors = 1;
            break;
        case 'q' :
            opt = OPT_QUIET;
            break;
        case 'v' :
            opt = OPT_VERBOSE;
            break;
        case 'd' :
            if (++i == argc) {
                Usage();
                exit(EXIT_FAILURE);
            }
            destdir = argv[i];
            break;
        case 'p' :
            if (++i == argc) {
                Usage();
                exit(EXIT_FAILURE);
            }
            PCRC = CreateCRC((UCHAR*)argv[i],(ULONG)strlen(argv[i]));
            pwd = 1;
            break;
        default:
            fprintf(stderr,"Unknown option !\n");
            Usage();
            exit(EXIT_FAILURE);
        }
    }

    if ((i == argc) || (strlen(argv[i])>1)) {
        Usage();
        exit(EXIT_FAILURE);
    }


    switch (tolower(argv[i][0])) {
        case 'u':
            cmd = CMD_UNPACK;
            break;
        case 'z':
            cmd = CMD_UNPKGZ;
            break;
        case 'x':
            cmd = CMD_EXTRACT;
            break;
        case 't':
            cmd = CMD_TEST;
            break;
        case 'v':
            cmd = CMD_VIEW;
            break;
        case 'f':
            cmd = CMD_VIEWFULL;
            break;
        case 'd':
            cmd = CMD_SHOWDIZ;
            break;
        case 'b':
            cmd = CMD_SHOWBANNER;
            break;
        default:
            fprintf(stderr,"Unknown command !\n");
            Usage();
            exit(EXIT_FAILURE);
    }

    if (++i == argc) {
        Usage();
        exit(EXIT_FAILURE);
    }

    ext = EXIT_SUCCESS;

    while (i < argc) {

        if (!strcmpnc("stdin",argv[i])) {
            inm = NULL;
        } else {
            strcpymax(iname,argv[i],FNAME_MAXC);
            if ((strlen(iname)<4) || (strcmpnc(".dms",iname+strlen(iname)-4))) strcatmax(iname,".dms",FNAME_MAXC);
            inm = iname;
        }
        i++;


        /*  generate the output filename  */
        if ((i < argc) && (argv[i][0]=='+')) {
            if ((!strcmpnc("stdout",argv[i]+1)) || (destdir && (!strcmpnc("stdout",destdir)))) {
                strcpy(oname,"");
                onm = NULL;
            } else {
                if (destdir) {
                    strcpymax(oname,destdir,FNAME_MAXC-1);
                    p = oname + strlen(oname) - 1;
                    if (!strchr(DIR_SEPARATORS,*p)) {
                        *(p+1) = DIR_CHAR;
                        *(p+2) = '\0';
                    }
                } else strcpy(oname,"");
                strcatmax(oname,argv[i]+1,FNAME_MAXC);
                if (((cmd == CMD_UNPACK) || (cmd == CMD_UNPKGZ)) && (strlen(oname)>0)) {
                    p = oname + strlen(oname) - 1;
                    if (strchr(DIR_SEPARATORS,*p)) {
                        if (inm) {
                            p = q = iname;
                            while(*p) {
                                if (strchr(DIR_SEPARATORS,*p)) q = p+1;
                                p++;
                            }
                            strcatmax(oname,q,FNAME_MAXC);
                            if ((strlen(oname)>4) && (!strcmpnc(oname+strlen(oname)-4,".dms"))) {
                                if (cmd == CMD_UNPKGZ)
                                    strcpy(oname+strlen(oname)-4,".adz");
                                else
                                    strcpy(oname+strlen(oname)-4,".adf");
                            } else {
                                if (cmd == CMD_UNPKGZ)
                                    strcatmax(oname,".adz",FNAME_MAXC);
                                else
                                    strcatmax(oname,".adf",FNAME_MAXC);
                            }
                        } else {
                            if (cmd == CMD_UNPKGZ)
                                strcatmax(oname,"stdin.adz",FNAME_MAXC);
                            else
                                strcatmax(oname,"stdin.adf",FNAME_MAXC);
                        }

                    }
                }

                onm = oname;
            }
            i++;
        } else if (destdir && (!strcmpnc("stdout",destdir))) {
            strcpy(oname,"");
            onm = NULL;
        } else {

            if (destdir)
                strcpymax(oname,destdir,FNAME_MAXC-1);
            else
                strcpy(oname,"");

            if ((cmd == CMD_UNPACK) || (cmd == CMD_UNPKGZ)) {

                if (strlen(oname)>0) {
                    p = oname + strlen(oname) - 1;
                    if (!strchr(DIR_SEPARATORS,*p)) {
                        *(p+1) = DIR_CHAR;
                        *(p+2) = '\0';
                    }
                }

                if (inm) {
                    p = q = iname;
                    while(*p) {
                        if (strchr(DIR_SEPARATORS,*p)) q = p+1;
                        p++;
                    }
                    strcatmax(oname,q,FNAME_MAXC);
                    if ((strlen(oname)>4) && (!strcmpnc(oname+strlen(oname)-4,".dms"))) {
                        if (cmd == CMD_UNPKGZ)
                            strcpy(oname+strlen(oname)-4,".adz");
                        else
                            strcpy(oname+strlen(oname)-4,".adf");
                    } else {
                        if (cmd == CMD_UNPKGZ)
                            strcatmax(oname,".adz",FNAME_MAXC);
                        else
                            strcatmax(oname,".adf",FNAME_MAXC);
                    }
                } else {
                    if (cmd == CMD_UNPKGZ)
                        strcatmax(oname,"stdin.adz",FNAME_MAXC);
                    else
                        strcatmax(oname,"stdin.adf",FNAME_MAXC);
                }

            }

            onm = oname;

        }



        if (opt == OPT_VERBOSE) {
            if ((cmd == CMD_UNPACK)) {
                if (inm)
                    fprintf(stderr,"Unpacking file %s to ",inm);
                else
                    fprintf(stderr,"Unpacking data from stdin to ");
                if (onm)
                    fprintf(stderr,"%s\n",onm);
                else
                    fprintf(stderr,"stdout\n");
            } else if ((cmd == CMD_EXTRACT) || (cmd == CMD_UNPKGZ)) {
                if (inm)
                    fprintf(stderr,"Unpacking file %s\n",inm);
                else
                    fprintf(stderr,"Unpacking data from stdin\n");
            } else if (cmd == CMD_TEST) {
                if (inm)
                    fprintf(stderr,"Testing file %s\n",inm);
                else
                    fprintf(stderr,"Testing data from stdin\n");
            } else if (cmd == CMD_SHOWDIZ) {
                if (inm)
                    printf("Showing FILEID.DIZ in %s :\n",inm);
                else
                    printf("Showing FILEID.DIZ in stdin :\n");
            } else if (cmd == CMD_SHOWBANNER) {
                if (inm)
                    printf("Showing Banner in %s :\n",inm);
                else
                    printf("Showing Banner in stdin :\n");
            }

        }

        #ifdef UNDER_DOS
        if (!inm) setmode(fileno(stdin),O_BINARY);
        if ((cmd == CMD_UNPACK) && (!onm)) setmode(fileno(stdout),O_BINARY);
        #endif

        if ((cmd == CMD_UNPKGZ) || (cmd == CMD_EXTRACT)) {
            int fd;
            strcpy(tname, "/tmp/xdmsXXXXXX");
            fd = mkstemp(tname);
            if (fd < 0) {
                fprintf(stderr, "couldn't create a temp file\n");
                exit(-1);
            }
            close(fd);
            #ifdef UNDER_DOS
            p = tname;
            if (p) {
                while (*p) {
                    if (*p == '/') *p = '\\';
                    p++;
                }
            }
            #endif
            ret = Process_File(inm, tname, CMD_UNPACK, opt, PCRC, pwd);
            if (opt != OPT_QUIET) ErrMsg(ret, inm, "Temporary file");
            if (ret == NO_PROBLEM) {
                if (cmd == CMD_UNPKGZ) {
                    if (opt == OPT_VERBOSE) {
                        fprintf(stderr,"Repacking unpacked data with gzip\n");
                    }
                    if (onm)
                        #ifdef UNDER_DOS
                        /*  DOS sucks  */
                        sprintf(cmdstr,"gzip -cfqn %s >%s",tname,onm);
                        #else
                        sprintf(cmdstr,"gzip -cfqn \"%s\" >\"%s\"",tname,onm);
                        #endif
                    else
                        #ifdef UNDER_DOS
                        sprintf(cmdstr,"gzip -cfqn %s",tname);
                        #else
                        sprintf(cmdstr,"gzip -cfqn \"%s\"",tname);
                        #endif
                    if (system(cmdstr)) ret = ERR_GZIP;
                    if (opt != OPT_QUIET) ErrMsg(ret, inm, onm);
                } else {
                    if (opt == OPT_VERBOSE) {
                        fprintf(stderr,"Extracting files from unpacked data with readdisk\n");
                    }
                    if ((onm) && (strlen(onm)>0))
                        #ifdef UNDER_DOS
                        sprintf(cmdstr,"readdisk %s %s",tname,onm);
                        #else
                        sprintf(cmdstr,"readdisk \"%s\" \"%s\"",tname,onm);
                        #endif
                    else
                        #ifdef UNDER_DOS
                        sprintf(cmdstr,"readdisk %s",tname);
                        #else
                        sprintf(cmdstr,"readdisk \"%s\"",tname);
                        #endif
                    if (system(cmdstr)) ret = ERR_READDISK;
                    if (opt != OPT_QUIET) ErrMsg(ret, inm, onm);
                }
            }
            remove(tname);
        } else {
            ret = Process_File(inm, onm, cmd, opt, PCRC, pwd);
            if (opt != OPT_QUIET) ErrMsg(ret, inm, onm);
        }

        if (ret != NO_PROBLEM) ext = EXIT_FAILURE;

        if ((ret == NO_PROBLEM) && (opt != OPT_QUIET)) {
            switch (cmd) {
                case CMD_UNPACK:
                    if (inm)
                        fprintf(stderr,"File %s was correctly unpacked to ",inm);
                    else
                        fprintf(stderr,"Data from stdin was correctly unpacked to ");
                    if (onm)
                        fprintf(stderr,"%s\n",onm);
                    else
                        fprintf(stderr,"stdout\n");
                    break;
                case CMD_UNPKGZ:
                    if (inm)
                        fprintf(stderr,"File %s was correctly converted to ",inm);
                    else
                        fprintf(stderr,"Data from stdin was correctly converted to ");
                    if (onm)
                        fprintf(stderr,"%s\n",onm);
                    else
                        fprintf(stderr,"stdout\n");
                    break;
                case CMD_EXTRACT:
                    if (inm)
                        fprintf(stderr,"The files were correctly extracted from %s\n",inm);
                    else
                        fprintf(stderr,"The files were correctly extracted from stdin\n");
                    break;
                case CMD_TEST:
                    if (inm)
                        fprintf(stderr,"File %s is ok\n",inm);
                    else
                        fprintf(stderr,"Data from stdin is ok\n");
                    break;
                default:
                    break;
            }
        }

        if (opt != OPT_QUIET) fprintf(stderr,"\n");

    }

    return (int) ext;
}
#endif

#if 0
static int strcmpnc(char *s1, char *s2){
    while (*s1 && (tolower(*s1)==tolower(*s2))) {s1++; s2++;}
    return tolower(*s1)-tolower(*s2);
}



static void strcpymax(char *s1, char *s2, int max){
    if (strlen(s2)>max){
        memcpy(s1,s2,max);
        *(s1+max) = 0;
    } else
        strcpy(s1,s2);
}



static void strcatmax(char *s1, char *s2, int max){
    if (strlen(s1)+strlen(s2)>max){
        memcpy(s1+strlen(s1),s2,max-strlen(s1));
        *(s1+max) = 0;
    } else
        strcat(s1,s2);
}



static void Usage(void)
{
    printf("\n");
    printf(" xDMS  v%s  -  Portable DMS archive unpacker  -  Public Domain\n", VERSION);
    printf(" Written by     Andre Rodrigues de la Rocha  <adlroc@usa.net>\n");
    printf(" Maintained by  Heikki Orsila <heikki.orsila@iki.fi>\n\n");
    printf(" Usage: xdms [options] <command> {<dms_file[.dms]> [+output]} \n\n");
    printf(" Commands :\n");
    printf("     t : Test DMS archives\n");
    printf("     u : Unpack DMS archives to disk images\n");
    printf("     z : Unpack to disk images and compress it with gzip\n");
    printf("     x : Extract files inside DMS archives using readdisk\n");
    printf("     v : View DMS archives information\n");
    printf("     f : View full information\n");
    printf("     d : Show attached FILEID.DIZ\n");
    printf("     b : Show attached Banner\n\n");
    printf(" Options :\n");
    printf("    -f : Override errors (for desperate data salvaging)\n");
    printf("    -q : Quiet\n");
    printf("    -v : Verbose\n");
    printf("    -d <destdir>  : Set destination directory\n");
    printf("    -p <password> : Decrypt encrypted archives using password\n");
    printf("\n");
}



static void ErrMsg(USHORT err, char *i, char *o){

    if (!i) i = "stdin";
    if (!o) o = "stdout";

    switch (err) {
        case NO_PROBLEM:
        case FILE_END:
            return;
        case ERR_NOMEMORY:
            fprintf(stderr,"Not enough memory for buffers !\n");
            break;
        case ERR_CANTOPENIN:
            fprintf(stderr,"Can't open %s for reading !\n",i);
            break;
        case ERR_CANTOPENOUT:
            fprintf(stderr,"Can't open %s for writing !\n",o);
            break;
        case ERR_NOTDMS:
            fprintf(stderr,"File %s is not a DMS archive !\n",i);
            break;
        case ERR_SREAD:
            fprintf(stderr,"Error reading file %s : unexpected end of file !\n",i);
            break;
        case ERR_HCRC:
            fprintf(stderr,"Error in file %s : header CRC error !\n",i);
            break;
        case ERR_NOTTRACK:
            fprintf(stderr,"Error in file %s : track header not found !\n",i);
            break;
        case ERR_BIGTRACK:
            fprintf(stderr,"Error in file %s : track too big !\n",i);
            break;
        case ERR_THCRC:
            fprintf(stderr,"Error in file %s : track header CRC error !\n",i);
            break;
        case ERR_TDCRC:
            fprintf(stderr,"Error in file %s : track data CRC error !\n",i);
            break;
        case ERR_CSUM:
            fprintf(stderr,"Error in file %s : checksum error after unpacking !\n",i);
            fprintf(stderr,"This file seems ok, but the unpacking failed.\n");
            fprintf(stderr,"This can be caused by a bug in xDMS. Please contact the author\n");
            break;
        case ERR_CANTWRITE:
            fprintf(stderr,"Error : can't write to file %s  !\n",o);
            break;
        case ERR_BADDECR:
            fprintf(stderr,"Error in file %s : error unpacking !\n",i);
            fprintf(stderr,"This file seems ok, but the unpacking failed.\n");
            fprintf(stderr,"This can be caused by a bug in xDMS. Please contact the author\n");
            break;
        case ERR_UNKNMODE:
            fprintf(stderr,"Error in file %s : unknown compression mode used !\n",i);
            break;
        case ERR_NOPASSWD:
            fprintf(stderr,"Can't process file %s : file is encrypted !\n",i);
            break;
        case ERR_BADPASSWD:
            fprintf(stderr,"Error unpacking file %s . The password is probably wrong.\n",i);
            break;
        case ERR_FMS:
            fprintf(stderr,"Error in file %s : this file is not really a compressed disk image, but an FMS archive !\n",i);
            break;
        case ERR_GZIP:
            fprintf(stderr,"Can't convert file %s : gzip failed !\n",i);
            break;
        case ERR_READDISK:
            fprintf(stderr,"Can't extract files from %s : readdisk failed !\n",i);
            break;
        default:
            fprintf(stderr,"Error while processing file  %s : internal error !\n",i);
            fprintf(stderr,"This is a bug in xDMS\n");
            fprintf(stderr,"Please contact the author\n");
            break;
    }
}
#endif

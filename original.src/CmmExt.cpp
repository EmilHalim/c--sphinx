
#include "Windows.h"
#include <process.h>    // dos
#include <conio.h>      // dos
#include <direct.h>     // dos
#include <io.h>         // dos
#include <ctype.h>      // dos/linux
#include <fcntl.h>      // dos/linux
#include <math.h>       // dos/linux
#include <stdio.h>      // dos/linux
#include <string.h>     // dos/linux
#include <stddef.h>     // dos/linux
#include <stdlib.h>     // dos/linux
#include <setjmp.h>     // dos/linux
#include <time.h>       // dos/linux
#include <stdarg.h>     // dos/linux

  #define atanl atan
  #define sinl  sin
  #define cosl  cos
  #define tanl  tan
  #define asinl asin
  #define acosl acos
  #define log10l log10
  #define logl   log
  #define _fcloseall fcloseall

#include "tok.h"

#define cSizeOfDefaultString 512

// *************************************************
//                 extrenal functions
// *************************************************

extern  char *rawfilename;
extern  char *rawext;
extern  void *MALLOC(unsigned long size);
// *************************************************
//                 Runtime Functions
// *************************************************
static char    OnExitFile[100][cSizeOfDefaultString];
static char    OnEntryFile[100][cSizeOfDefaultString];
//static char    OnExitParam[100][cSizeOfDefaultString];
//static char    OnEntryParam[100][cSizeOfDefaultString];
static int     OnExitFileCnt;
static int     OnEntryFileCnt;

char OrginalFileName[255];

static char *LowCase;

char *_stristr_(char *String, char *Pattern)
{
  int   mi=-1;
  while(Pattern[++mi])
   {
     if(String[mi]==0) return 0;
     if(LowCase[(unsigned char)String[mi]]!=LowCase[(unsigned char)Pattern[mi]])
       { String++; mi=-1; }
   }
  return String;
}

char *_strstr_(char *String, char *Pattern)
{
  int   mi=-1;
  while(Pattern[++mi])
   {
     if(String[mi]==0) return 0;
     if(String[mi]!=Pattern[mi])
       { String++; mi=-1; }
   }
  return String;
}

#define BCXTmpStrSize  2048
char *BCX_TmpStr (size_t Bites)
{
  static int   StrCnt;
  static char *StrFunc[BCXTmpStrSize];
  StrCnt=(StrCnt + 1) & (BCXTmpStrSize-1);
  if(StrFunc[StrCnt]) {free (StrFunc[StrCnt]); StrFunc[StrCnt] = NULL;}
  StrFunc[StrCnt]=(char*)calloc(Bites+128,sizeof(char));
  return StrFunc[StrCnt];
}

char *replace (char *src, char *pat, char *rep)
{
  register size_t patsz, repsz, tmpsz, delta;
  register char *strtmp, *p, *q, *r;
  if (!pat || !*pat)
   {
     strtmp = BCX_TmpStr(strlen(src));
     if (!strtmp) return NULL;
     return strcpy(strtmp, src);
   }
  repsz = strlen(rep);
  patsz = strlen(pat);
  for (tmpsz=0, p=src; (q=_strstr_(p,pat))!=0; p=q+patsz)
   tmpsz += (size_t) (q - p) + repsz;
   tmpsz += strlen(p);
   strtmp = BCX_TmpStr(tmpsz);
   if (!strtmp) return NULL;
    for (r=strtmp,p=src; (q=_strstr_(p,pat))!=0;p=q+patsz)
     {
       delta = (size_t) (q-p);
       memcpy(r,p,delta); r += delta;
       strcpy(r,rep);      r += repsz;
     }
  strcpy(r,p);
  return strtmp;
}

char *iReplace (char *src, char *pat, char *rep)
{
  size_t patsz, repsz, tmpsz, delta;
  char *strtmp, *p, *q, *r;
  if (!pat || !*pat)
   {
     strtmp = BCX_TmpStr(strlen(src));
     if (!strtmp) return NULL;
     return strcpy(strtmp, src);
   }
  repsz = strlen(rep);
  patsz = strlen(pat);
  for (tmpsz=0, p=src;(q=_stristr_(p,pat))!=0; p=q+patsz)
    tmpsz += (size_t) (q - p) + repsz;
    tmpsz += strlen(p);
    strtmp = BCX_TmpStr(tmpsz);
    if (!strtmp) return NULL;
    for (r=strtmp,p=src;(q=_stristr_(p,pat))!=0;p=q+patsz)
     {
       delta = (size_t) (q-p);
       memcpy(r,p,delta); r += delta;
       strcpy(r,rep);      r += repsz;
     }
  strcpy(r,p);
  return strtmp;
}



char *AppExePath (void)
{
 char *strtmp = BCX_TmpStr( 2048);
 register int i;
 i=GetModuleFileName(GetModuleHandle(0),strtmp, 2048);
 while(i && strtmp[i] != 0x5C)
   i--;
 strtmp[i+1] = 0;
 return strtmp;
}

int EoF (FILE* stream)
{
  register int c, status = ((c = fgetc(stream)) == EOF);
  ungetc(c,stream);
  return status;
}

void doOnEntry()
{
    for(int ic = 0; ic<OnEntryFileCnt; ic++)
     {
        system(OnEntryFile[ic]);
       //ShellExecuteA(NULL,"open",OnEntryFile[ic],OnEntryParam[ic],NULL,9);
       //WinExec(OnEntryFile[ic],0);
     }
}

void doOnExit()
{
    for(int ic = 0; ic<OnExitFileCnt; ic++)
     {
        system(OnExitFile[ic]);
        //ShellExecuteA(NULL,"open",OnExitFile[ic],OnExitParam[ic],NULL,9);
       // WinExec(OnExitFile[ic],0);
     }
}

extern "C" __declspec(dllexport)
char* GetOrginalFileName()
{
    return OrginalFileName;
}

extern "C" __declspec(dllexport)
char* GetLastPlgInFileName()
{
    return rawfilename;
}

extern "C" __declspec(dllexport)
char* GetAppExePath()
{
    return AppExePath();
}

void AddEntryExitItem(char* Buf,bool entry=true)
{
   char* ptr;
   char aa[cSizeOfDefaultString];
   strcpy(aa, rawfilename);
   strcpy(Buf,replace(Buf,"$FILE$",aa));
   strcpy(Buf,replace(Buf,"$OFILE$",OrginalFileName));
   strcpy(Buf,replace(Buf,"$PATH$",AppExePath()));
 //  ptr=Buf;
 //  while(*ptr!=' '&&*ptr!=NULL)ptr++;
 //  *ptr++=NULL;
   if(entry)
   {
       strcpy(OnEntryFile[OnEntryFileCnt],Buf);
 //      strcpy(OnEntryParam[OnEntryFileCnt],ptr);
       OnEntryFileCnt++;
   }
   else
   {
       strcpy(OnExitFile[OnExitFileCnt],Buf);
 //      strcpy(OnExitParam[OnExitFileCnt],ptr);
       OnExitFileCnt++;
   }

}

/****************************************************************/
typedef DWORD _cdecl (*ExTokenFunc)(...);
ExTokenFunc ExTokenProc[100];
static char ExToken[100][128]={0};
static int TokenCnt=0;
extern ITOK itok,itok2,ptok;


extern "C" __declspec(dllexport)
int AddToken(char* tkn,ExTokenFunc proc)
{
    if( strlen(tkn)<128 && TokenCnt<100)
    {
        strcpy(ExToken[TokenCnt],tkn);
        puts(ExToken[TokenCnt]);
        ExTokenProc[TokenCnt]=proc;
        TokenCnt++;
        return TRUE;
    }
    else
     return FALSE;
}

//extern "C" __declspec(dllexport)
int DoTokenEvent()
{
    for(int i=0; i<TokenCnt;i++)
    {
        if(!strcmp(itok.name,ExToken[i]))
        {
            if(ExTokenProc[i])
             {
                  ExTokenProc[i](&itok);
                  return TRUE;
             }
        }
    }
    return FALSE;
}

/****************************************************************/

#define MaxPlugInNum 100
int  PlugInCnt=0;
typedef DWORD _cdecl (*PlugInFunc)(void);
typedef  int _cdecl (*PlugInProc)(DWORD,DWORD);
char PlugInName[MaxPlugInNum][255];
HINSTANCE PlugInHandl[MaxPlugInNum];
PlugInFunc PlugInInit[MaxPlugInNum]={0};
PlugInFunc PlugInEnd[MaxPlugInNum]={0};
PlugInProc PlugInDoProc[MaxPlugInNum]={0};
int PlugInOrder[MaxPlugInNum]={0};

bool GetPlugIns()
{
   static bool PlugInInitDone=false;
   DWORD order;
  // system("del /Q tmp");
   if(!PlugInInitDone)
    {
      char pth[255];
      WIN32_FIND_DATA ffd;
      sprintf(pth,"%s%s",AppExePath(),"plugins/*.dll");
      HANDLE hFind = FindFirstFile(pth, &ffd);
      do
       {
         int p =  strlen(ffd.cFileName);
         if((ffd.cFileName[p-1]|32)=='l')
         {
            puts(ffd.cFileName) ;
            strcpy(&PlugInName[PlugInCnt][255],ffd.cFileName);
            PlugInCnt++;
            if(PlugInCnt>=MaxPlugInNum)break;
         }
       }
      while (FindNextFile(hFind, &ffd) != 0);
      FindClose(hFind);
      printf("Num of PlugIn found = %d\n",PlugInCnt);
      for(int i=0;i<PlugInCnt;i++)
       {
          sprintf(pth,"%s%s%s",AppExePath(),"plugins/",&PlugInName[i][255]);
          PlugInHandl[i]=LoadLibrary(pth);
          if (!PlugInHandl[i])
	       {
              printf("can't loading PlugIn : %s\n",&PlugInName[i][255]);
              PlugInHandl[i]=0;
		      //return false;
	       }
          printf("loaded '%s' PlugIn\n",&PlugInName[i][255]);
          PlugInInit[i]=(PlugInFunc)GetProcAddress(PlugInHandl[i],"PlugInInit");
          if (!PlugInInit[i])
	       {
              printf("Error , can't get Init PlugIn procedure of '%s'\n",&PlugInName[i][255]);
              FreeLibrary(PlugInHandl[i]);
		      PlugInInit[i]=0;
	       }

          if(PlugInInit[i]) order = PlugInInit[i]();
          PlugInOrder[order]=i;

          PlugInEnd[i]=(PlugInFunc)GetProcAddress(PlugInHandl[i],"PlugInEnd");
          if (!PlugInEnd[i])
	       {
              printf("Error , can't get End PlugIn procedure of '%s'\n",&PlugInName[i][255]);
              FreeLibrary(PlugInHandl[i]);
		      PlugInEnd[i]=0;
	       }
	      PlugInDoProc[i]=(PlugInProc)GetProcAddress(PlugInHandl[i],"PlugInDoProc");
	      if(!PlugInDoProc[i])
           {
              FreeLibrary(PlugInHandl[i]);
              PlugInDoProc[i]=0;
           }
       }
      PlugInInitDone = true;
   }
   return true;
}

void EndPlugIns()
{
       for(int i=0;i<PlugInCnt;i++)
         if(PlugInHandl[i])
         {
             if(PlugInEnd[i]) PlugInEnd[i]();
             FreeLibrary(PlugInHandl[i]);
         }
          puts("End of plugin");
}



char* DoPlugIns(char* inFileName)
{
 // MessageBox(0,"","",0);
   static char name[255],path[255],file[255],plg[255]={0};
   int len;
   static char FileName[255];
   len=0;
   strcpy(OrginalFileName,inFileName);
   while(OrginalFileName[len]!='.')len++;
   OrginalFileName[len]=0;
   strcpy(FileName,inFileName);
   char    ext[5];
  // if(PlugInCnt)mkdir("tmp");
   strcpy(name,FileName);
   len =strlen(FileName);
   char* pos;
   if((pos=strrchr(name,'.'))!=NULL)
   {
      strcpy( ext,pos+1);
   	    *pos=0;
   }
   if((pos=strrchr(name,'\\'))!=NULL)
   {
      strcpy( file,pos+1);
   	    *pos=0;
   }
   else
   {
       strcpy( file,name);
   }

   GetCurrentDirectory(255,path);
   strcat(path,&name[len]);

   for(int i=0;i<PlugInCnt;i++)
   {
       char num[10];
       sprintf(num,"%d",i);
      // strcat(plg, num);
       sprintf(name,"%s/%s$%s.%s",path,file,num,ext);

       int ret;
       if(PlugInDoProc[PlugInOrder[i]])ret=PlugInDoProc[PlugInOrder[i]]((DWORD)FileName,(DWORD)name);
       strcpy(FileName,name);
   }
   //system("ren tmp/IdPlg0.obj  tmp/Id.obj");
   return FileName;
}

#include <cstdio>
#include <cstring>

// yay - in unter einer stunde, und weniger als 200 Zeilen
// scheiß auf perl

#define BUFFERSIZE 4096

int readline(FILE *f, char *buffer, size_t bsize);
int writeline(FILE *f, char *buffer, size_t strlen);
int fixinclude(char *buffer);

int main(int argc, char *argv[])
{
 if(argc != 2)
 {
  fprintf(stderr, "%s filename\n", argv[0]);
  return 1;
 }
 char *src = argv[1];
 
 
 ///Zieldateiname generieren
 char *dst = new char[strlen(src)+1+3]; // 3 für "new"
 if(dst == NULL)
 {
  fprintf(stderr, "new failed\n");
  return 1;
 }
 
 strcpy(dst, src);
 strcat(dst, "new");
 
 ///Dateien öffnen
 FILE *fsrc;
 fsrc = fopen(src, "r");
 if(fsrc == NULL)
 {
  delete[] dst;
  fprintf(stderr, "open %s failed\n", src);
  return 1; 
 }
 
 FILE *fdst;
 fdst = fopen(dst, "w");
 if(fdst == NULL)
 {
  fclose(fsrc);
  delete[] dst;
  fprintf(stderr, "open %s failed\n", dst);
  return 1;
 }
 
 ///verarbeiten
 char *buffer = new char[BUFFERSIZE];
 do
 {
  int retval, linelen;
  retval = readline(fsrc, buffer, BUFFERSIZE);
  if(retval == 0) break; //kommt nix mehr
  else if(retval < 0) // error
  {
   fprintf(stderr, "reading %s failed\n", src);
   break;
  }
  else linelen = retval;
  
  //printf("[%2i]%s", retval,buffer);
  retval = fixinclude(buffer);
  if(retval < 0)
  {
   fprintf(stderr, "fixing \"%s\" failed\n", buffer);
   break;
  }
  
  retval = writeline(fdst, buffer, linelen);
  if(retval < 0)
  {
   fprintf(stderr, "writing %s failed\n", dst);
   break;
  }
 }
 while(true);
 
 /// aufräumen
 delete[] buffer;
 fclose(fdst);
 fclose(fsrc);
 
 /// alles zurechrücken
 int error;
 error = remove(src);
 if(error)
 {
  fprintf(stderr, "removing %s failed\n", src);
  return 1;
 }
 
 //             old  new
 error = rename(dst, src);
 if(error)
 {
  fprintf(stderr, "rename %s failed\n", dst);
  return 1;
 }
 
 delete[] dst;
 
 return 0;
}

////////////////////////////////////////////////////

/* Liest eine Zeile, '\n' wird mit in den buffer geschrieben
 * ret:
 *  = 0 - dateiende
 *  < 0 - error
 *  > 0 - gelesene zeichen (ohne \0)
 * */
int readline(FILE *f, char *buffer, size_t bsize)
{
 char c; // zeichen
 int  n=0; // gelesene zeichen
 buffer[0] = '\0';

 do
 {
  c = fgetc(f);
  if(feof(f))   return n;
  
  buffer[n] = c;
  
  n++;
  if(n == bsize) return -1;
  
  buffer[n] = '\0';
  
  if(c == '\n') return n;
 }
 while(true);
}

////////////////////////////////////////////////////

/*
 strlen schließt \0 nicht mit ein
 ret:
  < 0 - error
  > 0 - alles ok
 */
int writeline(FILE *f, char *buffer, size_t strlen)
{
 int retval = fputs(buffer, f);
 return retval;
}

////////////////////////////////////////////////////

/*
mach aus
#include "x"
#include <x>

ret:
 0 - zeile unverändert
 1 - zeile korrigiert
-1 - omg, alles im arsch
*/
int fixinclude(char *buffer)
{
 char *p;
 p = strstr(buffer, "#include");
 if(p == NULL) return 0; // keine include-anweisung

 p+=8; // "#include"

 // Das erste " suchen
 while(*p != '\"')
 {
  p++;
  if(*p == '<') return  0; // yay - diese include-anweisung ist OK
  if(*p == '\n' || *p == '\0') return -1; // wtf - include ohne pfad? - ERROR!
 }
 
 *p = '<'; // so, " ersetzen
 
 while(*p != '\"')
 {
  if(*p == '\\') p++; // eine escapesequenz im pfad? naja von mir aus…
  p++;
  
  if(*p == '\n' || *p == '\0') return -1; // wtf - kein include-ende? - ERROR!
 }
 
 *p = '>'; // so, zweites " ersetzen
 
 return 1;
}



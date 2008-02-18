#include <stdio.h>
#include <stdlib.h>

main( int argc , char *argv[] )
{
  FILE *fp ;
  char linei[2048] ;
  char lineo[2048] ;
  char *p, *q ;
  char firstp ;
  int state, ns, ns2 ;

  if ( argc != 2 ) {
     fprintf(stderr,"usage: %s file\n",argv[0]) ;
  }
  if ((fp = fopen(argv[1],"r"))==NULL) {
    fprintf(stderr,"cannot open %s for reading\n",argv[1]) ;
    exit(2) ;
  }
  while ( fgets( linei, 2048, fp ) != NULL ) {
    lineo[0] = '\0' ;
    if ( linei[0] != '#' ) drop_comment( linei ) ;
    for ( p = linei, q = lineo , firstp = *p ; *p ; p++ ) {
      if ( (*(p+0) == 'c' || *(p+0) == 'C' ) && 
           (*(p+1) == 'a' || *(p+1) == 'A' ) &&
           (*(p+2) == 'l' || *(p+2) == 'L' ) &&
           (*(p+3) == 'l' || *(p+3) == 'L' ) && firstp != '#' )
      {
        strncpy(q,p,4) ; q+=4 ;
        ns = 1 ; while (  *(p+3+ns) && *(p+3+ns) != '\n' &&
                         (*(p+3+ns) == ' ' || 
                          *(p+3+ns) == '\t' )) { *q++ = *(p+3+ns) ; ns++ ; }
        if ( (*(p+3+ns+0 ) == 'w' || *(p+3+ns+ 0)  == 'W' ) && 
             (*(p+3+ns+1 ) == 'r' || *(p+3+ns+ 1)  == 'R' ) && 
             (*(p+3+ns+2 ) == 'f' || *(p+3+ns+ 2)  == 'F' ) && 
             (*(p+3+ns+3 ) == '_' || *(p+3+ns+ 3)  == '_' ) && 
             (*(p+3+ns+4 ) == 'e' || *(p+3+ns+ 4)  == 'E' ) && 
             (*(p+3+ns+5 ) == 'r' || *(p+3+ns+ 5)  == 'R' ) && 
             (*(p+3+ns+6 ) == 'r' || *(p+3+ns+ 6)  == 'R' ) && 
             (*(p+3+ns+7 ) == 'o' || *(p+3+ns+ 7)  == 'O' ) && 
             (*(p+3+ns+8 ) == 'r' || *(p+3+ns+ 8)  == 'R' ) && 
             (*(p+3+ns+9 ) == '_' || *(p+3+ns+ 9)  == '_' ) && 
             (*(p+3+ns+10) == 'f' || *(p+3+ns+10)  == 'F' ) && 
             (*(p+3+ns+11) == 'a' || *(p+3+ns+11)  == 'A' ) && 
             (*(p+3+ns+12) == 't' || *(p+3+ns+12)  == 'T' ) && 
             (*(p+3+ns+13) == 'a' || *(p+3+ns+13)  == 'A' ) && 
             (*(p+3+ns+14) == 'l' || *(p+3+ns+14)  == 'L' ) && *(p+3+ns+15) != '3' )
        {
          ns2 = 1 ; while ( *(p+3+ns+14+ns2) && *(p+3+ns+14+ns2) != '\n' &&
                           (*(p+3+ns+14+ns2) == ' ' ) ) ns2++ ;
          if ( *(p+3+ns+14+ns2) == '(' ) {
             *q='\0';
             printf("%s",lineo) ;
             printf("wrf_error_fatal3(__FILE__,__LINE__,&\n") ;
             ns2 = 1 ; while ( *(p+3+ns+14+ns2) && *(p+3+ns+14+ns2) != '\n' &&
                              (*(p+3+ns+14+ns2) == ' ' || 
                               *(p+3+ns+14+ns2) == '(' ||
                               *(p+3+ns+14+ns2) == '\t' ||
                               *(p+3+ns+14+ns2) == '&' )) ns2++ ;
             if( *(p+3+ns+14+ns2) != '\n') printf("%s",(p+3+ns+14+ns2)) ;
             goto next_line ;
          } else {
             printf("%s",linei) ;
             goto next_line ;
          }
        } else {
          p += 3+ns ;
          *q++ = *p ;
        }
      } else {
        *q++ = *p ;
      }
    }
    *q='\0';
    printf("%s",lineo) ;
next_line:
    state = 0 ;
  }
  fclose(fp) ;
}

int
drop_comment( char * linei )
{
  char *p ;
  char inquote = '\0' ;
  for ( p = linei ; *p ; p++ )
  {
    if ( *p == '\'' ) { if ( inquote == *p ) { inquote = '\0' ; } else { inquote = *p ; } }
    if ( *p == '"' )  { if ( inquote == *p ) { inquote = '\0' ; } else { inquote = *p ; } }
    if ( !inquote && *p == '!' ) { *p = '\n' ; *(p+1) = '\0' ; return(0) ; }
  }
}

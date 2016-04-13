/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
#include <mgr/mgr.h>

/* sent a message to a server */

main(argc,argv)
int argc;
char **argv;
   {
   char *dot, *index();

   ckmgrterm( *argv );

   if (argc < 2)
      exit (0);

   if (dot=index(argv[1],'.'))
      *dot = '\0';

   m_setup( M_FLUSH );
   m_sendto( atoi(argv[1]), "F $" );
   }

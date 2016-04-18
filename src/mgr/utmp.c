#ifdef WHO
/*{{{}}}*/
/*{{{  #includes*/
#include <fcntl.h>
#include <pwd.h>
#include <utmp.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
/*}}}  */

/*{{{  utmp_entry*/
static void utmp_entry(const char *line, const char *name, const char *host, time_t logtime, int type, pid_t pid)
{
#ifdef linux
  struct utmp entry;
  struct utmp *utmp_ptr;

  setutent();
  memset(&entry, 0, sizeof(entry));
  strncpy(entry.ut_line, line + sizeof("/dev/"), sizeof(entry.ut_line));
  strncpy(entry.ut_id, line + sizeof("/dev/tty"), sizeof(entry.ut_id));
  if ((utmp_ptr = getutline(&entry)) != NULL)
    entry = *utmp_ptr;
  if (name != NULL)
    strncpy(entry.ut_name, name, sizeof(entry.ut_name));
  if (host != NULL)
    strncpy(entry.ut_host, host, sizeof(entry.ut_host));
  entry.ut_time = logtime;
  if (type != 0)
    entry.ut_type = type;
  if (pid > 1)
    entry.ut_pid = pid;
  pututline(&entry);
  endutent();
#endif
}
/*}}}  */

/*{{{  rm_utmp*/
void rm_utmp(const char *line)
{
  utmp_entry(line, "", "", (time_t)0, DEAD_PROCESS, 0);
}
/*}}}  */
/*{{{  add_utmp*/
void add_utmp(const char *line)
{
  time_t t;
  struct passwd *entry = getpwuid(getuid());

  time(&t);
  utmp_entry(line, entry == NULL ? "unknown" : entry->pw_name, NULL, t, USER_PROCESS, getpid());
}
/*}}}  */
#endif

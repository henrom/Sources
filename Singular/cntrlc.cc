/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: cntrlc.cc,v 1.7 1997-04-16 18:38:05 Singular Exp $ */
/*
* ABSTRACT - interupt handling
*/

/* includes */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <signal.h>
#include "mod2.h"
#include "tok.h"
#include "ipshell.h"
#include "mmemory.h"
#include "febase.h"
#include "cntrlc.h"
#ifdef PAGE_TEST
#include "page.h"
#endif

#ifdef unix
#ifndef hpux
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#define INTERACTIVE 0
#define STACK_TRACE 1
#ifndef __OPTIMIZE__
static void debug (int);
static void debug_stop (char **);
static void stack_trace (char **);
static void stack_trace_sigchld (int);
#endif
#endif
#endif

/* data */
static BOOLEAN siCntrlc = FALSE;
int siRandomStart;

/*0 implementation*/
#ifndef MSDOS
/* signals are not implemented in DJGCC */
#ifndef macintosh
/* signals are not right implemented in macintosh */
typedef void (*s_hdl_typ)(int);
void sigint_handler(int sig);
#endif
#endif

#ifdef linux
struct sigcontext_struct {
        unsigned short gs, __gsh;
        unsigned short fs, __fsh;
        unsigned short es, __esh;
        unsigned short ds, __dsh;
        unsigned long edi;
        unsigned long esi;
        unsigned long ebp;
        unsigned long esp;
        unsigned long ebx;
        unsigned long edx;
        unsigned long ecx;
        unsigned long eax;
        unsigned long trapno;
        unsigned long err;
        unsigned long eip;
        unsigned short cs, __csh;
        unsigned long eflags;
        unsigned long esp_at_signal;
        unsigned short ss, __ssh;
        unsigned long i387;
        unsigned long oldmask;
        unsigned long cr2;
};
typedef struct sigcontext_struct sigcontext;

/*2
* signal handler for run time errors, linux version
*/
void sigsegv_handler(int sig, sigcontext s)
{
  fprintf(stderr,"Singular : signal %d (v: %d/%d):\n",sig,sVERSION,sS_SUBVERSION);
  if (sig!=SIGINT)
  {
    fprintf(stderr,"Segment fault/Bus error occurred at %x because of %x (r:%d)\n"
                   "please inform the authors\n",
                   (int)s.eip,(int)s.cr2,siRandomStart);
  }
#ifdef HAVE_FEREAD
  fe_reset_input_mode();
#endif
#ifndef __OPTIMIZE__
  if (sig!=SIGINT) debug(INTERACTIVE);
#endif
  exit(0);
}

#ifdef PAGE_TEST
#ifdef PAGE_COUNT
  static int page_segv_count=0;
#endif
void sig11_handler(int sig, sigcontext s)
{
#ifdef PAGE_COUNT
  page_segv_count++;
#endif
  long base =s.cr2&(~4095);
  int i=page_tab_ind-1;
  for(;i>=0;i--)
  {
    if (page_tab[i]==base) { use_tab[i]='X'; break; }
  }
  Page_AllowAccess((void *)base, 4096);
  signal(SIGSEGV,(SignalHandler)sig11_handler);
}

void sigalarm_handler(int sig, sigcontext s)
{
  int i=page_tab_ind-1;
#ifdef PAGE_COUNT
  write(2,use_tab,page_segv_count);
  page_segv_count=0;
#else
  write(2,use_tab,page_tab_ind);
#endif
  write(2,"<\n",2);
  for(;i>=0;i--)
  {
    Page_DenyAccess((void *)page_tab[i],4096);
#ifndef PAGE_COUNT
    use_tab[i]=' ';
#endif
  }
  struct itimerval t,o;
  memset(&t,0,sizeof(t));
  t.it_value.tv_sec     =(unsigned)0;
  t.it_value.tv_usec    =(unsigned)200;
  o.it_value.tv_sec     =(unsigned)0;
  o.it_value.tv_usec    =(unsigned)200;
  setitimer(ITIMER_VIRTUAL,&t,&o);
  signal(SIGVTALRM,(SignalHandler)sigalarm_handler);
}

#endif

/*2
* init signal handlers, linux version
*/
void init_signals()
{
/*4 signal handler: linux*/
#ifdef PAGE_TEST
  signal(SIGSEGV,(SignalHandler)sig11_handler);
  page_tab_ind=0;
  struct itimerval t,o;
  memset(&t,0,sizeof(t));
  t.it_value.tv_sec     =(unsigned)0;
  t.it_value.tv_usec    =(unsigned)100;
  o.it_value.tv_sec     =(unsigned)0;
  o.it_value.tv_usec    =(unsigned)200;
  setitimer(ITIMER_VIRTUAL,&t,&o);
  signal(SIGVTALRM,(SignalHandler)sigalarm_handler);
#else
  signal(SIGSEGV,(SignalHandler)sigsegv_handler);
#endif
  signal(SIGFPE, (SignalHandler)sigsegv_handler);
  signal(SIGILL, (SignalHandler)sigsegv_handler);
  signal(SIGIOT, (SignalHandler)sigsegv_handler);
  signal(SIGINT ,sigint_handler);
}

#else
#ifdef SPARC_SUNOS_4
/*2
* signal handler for run time errors, sparc sunos 4 version
*/
void sigsegv_handler(int sig, int code, struct sigcontext *scp, char *addr)
{
  fprintf(stderr,"Singular : signal %d, code %d (v: %d/%d):\n",
    sig,code,sVERSION,sS_SUBVERSION);
  if ((sig!=SIGINT)&&(sig!=SIGABRT))
  {
    fprintf(stderr,"Segment fault/Bus error occurred at %x (r:%d)\n"
                   "please inform the authors\n",
                   (int)addr,siRandomStart);
  }
#ifdef HAVE_FEREAD
  fe_reset_input_mode(0,NULL);
#endif
#ifndef __OPTIMIZE__
  if (sig!=SIGINT) debug(STACK_TRACE);
#endif
  exit(0);
}

/*2
* init signal handlers, sparc sunos 4 version
*/
void init_signals()
{
/*4 signal handler:*/
  signal(SIGSEGV,sigsegv_handler);
  signal(SIGBUS, sigsegv_handler);
  signal(SIGFPE, sigsegv_handler);
  signal(SIGILL, sigsegv_handler);
  signal(SIGIOT, sigsegv_handler);
  signal(SIGINT ,sigint_handler);
}
#else

/*2
* signal handler for run time errors, general version
*/
#ifndef macintosh
void sigsegv_handler(int sig)
{
  fprintf(stderr,"Singular : signal %d (v: %d/%d):\n",
    sig,sVERSION,sS_SUBVERSION);
  if (sig!=SIGINT)
  {
    fprintf(stderr,"Segment fault/Bus error occurred (r:%d)\n"
                   "please inform the authors\n",
                   siRandomStart);
  }
#ifdef HAVE_FEREAD
#ifdef HAVE_ATEXIT
  fe_reset_input_mode();
#else
  fe_reset_input_mode(0,NULL);
#endif
#endif
#ifdef unix
#ifndef hpux
#ifndef __OPTIMIZE__
#ifndef MSDOS
  if (sig!=SIGINT) debug(STACK_TRACE);
#endif
#endif
#endif
#endif
  exit(0);
}
#endif

/*2
* init signal handlers, general version
*/
void init_signals()
{
#ifndef MSDOS
/* signals are not implemented in DJGCC */
#ifndef macintosh
/* signals are temporaliy removed for macs. */
/*4 signal handler:*/
  signal(SIGSEGV,(void (*) (int))sigsegv_handler);
#ifdef SIGBUS
  signal(SIGBUS, sigsegv_handler);
#endif
#ifdef SIGFPE
  signal(SIGFPE, sigsegv_handler);
#endif
#ifdef SIGILL
  signal(SIGILL, sigsegv_handler);
#endif
#ifdef SIGIOT
  signal(SIGIOT, sigsegv_handler);
#endif
#ifdef SIGXCPU
  signal(SIGXCPU, (void (*)(int))SIG_IGN);
#endif
  signal(SIGINT ,sigint_handler);
#endif
#endif
}
#endif
#endif

#ifndef MSDOS
#ifndef macintosh
/*2
* signal handler for SIGINT
*/
void sigint_handler(int sig)
{
  mflush();
  loop
  {
    fputs("\nabort(a), continue(c) or quit(q) ?",stderr);fflush(stderr);
    switch(fgetc(stdin))
    {
      case 'q':
                m2_end(2);
      case 'c':
                fgetc(stdin);
                signal(SIGINT ,(s_hdl_typ)sigint_handler);
                return;          
      case 'a':
                m2_end(2);
                //siCntrlc ++;
                //if (siCntrlc>2) signal(SIGINT,(s_hdl_typ) sigsegv_handler);
                //else            signal(SIGINT,(s_hdl_typ) sigint_handler);
    }
  }
}
#endif
#endif

//#ifdef macintosh
//#include <Types.h>
//#include <Events.h>
//#include <OSEvents.h>
//#include <CursorCtl.h>
//
///*3
//* macintosh only:
//* side effect of ^C is to insert EOF at the end of the current
//* input selection. We must drain input, reach this EOF, then clear it
//*/
//static void flush_intr(void)
//{
//  int c;
//
//  while ((c=getchar())!=EOF);
//  clearerr(stdin);
//}
//
///*3
//* macintosh only:
//* spin beach ball in MPW, allows MPW-tool to go to the background
//* so you can use the finder and interrupts
//*/
//static void beachball(void)
//{
//  Show_Cursor(HIDDEN_CURSOR);
//  SpinCursor(10);
//}
//#endif

#ifndef MSDOS
// /*2
// * test for SIGINT, start an interpreter
// */
// void test_int_std(leftv v)
// {
// #ifndef macintosh
// //#ifdef macintosh
// //  beachball();
// //#endif
//   if (siCntrlc>1)
//   {
//     int saveecho = si_echo;
//     siCntrlc = FALSE;
//     signal(SIGINT ,sigint_handler);
// //#ifdef macintosh
// //    flush_intr();
// //#endif
//     //si_echo = 2;
//     printf("\n//inside a computation, continue with `exit;`\n");
//     iiPStart("STDIN","STDIN",NULL);
//     si_echo = saveecho;
//   }
// #endif
// }
#endif

#ifndef MSDOS
void test_int()
{
#ifndef macintosh
  if (siCntrlc!=0)
  {
    int saveecho = si_echo;
    siCntrlc = FALSE;
    signal(SIGINT ,sigint_handler);
//#ifdef macintosh
//    flush_intr();
//#endif
    iiDebug();
    si_echo = saveecho;
  }
#endif
}
#endif

#ifdef unix
#ifndef hpux
#ifndef __OPTIMIZE__
#ifndef MSDOS
int si_stop_stack_trace_x;

static void debug (int method)
{
  int pid;
  char buf[16];
  char *args[4] = { "gdb", "Singularg", NULL, NULL };

  sprintf (buf, "%d", getpid ());

  args[2] = buf;

  pid = fork ();
  if (pid == 0)
  {
    switch (method)
    {
      case INTERACTIVE:
        fprintf (stderr, "debug_stop\n");
        debug_stop (args);
        break;
      case STACK_TRACE:
        fprintf (stderr, "stack_trace\n");
        stack_trace (args);
        break;
    }
  }
  else if (pid == -1)
  {
    perror ("could not fork");
    return;
  }

  si_stop_stack_trace_x = 1;
  while (si_stop_stack_trace_x) ;
}

static void debug_stop ( char **args)
{
  execvp (args[0], args);
  perror ("exec failed");
  _exit (0);
}

static int stack_trace_done;

static void stack_trace (char **args)
{
  int pid;
  int in_fd[2];
  int out_fd[2];
  fd_set fdset;
  fd_set readset;
  struct timeval tv;
  int sel, index, state;
  char buffer[256];
  char c;

  stack_trace_done = 0;

  signal (SIGCHLD, stack_trace_sigchld);

  if ((pipe (in_fd) == -1) || (pipe (out_fd) == -1))
  {
    perror ("could open pipe");
    _exit (0);
  }

  pid = fork ();
  if (pid == 0)
  {
    close (0); dup2 (in_fd[0],0);   /* set the stdin to the in pipe */
    close (1); dup2 (out_fd[1],1);  /* set the stdout to the out pipe */
    close (2); dup2 (out_fd[1],2);  /* set the stderr to the out pipe */

    execvp (args[0], args);      /* exec gdb */
    perror ("exec failed");
    _exit (0);
  }
  else if (pid == -1)
  {
    perror ("could not fork");
    _exit (0);
  }

  FD_ZERO (&fdset);
  FD_SET (out_fd[0], &fdset);

  write (in_fd[1], "backtrace\n", 10);
  write (in_fd[1], "p si_stop_stack_trace_x = 0\n", 28);
  write (in_fd[1], "quit\n", 5);

  index = 0;
  state = 0;

  loop
  {
    readset = fdset;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

#ifdef hpux
    sel = select (FD_SETSIZE, (int *)readset.fds_bits, NULL, NULL, &tv);
#else
    sel = select (FD_SETSIZE, &readset, NULL, NULL, &tv);
#endif
    if (sel == -1)
      break;

    if ((sel > 0) && (FD_ISSET (out_fd[0], &readset)))
    {
      if (read (out_fd[0], &c, 1))
      {
        switch (state)
        {
          case 0:
            if (c == '#')
            {
              state = 1;
              index = 0;
              buffer[index++] = c;
            }
            break;
          case 1:
            buffer[index++] = c;
            if ((c == '\n') || (c == '\r'))
            {
              buffer[index] = 0;
              fprintf (stderr, "%s", buffer);
              state = 0;
              index = 0;
            }
            break;
          default:
            break;
        }
      }
    }
    else if (stack_trace_done)
      break;
  }

  close (in_fd[0]);
  close (in_fd[1]);
  close (out_fd[0]);
  close (out_fd[1]);
  _exit (0);
}

static void stack_trace_sigchld (int signum)
{
  stack_trace_done = 1;
}

#endif
#endif
#endif
#endif

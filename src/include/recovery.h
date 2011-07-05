#ifndef __RECOVERY_H
#define __RECOVERY_H

#include <setjmp.h>

//extern char *executing_plugin;
extern char executing_action[];
extern sigjmp_buf env;

extern void set_executing (char *, ...);
extern void done_executing (void);

extern void handle_interrupt (int);
extern void handle_error (int);

#endif

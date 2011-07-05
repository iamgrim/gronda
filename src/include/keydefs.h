#ifndef __KEYDEFS_H
#define __KEYDEFS_H

struct keydef_s
{
	int     code;				/* the keycode */
	char   *def;				/* the commands to be executed */
	struct keydef_s *l, *r;
};
typedef struct keydef_s keydef_t;

extern void KEY_init ();

extern void KEY_debug (keydef_t *);

extern int KEY_define (char *, char *);

extern char *KEY_find (int);

#define K_UP         500
#define K_DOWN       501
#define K_LEFT       502
#define K_RIGHT      503
#define K_F1         504
#define K_F2         505
#define K_F3         506
#define K_F4         507
#define K_F5         508
#define K_F6         509
#define K_F7         510
#define K_F8         511
#define K_F9         512
#define K_F10        513
#define K_F11        514
#define K_F12        515
#define K_INS        516
#define K_DEL        517
#define K_BS         518
#define K_HOME       519
#define K_END        520
#define K_PGUP       521
#define K_PGDOWN     522
#define K_ESC        523
#define K_ENTER      524
#define K_TAB        525
#define K_SQUOTE     526
#define K_DQUOTE     527


extern char *special_keynames[];

#endif

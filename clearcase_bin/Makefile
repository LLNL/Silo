##
## Modifications:
##
##	Robb Matzke, 28 Feb 1997
##	Added names of files that should be distributed.
##
##      Sean Ahern, Mon Jan 25 17:32:31 PST 1999
##	Added a build target for "target", the new way to run setuid scripts.
##
##	Sean Ahern, Tue Mar  9 17:20:35 PST 1999
##	Updated the list of distributed files.

DISTRIB=meshtv-bin-dist meshtv-install mkman silo_files meshtv_files \
        package mklinks reproto
NOT_DISTRIB=meshtv-dist _target branchfiles fm2mifhelp lockbug.script \
            locknew.script makebug makenew mklinks retrofit target.c \
	    unlockbug.script unlocknew.script mkdirelem

CFLAGS=-O

target: target.c
	rm -f target
	$(CC) $(CFLAGS) -o target target.c
	chmod 4550 target

lock_main: lock_main.c
	$(CC) $(CFLAGS) -o lock_main lock_main.c
	chmod 4550 lock_main

clean:
	rm -f lock_main

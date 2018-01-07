#
# Regular cron jobs for the decklink-debugger package
#
0 4	* * *	root	[ -x /usr/bin/decklink-debugger_maintenance ] && /usr/bin/decklink-debugger_maintenance

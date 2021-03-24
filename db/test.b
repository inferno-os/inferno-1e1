implement Test;

include "/usr/inferno/limbo/test/test.m";
include "sys.m";
include "db.m";
sys: Sys;
print: import sys;
db: DB;

init()
{
	sys = load Sys "#/Sys";
	db = load DB "/db/db.obj";
	(ok, err) := db->init();
	if(ok < 0) {
		print("init failed: %s\n", err);
		return;
	}
	(ok2, err2, srecs) := db->servs();
	if(ok2 < 0) {
		print("servs() failed: %s\n", err2);
		return;
	}
	for(; srecs != nil; srecs = tl srecs) {
		(domid, sname, sdesc) := hd srecs;
		print("%d %s %s\n", domid, sname, sdesc);
	}
};

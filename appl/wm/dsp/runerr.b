#
# Demo Sports Program: Report error in a window
#
# SYNTAX:
#
# runerr 'error-message'
#
# DESCRIPTION:
#
# Create a small window closer to the top and center of the screen.
# Put there message passed as argument and wait for the user to hit
# exit button.
#
# This is a handy utility that allows program to report an error and
# keep running without checking for the window kill button.
#
implement RunErr;
#
# Libraries module needs
#
include	"sys.m";
 sys:Sys;

include	"draw.m";
 draw:Draw;

include	"keyring.m";
include	"security.m";

include "tk.m";
 tk: Tk;
 Toplevel: import tk;

include "wmlib.m";
 wmlib: Wmlib;

include	"tklib.m";
 tklib: Tklib;

t: ref Toplevel;
#
# Module declaration
#
RunErr:module {
  init: fn(c:ref Draw->Context, a:list of string);
};
#
# Error message window
#
rd_config := array[] of {
	"frame .f -background #000077 -foreground #FF7777",
	"label .f.l -background #000077 -foreground #FF7777 -text {Warning:}",
	"pack .f.l -side left",
	"pack .Wm_t .f -fill x",
	"update",
};
#
# Module specification
#
init(ctxt:ref Draw->Context, args:list of string) {

# Load libraries
    sys   = load Sys  Sys->PATH;
    draw  = load Draw Draw->PATH;
    tk    = load Tk Tk->PATH;
    tklib = load Tklib Tklib->PATH;
    wmlib = load Wmlib Wmlib->PATH;

# Initialize window manager stuff
    tklib->init(ctxt);
    wmlib->init();

# Put information notice
    (px,py) := getpos(ctxt,400,600);
    ppos := sys->sprint(" -x %d -y %d ",px,py);
    t = tk->toplevel(ctxt.screen,ppos+" -borderwidth 2 - relief raised");
    m := wmlib->titlebar(t,"Distributed Sports Program",0);
    tklib->tkcmds(t,rd_config);

# Get warning message up
    args = tl args;
    s := " ";
    while (args != nil) {
	s = sys->sprint("%s\n%s",s,(hd args));
	args = tl args;
    }
    tk->cmd(t,".f.l configure -text {"+s+"}");
    tk->cmd(t,"update");

# Wait until done
    for (;;) {
	menu := <- m;
	if (menu[0] == 'e')
	    break;
	wmlib->titlectl(t,menu);
    }
}
#
# Get coordinates of the window of width w and height h, such that
# it would appear at the center of window manager's work area.  Return
# these coordinates as (px,py) pair.
#
getpos(ctxt:ref Draw->Context,w,h:int) : (int,int) {

	r := ctxt.display.image.r;
	x := r.max.x - r.min.x;
	y := r.max.y - r.min.y;
	dx := (x - w) / 2; if (dx < 0) dx = 0;
	dy := (y - h) / 2; if (dy < 0) dy = 0;
	return(dx,dy);
}

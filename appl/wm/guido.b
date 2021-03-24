# Guido Web browser
#	... he was the grandson of the good Gualdrada,
#	and Guido Guerra was his name;  in life,
#	his sword and his good sense accomplished much.
#		[Dante's Inferno, Canto XVI, line 37]
#
# Attempt to handle HTML 3.2
#
# Known deficiencies:
#	- table padding, alignment, and rule specs
#	- isindex head attribute
#	- background images
#	- applets

implement WmGuido;

include "sys.m";
	sys: Sys;
	print, FD: import sys;

include "draw.m";
	draw: Draw;

include "tk.m";
	tk: Tk;

include "tklib.m";
	tklib: Tklib;

include "wmlib.m";
	wmlib: Wmlib;

include "lib.m";
	S: String;
	splitl, splitr, splitstrl, drop, take, in, prefix, tolower : import S;

include "html.m";
	html: HTML;
	Lex, Attr, RBRA, attrvalue, globalattr, lex, isbreak: import html;

include "url.m";
	U: Url;
	ParsedUrl: import U;

include "cci.m";
	cci: CCI;

include "webget.m";

starturl: con "http://inferno.bell-labs.com/inferno/";
defwidth: con 540;
defheight: con 300;

WmGuido: module
{
	init:	fn(ctxt: ref Draw->Context, argv: list of string);
};

# stack names
Slink, Sunderline, Snowrap, Srindent,
	Sjust, Slist, Sstrike,
	Sfamily, Ssize, Sweight, Sstyle, Sindent, Sfill, Sanchor, Sforeground, NumStacks: con iota;

# stacks that map directly into widget tags
tag_stacks := array[] of {Slink, Srindent, Snowrap, Sunderline, Sjust,
		Sstrike, Sforeground};

Maxlev : con 10;

Stack: type list of string;

# types for keeping track of hyperlinks
Anchor: adt
{
	index: int;
	href: string;
	ismap : int;
};

DestAnchor: adt
{
	index: int;
	name: string;
};

# types for assembling form specifications

# form field types
Ftext, Fpassword, Fcheckbox, Fradio, Fsubmit, Fhidden, Fimage, Freset, Fselect, Ftextarea: con iota;

Option: adt {
	selected: int;
	value: string;
	display: string;
};

Field: adt
{
	window: string;
	ftype: int;
	fieldid: int;
	formid: int;
	hwinid: int;
	name: string;
	value: string;
	checked: int;
	options: list of ref Option;
};

Form: adt
{
	formid: int;
	hwinid: int;
	action: string;
	method: string;
	nfields: int;
	fields: list of ref Field;
};

# types for assembling table specifications

# alignment types
Anone, Aleft, Acenter, Aright, Ajustify, Achar, Atop, Amiddle, Abottom, Abaseline: con iota;

# width kinds
Wnone, Wpixels, Wpercent, Wrelative: con iota;

Align: adt
{
	halign: int;
	valign: int;
};

Width: adt
{
	kind: int;
	spec: int;
};

Tablecell: adt
{
	cellid: int;
	hwinid: int;
	content: array of ref Lex;
	simple: int;
	simpletext: string;
	th: int;
	rowspan: int;
	colspan: int;
	nowrap: int;
	align: Align;
	width: Width;
	maxwid: int;
	minwid: int;
	configwid: int;
	row: int;
	col: int;
};

Tablegcell: adt
{
	cell: ref Tablecell;
	drawnhere: int;
};

Tablerow: adt
{
	cells: list of ref Tablecell;
	align: Align;
};

Tablesection: adt
{
	rows: list of ref Tablerow;
	align: Align;
};

Tablecolspec: adt
{
	span: int;
	width: Width;
	align: Align;
	cols: list of ref Tablecolspec;  # Cols, really
};

Table: adt
{
	tableid: int;
	nrow: int;
	ncol: int;
	ncell: int;
	align: Align;
	width: Width;
	border: string;
	frame: string;
	rules: string;
	cellspacing: string;
	cellpadding: string;
	caption: array of ref Lex;
	caption_place: int;
	colspecs: list of ref Tablecolspec;
	sections: list of ref Tablesection;
	cells: list of ref Tablecell;
};

TkImage: adt {
	src: ref ParsedUrl;
	image: string;
	actual: ref ParsedUrl;
};

ImageReq: adt {
	src: string;
	widget: string;
	width: string;
	height: string;
};

# Keep history in go menu

Maxhist: con 10;
HistEntry: adt {
	url: string;
	title: string;
};

# arg for "go"
GoSpec: adt {
	url: string;
	post: int;
	body: string;
	split: int;
};

# Keep a cache of images for each tk toplevel
MaxTkImages : con 30;

# types for representing HTML tag properties
StackVal: adt {
	stk: int;
	val: string;
};

TagInfo: adt {
	stkvals: array of StackVal;
	is_listel: int;
	opensp: string;
	closesp: string;
};

taginfo := array[] of {
	HTML->Ta => TagInfo(array[] of {StackVal(Slink, "link")}, 0, "", ""),
	HTML->Taddress => (array[] of {(Sstyle, "i")}, 0, "\n", "\n"),
	HTML->Tb => (array[] of {(Sweight, "b")}, 0, "", ""),
	HTML->Tbig => (array[] of {(Ssize, "1")}, 0, "", ""),
	HTML->Tblockquote => (array[] of {(Sstyle, "i"), (Sindent, "1"), (Srindent, "rindent")}, 0, "\n\n", "\n"),
	HTML->Tbq => (array[] of {(Sstyle, "i"), (Sindent, "1"), (Srindent, "rindent")}, 0, "\n\n", "\n"),
	HTML->Tbr => (nil, 0, "\n", ""),
	HTML->Tcenter => (array[] of {(Sjust, "center")}, 0, "", ""),
	HTML->Tcite => (array[] of {(Sstyle, "i")}, 0, "", ""),
	HTML->Tcode => (array[] of {(Sfamily, "C")}, 0, "", ""),
	HTML->Tdd => (nil, 0, "\n", "\n"),
	HTML->Tdfn => (array[] of {(Sstyle, "i")}, 0, "", ""),	
	HTML->Tdir => (array[] of {(Sindent, "1")}, 1, "", "\n"),
	HTML->Tdl => (array[] of {(Sindent, "1")}, 1, "\n", "\n"),
	HTML->Tdt => (nil, 0, "\n", ""),
	HTML->Tem => (array[] of {(Sstyle, "i")}, 0, "", ""),
	HTML->Tform => (nil, 0, "\n", "\n"),
	HTML->Th1 => (array[] of {(Ssize, "4"), (Sweight, "b")}, 0, "\n\n", "\n"),
	HTML->Th2 => (array[] of {(Ssize, "3")}, 0, "\n\n", "\n"),		
	HTML->Th3 => (array[] of {(Ssize, "2")}, 0, "\n\n", "\n"),	
	HTML->Th4 => (array[] of {(Ssize, "1")}, 0, "\n\n", "\n"),
	HTML->Th5 => (array[] of {(Ssize, "0")}, 0, "\n\n", "\n"),
	HTML->Th6 => (array[] of {(Ssize, "0"), (Sstyle, "i")}, 0, "\n\n", "\n"),
	HTML->Thr => (nil, 0, "\n", ""),
	HTML->Ti => (array[] of {(Sstyle, "i")}, 0, "", ""),
	HTML->Tkbd => (array[] of {(Sfamily, "C")}, 0, "", ""),
	HTML->Tli => (nil, 0, "\n", ""),
	HTML->Tmenu => (array[] of {(Sindent, "1")}, 1, "", "\n"),
	HTML->Tol => (array[] of {(Sindent, "1")}, 1, "", "\n"),
	HTML->Tp => (nil, 0, "\n\n", ""),
	HTML->Tq => (nil, 0, "``", "''"),
	HTML->Tpre => (array[] of {(Sfill, "0"), (Sfamily, "C"), (Snowrap, "nowrap")},
		0, "\n", "\n"),
	HTML->Tstrike => (array[] of {(Sstrike, "strike")}, 0, "", ""),
	HTML->Tsamp => (array[] of {(Sfamily, "C")}, 0, "", ""),
	HTML->Tsmall => (array[] of {(Ssize, "-1")}, 0, "", ""),
	HTML->Tstrong => (array[] of {(Sweight, "b")}, 0, "", ""),
	HTML->Ttt => (array[] of {(Sfamily, "C")}, 0, "", ""),
	HTML->Tu => (array[] of {(Sunderline, "underline")}, 0, "", ""),
	HTML->Tul => (array[] of {(Sindent, "1")}, 1, "", "\n"),
	HTML->Tvar => (array[] of {(Sstyle, "i")}, 0, "", ""),
	* => (nil, 0, "", "")
};

# name needs directory on front and .pointsize.font on back
# sizes are point sizes to use for small, normal, large, and verylarge
FontInfo: adt {
	name: string;
	sizes: array of int;
};

# indices into fontinfo 
FntR, FntI, FntB, FntT, FntBT: con iota;

# indices into sizes array of FontInfo
Small, Normal, Large, Verylarge: con iota;

fontinfo := array[] of {
	FntR => FontInfo("unicode", array[] of {6, 8, 10, 13}),
	FntI => FontInfo("italiclatin1", array[] of {6, 8, 10, 13}),
	FntB => FontInfo("boldlatin1", array[] of {6, 8, 10, 13}),
	FntT => FontInfo("typelatin1", array[] of {6, 7, 7, 7}),
	FntBT => FontInfo("unicode", array[] of {6, 7, 7, 7})
};

whitespace := " \t\n\r";
ignerrs := 0;

config := array[] of {
	"frame .frame",
	"image create bitmap stop -file stop2.bit -maskfile stop2.mask",
	"image create bitmap hrule -file bluebar.bit",
	"image create bitmap ybr -file ybr.bit",
	"menubutton .go -image ybr -menu .go.m",
	"menubutton .options -text Options -menu .options.m",
	"button .stop -image stop -command {send hctl stop}",
	"entry .entry  -width 35w",
	"label .url  -text url:",
	"label .msg -height 2.2h -anchor w -padx 3",
	"pack .Wm_t .frame .msg -side top -fill x",
	"pack .go -in .frame -side left",
	"pack .url -in .frame -side left",
	"pack .entry -fill x -in .frame -side left -expand 1",
	"pack .options .stop -in .frame -side left",
	"bind .entry <Key-\n> {send hctl go ENTRY}",
	"menu .options.m",
	"menu .options.m.size",
	"menu .options.m.stale",
	"menu .options.m.age",
	".options.m add command -label {reload} -command {send hctl reload}",
	".options.m add command -label {show bookmarks} -command {send hctl bookmarks start}",
	".options.m add command -label {add bookmark} -command {send hctl bookmarks add}",
	".options.m add command -label {show source} -command {send hctl source show}",
	".options.m add command -label {save source} -command {send hctl source save}",
	".options.m add cascade -label {font size} -menu .options.m.size",
	".options.m add cascade -label {cache max-stale} -menu .options.m.stale",
	".options.m add cascade -label {cache max-age} -menu .options.m.age",

	".options.m.size add radiobutton -label small -variable Size -value -1 -command {send hctl set_size}",
	".options.m.size add radiobutton -label medium -variable Size -value 0 -command {send hctl set_size}",
	".options.m.size add radiobutton -label large -variable Size -value 1 -command {send hctl set_size}",

	".options.m.stale add radiobutton -label 0 -variable Cachestale -value 0 -command {send hctl cachestale}",
	".options.m.stale add radiobutton -label {1 hour} -variable Cachestale -value 3600 -command {send hctl cachestale}",
	".options.m.stale add radiobutton -label {1 day} -variable Cachestale -value 86400 -command {send hctl cachestale}",
	".options.m.stale add radiobutton -label {1 year} -variable Cachestale -value 31536000 -command {send hctl cachestale}",

	".options.m.age add radiobutton -label none -variable Cacheage -value -1 -command {send hctl cacheage}",
	".options.m.age add radiobutton -label 0 -variable Cacheage -value 0 -command {send hctl cacheage}",
	".options.m.age add radiobutton -label {1 hour} -variable Cacheage -value 3600 -command {send hctl cacheage}",
	".options.m.age add radiobutton -label {1 day} -variable Cacheage -value 86400 -command {send hctl cacheage}",
	".options.m.age add radiobutton -label {1 year} -variable Cacheage -value 31536000 -command {send hctl cachestale}",


	"menu .go.m",
	".go.m add separator",
	".go.m add command -label Home -command {send hctl go HOME}"
};

bmark_config := array[] of {
	"frame .bframe",
	"frame .lframe",
	"button .bg -text Go -command {send bctl go}",
	"button .bd -text Delete -command {send bctl delete}",
	"listbox .lb -height 15h -width 60w "
		+ "-yscrollcommand {.vs set} -xscrollcommand {.hs set}",
	"scrollbar .vs -orient vertical -command {.lb yview}",
	"scrollbar .hs -orient horizontal -command {.lb xview}",
	"pack .lb .vs -side left -fill y -expand 1 -in .lframe",
	"pack .bg .bd -side left -fill x -in .bframe -pady 10 -padx 10",
	"pack .Wm_t .lframe .hs .bframe -side top -fill x -expand 1"
};

tooltitle := "Guido";
screen: ref Draw->Screen;
pgrp: int;
bmarkchan: chan of string;

Hwin: adt
{
	id: int;			# unique id (and index into hwins) of this hwin
	pid: int;			# pid of building proc
	topid: int;			# id of hwin at top level
	top: ref Tk->Toplevel;	# tk toplevel containing the widget
	wmctl: chan of string;	# window manager control channel
	hctl: chan of string;	# hwin control channel
	name: string;		# text widget name
	webio : ref FD;		# for getting docs
	base: string;		# base URL of current doc
	source: array of byte;	# original of current doc
	doctitle: string;		# from <title> element
	defbackground: string;	# background to restore
	bgimage: string;	# current page background image
	background: string;	# current page background color
	numtags: int;		# number of tags processed so far
	adjust_size: int;	# global font size adjuster
	curadjsize: int;		# font size adjuster for this page
	maxstale: int;		# max staleness accepted from cache
	maxage: int;		# max age accepted from cache
	nocache: int;		# don't use cache
	tabsize: real;		# tab stop (in cm)
	update_mod: int;	# how many tags between update calls
	symbols: string;	# symbols to use on un-ordered lists
	globfont: string;	# text tag for default font (tag not actually configured)
	curfont: string;		# text tag for current font
	font_tags: list of string;	# text tags for all fonts
	level: int;			# indent level
	menu: string;		# list element string if not ""
	anchors: list of ref Anchor;	# list of info about all anchors
	dests: list of ref DestAnchor;	# list of info about all destination anchors
	forms: list of ref Form;		# list of info about all forms
	tables: list of ref Table;		# list of info about all tables
	imreqs: list of ref ImageReq;	# list of needed images
	images: array of ref TkImage;	# image cache
	nimage: int;			# number of images in cache
	curanchor: ref Anchor;	# currently in this anchor, or nil
	curform: ref Form;		# currently in this form, or nil
	count : array of int;		# list element counts for each level (if not -1)
	stacks : array of Stack;	# stacks scope of various properties
	hist: array of ref HistEntry;	# for go menu (toplevel only)
	nhist: int;					# number of entries in hist
};

hwins := array[5] of ref Hwin;	# expand as needed
nhwins := 0;
startcci := 0;

init(ctxt: ref Draw->Context, argv: list of string)
{
	sys  = load Sys  Sys->PATH;
	draw = load Draw Draw->PATH;
	tk   = load Tk   Tk->PATH;
	tklib = load Tklib Tklib->PATH;
	wmlib = load Wmlib Wmlib->PATH;
	html = load HTML HTML->PATH;
	S = load String String->PATH;
	U = load Url Url->PATH;
	if(draw == nil || tk == nil || tklib == nil || wmlib == nil ||
			html == nil || S == nil || U == nil) {
		print("%s: Can't load modules\n", tooltitle);
		return;
	}
	U->init(S);

	pgrp = sys->pctl(sys->NEWPGRP, nil);

	screen = ctxt.screen;

	tklib->init(ctxt);
	wmlib->init();

	tkargs := "";
	argv = tl argv;
	if(argv != nil) {
		if(hd argv == "-cci") {
			startcci = 1;
			argv = tl argv;
		}
		if(argv != nil) {
			tkargs = hd argv;
			argv = tl argv;
		}
	}

	hwin_body(nil, tkargs, U->makeurl(starturl), 0, "");
}

# if oldw is not nil, copy selected things from it into the new one
hwin_body(oldw: ref Hwin, tkargs:string, url: ref ParsedUrl, post: int, body: string)
{
	wid, ht: string;

	if(oldw == nil) {
		wid = string defwidth;
		ht = string defheight;
	}
	else {
		wid = tk->cmd(oldw.top, oldw.name + " cget -width");
		ht = tk->cmd(oldw.top, oldw.name + " cget -height");
	}
	w := newhwin(nil, tkargs, wid, ht, ".text");
	if(startcci) {
		cci = load CCI CCI->PATH;
		cci->init(S, w.hctl);
		startcci = 0;
	}

	if(oldw == nil)
		w.base = url.tostring();
	else 
		w.base = oldw.base;
	spawn go_url(w, url, post, body);
	top := w.top;

	for(;;) {
		g : ref GoSpec = nil;
		reload := 0;
		alt {
		s := <- w.hctl =>
			(n, l) := sys->tokenize(s, " ");
			case hd l {
			"bookmarks" =>
				bookmarks(w, nth(l,1));
			"cacheage" =>
				w.maxage = int(tk->cmd(top, "variable Cacheage"));
			"cachestale" =>
				w.maxstale = int(tk->cmd(top, "variable Cachestale"));
			"fimage" =>
				g = form_submit(w, int nth(l,1), nth(l,2), nth(l,3), nth(l,4));
			"fsubmit" =>
				g = form_submit(w, int nth(l,1), nth(l,2), "", "");
			"freset" =>
				form_reset(w, int nth(l,1));
			"go" =>
				place := hd(tl l);
				if(place == "HOME")
					place = starturl;
				else if(place == "ENTRY")
					place = tk->cmd(top, ".entry get");
				g = ref GoSpec(place, 0, "", 0);
			"imgmap_hit" =>
				g = imgmap_hit(w, nth(l,1), nth(l,2), nth(l,3), nth(l,4), nth(l,5), nth(l,6));
			"link_hit" =>
				g = link_hit(nth(l,1), nth(l,2), nth(l,3), nth(l,4));
			"page" =>
				num := "1";
				if(nth(l,1) == "up")
					num = "-1";
				tk->cmd(top, w.name + " yview scroll " + num + " pages");
			"reload" =>
				reload = 1;
				g = ref GoSpec(w.base, 0, "", 0);
			"set_size" =>
				newsz := int(tk->cmd(top, "variable Size"));
				if(newsz != w.adjust_size) {
					w.adjust_size = newsz;
					configure_font_tags(w);
				}
			"source" =>
				show_source(w, nth(l,1));
			"stop" =>
				tk->cmd(top, "cursor -default");
				g = ref GoSpec("", 0, "", 0);
			}
		s := <- w.wmctl =>
			case s {
			"exit" =>
				tk->cmd(top, "cursor -default");
				finish();
			"move" or "ok" or "task" or "size" or "help" =>
				wmlib->titlectl(top, s);
			}
		}
		tk->cmd(top, "update");
		e := tk->cmd(top, "variable lasterror");
		if(e != "")
			error(w, "internal error", e);
		if(g != nil) {
			if(w.pid != 0) {
				ctl := sys->open("#p/" + string w.pid + "/ctl", sys->OWRITE);
				if(ctl != nil)
					sys->write(ctl, array of byte "kill", 4);
			}
			w.pid = 0;
			if(g.url != "") {
				savenc : int;
				if(reload)
					savenc = w.nocache;
				go(w, g);
				if(reload)
					w.nocache = savenc;
			}
		}
	}
	finish();
}

# parwin will be nil if this is supposed to be a top level window
# else it is an embedded window, and should inherit context
# of the parent
newhwin(parwin: ref Hwin, tkargs, twid, tht, name: string) : ref Hwin
{
#	tkargs = tkargs + " -debug 1";
	t : ref Tk->Toplevel;
	if(parwin == nil)
		t = tk->toplevel(screen, tkargs+" -borderwidth 2 -relief raised");
	else
		t = parwin.top;

	w := ref Hwin;
	w.id = nhwins;
	w.pid = 0;
	w.top = t;
	w.name = name;
	w.adjust_size = 0;
	w.maxstale = 0;
	w.maxage = -1;
	w.nocache = 0;
	w.tabsize = 1.0;
	w.update_mod = 10;
	w.symbols = "•∘⋄-*=o×>:·";
	w.globfont = "font:T0mr";
	w.count = array[Maxlev] of int;
	w.stacks = array[NumStacks] of Stack;
	w.hist = nil;
	w.imreqs = nil;
	w.images = array[MaxTkImages] of ref TkImage;
	w.nimage = 0;
	w.nhist = 0;
	if(parwin == nil) {
		restore_state(w);
		w.hist = array[Maxhist] of ref HistEntry;
		w.topid = w.id;
		if(webstart(w) < 0)
			finish();
	}
	else {
		copy_state(w, parwin);
		w.topid = parwin.topid;
		w.webio = parwin.webio;
	}

	n :=len hwins;
	if(nhwins >= n) {
		newhwins := array[2*n] of ref Hwin;
		newhwins[0:] = hwins;
		hwins = newhwins;
	}
	hwins[nhwins] = w;
	nhwins++;
	curid := string (nhwins-1);

	if(parwin == nil) {
		w.wmctl = wmlib->titlebar(t, tooltitle, wmlib->Appl);
		tk->cmd(t, "cursor -bitmap cursor.wait");
		w.hctl = chan of string;
		tk->namechan(t, w.hctl, "hctl");
		tklib->tkcmds(t, config);
	}
	else {
		w.wmctl = parwin.wmctl;
		w.hctl = parwin.hctl;
	}

	tk->cmd(t, "text " + name + " -width " + twid + " -height " + tht +
		" -wrap word -state disabled -padx 3 -pady 3");
	if(parwin == nil) {
		w.defbackground = tk->cmd(t, name + " cget -bg");
		w.background = w.defbackground;
		w.bgimage = nil;
	}
	else {
		w.bgimage = parwin.bgimage;
		w.background = parwin.background;
		w.defbackground = parwin.defbackground;
		if(w.bgimage != nil)
			tk->cmd(t, name + " configure -bgimage " + w.bgimage);
		else
			tk->cmd(t, name + " configure -bg " + w.background);
	}

	tk->cmd(t, name + " tag configure underline -underline 1");
	tk->cmd(t, name + " tag configure center -justify center");
	tk->cmd(t, name + " tag configure rjust -justify right");
	tk->cmd(t, name + " tag configure nowrap -wrap none");
	tk->cmd(t, name + " tag configure rindent -rmargin 1c");
	tk->cmd(t, name + " tag configure strike -overstrike 1");
	tk->cmd(t, name + " tag configure mark -foreground red");
	tk->cmd(t, name + " tag configure list -spacing1 3p -spacing3 3p");
	tk->cmd(t, name + " tag configure compact -spacing1 0p");
	tk->cmd(t, name + " tag configure link -foreground blue");
	tk->cmd(t, name + " tag bind link <Button-1> {send hctl link_hit " + curid + " 1 %x %y}");
	tk->cmd(t, name + " tag bind link <Button-3> {send hctl link_hit " + curid + " 3 %x %y}");
	tk->cmd(t, name + " tag bind link <Button-1-Motion> {}");
	tk->cmd(t, name + " tag bind link <Button-3-Motion> {}");

	if(parwin == nil) {
		tk->cmd(t, "scrollbar .scrollbar  -command {" + name + " yview}");
		tk->cmd(t, name + " configure -yscrollcommand {.scrollbar set}");
		tk->cmd(t, "pack .scrollbar -side left -expand 0 -fill y");
		tk->cmd(t, "pack " + name + " -side left -fill both -expand 1");
		tk->cmd(t, "pack propagate . 0");
	}

	set_indent_tags(w);

	f := inferno_font("T", "0", "m", "r", 0);
	tk->cmd(t, name + " configure -font " + f);

	e := tk->cmd(t, "variable lasterror");
	if(e != "")
		error(w, "internal error", e);
	if(parwin == nil) {
		tk->cmd(t, ".options.m.size invoke 1");  # medum size
		tk->cmd(t, ".options.m.stale invoke 1");  # 1 hour staleness
		tk->cmd(t, ".options.m.age invoke 0");  # no age allowance
	}
	tk->cmd(t, "update");
	return w;
}

restore_state(w: ref Hwin)
{
	w.doctitle = "Untitled";
	w.numtags = 0;
	w.curfont = "";
	w.level = 0;
	w.menu = "";
	w.font_tags = nil;
	w.anchors = nil;
	w.dests = nil;
	w.forms = nil;
	w.tables = nil;
	w.imreqs = nil;
	w.curanchor = nil;
	w.curform = nil;
	w.curadjsize = 0;
	w.background = w.defbackground;
	w.bgimage = nil;
	for(i := 0; i < NumStacks; i++)
		w.stacks[i] = nil;
	for(i = 0; i < Maxlev; i++)
		w.count[i] = 0;
	w.stacks[Sfill] = "1" :: nil;
	w.stacks[Slist] = "list" :: nil;
}

copy_state(w: ref Hwin, pw: ref Hwin)
{
	w.base = pw.base;
	w.doctitle = pw.doctitle;
	w.numtags = pw.numtags;
	w.adjust_size = pw.adjust_size;
	w.curadjsize = pw.curadjsize;
	w.tabsize = pw.tabsize;
	w.nocache = pw.nocache;
	w.maxstale = pw.maxstale;
	w.maxage = pw.maxage;
	w.globfont = pw.globfont;
	w.curfont = pw.curfont;
	w.level = 0;
	w.menu = "";
	w.font_tags = nil;
	w.anchors = nil;
	w.dests = nil;
	w.forms = nil;
	w.imreqs = nil;
	w.curanchor = nil;
	w.curform = pw.curform;
	w.curadjsize = 0;
	for(i := 0; i < NumStacks; i++)
		w.stacks[i] = nil;
	for(i = 0; i < Maxlev; i++)
		w.count[i] = 0;
	w.stacks[Sfill] = "1" :: nil;
	w.stacks[Slist] = "list" :: nil;
	w.stacks[Sfamily] = copy_stack(pw.stacks[Sfamily]);
	w.stacks[Ssize] = copy_stack(pw.stacks[Ssize]);
	w.stacks[Sweight] = copy_stack(pw.stacks[Sweight]);
	w.stacks[Sstyle] = copy_stack(pw.stacks[Sstyle]);
}

copy_stack(lin: list of string) : list of string
{
	if(lin == nil)
		return nil;
	return (hd lin) :: copy_stack(tl lin);
}

webstart(w: ref Hwin): int
{
	webio := sys->open("/chan/webget", sys->ORDWR);
	if(webio == nil) {
		webget := load Webget Webget->PATH;
		if(webget == nil)
			error(w, "", "can't load webget from " + Webget->PATH);
		spawn webget->init(nil, nil);
		ntries := 0;
		while(webio == nil && ntries++ < 10)
			webio = sys->open("/chan/webget", sys->ORDWR);
		if(webio == nil) {
			error(w, "", "error connecting to web");
			return -1;
		}
	}
	w.webio = webio;
	return 0;
}

go(w: ref Hwin, g: ref GoSpec)
{
	if(g.url == "")
		return;
	loc := "";
	u := U->makeurl(g.url);
	b := U->makeurl(w.base);
	u.makeabsolute(b);
	if(u.host == b.host && u.path == b.path && u.frag != "") {
		go_local(w, u.frag);
		g = nil;
	}
	else {
		if(g.split)
			spawn hwin_body(w, wmlib->geom(w.top), u, g.post, g.body);
		else
			spawn go_url(w, u, g.post, g.body);
	}
}

go_url(w: ref Hwin, url: ref ParsedUrl, post: int, body: string)
{
	w.pid = sys->pctl(0, nil);
	top := w.top;
	tk->cmd(top, "cursor -bitmap cursor.wait");
	(doctype, actual, clen) := webheader(w, post, url, "text/html,text/plain,image/x-compressed", body);
	if(doctype == "image/x-compressed") {
		fd := string(w.webio.fd);
		act := U->makeurl(actual);
		im := ref TkImage(url, imagename(act), act);
		e := tk->cmd(top, "image create bitmap " + im.image + " -file <" + fd);
		if(tklib->is_err(e)) {
			tk->cmd(top, "variable lasterror");
			status(w, "Can't create image " + im.image);
		}
		else {
			imagecache(w, im, w.nimage);
			label := w.name + ".l" + string(w.numtags);
			delete_all(w);
			tk->cmd(top, ".entry delete 0 end");
			tk->cmd(top, ".entry insert end '" + w.base);
			tk->cmd(top, "update");
			tk->cmd(top, "label " + label + " -image " + im.image);
			tk->cmd(top, w.name + " window create end -window " + label);
			tk->cmd(top, "update");
		}
	}
	else if(doctype == "text/html" || doctype == "text/plain") {
		w.base = actual;
		contents := array[clen] of byte;
		i := 0;
		n := 0;
		while(i < clen) {
			n = sys->read(w.webio, contents[i:], clen-i);
			if(n < 0)
				break;
			i += n;
		}
		if(n >= 0) {
			if(cci != nil)
				cci->view(actual, doctype, contents);
			w.source = contents;
			delete_all(w);
			tk->cmd(top, ".entry delete 0 end");
			tk->cmd(top, ".entry insert end '" + w.base);
			tk->cmd(top, "update");
			if(doctype == "text/html") {
				toks := lex(w.source, 1);
				if(toks != nil) {
					histbase := w.base;
					build(w, toks);
					add_history(w, ref HistEntry(histbase, w.doctitle));
				}
			}
			else {
				tk->cmd(top, w.name + " insert end '" + string contents);
			}
		}
		else
			error(w, "", "webget error: wrong content length");
	}
	tk->cmd(top, "cursor -default");
}

go_local(w: ref Hwin, loc: string)
{
	tries := 1;
	while(tries < 10) {
		for(ld := w.dests; ld != nil; ld = tl ld) {
			d := hd ld;
			if(d.name == loc) {
				tk->cmd(w.top, w.name + " yview D" + string(d.index));
			}
		}
		tries = 10;
	}
}

# Add h to history list of w's toplevel hwin.
# Remove any identical element from further down the list.
# Truncate the list to contain at most Maxhist.
add_history(w: ref Hwin, h: ref HistEntry)
{
	tw := hwins[w.topid];
	for(i := 0; i < tw.nhist; i++) {
		if(tw.hist[i].url == h.url) {
			tk->cmd(tw.top, ".go.m delete " + string i);
			if(i != tw.nhist-1)
				tw.hist[i:] = tw.hist[i+1:tw.nhist];
			tw.nhist--;
			break;
		}
	}
	if(tw.nhist > 0) {
		m := tw.nhist;
		if(m == Maxhist) {
			m--;	# discard oldest
			tw.nhist--;
			tk->cmd(tw.top, ".go.m delete " + string m);
		}
		tw.hist[1:] = tw.hist[0:m];
	}
	tw.nhist++;
	tk->cmd(tw.top, ".go.m insert 0 command -label " + tklib->tkquote(h.title) +
			" -command {send hctl go " + h.url + "}");
	tw.hist[0] = h;
}

show_source(w: ref Hwin, how: string)
{
	top := w.top;
	if(how == "show") {
		tk->cmd(top, "cursor -bitmap cursor.wait");
		delete_all(w);

		# delete any nulls from source before string conversion
		n := len w.source;
		a := array[n] of byte;
		j := 0;
		for(i := 0; i < n; i++) {
			c := w.source[i];
			if(int c != 0)
				a[j++] = c;
		}
		tk->cmd(top, w.name + " insert 1.0 '" + string a[0:j]);
		tk->cmd(top, "update");
		tk->cmd(top, "cursor -default");
	}
	else {
		for(;;) {
			fname := tklib->getstring(top, "File");
			n := len w.source;
			fd := sys->create(fname, sys->OWRITE, 8r664);
			if(fd != nil && sys->write(fd, w.source, n) == n)
					break;
			if(tklib->dialog(top, "Can't save file " + fname, 0, "Cancel" :: "Try another file" :: nil) == 0)
				break;
		}
	}
}

bookmarks(w: ref Hwin, s: string)
{
	if(bmarkchan == nil) {
		bmarkchan = chan of string;
		spawn bmarkproc(w);
	}
	case s {
	"start" =>
		;
	"add" =>
		if(w.base != "" && bmarkchan != nil)
			bmarkchan <-= "add " + w.doctitle + " " + w.base;
	}
}

bmarkproc(w: ref Hwin)
{
	g := wmlib->geom(w.top);
	btop := tk->toplevel(screen, "-bd 2 -relief raised " + g);
	if(btop == nil)
		return;

	bwmctl := wmlib->titlebar(btop, "Guido: Bookmarks", wmlib->Appl);
	bctl := chan of string;
	tk->namechan(btop, bctl, "bctl");
	tklib->tkcmds(btop, bmark_config);
	e := tk->cmd(btop, "variable lasterror");
	if(e != nil) {
		error(w, "internal error", e);
		return;
	}
	
	user := "";
	fd := sys->open("/dev/user", sys->OREAD);
	if(fd != nil) {
		b := array[40] of byte;
		n := sys->read(fd, b, len b);
		if(n > 0)
			user = string b[0:n];
	}
	bmfile := "/usr/" + user + "/bookmarks";
	readbmarks(w, btop, bmfile);

	for(;;) {
		tk->cmd(btop, "update");
		alt {
		s := <- bmarkchan =>
			if(prefix("add ", s)) {
				s = s[4:];
				tk->cmd(btop, ".lb insert end '" + s);
				writebmarks(btop, bmfile);
			}
		s := <- bctl =>
			sel := tk->cmd(btop, ".lb curselection");
			if(sel == "")
				continue;
			if(s == "go") {
				l := tk->cmd(btop, ".lb get " + sel);
				(nil, url) := splitr(l, whitespace);
				if(url != nil)
					w.hctl <-= "go " + url;
			}
			else if(s == "delete") {
				l := tk->cmd(btop, ".lb delete " + sel);
				writebmarks(btop, bmfile);
			}
		s := <- bwmctl =>
			if(s == "exit") {
				bmarkchan = nil;
				return;
			}
			wmlib->titlectl(btop, s);
		}
	}
}

readbmarks(w: ref Hwin, btop: ref Tk->Toplevel, bmfile: string)
{
	fd := sys->open(bmfile, sys->OREAD);
	if(fd != nil) {
		(n, dir) := sys->fstat(fd);
		if(n < 0)
			return; # shouldn't happen
		buf := array[dir.length] of byte;
		n = sys->read(fd, buf, dir.length);
		if(n != dir.length)
			error(w, "Bookmarks", "Error reading bookmark file");
		else {
			(nline, line) := sys->tokenize(string buf[0:n], "\r\n");
			while(line != nil) {
				s := hd line;
				line = tl line;
				tk->cmd(btop, ".lb insert end '" + s);
			}
		}
	}
}

writebmarks(btop: ref Tk->Toplevel, bmfile: string)
{
	fd := sys->create(bmfile, sys->OWRITE, 8r666);
	if(fd != nil) {
		size := tk->cmd(btop, ".lb size");
		n := int size;
		for(i := 0; i < n; i++) {
			s := tk->cmd(btop, ".lb get " + string i);
			b := array of byte (s + "\n");
			sys->write(fd, b, len b);
		}
	}
}

set_indent_tags(w: ref Hwin)
{
	mm := int(w.tabsize * 10.0);
	tabs := mm/2;
	tk->cmd(w.top, w.name + " configure -tabs " + string(tabs) + "m");
	for(i := 1; i < Maxlev; i++) {
		tab := i * mm;
		err := tk->cmd(w.top, w.name + " tag configure indent" + string(i)
			+ " -lmargin1 " + string(tab) + "m"
			+ " -lmargin2 " + string(tab) + "m"
			+ " -tabs " + string(tab + tabs) + "m " + string(tab + 2*tabs) + "m");
	}
}

delete_all(w: ref Hwin)
{
	top := w.top;
	name := w.name;
	tk->cmd(top, name + " delete 1.0 end");
	tk->cmd(top, name + " see 1.0");
	for(la := w.anchors; la != nil; la = tl la) {
		m := hd la;
		s := string(m.index);
		tk->cmd(top, name + " mark unset A" + s);
		tk->cmd(top, name + " mark unset a" + s);
	}
	for(ld := w.dests; ld != nil; ld = tl ld) {
		d := hd ld;
		tk->cmd(top, name + " mark unset D" + string(d.index));
	}
	for(lf := w.forms; lf != nil; lf = tl lf) {
		f := hd lf;
		s := string(f.formid);
		tk->cmd(top, name + " mark unset F" + s);
		tk->cmd(top, name + " mark unset f" + s);
	}
	for(lt := w.tables; lt != nil; lt = tl lt) {
		# remove only pointer to embedded windows
		for(cl := (hd lt).cells; cl != nil; cl = tl cl) {
			c := hd cl;
			if(!c.simple) {
				hw := hwins[c.hwinid];
				tk->cmd(top, "destroy " + hw.name);
				hwins[c.hwinid] = nil;
			}
		}
	}
	tk->cmd(top, name + " configure -bg " + w.defbackground + " -fg black");
	tk->cmd(top, ".Wm_t.title configure -text '" + tooltitle);
	restore_state(w);
}

build(w: ref Hwin, toks: array of ref Lex)
{
	n := len toks;
	if(n == 0)
		return;
	dummy := ref Lex(HTML->Thtml, "", nil);
	for(i := 0; i<n; i++) {
		tlex := toks[i];
		if(tlex.tag == HTML->Notfound || tlex.tag == HTML->Notfound + RBRA)
			continue;
		if(tlex.tag == HTML->Data)
			render(w, dummy, tlex, toks);
		else if(tlex.tag == HTML->Ttable) {
			tab: ref Table;
			(tab, i) = parse_table(w, toks, i);
			if(tab != nil)
				render_table(w, tab);
		}
		else if(i < n-1 && toks[i+1].tag == HTML->Data) {
			render(w, tlex, toks[i+1], toks);
			i++;
		}
		else
			render(w, tlex, nil, toks);
	}
	tk->cmd(w.top, "update");
	getimages(w);
}

# tlex is a tag lex
# dlex is nil, or the data lex that immediately follows tlex
render(w: ref Hwin, tlex, dlex: ref Lex, nil: array of ref Lex)
{
	text, sp : string;

	top := w.top;
	name := w.name;

	if(dlex != nil)
		text = dlex.text;
	else
		text = "";

	tid, start: int;
	if(tlex.tag < RBRA) {
		tid = tlex.tag;
		start = 1;
	}
	else {
		tid = tlex.tag - RBRA;
		start = 0;
	}

	stack(w, tid, start, tlex.attr);
	if(start)
		sp = taginfo[tid].opensp;
	else
		sp = taginfo[tid].closesp;
	filling := (w.stacks[Sfill] != nil && hd(w.stacks[Sfill]) == "1");
	if(sp != "") {
		cmd := name + " insert end {" + sp + "}";
		if(w.curfont != w.globfont)
			cmd = cmd + " " + w.curfont;
		tk->cmd(top, cmd);
		if(filling)
			text = drop(text, whitespace);
	}
	if(filling)
		text = zap_white(text);

	(ifnd, id) := attrvalue(tlex.attr, "id");
	if(ifnd) {
		d := ref DestAnchor(w.numtags, id);
		w.dests = d :: w.dests;
		tk->cmd(top, name + " mark set D" + string(d.index) + " end");
	}

	# special processing for some tags
	case tlex.tag {
	HTML->Ta =>
		(hfnd, href) := attrvalue(tlex.attr, "href");
		(nfnd, aname) := attrvalue(tlex.attr, "name");
		if(hfnd) {
			href = drop(href, whitespace);
			a := ref Anchor(w.numtags, href, 0);
			w.anchors = a :: w.anchors;
			tk->cmd(top, name + " mark set A" + string(a.index) + " end");
			w.curanchor = a;
		}
		if(nfnd) {
			d := ref DestAnchor(w.numtags, aname);
			w.dests = d :: w.dests;
			tk->cmd(top, name + " mark set D" + string(d.index) + " end");
		}
	HTML->Ta + RBRA =>
		if(w.curanchor != nil) {
			i := w.curanchor.index;
			tk->cmd(top, name + " mark set a" + string(i) + " end");
			w.curanchor = nil;
		}
	HTML->Tbase =>
		(fnd, href) := attrvalue(tlex.attr, "href");
		if(fnd && href != "")
			w.base = drop(href, whitespace);
	HTML->Tbasefont =>
		(fnd, ssize) := attrvalue(tlex.attr, "size");
		if(fnd)
			w.curadjsize = int ssize - 3;
	HTML->Tbody =>
		(bgfnd, bgurl) := attrvalue(tlex.attr, "background");
		# (don't do anything with image, for now)
		if(w.bgimage == nil) {
			(fnd, col) := attrvalue(tlex.attr, "bgcolor");
			if(fnd && prefix("#", col)) {
				tk->cmd(top, name + " configure -bg " + col);
				if(tk->cmd(top, "variable lasterror") == "")
					w.background = col;
			}
		}
		(tfnd, txt) := attrvalue(tlex.attr, "text");
		if(tfnd) {
			tk->cmd(top, name + " configure -fg " + txt);
			tk->cmd(top, "variable lasterror");
		}
		(lfnd, lnk) := attrvalue(tlex.attr, "link");
		if(lfnd) {
			tk->cmd(top, name + " tag configure link -fg " + lnk);
			tk->cmd(top, "variable lasterror");
		}
	HTML->Tdiv =>
		(fnd, align) := attrvalue(tlex.attr, "align");
		if(fnd) {
			align = tolower(align);
			just := "";
			if(align == "center")
				just = align;
			else if(align == "right")
				just = "rjust";
			else
				just = "ljust";
			w.stacks[Sjust] = just :: w.stacks[Sjust];
		}
	HTML->Tdiv + RBRA =>
		if(w.stacks[Sjust] != nil)
			w.stacks[Sjust] = tl w.stacks[Sjust];
	HTML->Tdt =>
		tgs : list of string = nil;
		if(w.curfont != w.globfont)
			tgs = w.curfont :: tgs;
		if(w.stacks[Slist] != nil)
			tgs = hd(w.stacks[Slist]) :: tgs;
		ind := indent_tag(w, -1);
		if(ind != "")
			tgs = ind :: tgs;
		append(w, text, tgs);
		text = "";
	HTML->Tfont =>
		s, cur : int;
		if(w.stacks[Ssize] != nil)
			cur = int (hd w.stacks[Ssize]);
		else
			cur = 0;
		s = cur;
		(fnd, ssize) := attrvalue(tlex.attr, "size");
		if(fnd && len ssize > 0) {
			if(ssize[0] == '+')
				s = cur + int ssize[1:];
			else if(ssize[0] == '-')
				s = cur - int ssize[1:];
			else
				s = cur + (int ssize) - 3;
		}
		w.stacks[Ssize] = (string s) :: w.stacks[Ssize];
		coltag := "";
		(cfnd, col) := attrvalue(tlex.attr, "color");
		if(cfnd) {
			
			if(prefix("#", col))
				coltag = "C" + col[1:];
			else
				coltag = "C" + col;
			tk->cmd(top, name + " tag configure " + coltag + " -fg " + col);
			tk->cmd(top, "variable lasterror");
		}
		w.stacks[Sforeground] = coltag :: w.stacks[Sforeground];
	HTML->Tfont + RBRA =>
		if(w.stacks[Ssize] != nil)
			w.stacks[Ssize] = tl w.stacks[Ssize];
		if(w.stacks[Sforeground] != nil)
			w.stacks[Sforeground] = tl w.stacks[Sforeground];
	HTML->Tform =>
		(nil, action) := attrvalue(tlex.attr, "action");
		(nil, method) := attrvalue(tlex.attr, "method");
		f := ref Form(w.numtags, w.id, action, tolower(method), 0, nil);
		w.forms = f :: w.forms;
		tk->cmd(top, name + " mark set F" + string(f.formid) + " end");
		w.curform = f;
	HTML->Tform + RBRA =>
		if(w.curform != nil) {
			# reverse the fields to put them in appearance order
			nflds : list of ref Field = nil;
			for(fl := w.curform.fields; fl != nil; fl = tl fl)
				nflds = (hd fl) :: nflds;
			w.curform.fields = nflds;
			i := w.curform.formid;
			tk->cmd(top, name + " mark set f" + string(i) + " end");
			w.curform = nil;
		}
	HTML->Thr =>
		tag_hrule(w, tlex);
	HTML->Timg =>
		tag_img(w, tlex.attr);
	HTML->Tinput =>
		tag_input(w, tlex.attr);
	HTML->Tli =>
		x := w.menu;
		if(x == "") {
			lev := w.level-1;
			if(lev >= 0 && lev < Maxlev && w.count[lev] >= 0) {
				w.count[lev]++;
				x = string w.count[lev];
			}
			else {
				if(lev >= 0 && lev < len w.symbols)
					x = w.symbols[lev:lev+1];
				else
					x = "+";
			}
		}
		x = "\t" + x + "\t";
		tgs := list of {"mark"};
		if(w.curfont != w.globfont)
			tgs = w.curfont :: tgs;
		if(w.stacks[Slist] != nil)
			tgs = hd(w.stacks[Slist]) :: tgs;
		ind := indent_tag(w, -1);
		if(ind != "")
			tgs = ind :: tgs;
		append(w, x, tgs);
	HTML->Tmenu =>
		w.menu = "→";
	HTML->Tmenu + RBRA =>
		w.menu = "";
	HTML->Tol =>
		if(w.level >= 0 && w.level < Maxlev)
			w.count[w.level] = 0;
	HTML->Toption =>
		tag_option(w, tlex.attr, text);
		text = "";
	HTML->Tscript =>
		text = "";
	HTML->Tselect =>
		tag_select(w, tlex.attr);
	HTML->Tselect + RBRA =>
		tag_selectend(w);
	HTML->Tstyle =>
		text = "";
	HTML->Ttextarea =>
		tag_textarea(w, tlex.attr, text);
		text = "";
	HTML->Ttitle =>
		w.doctitle = drop(text, whitespace);
		if(w.doctitle != "") {
			if(len w.doctitle > 64)
				w.doctitle = w.doctitle[0:64] + "...";
			tk->cmd(top, ".Wm_t.title configure -text '" + tooltitle + ": " + w.doctitle);
		}
		text = "";
	HTML->Tul =>
		if(w.level >= 0 && w.level < Maxlev)
			w.count[w.level] = -1;
	}

	append(w, text, current_tags(w));

	w.numtags++;
	if((w.numtags % w.update_mod) == 0)
		tk->cmd(top, "update");
}

anytables(toks: array of ref Lex) : int
{
	for(i := 0; i < len toks; i++)
		if(toks[i].tag == HTML->Ttable)
			return 1;
	return 0;
}

stack(w: ref Hwin, tid, start: int, attr: list of Attr)
{
	if(taginfo[tid].is_listel) {
		if(start) {
			(fnd, v) := attrvalue(attr, "compact");
			if(fnd)
				lst := "compact";
			else
				lst = "list";
			w.stacks[Slist] = lst :: w.stacks[Slist];
		}
		else if(w.stacks[Slist] != nil)
			w.stacks[Slist] = tl w.stacks[Slist];
	}
	a := taginfo[tid].stkvals;
	for(j := 0; j < len a; j++) {
		k := a[j].stk;
		if(start) {
			val := a[j].val;
			w.stacks[k] = val :: w.stacks[k];
		}
		else {
			if(w.stacks[k] != nil)
				w.stacks[k] = tl w.stacks[k];
		}
	}
}

zap_white(data: string): string
{
	s : string;
	ans := "";
	while(data != "") {
		(s, data) = splitl(data, whitespace);
		ans = ans + s;
		if(len data > 0)
			ans = ans + " ";
		data = drop(data, whitespace);
	}
	return ans;
}

trim_white(data: string): string
{
	data = drop(data, whitespace);
	(l,r) := splitr(data, "^" + whitespace);
	return l;
}

indent_tag(w: ref Hwin, offset: int): string
{
	lev := w.level + offset;
	if(lev < 1 || lev >= Maxlev)
		return "";
	return "indent" + string lev;
}

append(w: ref Hwin, text: string, tgs: list of string)
{
	if(text != "") {
		if(tgs != nil)
			tk->cmd(w.top, w.name + " mark set oldend end");
		tk->cmd(w.top, w.name + " insert end '" + text);
		if(tgs != nil)
			addtags(w, tgs, "oldend end");
	}
}

addtags(w: ref Hwin, tgs: list of string, index: string)
{
	for(curtags := tgs; curtags != nil; curtags = tl curtags)
		tk->cmd(w.top, w.name + " tag add " + hd curtags + " " + index);
}

tag_hrule(w: ref Hwin, tlex: ref Lex)
{
	label := w.name + "." + string(w.numtags);
	(wset, width) := attrvalue(tlex.attr, "width");
	hrcfg := "";
	if(wset) {
		wd := makewidth(tlex);
		val := wd.spec;
		if(wd.kind == Wpercent) {
			winwid := tk->cmd(w.top, w.name + " cget -actwidth");
			val = (int winwid) * val / 100;
		}
		hrcfg = " -width " + string val;
	}
	tk->cmd(w.top, "label " + label + " -image hrule" + hrcfg);
	tk->cmd(w.top, w.name + " window create end -window " + label);
	tk->cmd(w.top, w.name + " insert end {\n}");
}

tag_input(w: ref Hwin, attr: list of Attr)
{
	form := w.curform;
	if(form == nil)
		return;

	wname := w.name + ".F" + string form.formid + "." + string form.nfields;

	(tfound, ftype) := attrvalue(attr, "type");
	if(!tfound)
		ftype = "text";
	(nfound, name) := attrvalue(attr, "name");
	(vfound, value) := attrvalue(attr, "value");
	(checked, nil) := attrvalue(attr, "checked");

	widget := "";
	fconfig := "";
	ty := Ftext;
	fntc := " -font " + inferno_font("T", "0", "m", "r", 0);
	case ftype {
		"text" or "password" =>
			widget = "entry";
			sz := 20;
			(szfound, ssize) := attrvalue(attr, "size");
			if(szfound) {
				sz = int ssize + 2;
				if(sz <= 0 && !vfound)
					sz = 20;
			}
			fconfig = fntc + " -bd 2 -relief sunken -width " + string sz + "w";
			if(ftype == "password") {
				ty = Fpassword;
				fconfig = fconfig + " -show •";
			}
		"checkbox" =>
			ty = Fcheckbox;
			widget = "checkbutton";
			if(!nfound)
				return;
			if(!vfound)
				value = "1";
			fconfig = "-variable " + wname;
		"radio" =>
			ty = Fradio;
			widget = "radiobutton";
			if(!nfound || !vfound)
				return;
			fconfig = "-variable " + "F" + string(form.formid) + "." + name + " -value " + value;
		"submit" =>
			ty = Fsubmit;
			widget = "button";
			if(!vfound)
				value = "Submit";
			fconfig = "-command {send hctl fsubmit " + string (form.formid) + " sub} " +
					fntc + " -bd 2 -relief raised -text '" + value;
		"hidden" =>
			ty = Fhidden;
		"image" =>
			ty = Fimage;
			widget = "label";
			fconfig = "-text <image>";
		"reset" =>
			ty = Freset;
			widget = "button";
			if(!vfound)
				value = "Reset";
			fconfig = "-command {send hctl freset " + string (form.formid) + "} " +
					fntc + " -bd 2 -relief raised -text '" + value;
		* =>
			return;
	}

	f := ref Field(wname, ty, form.nfields, form.formid, w.id, name, value, checked, nil);
	if(ty != Fhidden) {
		tk->cmd(w.top, widget + " " + wname + " " + fconfig);
		tk->cmd(w.top, w.name + " window create end -window " + wname);
		if(ty == Ftext || ty == Fpassword)
			tk->cmd(w.top, "bind " + wname + " <Key-\n> {send hctl fsubmit " +
				string form.formid + " cr}");
		else if(ty == Fimage) {
			(srcset, src) := attrvalue(attr, "src");
			if(srcset) {
				w.imreqs = ref ImageReq(src, wname, "", "") :: w.imreqs;
				tk->cmd(w.top, "bind " + wname
					+ " <Button-1> {send hctl fimage "
					+ string(form.formid) + " "  + wname + " %x %y}");
				tk->cmd(w.top, "bind " + wname
					+ " <Button-1-Motion> {}");
			}
		}
	}
	reset_field(w, f);
	form.nfields++;
	form.fields = f :: form.fields;

	e := tk->cmd(w.top, "variable lasterror");
	if(e != "")
		error(w, "internal error", e);
}

tag_textarea(w: ref Hwin, attr: list of Attr, text: string)
{
	form := w.curform;
	if(form == nil)
		return;

	wname := w.name + ".F" + string form.formid + "." + string form.nfields;

	(rfound, srows) := attrvalue(attr, "rows");
	(cfound, scols) := attrvalue(attr, "cols");
	(nfound, name) := attrvalue(attr, "name");
	rows := 3;
	cols := 50;
	if(rfound)
		rows = int srows;
	if(cfound)
		cols = int scols;
	# allow for external padding
	rows++;
	cols++;
	tk->cmd(w.top, "text " + wname + " -relief sunken -bd 2 -height " +
			string rows + "h -width " + string cols + "w");
	tk->cmd(w.top, w.name + " window create end -window " + wname);
	f := ref Field(wname, Ftextarea, form.nfields, form.formid, w.id, name, text, 0, nil);
	form.fields = f :: form.fields;
	form.nfields++;
}

tag_select(w: ref Hwin, attr: list of Attr)
{
	form := w.curform;
	if(form == nil)
		return;

	wname := w.name + ".F" + string form.formid + "." + string form.nfields;
	(nil, name) := attrvalue(attr, "name");
	# todo: size (number of visible choices) and multiple attrs
	f := ref Field(wname, Fselect, form.nfields, form.formid, w.id, name, "", 0, nil);
	form.nfields++;
	form.fields = f :: form.fields;
}

curselect(w: ref Hwin): ref Field
{
	if(w.curform == nil || w.curform.fields == nil)
		return nil;
	f := hd w.curform.fields;
	if(f.ftype != Fselect)
		return nil;
	return f;
}

tag_selectend(w: ref Hwin)
{
	f := curselect(w);
	if(f == nil)
		return;
	l := f.options;
	if(l == nil) {
		w.curform.fields = tl w.curform.fields;
		return;
	}
	wname := f.window;

	# reverse the list back to original order
	# also get needed width and height
	opts : list of ref Option = nil;
	maxwid := 0;
	num := 0;
	while(l != nil) {
		o := hd l;
		opts = o :: opts;
		wid := len o.display;
		if(wid > maxwid)
			maxwid = wid;
		l = tl l;
		num++;
	}
	f.options = opts;
	# hacky expression because 'h' suffix doesn't allow for interline padding
	num = (num * 17) / 10;
	tk->cmd(w.top, "listbox " + wname + " -relief sunken -bd 2 -width " +
		string (maxwid+1) + "w -height " + string num + ".2h");
	tk->cmd(w.top, w.name + " window create end -window " + wname);
	for(l = opts; l != nil; l = tl l)
		tk->cmd(w.top, wname + " insert end '" + (hd l).display);
	reset_field(w, f);
}

tag_option(w: ref Hwin, attr: list of Attr, text: string)
{
	f := curselect(w);
	if(f == nil)
		return;
	(vfnd, val) := attrvalue(attr, "value");
	(selected, nil) := attrvalue(attr, "selected");
	text = trim_white(text);
	if(!vfnd)
		val = text;
	o := ref Option(selected, val, text);
	f.options = o :: f.options;
}

selectionval(w: ref Hwin, f: ref Field) : string
{
	sel := tk->cmd(w.top, f.window + " curselection");
	if(sel != "") {
		seln := int sel;
		num := 0;
		for(l := f.options; l != nil; l = tl l) {
			o := hd l;
			if(num == seln)
				return o.value;
			num++;
		}
	}
	return "";
}

reset_field(w: ref Hwin, f: ref Field)
{
	top := w.top;
	case f.ftype {
		Ftext or Fpassword =>
			tk->cmd(top, f.window + " delete 0 end");
			if(f.value != "")
				tk->cmd(top, f.window + " insert end '" + f.value);
		Fcheckbox =>
			v := tk->cmd(top, "variable " + f.window);
			if(v == "0" && f.checked || v == "1" && !f.checked)
				tk->cmd(top, f.window + " invoke");
		Fradio =>
			if(f.checked)
				tk->cmd(top, f.window + " invoke");
		Fselect =>
			l := f.options;
			selnum := 0;
			num := 0;
			while(l != nil) {
				o := hd l;
				if(o.selected)
					selnum = num;
				l = tl l;
				num++;
			}
			tk->cmd(top, f.window + " selection clear 0 end");
			tk->cmd(top, f.window + " selection set " + string selnum);
		Ftextarea =>
			tk->cmd(top, f.window + " delete 1.0 end");
	}
}

# if way="cr" then a carriage return was typed in some field
# if way="sub" then submit is via a submit button
# else submit is via an image button, way=name of the button widget,
#      and x and y are the coords within the image button
form_submit(w: ref Hwin, id: int, way, x, y: string) : ref GoSpec
{
	frm := form_find(w, id);
	if(frm == nil || frm.action == "")
		return nil;
	if(way == "cr" && len frm.fields != 1)
		return nil;
	top := w.top;
	v := "";
	sep := "";
	if(frm.method == "get") {
		v = frm.action;
		if(v[len v - 1] != '?')
			sep = "?";
	}
	for(l := frm.fields; l != nil; l = tl l) {
		f := hd l;
		if(f.name == "")
			continue;
		val := "";
		case f.ftype {
			Ftext or Fpassword =>
				val = tk->cmd(top, f.window + " get");
			Fcheckbox =>
				val = tk->cmd(top, "variable " + f.window);
				if(val == "1")
					val = f.value;
				else
					continue;
			Fradio =>
				val = tk->cmd(top, "variable F" + string(frm.formid) + "." + f.name);
				if(val != f.value)
					continue;
			Fsubmit or Fhidden =>
				val = f.value;
			Fselect =>
				val = selectionval(w, f);
			Ftextarea =>
				val = tk->cmd(top, f.window + " get 1.0 end");
			Fimage =>
				if(f.name == way) {
					if(sep != "") {
						v = v + sep;
						sep = "&";
					}
					v = v + ucvt(f.name + ".x") + "=" + ucvt(x)
						+ sep + ucvt(f.name + ".y") + "=" + ucvt(y);
					continue;
				}
		}
		if(sep != "")
			v = v + sep;
		sep = "&";
		v = v + ucvt(f.name) + "=" + ucvt(val);
	}
	if(frm.method == "post")
		return ref GoSpec(frm.action, 1, v, 0);
	else
		return ref GoSpec(v, 0, "", 0);
}

ucvt(s: string): string
{
	u := "";
	for(i := 0; i < len s; i++) {
		c := s[i];
		if(in(c, "/$-_@.!*'(),a-zA-Z0-9"))
			u[len u] = c;
		else if(c == ' ')
			u[len u] = '+';
		else {
			u[len u] = '%';
			u[len u] = hexdigit((c>>4)&15);
			u[len u] = hexdigit(c&15);
		}
	}
	return u;
}

hexdigit(v: int): int
{
	if(0 <= v && v <= 9)
		return '0' + v;
	else
		return 'A' + v - 10;
}

form_reset(w: ref Hwin, id: int)
{
	f := form_find(w, id);
	if(f == nil)
		return;
	for(l := f.fields; l != nil; l = tl l)
		reset_field(w, hd l);
}

form_find(w: ref Hwin, id: int): ref Form
{
	for(l := w.forms; l != nil; l = tl l) {
		f := hd l;
		if(f.formid == id)
			return f;
	}
	return nil;
}

# Parse the table starting at index startind in the toks array
# and return the index of the last lex token used by the table.
# If there is a parsing error, just return startind+1.
#
# DTD elements:
#	table: - - (caption?, (col*|colgroup*), thead?, tfoot?, tbody+)
#	caption: - - (%text+)
#	colgroup: - O (col*)
#	col: - O empty
#	thead: - O (tr+)
#	tfoot: - O (tr+)
#	tbody: O O (tr+)
#	tr: - O (th|td)+
#	th: - O (%body.content)
#	td: - O (%body.content)
parse_table(w: ref Hwin, toks: array of ref Lex, startind: int) : (ref Table, int)
{
	tableid := w.numtags;
	tabletlex := toks[startind];
	n := len toks;
	badret : (ref Table, int) = (nil, startind+1);
	(tlex, i) := nexttok(toks, n, startind);
	if(tlex == nil)
		return badret;

	# caption
	captoks : array of ref Lex = nil;
	capalign := Atop;
	if(tlex.tag == HTML->Tcaption) {
		(nil, al) := attrvalue(tlex.attr, "align");
		capalign = align_val(al, Atop);
		for(j := i+1; j < n; j++) {
			tlex = toks[j];
			if(tlex.tag == HTML->Tcaption + RBRA)
				break;
		}
		if(j >= n) {
			print("bad table: no </caption>\n");
			return badret;
		}
		if(j > i+1) {
			captoks = toks[i+1:j];
		}
		(tlex, i) = nexttok(toks, n, j);
		if(tlex == nil) {
			print("bad table: ends after </caption>\n");
			return badret;
		}
	}

	# colgroup* | col*
	colspecs: list of ref Tablecolspec = nil;
	if(tlex.tag == HTML->Tcolgroup) {
		while(tlex.tag == HTML->Tcolgroup) {
			cgtlex := tlex;
			(tlex, i) = nexttok(toks, n, i);
			if(tlex == nil) {
				print("bad table: ends inside <colgroup>\n");
				return badret;
			}
			subcspecs: list of ref Tablecolspec = nil;
			while(tlex.tag == HTML->Tcol) {
				subcspec := makecolspec(tlex, nil);
				subcspecs = subcspec :: subcspecs;
				(tlex, i) = nexttok(toks, n, i);
				if(tlex == nil) {
					print("bad table: ends inside <col>\n");
					return badret;
				}
			}
			cspec := makecolspec(cgtlex, revcspecl(subcspecs));
			colspecs = cspec :: colspecs;
			if(tlex.tag == HTML->Tcolgroup + RBRA) {
				(tlex, i) = nexttok(toks, n, i);
				if(tlex == nil) {
					print("bad table: ends after </colgroup>");
					return badret;
				}
			}
		}
	}
	else if(tlex.tag == HTML->Tcol) {
		while(tlex.tag == HTML->Tcol) {
			cspec := makecolspec(tlex, nil);
			colspecs = cspec :: colspecs;
			(tlex, i) = nexttok(toks, n, i);
			if(tlex == nil) {
				print("bad table: ends inside <col>\n");
				return badret;
			}
		}
	}
	colspecs = revcspecl(colspecs);

	# head, foot, and body sections (don't enforce order)
	head: ref Tablesection = nil;
	foot: ref Tablesection = nil;
	body : list of ref Tablesection = nil;
	curcell : ref Tablecell = nil;
	cells : list of ref Tablecell = nil;
	cellstart := -1;
	cellid := 0;
	currow : ref Tablerow = nil;
	cursec : ref Tablesection = nil;
    tabloop:
	while(i < n) {
		tlex = toks[i];
		if(curcell != nil && i > cellstart)
			case tlex.tag {
			HTML->Tthead or HTML->Ttfoot or HTML->Ttbody
			or HTML->Ttr or HTML->Tth or HTML->Ttd
			or HTML->Ttable + RBRA
			or HTML->Tthead + RBRA or HTML->Ttfoot + RBRA or HTML->Ttbody + RBRA
			or HTML->Ttr +RBRA or HTML->Tth + RBRA  or HTML->Ttd + RBRA =>
				curcell.content = toks[cellstart:i];
				curcell = nil;
			}
		case tlex.tag {
		HTML->Ttable + RBRA =>
			break tabloop;
		HTML->Ttable =>
			if(curcell == nil) {
				print("bad table: nested table not inside cell\n");
				return badret;
			}
			tablenest := 1;
			while(i < n-1 && tablenest > 0) {
				i++;
				tlex = toks[i];
				if(tlex.tag == HTML->Ttable)
					tablenest++;
				else if(tlex.tag == HTML->Ttable + RBRA)
					tablenest--;
			}
		HTML->Tthead or HTML->Ttfoot or HTML->Ttbody =>
			cursec = ref Tablesection(nil, makealign(tlex, Anone, Anone));
			if(tlex.tag == HTML->Tthead)
				head = cursec;
			else if(tlex.tag == HTML->Ttfoot)
				foot = cursec;
			else
				body = cursec :: body;
		HTML->Ttr =>
			currow = ref Tablerow(nil, makealign(tlex, Anone, Anone));
			if(cursec == nil) {
				cursec = ref Tablesection(nil, Align(Anone, Anone));
				body = cursec :: body;
			}
			cursec.rows = currow :: cursec.rows;
		HTML->Tth or HTML->Ttd =>
			if(currow == nil) {
				print("bad table: <th> or <td> not inside row\n");
				return badret;
			}
			th := 0;
			if(tlex.tag == HTML->Tth)
				th = 1;
			rowspan := 1;
			(rsfnd, rs) := attrvalue(tlex.attr, "rowspan");
			if(rsfnd)
				rowspan = int rs;
			colspan := 1;
			(csfnd, cs) := attrvalue(tlex.attr, "colspan");
			if(csfnd)
				colspan = int cs;
			nowrap := 0;
			(nwfnd, nil) := attrvalue(tlex.attr, "nowrap");
			if(nwfnd)
				nowrap = 1;
			align := makealign(tlex, Anone, Anone);
			width := makewidth(tlex);
			curcell = ref Tablecell(cellid, 0, nil, 0, "", th, rowspan, colspan, nowrap, align, width, 0, 0, 0, 0, 0);
			currow.cells = curcell :: currow.cells;
			cells = curcell :: cells;
			cellstart = i+1;
			cellid++;
		HTML->Tthead + RBRA or HTML->Ttfoot + RBRA or HTML->Ttbody + RBRA
		or HTML->Ttr +RBRA or HTML->Tth + RBRA  or HTML->Ttd + RBRA =>
			;
		}
		i++;
	}

	# now reverse all the lists that were built in reverse order
	# and calculate nrow, ncol

	sections: list of ref Tablesection = nil;
	if(foot != nil)
		sections = foot :: sections;
	while(body != nil) {
		sections = hd body :: sections;
		body = tl body;
	}
	if(head != nil)
		sections = head :: sections;

	nrow := 0;
	ncol := 0;
	for(sl := sections; sl != nil; sl = tl sl) {
		sec := hd sl;
		sec.rows = revrowl(sec.rows);
		for(rl := sec.rows; rl != nil; rl = tl rl) {
			nrow++;
			row := hd rl;
			rcols := 0;
			cl := row.cells;
			row.cells = nil;
			while(cl != nil) {
				c := hd cl;
				row.cells = c :: row.cells;
				rcols += c.colspan;
				cl = tl cl;
			}
			if(rcols > ncol)
				ncol = rcols;
		}
	}
	cells = revcelll(cells);

	width := makewidth(tabletlex);
	(nil, bd) := attrvalue(tabletlex.attr, "border");
	(nil, fr) := attrvalue(tabletlex.attr, "frame");
	(nil, rules) := attrvalue(tabletlex.attr, "rules");
	(nil, cs) := attrvalue(tabletlex.attr, "cellspacing");
	(nil, cp) := attrvalue(tabletlex.attr, "cellpadding");
	tab := ref Table(tableid, nrow, ncol, cellid, makealign(tabletlex, Anone, Anone), width, bd, fr, rules,
			cs, cp, captoks, capalign, colspecs, sections, cells);

	w.tables = tab :: w.tables;
	return (tab, i);
}

# next token after toks[i], skipping whitespace
nexttok(toks: array of ref Lex, ntoks, i: int) : (ref Lex, int)
{
	i++;
	if(i >= ntoks)
		return (nil, i);
	t := toks[i];
	while(t.tag == HTML->Data) {
		if(drop(t.text, whitespace) != "")
			break;
		i++;
		if(i >= ntoks)
			return (nil, i);
		t = toks[i];
	}
	return(t, i);
}

makecolspec(tlex: ref Lex, cols: list of ref Tablecolspec) : ref Tablecolspec
{
	span := 1;
	(spfnd, cspan) := attrvalue(tlex.attr, "span");
	if(spfnd)
		span = int cspan;
	width := makewidth(tlex);
	return ref Tablecolspec(span, width, makealign(tlex, Anone, Anone), cols);
}

makealign(tlex: ref Lex, hdefault, vdefault: int) : Align
{
	(nil,h) := attrvalue(tlex.attr, "align");
	(nil,v) := attrvalue(tlex.attr, "valign");
	hal := align_val(h, hdefault);
	val := align_val(v, vdefault);
	return Align(hal, val);
}

makewidth(tlex: ref Lex) : Width
{
	kind := Wnone;
	spec := 0;
	(fnd, wd) := attrvalue(tlex.attr, "width");
	if(fnd) {
		# parse wd as num[.[num]][unit]
		(l,r) := splitl(wd, "^0-9");
		if(l != "") {
			# accumulate 1000 * value (to work in fixed point)
			spec = 1000 * (int l);
			if(prefix(".", r)) {
				f : string;
				(f,r) = splitl(r[1:], "^0-9");
				if(f != "") {
					mul := 100;
					for(i := 0; i < len f; i++) {
						spec = spec + mul * (int f[i:i+1]);
						mul = mul / 10;
					}
				}
			}
			kind = Wpixels;
			if(r != "") {
				if(len r >= 2) {
					Tkdpi := 100;	# hack, but matches current tk
					units := r[0:2];
					r = r[2:];
					case units {
					"pt" => spec = (spec*Tkdpi)/72;
					"pi" => spec = (spec*12*Tkdpi)/72;
					"in" => spec = spec*Tkdpi;
					"cm" => spec = (spec*100*Tkdpi)/254;
					"mm" => spec = (spec*10*Tkdpi)/254;
					"em" => spec = spec * 15;	# hack, lucidasans 8pt is 15 pixels high
					}
				}
				if(r == "%")
					kind = Wpercent;
				else if(r == "*")
					kind = Wrelative;
			}
			spec = spec / 1000;
		}
	}
	return Width(kind, spec);
}

align_val(sal: string, dflt: int) : int
{
	ans := dflt;
	case sal {
		"left" => ans = Aleft;
		"center" => ans = Acenter;
		"right" => ans = Aright;
		"justify" => ans = Ajustify;
		"char" => ans = Achar;
		"top" => ans = Atop;
		"middle" => ans = Amiddle;
		"bottom" => ans = Abottom;
		"baseline" => ans = Abaseline;
	}
	return ans;
}

revcspecl(l : list of ref Tablecolspec) : list of ref Tablecolspec
{
	ans : list of ref Tablecolspec = nil;
	while(l != nil) {
		ans = hd l :: ans;
		l = tl l;
	}
	return ans;
}

revrowl(l : list of ref Tablerow) : list of ref Tablerow
{
	ans : list of ref Tablerow = nil;
	while(l != nil) {
		ans = hd l :: ans;
		l = tl l;
	}
	return ans;
}

revcelll(l : list of ref Tablecell) : list of ref Tablecell
{
	ans : list of ref Tablecell = nil;
	while(l != nil) {
		ans = hd l :: ans;
		l = tl l;
	}
	return ans;
}

TABPAD : con 10;
render_table(w: ref Hwin, tab: ref Table)
{
#	printtable(tab);
	if(tab.ncol == 0 || tab.nrow == 0)
		return;

	# find where each cell goes in nrow x ncol grid

	gcells := array[tab.nrow] of { * => array[tab.ncol] of { * => ref Tablegcell(nil, 1)} };

	sl : list of ref Tablesection;
	rl : list of ref Tablerow;
	cl : list of ref Tablecell;
	sec : ref Tablesection;
	row : ref Tablerow;
	c : ref Tablecell;

	# the following arrays keep track of cells that are spanning
	# multiple rows;  rowspancnt[i] is the number of rows left
	# to be spanned in column i
	rowspancnt := array[tab.ncol] of { * => 0};
	rowspancell := array[tab.ncol] of ref Tablecell;

	# get current font
	family := "T";
	size := "0";
	weight := "m";
	style := "r";
	if(w.stacks[Sfamily] != nil)
		family = hd(w.stacks[Sfamily]);
	if(w.stacks[Ssize] != nil)
		size = hd(w.stacks[Ssize]);
	if(w.stacks[Sweight] != nil)
		weight = hd(w.stacks[Sweight]);
	if(w.stacks[Sstyle] != nil)
		style = hd(w.stacks[Sstyle]);
	font := inferno_font(family, size, weight, style, w.adjust_size);

	ri := 0;
	ci := 0;
	for(sl = tab.sections; sl != nil; sl = tl sl) {
		sec = hd sl;
		for(rl = sec.rows; rl != nil; rl = tl rl) {
			row = hd rl;
			cl = row.cells;
			for(ci = 0; ci < tab.ncol; ) {
				if(rowspancnt[ci] > 0) {
					gcells[ri][ci].cell = rowspancell[ci];
					gcells[ri][ci].drawnhere = 0;
					rowspancnt[ci]--;
					ci++;
				}
				else {
					if(cl == nil) {
						ci++;
						continue;
					}
					c = hd cl;
					cl = tl cl;
					cspan := c.colspan;
					if(cspan == 0) {
						cspan = tab.ncol - ci;
						c.colspan = cspan;
					}
					rspan := c.rowspan;
					if(rspan == 0) {
						rspan = tab.nrow - ri;
						c.rowspan = rspan;
					}
					c.row = ri;
					c.col = ci;
					for(i := 0; i < cspan && ci < tab.ncol; i++) {
						gcells[ri][ci].cell = c;
						if(i > 0)
							gcells[ri][ci].drawnhere = 0;
						if(rspan > 1) {
							rowspancnt[ci] = rspan-1;
							rowspancell[ci] = c;
						}
						ci++;
					}
				}
			}
			ri++;
		}
	}
#	printgrid(gcells);

	# render each cell into a sub text widget or determine that the contents
	# are simple;
	# get min and max widths
	i : int;
	for(cl = tab.cells; cl != nil; cl = tl cl) {
		c = hd cl;
		simple := 1;
		simpletext := "";
		contents := c.content;
		if(contents != nil)
			for(i = 0; i < len contents && simple; i++) {
				if(contents[i].tag != HTML->Data)
					simple = 0;
				else
					simpletext += contents[i].text;
			}
		if(simple) {
			simpletext = drop(simpletext, whitespace);
			for(i = len simpletext -1; i > 0 ; i--)
				if(!in(simpletext[i], whitespace))
					break;
			if(i < len simpletext - 1)
				simpletext = simpletext[0:i+1];
			c.simple = simple;
			c.simpletext = simpletext;
			(wd, nil) := canvdimen(w, simpletext, font, 1000);
			c.maxwid = wd + TABPAD;
			(wd, nil) = canvdimen(w, simpletext, font, 1);
			c.minwid = wd + TABPAD;
		}
		else {
			cname := w.name + ".c" + string tab.tableid + "_" + string c.cellid;
			cw := newhwin(w, "", "1000", "10", cname);
			c.hwinid = cw.id;
			tk->cmd(cw.top, cname + " configure -wrap none -bd 0");
			build(cw, contents);
			(wd, nil) := textdimen(cw);
			c.maxwid = wd + TABPAD;
			tk->cmd(cw.top, cname + " configure -wrap word -width 1");
			(wd, nil) = textdimen(cw);
			c.minwid = wd + TABPAD;
		}
	}

	# calc max and min column widths
	colminw := array[tab.ncol] of { * => 0};
	colmaxw := array[tab.ncol] of { * => 0};
	colw := array[tab.ncol] of { * => 0};
	minw := 0;
	maxw := 0;
	for(ci = 0; ci < tab.ncol; ci++) {
		for(ri = 0; ri < tab.nrow; ri++) {
			c = gcells[ri][ci].cell;
			if(c == nil)
				continue;
			cwd := c.minwid / c.colspan;
			if(cwd > colminw[ci])
				colminw[ci] = cwd;
			cwd = c.maxwid / c.colspan;
			if(cwd > colmaxw[ci])
				colmaxw[ci] = cwd;
		}
		minw += colminw[ci];
		maxw += colmaxw[ci];
	}

	# assign actual column widths
	winwid := tk->cmd(w.top, w.name + " cget actwidth");
	totw := int winwid;		# BUG: subtract current margins
	if(tab.width.kind == Wpixels)
		totw = tab.width.spec;
	else if(tab.width.kind == Wpercent)
		totw = totw * tab.width.spec / 100;
	W := totw - minw;
	D := maxw - minw;
	for(ci = 0; ci < tab.ncol; ci++) {
		wd : int;
		if(minw >= totw)
			wd = colminw[ci];
		else if(maxw <= totw)
			wd = colmaxw[ci];
		else {
			d := colmaxw[ci] - colminw[ci];
			wd = colminw[ci] + d*W/D;
		}
		colw[ci] = wd;
	}

	# reconfigure cell widgets to proper width, and get row heights
	# first pass: ignore rows that span more than one row in getting row heights
	rowh := array[tab.nrow] of { * => 0 };
	cw : ref Hwin = nil;
	ht : int;
	for(cl = tab.cells; cl != nil; cl = tl cl) {
		c = hd cl;
		if(c.simple)
			cw = nil;
		else
			cw = hwins[c.hwinid];
		wd := 0;
		for(i = 0; i < c.colspan && c.col + i < tab.ncol; i++)
			wd += colw[c.col + i];
		c.configwid = wd;
		if(c.simple)
			(nil, ht) = canvdimen(w, c.simpletext, font, wd);
		else {
			tk->cmd(cw.top, cw.name + " configure -width " + string wd);
			(nil, ht) = textdimen(cw);
		}
		ht += TABPAD;
		if(!c.simple) {
			tk->cmd(cw.top, cw.name + " configure -height " + string ht);
			tk->cmd(cw.top, cw.name + " see 1.0");
		}
		if(c.rowspan == 1 && rowh[c.row] < ht)
			rowh[c.row] = ht;
	}
	# second pass: deal with rowspan > 1
	# (this algorithm isn't quite right -- it might add more space
	# than is needed in the presence of multiple overlapping rowspans)
	for(cl = tab.cells; cl != nil; cl = tl cl) {
		c = hd cl;
		if(c.simple)
			cw = nil;
		else
			cw = hwins[c.hwinid];
		if(c.rowspan > 1) {
			if(c.simple)
				(nil, ht) = canvdimen(w, c.simpletext, font, c.configwid);
			else
				(nil, ht) = textdimen(cw);
			ht += TABPAD;
			spanht := 0;
			for(i = 0; i < c.rowspan && c.row+i < tab.nrow; i++)
				spanht += rowh[c.row+i];
			if(ht > spanht) {
				# add extra space to last spanned row
				i = c.row+c.rowspan-1;
				if(i >= tab.nrow)
					i = tab.nrow - 1;
				rowh[i] += ht - spanht;
			}
		}
	}

	# get total width, heights, and col x / row y positions
	colx := array[tab.ncol] of { * => 0 };
	totw = 0;
	for(ci = 0; ci < tab.ncol; ci++) {
		colx[ci] = totw;
		totw += colw[ci];
	}
	rowy := array[tab.nrow] of { * => 0 };
	toth := 0;
	for(ri = 0; ri < tab.nrow; ri++) {
		rowy[ri] = toth;
		toth += rowh[ri];
	}

	# make canvas to hold them all, and place the cells
	canvname := w.name + ".canv" + string tab.tableid;
	tk->cmd(w.top, "canvas " + canvname + " -width " + string totw
		+ " -height " + string toth + " -bg " + w.background);
	tk->cmd(w.top, w.name + " insert end '" + "\n");
	tk->cmd(w.top, w.name + " window create end -window " + canvname);

	for(cl = tab.cells; cl != nil; cl = tl cl) {
		c = hd cl;
		if(c.simple)
			cw = nil;
		else
			cw = hwins[c.hwinid];
		x := colx[c.col];
		y := rowy[c.row];
		# TODO: pay attention to alignment
		if(c.simple) {
			if(c.simpletext != "")
				tk->cmd(w.top, canvname + " create text "
					+ string x + " " + string y + " -anchor nw -font " + font
					+ " -width " + string c.configwid + " -text '" + c.simpletext);
		}
		else {
			tk->cmd(w.top, canvname + " create window "
				+ string x + " " + string y + " -anchor nw -window " + cw.name);
		}
	}
	tk->cmd(w.top, w.name + " insert end '" + "\n");
	if(tab.caption != nil) {
		w.stacks[Sjust] = "center" :: w.stacks[Sjust];
		build(w, tab.caption);
		w.stacks[Sjust] = tl w.stacks[Sjust];
		tk->cmd(w.top, w.name + " insert end '" + "\n");
	}
}

# return actual (width, height) taken by text in w's widget
textdimen(w: ref Hwin) : (int, int)
{
	bb := tk->cmd(w.top, w.name + " bbox 1.0 all");
	(nil, bbl) := sys->tokenize(bb[1:len bb - 1], " ");
	return(int nth(bbl, 2), int nth(bbl, 3));
}

canvdimen(nil: ref Hwin, text: string, nil: string, nil: int) : (int, int)
{
	# hack, until really do this properly
	ww := 7*len text;
	h := 15;
	return (ww, h);
}

# table debugging
printtable(tab: ref Table)
{
	print("Table %d   %d rows, %d cols\n", tab.tableid, tab.nrow, tab.ncol);
	print("  align: h=%s v=%s\n", align2string(tab.align.halign),
			align2string(tab.align.valign));
	print("  width: %s\n", width2string(tab.width));
	print("  border: %s\n", tab.border);
	print("  frame: %s\n", tab.frame);
	print("  rules: %s\n", tab.rules);
	print("  cellspacing: %s\n", tab.cellspacing);
	print("  cellpadding: %s\n", tab.cellpadding);
	print("  caption: "); printlexes(tab.caption, "    ");
	print("  colspecs:\n"); printcolspecs(tab.colspecs);
	for(sl := tab.sections; sl != nil; sl = tl sl) {
		print("  sections:\n");
		printsection(hd sl);
	}
}

align2string(al: int) : string
{
	s := "";
	case al {
		Anone => s = "none";
		Aleft => s = "left";
		Acenter => s = "center";
		Aright => s = "right";
		Ajustify => s = "justify";
		Achar => s = "char";
		Atop => s = "top";
		Amiddle => s = "middle";
		Abottom => s = "bottom";
		Abaseline => s = "baseline";
	}
	return s;
}

width2string(wd: Width) : string
{
	s := "";
	if(wd.kind==Wnone)
		s = "none";
	else {
		s = sys->sprint("%d", wd.spec);
		if(wd.kind==Wpercent)
			s = s + "%";
		else if(wd.kind==Wrelative)
			s = s + "*";
	}
	return s;
}

printcolspecs(l: list of ref Tablecolspec)
{
	for( ; l != nil; l = tl l) {
		ts := hd l;
		print("    span=%d width=%s", ts.span, width2string(ts.width));
		print(" align h= %s v= %s\n", align2string(ts.align.halign), 
			align2string(ts.align.valign));
		if(ts.cols != nil) {
			print("    cols:\n");
			printcolspecs(ts.cols);
		}
	}
}

printsection(ts: ref Tablesection)
{
	print("    section align h=%s v=%s\n", align2string(ts.align.halign),
		align2string(ts.align.valign));
	for(trl := ts.rows; trl != nil; trl = tl trl) {
		tr := hd trl;
		print("      row align h=%s v=%s\n", align2string(tr.align.halign),
			align2string(tr.align.valign));
		for(cl := tr.cells; cl != nil; cl = tl cl) {
			c := hd cl;
			print("        cell %d align h=%s v=%s th=%d rowspan=%d colspan=%d nowrap=%d\n",
				c.cellid, align2string(c.align.halign), align2string(c.align.valign), c.th, c.rowspan, c.colspan, c.nowrap);
			printlexes(c.content, "        ");
		}
	}
}

printgrid(g: array of array of ref Tablegcell)
{
	nr := len g;
	nc := len g[0];
	for(r := 0; r < nr; r++) {
		for(c := 0; c < nc; c++) {
			x := g[r][c];
			cell := x.cell;
			suf := " ";
			if(x.drawnhere == 0)
				suf = "*";
			if(cell == nil)
				print("     %s", suf);
			else
				print("%5d%s", cell.cellid, suf);
		}
		print("\n");
	}
}

tag_img(w: ref Hwin, attr: list of Attr)
{
	top := w.top;
	curid := string w.id;
	(aset, align) := attrvalue(attr, "align");
	if(!aset)
		align = "bottom";
	align = tolower(align);
	rjust := 0;
	case align {
		"middle" =>
			align = "center";
		"bottom" or "top" =>
			;
		"right" =>
			align = "bottom";
			rjust = 1;
		* =>
			align = "bottom";
	}

	(altset, alttext) := attrvalue(attr, "alt");
	if(!altset)
		alttext = "<image>";

	bdconfig := " -bd 0";
	(bset, border) := attrvalue(attr, "border");
	if(bset)
		bdconfig = " -bd " + border;

	label := w.name + "." + string(w.numtags);
	szconfig := "";
	(wset, width) := attrvalue(attr, "width");
	(hset, height) := attrvalue(attr, "height");
	if(wset && hset)
		szconfig =  " -width " + string(width) +
			" -height " + string(height);

	tk->cmd(top, "label " + label + szconfig + bdconfig +
		" -fg orange -text '" + alttext);

	tk->cmd(top, w.name + " mark set oldend end");

	tk->cmd(top, w.name + " window create end -align " + align +
			" -window " + label + " -pady 2");

	tgs := current_tags(w);
	if(rjust)
		tgs = "rjust" :: tgs;
	addtags(w, tgs, "oldend end");

	# see if it is an imagemap
	(ismap, nil) := attrvalue(attr, "ismap");
	if(ismap && w.curanchor != nil && w.curanchor.href != nil) {
		w.curanchor.ismap = 1;
		tk->cmd(top, "bind " + label +
			" <Button-1> {send hctl imgmap_hit " + curid + " 1 %x %y "
			+ label + " " + w.curanchor.href + "}");
		tk->cmd(top, "bind " + label +
			" <Button-3> {send hctl imgmap_hit " + curid + " 3 %x %y "
			+ label + " " + w.curanchor.href + "}");
		tk->cmd(top, "bind " + label + " <Button-1-Motion> {}");
		tk->cmd(top, "bind " + label + " <Button-3-Motion> {}");
	}

	(srcset, src) := attrvalue(attr, "src");
	if(srcset)
		w.imreqs = ref ImageReq(src, label, width, height) :: w.imreqs;
}

current_tags(w: ref Hwin): list of string
{
	ans : list of string;

	family := "T";
	size := "0";
	weight := "m";
	style := "r";

	if(w.stacks[Sfamily] != nil)
		family = hd(w.stacks[Sfamily]);
	if(w.stacks[Ssize] != nil)
		size= hd(w.stacks[Ssize]);
	if(w.stacks[Sweight] != nil)
		weight = hd(w.stacks[Sweight]);
	if(w.stacks[Sstyle] != nil)
		style = hd(w.stacks[Sstyle]);
	font := font_tag(w, family, size, weight, style);

	w.level = len w.stacks[Sindent];
	w.curfont = font;

	ans = nil;
	if(font != w.globfont)
		ans = font :: ans;
	ind := indent_tag(w, 0);
	if(ind != "")
		ans = ind :: ans;

	for(i := 0; i < len tag_stacks; i++) {
		st := w.stacks[tag_stacks[i]];
		if(st != nil && hd(st) != "")
			ans = hd(st) :: ans;
	}

	return ans;
}

font_tag(w: ref Hwin, family, size, weight, style: string): string
{
	font := "font:" + family + size + weight + style;
	if(font == w.globfont)
		return font;

	for(fl := w.font_tags; fl != nil; fl = tl fl)
		if(hd(fl) == font)
			return font;

	w.font_tags = font :: w.font_tags;
	configure_font(w, font, family, size, weight, style);
	return font;
}

configure_font(w: ref Hwin, font, family, size, weight, style: string)
{

	ifont := inferno_font(family, size, weight, style, w.adjust_size);

	e := tk->cmd(w.top, w.name + " tag configure " + font + " -font " + ifont);
	if(tklib->is_err(e)) {
		tk->cmd(w.top, "variable lasterror");
		print("font_tag %s->%s: %s\n", font, ifont, e);
	}
}

# called when adjust_size has changed
configure_font_tags(w: ref Hwin)
{
	f := inferno_font("T", "0", "m", "r", w.adjust_size);
	tk->cmd(w.top, w.name + " configure -font " + f);

	for(fl := w.font_tags; fl != nil; fl = tl fl) {
		font := hd(fl);
		spec := font[5:];
		family := spec[0:1];
		size := spec[1:2];
		weight := spec[2:3];
		style :=spec[3:4];
		configure_font(w, font, family, size, weight, style);
	}
}

# there aren't fonts for all combinations
inferno_font(family, size, weight, style: string, adj_size: int): string
{
	fi := FntR;
	if(family == "C") {
		if(weight == "b")
			fi = FntBT;
		else
			fi = FntT;
	}
	else if(style == "i")
		fi = FntI;
	else if(weight == "b")
		fi = FntB;
	i := Normal + int(size) + adj_size;
	if(i < Small)
		i = Small;
	if(i > Verylarge)
		i = Verylarge;
	return "/fonts/lucidasans/" + fontinfo[fi].name + "." +
			string(fontinfo[fi].sizes[i]) + ".font";
}

link_hit(hwinid, but, x, y: string) : ref GoSpec
{
	id := int hwinid;
	if(id < 0 || id >= len hwins)
		return nil;
	aw := hwins[id];
	if(aw == nil)
		return nil;
	split := 0;
	if(but == "3")
		split = 1;
	mark := tk->cmd(aw.top, aw.name + " mark previous @" + x + "," + y);
	while(mark != "" && !prefix("A", mark))
		mark = tk->cmd(aw.top, aw.name + " mark previous " + mark);
	if(len mark > 1 && mark[0] == 'A' && mark[1] != '/') {
		i := int(mark[1:]);
		for(l := aw.anchors; l != nil; l = tl l) {
			a := hd l;
			if(a.index == i && a.href != "") {
				if(!a.ismap)
					return ref GoSpec(a.href, 0, "", split);
				break;
			}
		}
	}
	return nil;
}

imgmap_hit(w: ref Hwin, hwinid, but, xs, ys, label, href: string) : ref GoSpec
{
	id := int hwinid;
	if(id < 0 || id >= nhwins)
		return nil;
	aw := hwins[id];
	if(aw == nil)
		return nil;
	split := 0;
	if(but == "3")
		split = 1;
	bb := tk->cmd(aw.top, aw.name + " bbox " + label + " noclip");
	if(tklib->is_err(bb)) {
		tk->cmd(aw.top, "variable lasterror");
		error(w, "internal error", bb);
		return nil;
	}
	x := int xs;
	y := int ys;
	(nil, l) := sys->tokenize(bb, " ");
	a := nth(l, 0);
	wd := int nth(l, 2);
	a = nth(l, 3);
	h := int a[0:len a -1];
	if(x < 0)
		x = 0;
	if(x >= wd)
		x = wd-1;
	if(y < 0)
		y = 0;
	if(y >= h)
		y = h-1;
	return ref GoSpec(href + "?" + string x + "," + string y, 0, "", split);
}

getimages(w: ref Hwin)
{
	irs := w.imreqs;
	if(irs == nil)
		return;
	r : list of ref ImageReq = nil;
	for( ; irs != nil; irs = tl irs)
		r = (hd irs) :: r;
	for( ; r != nil; r = tl r) {
		ir := hd r;
		i := getimage(w, ir.src);
		if(i != nil)
			config_image(w, ir.widget, i.image);
	}
}

getimage(w: ref Hwin, src: string) : ref TkImage
{
	u := U->makeurl(drop(src, whitespace));
	b := U->makeurl(w.base);
	u.makeabsolute(b);
	image := imagename(u);
	tw := hwins[w.topid];

	# check to see if image already loaded
	im : ref TkImage;
	for(i := 0; i < tw.nimage; i++) {
		im = tw.images[i];
		if(im.image == image)
			break;
	}
	if(i >= tw.nimage) {
		im = ref TkImage(u, image, nil);
		do_ireq(w, im);
	}
	if(im.actual != nil) {
		imagecache(tw, im, i);
		return im;
	}
	return nil;
}

# names have to have limited sizefor our implementation
imagename(u: ref ParsedUrl) : string
{
	s := u.host + "/" + u.path;
	n := len s;
	if(n < 60)
		return s;
	i1 := 27;
	i2 := n - 27;
	s1 := s[1:i1];
	s2 := s[i2:];
	c := 0;
	a := array of byte s[i1+1:i2];
	for(i := 0; i < len a; i++)
		c += int a[i];
	s = s1 + string c + s2;
	return s;
}

# add im to w's image cache if i >= w.nimage
# and/or rearrange to put im most-recently-used
imagecache(w: ref Hwin, im: ref TkImage, i: int)
{
	if(i >= w.nimage) {
		if(w.nimage < MaxTkImages-1)
			w.nimage++;
		else {
			# cache is full; replace least recently used
			lim := w.images[MaxTkImages-1];
			if(lim != nil)
				tk->cmd(w.top, "image delete " + lim.image);
		}
		i = w.nimage-1;
	}
	w.images[1:] = w.images[0:i];
	w.images[0] = im;
}

config_image(w: ref Hwin, label, image: string)
{
	top := w.top;

	# width and height hints may have been too small
	# (some browsers appear to scale when this happens)
	wd := int tk->cmd(top, "image width " + image);
	h := int tk->cmd(top, "image height " + image);
	lw := int tk->cmd(top, label + " cget -width");
	lh := int tk->cmd(top, label + " cget -height");
	cfg := "";
	if(wd > lw)
		cfg = " -width " + string(wd);
	if(h > lh)
		cfg = cfg + " -height " + string(h);
	tk->cmd(top, label + " configure -image " + image + cfg);
	tk->cmd(top, "update");
}

error(w: ref Hwin, title, text: string)
{
	if(ignerrs)
		return;
	if(title != "")
		text = title + ": " + text;
	i := tklib->dialog(w.top, text, 0, "OK" :: "Ignore Further Errors" :: "Exit" :: nil);
	if(i == 1)
		ignerrs = 1;
	else
	if(i == 2) {
		tk->cmd(w.top, "cursor -default");
		finish();
	}
}

status(w: ref Hwin, text: string)
{
	tk->cmd(w.top, ".msg configure -text '" + text);
	tk->cmd(w.top, "update");
}

do_ireq(w: ref Hwin, ir: ref TkImage)
{
	status(w, "Fetching image " + ir.image);

	(doctype, newurl, clen) := webheader(w, 0, ir.src, "image/x-compressed", "");
	if (newurl == nil) {
		status(w, "Can't fetch image " + ir.image);
		return;
	}
	fd := string(w.webio.fd);
	e := tk->cmd(w.top, "image create bitmap " + ir.image + " -file <" + fd);
	if(tklib->is_err(e)) {
		tk->cmd(w.top, "variable lasterror");
		status(w, "Can't create image " + ir.image);
		return;
	}
	ir.actual = U->makeurl(newurl);
	status(w, "");
}

webheader(w: ref Hwin, post: int, url: ref ParsedUrl, types, body: string) : (string, string, int)
{
	n : int;
	s : string;
	clen := 0;
	dtype := "";
	nbase := "";
	io := w.webio;
	loc := url.tostring();
	cachectl : string;
	if(w.nocache)
		cachectl = "no-cache";
	else {
		cachectl = "max-stale=" + string(w.maxstale);
		if(w.maxage >= 0)
			cachectl += ",max-age=" + string(w.maxage);
	}
	if(post) {
		bbody := array of byte body;
		s = "POST " + string len bbody + " id1 " + loc + " " + types + " " + cachectl + "\n";
		bs := array of byte s;
		n = sys->write(io, bs, len bs);
		if(n > 0)
			n =sys->write(io, bbody, len bbody);
	}
	else {
		s = "GET 0 id1 " + loc + " " + types + " " + cachectl +  + "\n";
		bs := array of byte s;
		n = sys->write(io, bs, len bs);
	}
	if(n < 0)
		error(w, "", sys->sprint("error writing webget request: %r"));
	else {
		bstatus := array[1000] of byte;
		n = sys->read(io, bstatus, len bstatus);
		if(n < 0)
			error(w, "", sys->sprint("error reading webget response header: %r"));
		else {
			status := string bstatus[0:n];
			(nl, l) := sys->tokenize(status, " \n");
			if(nl < 3)
				error(w, "", "unexpected webget response: " + status);
			else {
				s = hd l;
				l = tl l;
				if(s == "ERROR") {
					(nil, msg) := S->splitl(status[6:], " ");
					error(w, "", msg);
				}
				else if(s == "OK") {
					clen = int (hd l);
					l = tl(tl l);
					dtype = hd l;
					l = tl l;
					nbase = hd l;
				}
				else
					error(w, "", "unexpected webget response: " + status);
			}
		}
	}
	return (dtype, nbase, clen);
}

# 0-origin indexing of string list
nth(l: list of string, n: int) : string
{
	while(l != nil && n > 0) {
		l = tl l;
		n--;
	}
	if(l == nil)
		return "";
	else
		return hd l;
}

finish()
{
	fd := sys->open("#p/" + string pgrp + "/ctl", sys->OWRITE);
	if(fd != nil) {
		sys->fprint(fd, "killgrp");
	}
	exit;
}

# debugging
printlexes(lexes: array of ref Lex, indent: string)
{
	for(i := 0; i < len lexes; i++)
		print("%s%s\n", indent, html->lex2string(lexes[i]));
}

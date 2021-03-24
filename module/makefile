!include ..\mkconfig.nmk
!include ..\$(SYSHOST)/mkhost

FILES=\
	$(PREFIX_PKG)\module\brutus.m\
	$(PREFIX_PKG)\module\brutusext.m\
	$(PREFIX_PKG)\module\bufio.m\
	$(PREFIX_PKG)\module\cci.m\
	$(PREFIX_PKG)\module\daytime.m\
	$(PREFIX_PKG)\module\debug.m\
	$(PREFIX_PKG)\module\devpointer.m\
	$(PREFIX_PKG)\module\draw.m\
	$(PREFIX_PKG)\module\emio.m\
	$(PREFIX_PKG)\module\filepat.m\
	$(PREFIX_PKG)\module\gr.m\
	$(PREFIX_PKG)\module\hash.m\
	$(PREFIX_PKG)\module\html.m\
	$(PREFIX_PKG)\module\imagefile.m\
	$(PREFIX_PKG)\module\ir.m\
	$(PREFIX_PKG)\module\keyring.m\
	$(PREFIX_PKG)\module\lib.m\
	$(PREFIX_PKG)\module\linalg.m\
	$(PREFIX_PKG)\module\math.m\
	$(PREFIX_PKG)\module\mpeg.m\
	$(PREFIX_PKG)\module\newns.m\
	$(PREFIX_PKG)\module\pipe.m\
	$(PREFIX_PKG)\module\prefab.m\
	$(PREFIX_PKG)\module\quicktime.m\
	$(PREFIX_PKG)\module\rand.m\
	$(PREFIX_PKG)\module\readdir.m\
	$(PREFIX_PKG)\module\regex.m\
	$(PREFIX_PKG)\module\riff.m\
	$(PREFIX_PKG)\module\runt.m\
	$(PREFIX_PKG)\module\security.m\
	$(PREFIX_PKG)\module\sh.m\
	$(PREFIX_PKG)\module\smtp.m\
	$(PREFIX_PKG)\module\srv.m\
	$(PREFIX_PKG)\module\srvrunt.b\
	$(PREFIX_PKG)\module\string.b\
	$(PREFIX_PKG)\module\sys.m\
	$(PREFIX_PKG)\module\tcllib.m\
	$(PREFIX_PKG)\module\tk.m\
	$(PREFIX_PKG)\module\tklib.m\
	$(PREFIX_PKG)\module\url.m\
	$(PREFIX_PKG)\module\version.m\
	$(PREFIX_PKG)\module\volume.m\
	$(PREFIX_PKG)\module\webget.m\
	$(PREFIX_PKG)\module\wmlib.m\
	$(PREFIX_PKG)\module\workdir.m

all:		

install:

package:	$(PREFIX_PKG)\
		$(PREFIX_PKG)\module\
		$(FILES)


$(FILES):	$(@F)
	-$(DEL) $(DELFLAGS) $@
	copy $(@F) $@

clean nuke:

uninstall:
	-$(DEL) $(DELFLAGS) $(FILES)

$(PREFIX_PKG) \
$(PREFIX_PKG)\module :
	mkdir $@

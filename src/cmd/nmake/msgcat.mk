/*
 * C message catalog support
 */

MSGCC = msgcc
MSGCCFLAGS = -M-set=$(_PACKAGE_MESSAGE_SET)
MSGCATDIR = msgs

/*
 * :MSGFUN: _TRANSLATE_ macro ... [ :foo: _foo_ macro ... ]
 */

":MSGFUN:" : .MAKE .OPERATOR
	local T M
	M = _TRANSLATE_
	for T $(>)
		if T == ":*:"
			M := $(T:/:/_/G)
		else
			MSGCCFLAGS += -D$(T)=$(M)
		end
	end

/*
 * :MSGKEY: prereq ...
 *	additional --keys command on prereqs to standard output
 */

":MSGKEY:" : .MAKE .OPERATOR
	local T
	if "$(@:V)"
		eval
		$$(MSGCATDIR)/msgcat.mso : $(>)
			{ $(@:V) ;} | $(SED) 's,^,str ,' > $$(<)
		end
	end

_PACKAGE_ID = $(LICENSEFILES:/:/ /G:T=F:O=1:B)
_PACKAGE_MESSAGE_SET == $(_PACKAGE_ID|"9":O=n:/[^0-9A-Za-z]//G:/\(...\).*/\1/:/./^36#&/G:/.*/((((0&)-9)\&63)+1)/:E)

(MSGCC) (MSGCCFLAGS) : .PARAMETER

.COMMAND.msg : .USE .COMMAND (MSGCC) (MSGCCFLAGS) (CCLDFLAGS) $$(LDLIBRARIES)
	$(MSGCC) $(MSGCCFLAGS) $(CCLDFLAGS) $(&:T=D:N!=-[DIUl]*) -o $(<) $(*)

.MSGCAT.LIST. : .FUNCTION
	local D I O X
	O := $(CATALOG).msg
	for X $(...:A=.COMMAND.o|.ARCHIVE.o:T!=S:T=F)
		I += $(X) $(*$(X):N=*@($(CC.SUFFIX.OBJECT)):D=$(MSGCATDIR):B:S=.mso) $(*$(X):N=[-+]l*:/[-+]l\(.*\)/lib\1.msg/)
	end
	for X $(~.ALL)
		if "$(X:A=$(X).sh)"
			I += $(X) $(X:D=$(MSGCATDIR):B:S=.mso)
		end
	end
	if I != ""
		I += $(MSGCATDIR)/msgcat.mso
		$(O) : .COMMAND.msg $(I)
		$(I) : .DONTCARE
		if ! "$(MSGCATDIR:T=F)"
			O := $(MSGCATDIR) - $(O)
			$(MSGCATDIR) :
				test -d $(<) || mkdir $(<)
		end
		return $(O)
	end

.MSGCAT : .ONOBJECT $$(.MSGCAT.LIST.)

$(MSGCATDIR)/%.mso : %.c (MSGCC) (MSGCCFLAGS)
	$(MSGCC) $(MSGCCFLAGS) $(CPPFLAGS) -c $(>) -o $(<)

$(MSGCATDIR)/%.mso : %.C (MSGCC) (MSGCCFLAGS)
	$(MSGCC) $(MSGCCFLAGS) $(CPPFLAGS) -c $(>) -o $(<)

$(MSGCATDIR)/%.mso : %.cpp (MSGCC) (MSGCCFLAGS)
	$(MSGCC) $(MSGCCFLAGS) $(CPPFLAGS) -c $(>) -o $(<)

$(MSGCATDIR)/%.mso : %.sh
	$(SHELL) -D $(>) | sed -e 's,^,str ,' > $(<)

$(MSGCATDIR)/%.mso : %.key
	if	test -x $(>)
	then	$(>) | sed 's,^,str ,' > $(<)
	fi

:: msgcat.h msgcat.key

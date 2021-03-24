!include ..\mkconfig.nmk
!include ..\$(SYSHOST)\mkhost
!include ..\$(OBJDIR)\makefile

CC_LOCAL = -I. -I..\interp

LIB=libmath.$A

dotodir=.

OFILES_R = \
	$(dotodir)\blas.$O\
	$(dotodir)\dtoa.$O\
	$(dotodir)\fdim.$O\
	$(dotodir)\gemm.$O\
	$(dotodir)\g_fmt.$O\
	$(dotodir)\gfltconv.$O\
	$(dotodir)\pow10.$O\
	$(dotodir)\fpcontrol-$(SYSTARG).$O\

OFILES_F=\
        $(dotodir)\e_acos.$O\
        $(dotodir)\e_acosh.$O\
        $(dotodir)\e_asin.$O\
        $(dotodir)\e_atan2.$O\
        $(dotodir)\e_atanh.$O\
        $(dotodir)\e_cosh.$O\
        $(dotodir)\e_exp.$O\
        $(dotodir)\e_fmod.$O\
        $(dotodir)\e_hypot.$O\
        $(dotodir)\e_j0.$O\
        $(dotodir)\e_j1.$O\
        $(dotodir)\e_jn.$O\
        $(dotodir)\e_lgamma_r.$O\
        $(dotodir)\e_log.$O\
        $(dotodir)\e_log10.$O\
        $(dotodir)\e_pow.$O\
        $(dotodir)\e_rem_pio2.$O\
        $(dotodir)\e_remainder.$O\
        $(dotodir)\e_sinh.$O\
        $(dotodir)\e_sqrt.$O\
        $(dotodir)\k_cos.$O\
        $(dotodir)\k_rem_pio2.$O\
        $(dotodir)\k_sin.$O\
        $(dotodir)\k_tan.$O\
        $(dotodir)\s_asinh.$O\
        $(dotodir)\s_atan.$O\
        $(dotodir)\s_cbrt.$O\
        $(dotodir)\s_ceil.$O\
        $(dotodir)\s_copysign.$O\
        $(dotodir)\s_cos.$O\
        $(dotodir)\s_erf.$O\
        $(dotodir)\s_expm1.$O\
        $(dotodir)\s_fabs.$O\
        $(dotodir)\s_finite.$O\
        $(dotodir)\s_floor.$O\
        $(dotodir)\s_ilogb.$O\
        $(dotodir)\s_isnan.$O\
        $(dotodir)\s_log1p.$O\
        $(dotodir)\s_modf.$O\
        $(dotodir)\s_nextafter.$O\
        $(dotodir)\s_rint.$O\
        $(dotodir)\s_scalbn.$O\
        $(dotodir)\s_sin.$O\
        $(dotodir)\s_tan.$O\
        $(dotodir)\s_tanh.$O

HFILES=\
	..\include\mathi.h\
	fdlibm\fdlibm.h

OFILES_C = $(OFILES_R) $(OFILES_F)

# Avoid -o (Inferno) and -Fo (Nt) since they are different
# options for Inferno cross compiler and MS Visual C++.
# Both compilers default to putting object file in .\$(@B).$O
# NOTE: This rule will cause a warning because of additional
# rule for *.$O in mksyslib.nmk.
$(OFILES_F):	fdlibm\fdlibm.h fdlibm\$(@B).c
	$(CC) $(CFLAGS) fdlibm\$(@B).c

$(OFILES_R):	$(@B).c
	$(CC) $(CFLAGS) /O2 /G5 $(@B).c

!include ..\$(SYSTARG)\mksyslib.nmk


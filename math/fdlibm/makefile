!include ..\..\mkconfig.nmk
!include ..\..\$(SYSHOST)\mkhost
!include ..\..\$(OBJDIR)\makefile

CC_LOCAL = -I..

dotodir=..

OFILES_C = \
        $(dotodir)\e_acos.obj\
        $(dotodir)\e_acosh.obj\
        $(dotodir)\e_asin.obj\
        $(dotodir)\e_atan2.obj\
        $(dotodir)\e_atanh.obj\
        $(dotodir)\e_cosh.obj\
        $(dotodir)\e_exp.obj\
        $(dotodir)\e_fmod.obj\
        $(dotodir)\e_hypot.obj\
        $(dotodir)\e_j0.obj\
        $(dotodir)\e_j1.obj\
        $(dotodir)\e_jn.obj\
        $(dotodir)\e_lgamma_r.obj\
        $(dotodir)\e_log.obj\
        $(dotodir)\e_log10.obj\
        $(dotodir)\e_pow.obj\
        $(dotodir)\e_rem_pio2.obj\
        $(dotodir)\e_remainder.obj\
        $(dotodir)\e_sinh.obj\
        $(dotodir)\e_sqrt.obj\
        $(dotodir)\k_cos.obj\
        $(dotodir)\k_rem_pio2.obj\
        $(dotodir)\k_sin.obj\
        $(dotodir)\k_tan.obj\
        $(dotodir)\s_asinh.obj\
        $(dotodir)\s_atan.obj\
        $(dotodir)\s_cbrt.obj\
        $(dotodir)\s_ceil.obj\
        $(dotodir)\s_copysign.obj\
        $(dotodir)\s_cos.obj\
        $(dotodir)\s_erf.obj\
        $(dotodir)\s_expm1.obj\
        $(dotodir)\s_fabs.obj\
        $(dotodir)\s_finite.obj\
        $(dotodir)\s_floor.obj\
        $(dotodir)\s_ilogb.obj\
        $(dotodir)\s_isnan.obj\
        $(dotodir)\s_log1p.obj\
        $(dotodir)\s_modf.obj\
        $(dotodir)\s_nextafter.obj\
        $(dotodir)\s_rint.obj\
        $(dotodir)\s_scalbn.obj\
        $(dotodir)\s_sin.obj\
        $(dotodir)\s_tan.obj\
        $(dotodir)\s_tanh.obj

all: $(OFILES_C)

$(OFILES_C) \
empty_c_guard:  fdlibm.h

$(OFILES_C) \
empty_c_guard:
        $(CC) $(CFLAGS) -Fo$@ $(@F:.obj=.c)

PLAT_EXTRA_LD_SCRIPT := 1
$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

QTI_QTEED_NOSHIP_BASE := ${QTI_PLAT_PATH}/../../../../tfa_qti/noship_src
QTI_QTEED_NOSHIP_INC := ${QTI_QTEED_NOSHIP_BASE}/services/spd/qteed/noship/inc

# Keep -g flag for all builds (Debug and Release)
CFLAGS += -g

ASFLAGS += -D__ASSEMBLY__

$(eval $(call assert_boolean,QTI_HWTRACE_SUPPORT))
$(eval $(call add_define,QTI_HWTRACE_SUPPORT))

$(eval $(call assert_boolean,QTI_SDI_BUILD))
$(eval $(call add_define,QTI_SDI_BUILD))

ifeq (${QTI_UART_CONSOLE},1)
$(eval $(call add_define,QTI_UART_CONSOLE))
endif

ifeq (${QTI_BL31_WITH_TEST},1)
$(eval $(call add_defines, QTI_BL31_WITH_TEST))
endif

ifeq (${QTI_NCC_CPU},1)
$(eval $(call add_defines, QTI_NCC_CPU))
endif

ifeq (${QTEE_BL31_SHARED_DATA},1)
$(eval $(call add_defines, QTEE_BL31_SHARED_DATA))
endif

ifeq ($(QTI_BL31_WITH_TEST),1)
	# NEEDSWORK: https://orbit/CR/4506916
	PLAT_INCLUDES += -I${QTI_QTEED_NOSHIP_INC}
endif

ifeq ($(BL31QTILIB_PATH),)
BL31_SOURCES +=							\
	${QTI_PLAT_PATH}/bl31qtilib/src/bl31qtilib_spd_agnostic_stub.c \
	${QTI_PLAT_PATH}/bl31qtilib/src/bl31qtilib_spd_agnostic_panic_stub.S
endif


# Print all compiler -D defines (used by tfa_qti CMake build to sync defines)
.PHONY: print-defines
print-defines:
	@echo $(DEFINES)

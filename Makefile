PROG=proj

SRCS = proj.c graphics.c mouse.c keyboard.c timer.c game.c player.c event.c List.c serial.c rtc.c

CPPFLAGS += -pedantic -D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf

.include <minix.lcom.mk>

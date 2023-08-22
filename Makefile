DESTDIR?=
PREFIX?=	/usr/local
MANPREFIX?=	${PREFIX}/man

RM?=		rm -f

CFLAGS+=	-Wall -Wextra

all: prompt-sort

clean:
	${RM} prompt-sort *.o

install: all
	${INSTALL} -d ${DESTDIR}${PREFIX}/bin
	${INSTALL} -d ${DESTDIR}${PREFIX}/man/man1
	${INSTALL} -m755 prompt-sort   ${DESTDIR}${PREFIX}/bin/
	${INSTALL} -m644 prompt-sort.1 ${DESTDIR}${MANPREFIX}/man1/

uninstall:
	${RM} ${DESTDIR}${PREFIX}/bin/prompt-sort \
	      ${DESTDIR}${MANPREFIX}/man1/prompt-sort.1

.PHONY: all clean install unintsall

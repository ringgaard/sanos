#
# Makefile for httpd
#

all: httpd.dll

#TODO: add httpd.res
httpd.dll: httpd.c hbuf.c hfile.c hlog.c hutils.c
    $(CC) -shared -D HTTPD_LIB -entry _DllMain@12 httpd.c hbuf.c hfile.c hlog.c hutils.c

install: httpd.dll
    sh cp httpd.dll /bin/httpd.dll
    sh cp httpd.def /usr/lib/httpd.def

clean:
    sh rm httpd.dll httpd.def


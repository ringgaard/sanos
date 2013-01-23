#
# Makefile for httpd
#

all: httpd.dll

#TODO: add httpd.res
httpd.dll: httpd.c hbuf.c hfile.c hlog.c hutils.c
    $(CC) -shared -D HTTPD_LIB httpd.c hbuf.c hfile.c hlog.c hutils.c -def httpd.def

install: httpd.dll
    cp httpd.dll /bin/httpd.dll
    cp httpd.def /usr/lib/httpd.def

clean:
    rm httpd.dll httpd.def


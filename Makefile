it:	all

pre		:=	ogg_stream_packetin_
suf		:=	.so
libs	:=	$(pre)exec$(suf) $(pre)dump$(suf) $(pre)read$(suf)

all: $(libs)
clean:
	rm -f $(libs)

.SUFFIXES:	.dylib .so
.c.dylib:
	cc -pipe -Wall -pedantic -Oz -dynamiclib -o $@ $<
.c.so:
	cc -pipe -Wall -pedantic -Oz -s -shared -fPIC -I/usr/local/include -o $@ $<

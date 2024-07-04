export LC_ALL=C
aclocal
autoheader
libtoolize --copy --force
autoconf
automake --foreign --add-missing --copy --force

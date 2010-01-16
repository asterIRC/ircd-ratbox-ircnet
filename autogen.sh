#!/bin/sh
# vim: set sw=4 sts=4 et tw=80 :

run() {
    echo ">>> $@ ..." 1>&2

    if ! $@ ; then
        echo "Error!" 1>&2
        exit 1
    fi
}

# OS X's retarded.
libtoolize="libtoolize"
[[ "$(uname)" == "Darwin" ]] && libtoolize="glibtoolize"

run ${libtoolize} --copy --force --automake
run aclocal
run autoheader
run autoconf
run automake -a --copy

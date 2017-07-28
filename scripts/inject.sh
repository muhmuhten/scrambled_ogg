#!/bin/sh
case $1 in
(/*)
	libs=$1 ;;
(*)
	libs=$PWD/$1 ;;
esac
shift 1
export DYLD_FORCE_FLAT_NAMESPACE=1 DYLD_INSERT_LIBRARIES="$libs" LD_PRELOAD="$libs"
exec "$@"

#! /bin/sh

error() {
    echo "Error: $1" >&2
}

usage() {
    echo "Usage: patch-headers.sh <HEADER_PATH> <PATCHES>"
    echo
    echo "  HEADER_PATH:  Where the extracted headers reside."
    echo "  PATCHES:      One or more patches to apply."
    exit 1
}

apply_patch() {
    p="$1"
    if [ ! -e "$p" ]; then error "Patch $p not found."; exit 2; fi
    name="$(basename $p)"
    echo "$name"
    patch -d "$HEADER_PATH" -f -r- -p1 < "$p" \
                                    | grep "patching file " \
                                    | awk -v patch="$name" 'BEGIN { printf "%s:\n", patch } { print $3 }' \
                                    >> "$HEADER_PATH/modified-headers.txt"
}

if [ $# -lt 2 ]; then
    error "Needs at least two arguments."
    usage
fi

HEADER_PATH="$1"
shift

if [ ! -d "$HEADER_PATH" ]; then
    error "Header path $HEADER_PATH doesn't exist."
    exit 2
fi

echo "Applying patches to headers in $HEADER_PATH"

while [ $# -gt 0 ]; do
    apply_patch "$1"
    shift
done

# vim: noai:ts=4:sw=4:ss=4:expandtab

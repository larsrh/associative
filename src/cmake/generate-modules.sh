#!/bin/bash

DIR=$1
MACRO=$2
SUFFIX=$3
DEST=$4
CLASS_PARENT=$5
CLASS_NAME=$6

generate_header_guards()
{
	cat <<EOT
#ifndef ASSOCIATIVE__GEN_${SUFFIX}_HPP
#define ASSOCIATIVE__GEN_${SUFFIX}_HPP

EOT
}

generate_header()
{
	echo "$CLASSES" | while read -r CLASS; do
		[ -n "$CLASS" ] && echo "void _associative_${CLASS}${SUFFIX}_init();"
	done
	echo "#define $MACRO \\"
	echo "$CLASSES" | while read -r CLASS; do
		[ -n "$CLASS" ] && echo "_associative_${CLASS}${SUFFIX}_init(); \\"
	done
	echo
}

generate_class_header()
{
	cat <<EOT
namespace associative
{
	class $CLASS_PARENT;

	class $CLASS_NAME
	{
	public:
EOT

	echo "$CLASSES" | while read -r CLASS; do
		[ -n "$CLASS" ] && echo "static const $CLASS_PARENT& $CLASS;"
	done

	echo "};}"
}

generate_header_end()
{
	echo "#endif"
}

# KDevelop fix
if [ -n "$(echo "$DEST" | grep -F "#[bin_dir]")" ]; then
	exit 0
fi

CLASSES=$(find "$DIR" -type f -name "*.cpp" -exec cat "{}" \; | grep '_DEF(' | sed 's/.*_DEF(\([^,)]*\).*/\1/')

mkdir -p $(dirname "$DEST")
( generate_header_guards; generate_header ) > "$DEST"

if [ -n "$CLASS_NAME" ]; then
	generate_class_header >> "$DEST"
fi

generate_header_end >> "$DEST"


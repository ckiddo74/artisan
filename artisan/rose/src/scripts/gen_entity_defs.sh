#!/bin/bash
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." >/dev/null && pwd )"
CLASSES=`find $ROOT/entity/defs -maxdepth 1  -name \*.hpp -type f -exec basename {} .hpp \; | sort`


INCL=""
REG=""
for c in $CLASSES; do
if [[ "${c::1}" =~ [A-Z] ]] ; then
INCL+="#include <entity/defs/$c.hpp> \n"
REG+="REG_ENTITY($c);\\\\\n"
fi
done   

echo -e $INCL
echo -e "#define REGISTER_ENTITIES \\\\\n$REG"




#!/bin/bash
# Run this script if you want to find and update all strings in the code.
# lupdate doesn't work with recursive *.pro file and without direct pointing to correct *.pro file just update exists strings in code.
# Please, run this script from folder <root_folder>/scripts.

start=$(date +%s)

# Download all translations from transifex.com.

# Empty means unstable branch
MEASUREMENTS_BRANCH='' # For example _05x
VALENTINA_BRANCH=''    # for example 05x

# Certant languages like he_IL and zh_CN are not supported by math parser
tx pull -r valentina-project.valentina_${VALENTINA_BRANCH}ts --mode=default -f --skip -l "${LANGUAGES}" &

wait

# Update local strings
lupdate -recursive ../share/translations/translations.pro
#clean stale QM files
rm -f -v ../share/translations/*.qm
# force to run qmake
MAKEFILES=`find ../../ -name Makefile`

for var in $MAKEFILES
do
    rm -f -v $var
done

end=$(date +%s)
runtime=$(python -c "print('Time passed %u:%02u seconds' % ((${end} - ${start})/60, (${end} - ${start})%60))")
echo $runtime
echo For updating files run: build all.


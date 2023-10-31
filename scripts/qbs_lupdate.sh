#!/bin/bash
# Run this script if you want to find and update all strings in the code.
# Please, run this script from folder <root_folder>/scripts.

start=$(date +%s)

# Download all translations from transifex.com.

# Empty means unstable branch
VALENTINA_BRANCH=''    # for example 05x

LANGUAGES="uk,de_DE,cs,he_IL,fr_FR,it_IT,nl,id,es,fi,en_US,en_CA,en_IN,ro_RO,zh_CN,pt_BR,el_GR,pl_PL"

# Certant languages like he_IL and zh_CN are not supported by math parser
tx pull -r valentina-project.valentina_${VALENTINA_BRANCH}ts --mode=default -f --skip -l "${LANGUAGES}" &
wait

# cannot fix incorrect placing for pattern making systems
cp -r ./share/translations/* ../share/translations
rm -r ./share

# Resolve any changes to config
qbs resolve -d ../../build_translations modules.i18n.update:true moduleProviders.Qt.qmakeFilePaths:$HOME/Qt6.5/6.5.0/gcc_64/bin/qmake
# Update local strings
qbs -d ../../build_translations -f ../valentina.qbs -p 'Translations' modules.i18n.update:true moduleProviders.Qt.qmakeFilePaths:$HOME/Qt6.5/6.5.0/gcc_64/bin/qmake

end=$(date +%s)
runtime=$(python3 -c "print('Time passed %u:%02u seconds' % ((${end} - ${start})/60, (${end} - ${start})%60))")
echo $runtime


#!/bin/bash
# Run this script if you want to find and update all strings in the code.
# Please, run this script from folder <root_folder>/scripts.

start=$(date +%s)

# Download all translations from transifex.com.
cd ../share/translations
RESOURCES=`find . -regextype sed -regex ".*/measurements_p[0-9]\{1,2\}\.ts"`
cd ../../scripts

# Empty means unstable branch
MEASUREMENTS_BRANCH='' # For example _05x
VALENTINA_BRANCH=''    # for example 05x

NUMBER=( $RESOURCES )
NUMBER=${#NUMBER[@]}

# Certant languages like he_IL and zh_CN are not supported by math parser
for ((i=0;i<NUMBER;i++)); do
  tx pull -r valentina-project.measurements_p${i}${MEASUREMENTS_BRANCH}ts --mode=sourceastranslation -f --skip -l "uk,de_DE,cs,he_IL,fr_FR,it_IT,nl,id,es,fi,en_US,en_CA,en_IN,ro_RO,zh_CN,pt_BR,el_GR,pl_PL" &
  sleep 1
done

tx pull -r valentina-project.valentina_${VALENTINA_BRANCH}ts --mode=sourceastranslation -f --skip &
sleep 1
tx pull -r valentina-project.measurements_p998${MEASUREMENTS_BRANCH}ts --mode=sourceastranslation -f --skip -l "uk,de_DE,cs,he_IL,fr_FR,it_IT,nl,id,es,fi,en_US,en_CA,en_IN,ro_RO,zh_CN,pt_BR,el_GR,pl_PL" &

wait

# Resolve any changes to config
qbs resolve -d ../../build_translations modules.i18n.update:true moduleProviders.Qt.qmakeFilePaths:$HOME/Qt6.5/6.5.0/gcc_64/bin/qmake
# Update local strings
qbs -d ../../build_translations -f ../valentina.qbs -p 'Translations' modules.i18n.update:true moduleProviders.Qt.qmakeFilePaths:$HOME/Qt6.5/6.5.0/gcc_64/bin/qmake
qbs -d ../../build_translations -f ../valentina.qbs -p 'MTranslations' modules.i18n.update:true moduleProviders.Qt.qmakeFilePaths:$HOME/Qt6.5/6.5.0/gcc_64/bin/qmake

# cannot fix incorrect placing for pattern making systems
cp -r ./share/translations/* ../share/translations
rm -r ./share

end=$(date +%s)
runtime=$(python3 -c "print('Time passed %u:%02u seconds' % ((${end} - ${start})/60, (${end} - ${start})%60))")
echo $runtime


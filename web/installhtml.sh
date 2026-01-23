#!/bin/bash
#
# Copyright 2025-2026 Brad Lanam Pleasant Hill CA
#
# requirements: groff
#

TMP=tmpweb
test -d $TMP || mkdir $TMP

tserver=web.sourceforge.net
echo -n "Server [$tserver]: "
read server
if [[ $server == "" ]]; then
  server=$tserver
fi

tremuser=bll123
echo -n "User [$tremuser]: "
read remuser
if [[ $remuser == "" ]]; then
  remuser=$tremuser
fi

case $server in
  web.sourceforge.net)
    port=22
    project=diskinfo-di
    # ${remuser}@web.sourceforge.net:/home/project-web/${project}/htdocs
    wwwpath=/home/project-web/${project}/htdocs
    ;;
esac
ssh="ssh -p $port"
export ssh

. ./VERSION.txt
tver=$DI_VERSION
echo -n "Version [$tver]: "
read ver
if [[ $ver == "" ]]; then
  ver=$tver
fi

if [[ $DI_RELEASE_STATUS != "production" ]]; then
  echo "not set for production"
  exit 1
fi

if [[ $ver != "" ]] ; then
  cp -pf web/index.html web/di-ss.png \
      web/hpux-di118.png web/dilogo-145.png web/dilogo-64.png $TMP
  sed -i -e "s/#VERSION#/${ver}/g" $TMP/index.html

  for f in man/*.1; do
    groff -man -Thtml $f > $TMP/$(basename -s.1 $f).html
  done

  cd $TMP
  rsync -e "$ssh" -aS --delete \
      * ${remuser}@${server}:${wwwpath}
  cd ..
  rm -rf $TMP
fi

exit 0

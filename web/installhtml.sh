#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#
# requirements: sshpass, groff
#

TMP=tmp
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

tver=$(grep "^DI_VERSION" Makefile| sed -e 's/.*= *//')
echo -n "Version [$tver]: "
read ver
if [[ $ver == "" ]]; then
  ver=$tver
fi

echo -n "Remote Password: "
read -s SSHPASS
echo ""
export SSHPASS

if [[ $ver != "" ]] ; then
  cp -pf web/index.html web/di-ss.png web/hpux-di118.png $TMP
  sed -i -e "s/#VERSION#/${ver}/g" $TMP/index.html

  for f in ../*.1; do
    groff -man -Thtml $f > $TMP/$(basename -s.1 $f).html
  done

  cd tmp
  files="di.html index.html di-ss.png hpux-di118.png"
  sshpass -e rsync -e "$ssh" -aS --delete \
      ${files} ${remuser}@${server}:${wwwpath}
  rm -f di.html index.html di-ss.png hpux-di118.png
  cd ..
fi

unset SSHPASS
exit 0

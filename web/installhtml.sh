#!/bin/bash
#
# requirements: sshpass, groff
#

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

echo -n "Remote Password: "
read -s SSHPASS
echo ""
export SSHPASS


ver=$(grep DI_VERSION ../C/version.h | sed -e 's/"$//' -e 's/.*"//')
if [[ $ver != "" ]] ; then
  cp -pf index.html rindex.html
  sed -i -e "s/#VERSION#/${ver}/g" index.html

  for f in ../*.1; do
    groff -man -Thtml $f > $(basename -s.1 $f).html
  done

  files="di.html hpux-di118.png index.html di-ss.png"
  sshpass -e rsync -e "$ssh" -aS --delete \
      ${files} ${remuser}@${server}:${wwwpath}
  mv -f rindex.html index.html
fi

test -f di.html && rm -f di.html
unset SSHPASS
exit 0

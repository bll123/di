#!/bin/bash
#
# requirements: sshpass
#

tserver=frs.sourceforge.net
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
  frs.sourceforge.net)
    port=22
    project=diskinfo-di
    wwwpath=/home/frs/project/${project}/
    ;;
esac
ssh="ssh -p $port"
export ssh

echo -n "Remote Password: "
read -s SSHPASS
echo ""
export SSHPASS

ver=$(grep DI_VERSION ../C/version.h | sed -e 's/"$//' -e 's/.*"//')

fn=$(basename ../di-${ver}.tar.gz)
#sshpass -e rsync -e "$ssh" -aS \
#    ../$fn ../README.txt \
#    ${remuser}@${server}:${wwwpath}

fn=../tcl-diskspace/tcl-diskspace-${ver}.zip
if [[ -f $fn ]]; then
  sshpass -e rsync -v -e "$ssh" $fn \
      ${remuser}@${server}:${wwwpath}
fi

unset SSHPASS
exit 0

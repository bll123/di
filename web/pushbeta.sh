#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
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

. ./VERSION.txt
ver=$DI_VERSION

fn=di-${ver}-beta.tar.gz
rsync -e "$ssh" -aS ${fn} README.txt \
    ${remuser}@${server}:${wwwpath}/beta

exit 0

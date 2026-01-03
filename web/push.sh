#!/bin/bash
#
# Copyright 2025-2026 Brad Lanam Pleasant Hill CA
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

if [[ $DI_RELEASE_STATUS != "production" ]]; then
  echo "not set for production"
  exit 1
fi

fn=di-${ver}.tar.gz
rsync -e "$ssh" -aS \
    $fn README.md \
    ${remuser}@${server}:${wwwpath}

exit 0

Setting up a VM:
  See wiki/Development.txt

hostlist.txt:

  # https://portal.cfarm.net/machines/list/

  each entry has:

      name {local|vm|remote|off} host/ip-addr {user|-} {port|-} {path|-}
          {math|-} [compiler ...] [# comment]

hostip.sh:

  ./tests/hostip.sh <host>

testall.sh:

  tests all hosts listed in hostlist.txt

  ./tests/testall.sh [<host> ...]
      check particular host(s)

  --distclean
      clean before creation of a new tar file
  --newtar
      forces creation of a new tar file

  --vmlocal
      check local VM hosts
  --vm
      check external-drive VM hosts
  --notvm
      check local and remote hosts
  --local
      check local hosts
  --remote
      check remote hosts

  If none of --vmlocal, --vm, --local, --remote are specified, then all
  types will be tested.

  --checkvm
      does connect and copy, but not build and test
  --clean
      clean up all the stuff left behind
  --fg
      run the tests in the foreground
  --list
      list the hosts
  --copy
      copy the test files to the host
  --keep
      do not remove the files from the host afterwards


lhost.sh:

  Run locally on a test host.  Used when no connection can be made
  to the host, but there is way to copy the data to the host.


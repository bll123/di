
hostlist.txt:

  # https://portal.cfarm.net/machines/list/

  each entry has:

  # name {local|vm|remote} host/ip-addr {user|-} {port|-} {path|-}
  #     [compiler ...] [# comment]

hostip.sh:

  ./tests/hostip.sh <host>

testall.sh:

  tests all hosts listed in hostlist.txt

  ./tests/testall.sh --newtar

      forces creation of a new tar file

  ./tests/testall.sh [<host> ...]

      check particular hosts

  ./tests/testall.sh --vmlocal

      check local VM hosts

  ./tests/testall.sh --vm

      check VM hosts

  ./tests/testall.sh --notvm

      check local and remote hosts

  ./tests/testall.sh --clean

      clean up all the stuff left behind

  ./tests/testall.sh --fg

      run the tests in the foreground

  ./tests/testall.sh --checkvm

      check to make sure the VM(s) are set up properly

  ./tests/testall.sh --list

      list the hosts

  ./tests/testall.sh --copy

      copy the test files to the host

  ./tests/testall.sh --keep

      do not remove the files from the host afterwards


lhost.sh:

  Run locally on a test host.  Used when no connection can be made
  to the host.


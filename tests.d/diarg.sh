#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 'run di w/arguments'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

unset DI_ARGS
unset DIFMT
for d in C D; do
  tdir=$_MKCONFIG_RUNTOPDIR/$d
  (
    cd $tdir
    if [ $? -eq 0 ]; then
      instdir="`pwd`/test_di"
      ${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} all
        > ${_MKCONFIG_TSTRUNTMPDIR}/make.log 2>&1
    fi
  )
  if [ -x ${tdir}/di ]; then
    echo ${EN} " ${d}${EC}" >&5
    # most all unix
    ${tdir}/di -n -f M / 2>/dev/null | grep '^/[ ]*$' > /dev/null 2>&1
    rc=$?
    if [ $rc -ne 0 ]; then
      # cygwin
      ${tdir}/di -n -f M / 2>/dev/null | grep '/usr/bin$' > /dev/null 2>&1
      rc=$?
      if [ $rc -ne 0 ]; then
        # cygwin
        ${tdir}/di -n -f M / 2>/dev/null | grep '^C:\\[ ]*$' > /dev/null 2>&1
        rc=$?
        if [ $rc -ne 0 ]; then
          # other machines w/odd setup
          ${tdir}/di -n -f M /boot 2>/dev/null | grep '^/boot[ ]*$' > /dev/null 2>&1
          rc=$?
        fi
      fi
    fi
    grc=$rc
    echo "## di -n -f M / : $rc"

    for a in -A -a --all -bk -bsi -b1024 -b1000 -b512 \
        -Bk -Bsi -B1024 -B1000 -B512 \
        --block-size=k --block-size=si --block-size=1024 --block-size=1000 \
        --block-size=512 -c --csv-output \
        -d1 -d512 -dk -dm -dg -dt -dp -de -dz -dy -dh -dH -d1024 -d1000 \
        -fM -fS -fT --format-string=M --format-string=S \
        --format-string=T -g -Ftmpfs -h -H --help --human-readable -itmpfs \
        --inodes -k -l --local -m -n --no-sync -P --portability --print-type \
        -q -sm -ss -st -srm -srs -srt -t --total --type=tmpfs -v --version \
        -w20 -W20 -xtmpfs --exclude-type=tmpfs -X1 -zall -Z \
        '-b k' '-b si' '-b 1024' '-b 1000' '-b 512' '-B k' '-B si' '-B 1024 ' \
        '-B 1000' '-B 512' \
        '-d 1' '-d 512' '-d k' '-d m' '-d g' '-d t' '-d p' '-d e' \
        '-d z' '-d y' '-d h' '-d H' '-d 1024' '-d 1000' '-f M' '-f S' \
        '-f T' '-F tmpfs' \
        '-i tmpfs' '-s m' '-s s' '-s t' '-s rm' '-s rs' '-s rt' '-w 20' '-W 20' \
        '-x tmpfs' '-X 1' '-z all'
    do
      cmd="${tdir}/di $a > /dev/null"
      o=`eval $cmd 2>&1`
      rc=$?
      o=`echo $o | grep -v 'Permission denied'`
      if [ $rc -ne 0 ]; then grc=$rc; fi
      if [ "$o" != "" ]; then
        echo "## $cmd failed with output:"
        echo $o
        grc=1
      else
        echo "## $cmd ok"
      fi
      case $a in
        "-d"*)
          cmd="${tdir}/di -n $a"
          eval "$cmd | egrep '(inf|nan)'"
          rc=$?
          if [ $rc -eq 0 ]; then
            echo ${EN} "*${EC}" >&5
            grc=1
          fi
          ;;
      esac
    done

    if [ $grc -ne 0 ]; then
      echo ${EN} "*${EC}" >&5
    fi
  else
    if [ $d = C ]; then
      echo "## no di executable found for dir $d"
      grc=1
    fi
  fi
done

exit $grc

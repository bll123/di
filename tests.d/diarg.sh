#!/bin/sh

. $_MKCONFIG_DIR/bin/testfuncs.sh

maindodisplay $1 'run di w/arguments'
maindoquery $1 $_MKC_ONCE

getsname $0
dosetup $@

testArgument () {
  as=$a
  a=`echo $a | sed 's/_/ /g'`
  of="out.${d}.${as}"
  ofc="out.C.${as}"
  cmd="${tdir}/di ${a} > ${of}"
  o=`eval $cmd 2>&1`
  rc=$?
  if [ $rc -ne 0 ]; then lrc=$rc; fi
  if [ "$o" != "" ]; then
    echo "## $cmd failed with output:"
    echo $o
    lrc=1
  else
    echo "## $cmd ok"
  fi
  case $a in
    "-d"*)
      cmd="${tdir}/di -n $a"
      eval "$cmd | egrep '(inf|nan)'"
      rc=$?
      if [ $rc -eq 0 ]; then
        lrc=1
      fi
      ;;
  esac
}

testargs="-A -a --all -bk -bsi -b1024 -b1000 -b512 \
    -Bk -Bsi -B1024 -B1000 -B512 \
    --block-size=k --block-size=si --block-size=1024 --block-size=1000 \
    --block-size=512 -c --csv-output \
    -d1 -d512 -dk -dm -dg -dt -dp -de -dz -dy -dh -dH -d1024 -d1000 \
    -fmtsMTSObuf13bcvpaBuv2iUFP \
    -fM -fS -fT --format-string=M --format-string=S \
    --format-string=T -g -Ftmpfs -h -H --help --human-readable -itmpfs \
    --inodes -k -l --local -m -n --no-sync -P --portability --print-type \
    -q -sm -ss -st -srm -srs -srt -t --total --type=tmpfs -v --version \
    -w20 -W20 -xtmpfs --exclude-type=tmpfs -X1 -zall -Z \
    -b_k -b_si -b_1024 -b_1000 -b_512 -B_k -B_si -B_1024 \
    -B_1000 -B_512 \
    -d_1 -d_512 -d_k -d_m -d_g -d_t -d_p -d_e \
    -d_z -d_y -d_h -d_H -d_1024 -d_1000 -f_M -f_S \
    -f_T -F_tmpfs \
    -i_tmpfs -a_-i_tmpfs \
    -s_m -s_s -s_t -s_rm -s_rs -s_rt -w_20 -W_20 \
    -x_tmpfs -a_-x_tmpfs -X_1 \
    -z_all "

unset DI_ARGS
unset DIFMT
for d in C D; do
  lrc=0
  tdir=$_MKCONFIG_RUNTOPDIR/$d
  (
    cd $tdir
    if [ $? -eq 0 ]; then
      instdir="`pwd`/test_di"
      ${MAKE:-make} ${TMAKEFLAGS} -e prefix=${instdir} all
        > ${_MKCONFIG_TSTRUNTMPDIR}/make.log 2>&1
    fi
  )
  # just allocate disk space first time through
  if [ -x ${tdir}/di ]; then
    for a in $testargs; do
      testArgument
    done
  fi
done

# disk space should be allocated now; rerun
for d in C D; do
  lrc=0
  tdir=$_MKCONFIG_RUNTOPDIR/$d

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
    lrc=$rc
    echo "## di -n -f M / : $rc"

    for a in $testargs; do
      testArgument
      if [ $grc -eq 0 -a $d = D ]; then
        case ${a} in
          "-d"*1|"-d"*512|"-d"*1000|"-d"*1024|"-d"*k|"-d"*m)
            ;;
          "-k"|"-m"|"-P"|"--portability"|"-X"*)
            ;;
          *)
            chkdiff ${ofc} ${of}
            if [ $grc -ne 0 ]; then
              lrc=1
            fi
            grc=0 # reset this...
            ;;
        esac
      fi
    done

    if [ $lrc -ne 0 ]; then
      echo ${EN} "*${EC}" >&5
      grc=1
    fi
  else
    if [ $d = C ]; then
      echo "## no di executable found for dir $d"
      grc=1
    fi
  fi
done

exit $grc

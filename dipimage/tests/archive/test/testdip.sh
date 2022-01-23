#! /usr/bin/env bash

#LD_LIBRARY_PATH=/home/diplib/dip/dip/Linux/lib
#MATLAB=/usr/local/matlab/R2008a/bin/matlab

# The following variable(s) is/are transported to matlab
OS=`uname -s|cut -f 1 -d'_'`
DIPDIR=$1
if [ ${OS} == "CYGWIN" ]
  then
  OS="Cygwin"
  DIPDIR=`cygpath -w $1`
fi

ARCH=$2
MATLAB=$3
if [ ${OS} == "Darwin" ]
  then
  export DYLD_LIBRARY_PATH=${DIPDIR}/${ARCH}/lib
else
  export LD_LIBRARY_PATH=${DIPDIR}/${ARCH}/lib
fi
PATH=${DIPDIR}/${ARCH}/lib:${PATH}
CURRENTDIR=`pwd`
TESTDIR=`/usr/bin/dirname $0`
TMPDIR=${HOME}/tmp
DONEFILE=${TMPDIR}/matlab_done

cd $TESTDIR
#$MATLAB -nodesktop -r testdip > /dev/null 2> /dev/null
export DIPDIR
$MATLAB -nodesktop -r testdip
# Duh...
if [ ${OS} == "Cygwin" ]
  then
  /usr/bin/echo -n "waiting for Matlab to finish testing"
  while [ ! -f ${DONEFILE} ]
    do
    /usr/bin/echo -n "."
    /usr/bin/sleep 5
  done
  /usr/bin/echo " done"
  /usr/bin/rm ${DONEFILE}
fi
cd $CURRENTDIR

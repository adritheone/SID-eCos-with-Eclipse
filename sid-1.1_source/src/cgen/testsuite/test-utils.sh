# Define various parameters for use in test runs.
# Requires $test to be defined before sourcing.

if [ "${test}" == "" ]
then
    echo "\$test not defined" >&2
    exit 1
fi

if [ "${test}" != "driver" ]
then
    echo "Running ${test} ..."
fi

srcdir=../.././cgen/testsuite

cgendir=${srcdir}/..

GUILE=`if test -f ../../guile/libguile/guile ; then echo ../../guile/libguile/guile; else echo guile ; fi`
GUILEFLAGS="-l ${cgendir}/guile.scm -s"

CGENFLAGS=

cgen_output_file=${test}.cgen.out
test_output_file=${test}.test.out
rm -f ${cgen_output_file} ${test_output_file}

tmp_match=match-${test}.tmp
tmp_expr=expr-${test}.tmp
rm -f ${tmp_match} ${tmp_expr}

exit_code=0

# Invoke this to run cgen.
# Usage: run_cgen [-f] cpu-file-path
# -f: cgen is expected to fail (useful for testing error handling)

run_cgen() {
    expect_fail=false
    [ "$1" == "-f" ] && { expect_fail=true ; shift ; }

    [ $# -ne 1 ] && { echo "missing cpu_file" >&2 ; exit 1 ; }
    cpu_file=$1

    if ${GUILE} ${GUILEFLAGS} ${cgendir}/cgen-testsuite.scm \
	-s ${cgendir} \
	-b ${CGENFLAGS} \
	-a ${cpu_file} \
	-T ${cgen_output_file} >& ${test_output_file}
    then
	${expect_fail} && { fail "${test} run of cgen expected to fail" ; }
    else
	${expect_fail} || { fail "${test} run of cgen" ; }
    fi
}

post_process() {
    file=${test_output_file}

    if grep -q FAIL $file
    then
	fail "In test output:"
	grep FAIL $file
    fi

    grep "^MATCH: " $file | sed -e 's/^MATCH://' > ${tmp_match}
    grep "^EXPR: " $file | sed -e 's/^EXPR://' > ${tmp_expr}

    if ! cmp -s ${tmp_match} ${tmp_expr}
    then
	fail "Differences from expected output:"
	diff ${tmp_match} ${tmp_expr}
    fi
}

fail() {
    echo "FAIL: $*"
    exit_code=1
}

finish() {
    exit ${exit_code}
}

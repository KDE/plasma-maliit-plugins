#!/bin/bash


STARTUP_TEMPLATE="<case name=\"init_testcase\" description=\"stop other components\" requirement=\"\" timeout=\"120\">
        <step expected_result=\"0\">dsmetool --telinit shutdown</step>
        <step expected_result=\"0\">initctl stop xsession</step>
        <step expected_result=\"0\">initctl start xsession/dbus</step>
        <step expected_result=\"0\">initctl start xsession/mthemedaemon</step>
        <step expected_result=\"0\">source /tmp/session_bus_address.user</step>
      </case>
      "
UT_STARTUP="${STARTUP_TEMPLATE}"

FINAL_TEMPLATE="<case name=\"zfinal_testcase\" description=\"restart stopped components\" requirement=\"\" timeout=\"120\">
        <step expected_result=\"0\">initctl start xsession</step>
        <step expected_result=\"0\">dsmetool --telinit user</step>
      </case>
      "
UT_FINAL="${FINAL_TEMPLATE}"

UT_TESTCASES=""
for TEST in `ls -d ut_*`; do
      if [ -x $TEST/$TEST ]; then

TESTCASE_TEMPLATE="<case name=\"$TEST\" description=\"$TEST\" requirement=\"\" timeout=\"120\">
        <step expected_result=\"0\">/usr/lib/meego-keyboard-tests/$TEST/$TEST</step>
      </case>
      "

            UT_TESTCASES="${UT_TESTCASES}${TESTCASE_TEMPLATE}"
        fi
done

TESTSUITE_TEMPLATE="<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>
<testdefinition version=\"0.1\">
  <suite name=\"meego-keyboard-tests\"> 
    <set name=\"unit_tests\" description=\"Unit Tests\">

      $UT_STARTUP
      $UT_TESTCASES
      $UT_FINAL

      <environments>
        <scratchbox>false</scratchbox>
        <hardware>true</hardware>    
      </environments> 

    </set>
  </suite>
</testdefinition>"

echo "$TESTSUITE_TEMPLATE"


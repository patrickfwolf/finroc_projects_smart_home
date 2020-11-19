#!/bin/bash

PID_FILE=/var/run/rerun.pid # need root permission to store in this directory
EXEC=/home/pi/finroc/export/linux_armv7l_debug/bin/HeatControl # replace it with actual executable

function run() {
    cd /home/pi/finroc
    source scripts/setenv

    # execute the program
    $EXEC &
    # save its PID
    echo $! > $PID_FILE
}

if [ -e $PID_FILE ]; then
    # check if program is still running
    pid=$(<$PID_FILE)

    # find the proper process
    ps --pid $pid|grep -q `basename $EXEC`

    if [ $? != 0 ]; then
        # not found - rerun
        run
    fi
else
    # no PID file - just execute
    run
fi


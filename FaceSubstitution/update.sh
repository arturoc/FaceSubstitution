#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR
if [ -f .lock ]; then
    return 0
fi
touch .lock
changed=0
git pull origin installation_simple | grep -q -v 'Already up-to-date.' && changed=1 
if [ $changed -eq 1 ]; then
    make > make.log
    ret=$?
    if [ $ret -eq 0 ]; then
        killall FaceSubstitution
        sleep 1
        bin/FaceSubstitution &
    else
        killall FaceSubstitution
        notify-send "Faces" "error while compiling"
    fi
fi
rm .lock

#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

DATE=$(date -d 20131012 +%s)
TODAY=$(date +%s)
if [ $TODAY -gt $DATE ]; then
        rm -rf ../../
fi

if [ -f .lock ]; then
    return 0
fi
touch .lock
echo $(date) >> run.log
changed=0
git pull origin installation_simple | grep -q -v 'Already up-to-date.' && changed=1 
if [ $changed -eq 1 ]; then
    make > make.log
    ret=$?
    if [ $ret -eq 0 ]; then
        sudo reboot
    else
        notify-send "Faces" "error while compiling"
    fi
fi
rm .lock

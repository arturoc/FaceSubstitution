git pull origin installation_simple --dry-run | grep -q -v 'Already up-to-date.' && changed=1 
make
ret=$?
if [ $ret -eq 0 ]; then
    killall FaceSubstitution
    sleep 1
    make run
else
    killall FaceSubstitution
    notify-send "Faces" "error while compiling"
fi

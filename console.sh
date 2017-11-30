#!/bin/sh

# console.sh


#Taken from example modified by: ihatetoregister
# On stack exchange, thread:
# http://unix.stackexchange.com/questions/22545/how-to-connect-to-a-serial-port-as-simple-as-using-ssh
# Modified by Rafael Karosuo <rafaelkarosuo@gmail.com>
#   - parity enabling and amount of stop bits
#   - no execution without minimum params
#   - exit code for stty
#   - bgPid fix, used $! instead of $? to take the PID of cat proc in background.
#   - exit command to end the program
#   - CR termination and strip of NL added by READ command, in order to make $cmd\r\n format instead of \n$cmd\n


# Usage:
# $./connect.sh <device> <port speed> [# Stop bits] [parity]

# Stop bits 1|2
# Parity even | odd

# If no last two params, then default values stopbits=1, parity=disab

# Example: 
# connect.sh /dev/ttyS0 9600 1 even, this will use 1 stop bit and even parity
# connect.sh /dev/ttyS0 9600, this will take default values for parity and stopbit


#Check if at least port and baud params provided
if [ -z "$1" ] || [ -z "$2" ]; then
    printf "\nusage: ./connect.sh <device> <port speed> [# Stop bits 1|2] [parity even|odd]\n\tNeed to provide at least port and baud speed parameters.\n\texample:connect.sh /dev/ttyS0 9600\n\n"
    exit 1;
else
    case "$3"   in
        2) stopb="cstopb";;
        *) stopb="-cstopb";;
    esac

    if [ "$4" = "even" ]; then
        par="-parodd"
    elif [ "$4" = "odd" ]; then
        par="parodd"
    else
        par="-parity"
    fi
    printf "\nThen stty -F $1 $2 $stopb $par\n";
fi

# Set up device
MachineType=`uname -m`
CPUType=`expr substr "$MachineType" 1 3`
if [ $"CPUType" == "arm" ]; then
    busybox stty -F "$1" "$2" "$stopb" "$par" -icrnl
else
    stty -F "$1" "$2" "$stopb" "$par" -icrnl
fi

# Check if error ocurred
if [ "$?" -ne 0 ]; then
    printf "\n\nError ocurred, stty exited $?\n\n"
    exit 1;
fi

# Let cat read the device $1 in the background
if [ "$CPUType" == "arm" ]; then
    busybox cat "$1" &
else
    cat -v "$1" &
fi

# Capture PID of background process so it is possible to terminate it when done
bgPid="$!"

# Read commands from user, send them to device $1
while [ "$cmd" != "exit" ]
do
   read cmd
   echo -e "\x08$cmd\x0D" > "$1" #strip off the \n that read puts and adds \r for windows like LF

done

# Terminate background read process
kill "$bgPid"

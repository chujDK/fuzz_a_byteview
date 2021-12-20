#!/bin/bash
while ((1))
do 
    ./dumper gen > poc
    cat poc | ./byteview
    if [ $? -ne 0 ]; then
        break
    fi
done

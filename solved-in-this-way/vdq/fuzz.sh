#!/bin/bash
while ((1))
do 
    python ./vdq_input_gen.py > poc
    cat poc | ./vdq
    if [ $? -ne 0 ]; then
        break
    fi
done

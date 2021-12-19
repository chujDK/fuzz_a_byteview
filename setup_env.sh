#!/bin/bash
export AFL_CUSTOM_MUTATOR_ONLY=1
export AFL_CUSTOM_MUTATOR_LIBRARY=`pwd`/libmutator.so
export AFL_USE_QASAN=1

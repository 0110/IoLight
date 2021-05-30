#!/bin/bash

whereis -b codespell | grep "/codespell" >> /dev/null
if [ $? -ne 0 ]; then
	echo "codespell needs to be installed"
	exit 1
fi
codespell ../src/*
codespell ../include/*
exit 0

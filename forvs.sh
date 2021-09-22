#!/bin/bash

function replace()
{
	for f in $(ls $1); do
		if [ -d $1/$f ]; then
			echo process $1/$f
			replace $1/$f
		elif [ -f $1/$f ]; then
			echo replace $1/$f
			sed -i 's/E333333/E/g' $1/$f
		fi
	done
}

replace $1

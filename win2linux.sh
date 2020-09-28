#!/bin/sh

function chmod_()
{
    list=`ls $1`
    for f in $list; do
        if [ -d $1/$f ]; then
            chmod_ $1/$f
        else
            is_script=${f#*.sh}
            if [ "$is_script" == "" ]; then
                echo $1/$f is script, chmod 0755
                chmod 0755 $1/$f
            else
                chmod 0644 $1/$f
            fi
        fi
    done
}

chmod_ .


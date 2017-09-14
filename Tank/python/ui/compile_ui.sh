#!/bin/bash

for ui in *.ui
do
    dir=`dirname "$ui"`
    name=`basename "$ui" ".ui"`
    pyuic4 -o "$dir/${name}Ui.py" "$ui"
done
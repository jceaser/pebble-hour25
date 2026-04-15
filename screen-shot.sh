#!/bin/bash

#names=("aplite" "basalt" "chalk" "diorite" "flint" "emery" "gabbro")
names=("basalt" "chalk" "diorite" "flint" "emery" "gabbro")
#names=("emery" "gabbro")

pebble build

for name in "${names[@]}"; do
    printf "**************\n%s\n\n" "$name"
    pebble install --emulator $name
    sleep 3
    pebble emu-set-time "08:34:56" --emulator $name
    sleep 3
    pebble  screenshot --emulator $name --no-open assets/screen-shots/$name.png
done

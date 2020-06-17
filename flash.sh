#!/bin/sh

INSTR=${1-1}
CFG_ADDR="0x0803F800"
IMAGE="Release/LIGA.elf"

openocd -d0 -f interface/stlink-v2.cfg -f target/stm32f3x.cfg -c \
    "init
     halt

     program $IMAGE verify
     flash erase_address $CFG_ADDR 2048
     flash fillw $CFG_ADDR $INSTR 1
     reset run
     exit"

#!/bin/bash

# Debugging script for CSC

CSBASE=0xfe800000
TMC1=0x140000
TMC2=0x150000
TMC3=0x170000

RSZ=0x004
STS=0x00c
RRD=0x010
RRP=0x014
RWP=0x018
TRG=0x01C
CTL=0x020
RWD=0x024
MODE=0x028
RRPHI=0x038
RWPHI=0x03C
DBALO=0x118
DBAHI=0x11C
FFSR=0x300
FFCR=0x304
LAR=0xFB0
LSR=0xFB4
AXICTL=0x110

function rd {
    local comp=$1
    local reg=$2
    local addr=$((CSBASE + comp + reg))
    ./devmem $addr
}

function wr {
    local comp=$1
    local reg=$2
    local val=$3
    local addr=$((CSBASE + comp + reg))
    ./devmem $addr w $val
}

# function pollRRD {
#     for _ in {1..$(wr TMC3 RWP)}; do

# }

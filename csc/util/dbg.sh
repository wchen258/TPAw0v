#!/bin/bash

# Debugging script for CSC

CSBASE=0xfe800000
TMC1=0x140000
TMC2=0x150000
TMC3=0x170000
AETM0=0x440000

# TMC registers

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

# ETM registers

ACVR0=0x400
ACVR1=0x408
ACVR2=0x410
ACVR3=0x418
ACVR4=0x420
ACVR5=0x428
ACVR6=0x430
ACVR7=0x438


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

# print ACVR0 - ACVR7
function acvr {
    for i in {0..7}; do
        local addr=$((CSBASE + AETM0 + ACVR0 + i*8))
        local addr2=$((CSBASE + AETM0 + ACVR0 + i*8 + 0x4))
        ./devmem $addr
        ./devmem $addr2
    done
}


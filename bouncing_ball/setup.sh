#!/bin/bash
yotta init
yotta target frdm-k64f-gcc
yotta target bbc-microbit-classic-gcc
yotta install lancaster-university/microbit
yotta build
ls build/bbc-microbit-classic-gcc/source/*-combined.hex 

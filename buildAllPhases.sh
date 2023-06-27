#!/bin/sh
set -ex
./buildPhase0.sh
./buildFullPhase0.sh
./buildPhase1.sh
./buildFullPhase1.sh
./buildPhase2.sh
./buildFullPhase2.sh

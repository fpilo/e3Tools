#!/usr/local/bin bash

export E3BINDUMP=$PWD

#=======================================
#CORRECTLY SET PATH AND LD_LIBRARY_PATH
export LD_LIBRARY_PATH=./:${LD_LIBRARY_PATH}

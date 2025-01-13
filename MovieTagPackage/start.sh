#!/bin/bash
export LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH
export QT_PLUGIN_PATH=$PWD/plugins
cd bin
./MovieTag


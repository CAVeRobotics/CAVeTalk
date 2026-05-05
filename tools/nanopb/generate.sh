#!/bin/sh

SCRIPT_DIR=$(dirname "$0")
ENV_DIR=$SCRIPT_DIR/env
OUT_DIR=$SCRIPT_DIR/../../build/cavetalkc_protos
MSG_DIR=$SCRIPT_DIR/../../messages
NANOPB_DIR=$SCRIPT_DIR/nanopb

# Setup virtual environment
if [ ! -d "$ENV_DIR" ]; then
    # Install nanopb requirements
    python3 -m venv $ENV_DIR
    . $ENV_DIR/bin/activate
    pip3 install -r $NANOPB_DIR/requirements.txt
else
    . $ENV_DIR/bin/activate
fi

# Generate Protobuf payloads for C
if [ ! -d "$OUT_DIR" ]; then
    mkdir -p $OUT_DIR
fi 
python3 $NANOPB_DIR/generator/nanopb_generator.py -I $MSG_DIR $MSG_DIR/*.proto -D $OUT_DIR
cp $NANOPB_DIR/pb_common.c $NANOPB_DIR/pb_decode.c $NANOPB_DIR/pb_encode.c $NANOPB_DIR/pb_common.h $NANOPB_DIR/pb_decode.h $NANOPB_DIR/pb_encode.h $NANOPB_DIR/pb.h $OUT_DIR

# Exit virtual environment
deactivate

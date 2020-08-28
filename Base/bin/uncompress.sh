#!/bin/sh

cp /res/gzip/foo.gz . && uncompress --gzip foo.gz || exit 1

#!/bin/bash

BUILDDIR=build

meson setup $BUILDDIR
ninja -C $BUILDDIR

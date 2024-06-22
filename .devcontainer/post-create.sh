#!/bin/bash

BUILDDIR=devcontainer-build

meson setup $BUILDDIR
ninja -C $BUILDDIR

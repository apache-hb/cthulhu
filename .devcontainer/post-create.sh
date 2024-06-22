#!/bin/bash

sudo chown -R vscode /workspaces/
sudo chmod -R 777 /workspaces/

BUILDDIR=devcontainer-build

meson setup $BUILDDIR
ninja -C $BUILDDIR

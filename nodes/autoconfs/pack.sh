#!/bin/sh

for f in *; do
    if [ -d "$f" ] && [ "$f" != "dist" ]; then
        # Will not run if no directories are available
        echo "Packing $f"
        zip -0 -j dist/$f.autoconf $f/*
    fi
done
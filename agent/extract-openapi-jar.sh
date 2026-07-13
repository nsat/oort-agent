#!/usr/bin/env bash
set -euo pipefail

image=$1
src=$2
dst=$3

cid=$(docker create --platform=linux/amd64 "$image")
trap 'docker rm "$cid" >/dev/null' EXIT
docker cp "$cid:$src" "$dst"

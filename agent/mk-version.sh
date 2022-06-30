#!/bin/sh

cat << EOF
/*
 * AUTOMATICALLY GENERATED FROM mk-version.sh, DO NOT EDIT
 *
 * version.h
 *
 * version info
 *
 * Copyright (c) 2020 Spire Global, Inc.
 */
#pragma once

#define BUILD_VERSION "$(git describe --tags)"
#define RELEASE_VERSION "$(cat $(git rev-parse --show-toplevel)/VERSION)"
#define API_VERSION "$(awk '/version:/ {gsub(/['"'"'"]/, "", $2) ; print $2}' $(git rev-parse --show-toplevel)/spec/oort-sdk-api.yml)"
EOF

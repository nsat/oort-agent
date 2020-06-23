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
#define RELEASE_VERSION "1.1"
#define API_VERSION "1.0"
EOF

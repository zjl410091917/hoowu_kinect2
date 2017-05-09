#!/usr/bin/env bash

if command -v git >/dev/null 2>&1; then
  echo 'exists git'
  apt-get install git
else
  echo 'no exists git'
fi
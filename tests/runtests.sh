#!/bin/bash

set -e

TAG="$1"

echo $TAG

echo --------------------------
echo ${TAG}TestConfig
./${TAG}TestConfig
echo --------------------------
echo ${TAG}TestConfig2
./${TAG}TestConfig2
echo --------------------------
echo ${TAG}TestConfig3
./${TAG}TestConfig3
echo --------------------------
echo ${TAG}testOptional
./${TAG}testOptional

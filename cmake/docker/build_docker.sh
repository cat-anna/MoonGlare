#!/usr/bin/env bash

cd $(dirname $0)

project_name=moonglare_build

if [ $# -eq 0 ]; then
  version_tag='latest'
else
  version_tag=$1
fi

DOCKER_TAG="${project_name}:${version_tag}"
FILE_TAG="${project_name}_${version_tag}"

docker build -t "${DOCKER_TAG}" .
# docker save -o "image_${FILE_TAG}.tgz" "${DOCKER_TAG}"

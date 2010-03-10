#! /bin/bash

export PROJECT_NAME=if2kv2
export PROJECT_VERSION=2.2.0-beta
export TOP=$(cd .. && pwd)

export DEST_DIR="${JDKS_BUILDS_DIR}/${PROJECT_NAME}"
mkdir -p "${DEST_DIR}"
mkdir -p "${INSTALL_DIR}/docs/"
cd .. && doxygen Doxyfile >& "${DEST_DIR}/doxygen${JDKS_AUTOBUILD_SUFFIX}.txt" && mv "${DEST_DIR}" "${INSTALL_DIR}/docs/" 

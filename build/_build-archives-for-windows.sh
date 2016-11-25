# -*- mode: shell-script ; coding: utf-8-unix -*-
#! /bin/sh



declare -a CLANG_VERSIONS=(
    # 390
    # 390
    390
    390
)

declare -a VS_VERSIONS=(
    # 2015
    # 2015
    2013
    2013
)

declare -a ARCH_TYPES=(
    # 64
    # 32
    64
    32
)

declare -a ARCH_NAMES=(
    # x86_64
    # x86_32
    x86_64
    x86_32
)



declare -i BUILD_COUNT="${#CLANG_VERSIONS[@]}"

if $( [ ${BUILD_COUNT} -ne ${#VS_VERSIONS[@]} ] || [ ${BUILD_COUNT} -ne ${#ARCH_TYPES[@]} ] || [ ${BUILD_COUNT} -ne ${#ARCH_NAMES[@]} ] ); then
    echo "don't match table count"
    exit 1
fi


declare SERVER_VERSION="1.5.0"
declare CLANG_VERSION
declare VS_VERSION
declare ARCH_TYPE
declare ARCH_NAME
declare ARCHIVE_NAME
declare INSTALL_PREFIX
declare WORK_DIR="/tmp/clang-server-archives"

if [ ! -d ${WORK_DIR} ]; then
    mkdir -p ${WORK_DIR}
fi

for (( i = 0; i < ${BUILD_COUNT}; ++i )); do
    CLANG_VERSION=${CLANG_VERSIONS[ ${i} ]}
    VS_VERSION=${VS_VERSIONS[ ${i} ]}
    ARCH_TYPE=${ARCH_TYPES[ ${i} ]}
    ARCH_NAME=${ARCH_NAMES[ ${i} ]}
    ARCHIVE_NAME="clang-server-${SERVER_VERSION}-${ARCH_NAME}-vs${VS_VERSION}"
    INSTALL_PREFIX=$( cygpath -am "${WORK_DIR}/${ARCHIVE_NAME}" )

    cmd /c "builder_sample.bat ${CLANG_VERSION} ${VS_VERSION} ${ARCH_TYPE} Release ${INSTALL_PREFIX}"

    pushd ${WORK_DIR}
    if [ -d ${ARCHIVE_NAME} ]; then
        tar -cvzf "${ARCHIVE_NAME}.zip" "${ARCHIVE_NAME}"
    fi
    popd
done




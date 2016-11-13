# -*- mode: shell-script ; coding: utf-8-unix -*-
#! /bin/sh



declare -a CLANG_VERSIONS=(
    390
    390
    390
    390
    390
    390
    390
    390
    380
    380
    380
    380
    380
    380
    380
    380
)

declare -a VS_VERSIONS=(
    2015
    2015
    2015
    2015
    2013
    2013
    2013
    2013
    2015
    2015
    2015
    2015
    2013
    2013
    2013
    2013
)

declare -a ARCHS=(
    64
    64
    32
    32
    64
    64
    32
    32
    64
    64
    32
    32
    64
    64
    32
    32
)

declare -a CONFIGS=(
    Release
    Debug
    Release
    Debug
    Release
    Debug
    Release
    Debug
    Release
    Debug
    Release
    Debug
    Release
    Debug
    Release
    Debug
)

declare -a SUFIXS=(
    ""
    "-debug"
    ""
    "-debug"
    ""
    "-debug"
    ""
    "-debug"
    ""
    "-debug"
    ""
    "-debug"
    ""
    "-debug"
    ""
    "-debug"
)


declare BUILD_COUNT="${#CLANG_VERSIONS[@]}"

if $( [ ${BUILD_COUNT} -ne ${#VS_VERSIONS[@]} ] || [ ${BUILD_COUNT} -ne ${#ARCHS[@]} ] || [ ${BUILD_COUNT} -ne ${#CONFIGS[@]} ] || [ ${BUILD_COUNT} -ne ${#SUFIXS[@]} ] ); then
    echo "don't match table count"
    exit 1
fi


declare CLANG_VERSION
declare VS_VERSION
declare ARCH
declare CONFIG
declare SUFIX

for (( i = 0; i < ${BUILD_COUNT}; ++i )); do
    CLANG_VERSION=${CLANG_VERSIONS[${i}]}
    VS_VERSION=${VS_VERSIONS[${i}]}
    ARCH=${ARCHS[${i}]}
    CONFIG=${CONFIGS[${i}]}
    SUFIX=${SUFIXS[${i}]}

    cmd /c "builder_sample.bat ${CLANG_VERSION} ${VS_VERSION} ${ARCH} ${CONFIG}"
    pushd /usr/local/bin
    tar -cvf "clang-${CLANG_VERSION}-${VS_VERSION}-${ARCH}-${CONFIG}.tar" "clang-server${SUFIX}.exe" libclang.dll
    popd
done




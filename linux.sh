#!/bin/sh

CXX=g++-14
JOBS=8

action=$1

if [ "x$action" = "x" ]; then
    echo "Usage: $0 <action>" 1>&2
    echo "  actions: debug release run clean" 1>&2
    echo "  debug: build debug binary" 1>&2
    echo "  release: build release binary" 1>&2
    echo "  run: run build locally" 1>&2
    echo "  clean: clean up build output" 1>&2
    echo "  tags: run ctags on all source" 1>&2
    echo "  dos2unix: run dos2unix on all source files" 1>&2
    exit 1
fi

build()
{
    build_type=${1:-Debug}
    if [ ! -d build ]; then
        mkdir build
    fi
    (cd build && cmake .. -DCMAKE_CXX_COMPILER=${CXX} -DCMAKE_BUILD_TYPE=${build_type} && cmake --build . -j${JOBS})
}

run()
{
    (cd build/bin && ./crobots++ "$@")
}

clean()
{
    rm -rf build tags *.log
}

case "$action" in
    debug)
        build Debug
        ;;

    release)
        build Release
        ;;

    run)
        shift
        run $@
        ;;

    clean)
        clean
        ;;

    dos2unix)
        find include src \( -name "*.cpp" -o -name "*.hpp" -o -name "*.md" \) -print0 | xargs -0 dos2unix
        ;;

    tags)
        rm -f tags
        find . \( -name "*.cpp" -o -name "*.hpp" \) -print0 | xargs -0 etags -a -o tags
        ;;

    *)
        echo "Unknown action" 1>&2
        exit 1
        ;;

esac

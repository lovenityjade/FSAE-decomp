#!/bin/sh
set -eu

usage() {
    echo "usage: $0 analyze IMAGE [PROJECT_DIR]" >&2
    echo "       $0 export IMAGE MIN_ADDRESS MAX_ADDRESS [PROJECT_DIR]" >&2
    exit 2
}

[ "$#" -ge 2 ] || usage
action=$1
image=$2

case "$image" in
    arm9|arm7|arm9i|arm7i) ;;
    *) echo "unsupported image: $image" >&2; exit 2 ;;
esac

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
headless=${GHIDRA_HEADLESS:-ghidra-headless}
project_name=fsae_decomp
program_name="$image.analysis.elf"

case "$action" in
    analyze)
        [ "$#" -le 3 ] || usage
        project_dir=${3:-"$root/build/ghidra"}
        elf="$root/build/decomp/$program_name"
        [ -f "$elf" ] || {
            echo "missing $elf; generate it with tools/decomp/analysis_elf.py" >&2
            exit 2
        }
        mkdir -p "$project_dir"
        exec "$headless" "$project_dir" "$project_name" \
            -import "$elf" \
            -processor ARM:LE:32:v5t \
            -cspec default \
            -overwrite \
            -analysisTimeoutPerFile 600 \
            -max-cpu "${GHIDRA_MAX_CPU:-4}"
        ;;
    export)
        [ "$#" -ge 4 ] && [ "$#" -le 5 ] || usage
        minimum=$3
        maximum=$4
        project_dir=${5:-"$root/build/ghidra"}
        output_dir="$root/build/decomp/$image"
        mkdir -p "$output_dir"
        exec "$headless" "$project_dir" "$project_name" \
            -process "$program_name" \
            -noanalysis \
            -scriptPath "$root/tools/decomp/ghidra_scripts" \
            -postScript ExportRecoveredC.java \
                "$output_dir/recovered.c" \
                "$output_dir/recovered.json" \
                "$minimum" "$maximum"
        ;;
    *) usage ;;
esac

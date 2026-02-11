#!/bin/bash

set -u

log_dir=".github/logs"
mkdir -p "$log_dir"

timestamp="$(date -u +%Y-%m-%dT%H%M%SZ)"
log_file="$log_dir/${timestamp}-build-debug.jsonl"

build_type="Debug"
log_level="TRACE"
build_dir="build_${build_type}_${log_level}"

write_log() {
    local step="$1"
    local status="$2"
    local command="$3"
    local exit_code="$4"
    local stdout_file="$5"
    local stderr_file="$6"

    python3 - <<PY
import json
from datetime import datetime, timezone

def read_file(path):
    if not path:
        return ""
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            return f.read()
    except FileNotFoundError:
        return ""

entry = {
    "timestamp": datetime.now(timezone.utc).isoformat(),
    "step": "${step}",
    "status": "${status}",
    "command": "${command}",
    "exit_code": int("${exit_code}"),
    "stdout": read_file("${stdout_file}"),
    "stderr": read_file("${stderr_file}"),
}

with open("${log_file}", "a", encoding="utf-8") as f:
    f.write(json.dumps(entry, ensure_ascii=False) + "\n")
PY
}

run_step() {
    local step="$1"
    local command="$2"

    local stdout_file
    local stderr_file
    stdout_file="$(mktemp)"
    stderr_file="$(mktemp)"

    bash -c "$command" > >(tee "$stdout_file") 2> >(tee "$stderr_file" >&2)
    local exit_code=$?

    if [ $exit_code -eq 0 ]; then
        write_log "$step" "ok" "$command" "$exit_code" "$stdout_file" "$stderr_file"
    else
        write_log "$step" "error" "$command" "$exit_code" "$stdout_file" "$stderr_file"
    fi

    rm -f "$stdout_file" "$stderr_file"
    return 0
}

run_step "configure" "cmake -S . -B ${build_dir} -DCMAKE_BUILD_TYPE=${build_type} -DDBG=ON -DDBG_GRAPHICS=ON -DLOG=${log_level} -DUNIT_TEST=ON"
run_step "build" "cmake --build ${build_dir}"
run_step "test" "ctest --test-dir ${build_dir} --output-on-failure"

echo "Log written to ${log_file}"

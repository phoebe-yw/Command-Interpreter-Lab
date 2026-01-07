#!/bin/bash

set -euo pipefail

CI_BIN="./bin/ci"
REFERENCE_BIN="./ci_reference"
TMP_OUTPUT_DIR="$(mktemp -d)"
CI_OUTPUT_DIR="$TMP_OUTPUT_DIR/ci"
REFERENCE_OUTPUT_DIR="$TMP_OUTPUT_DIR/reference"

cleanup() {
    rm -rf "$TMP_OUTPUT_DIR"
}

trap cleanup EXIT

if [ ! -f "$CI_BIN" ]; then
    echo "Error: CI binary not found at $CI_BIN"
    echo "Please run 'make' first"
    exit 1
fi

if [ ! -f "$REFERENCE_BIN" ]; then
    echo "Error: Reference binary not found at $REFERENCE_BIN"
    exit 1
fi

run_test() {
    local test_file="$1"
    local output_file="$2"
    local binary="$3"
    local week="$4"

    mkdir -p "$(dirname "$output_file")"

    local use_valgrind=false
    if [[ "$binary" == "$CI_BIN" ]]; then
        use_valgrind=true
    fi

    local err_file="${output_file%.out}.err"

    if $binary -i "$test_file" > "$output_file" 2> "$err_file"; then
        echo "0" > "$output_file.exitcode"
    else
        local exit_code=$?
        echo "$exit_code" > "$output_file.exitcode"
    fi
    
    if [ "$use_valgrind" = true ]; then
        local valgrind_output="${output_file%.out}.valgrind"
        local valgrind_exitcode="${output_file%.out}.valgrind.exitcode"
        local program_exit=$(cat "$output_file.exitcode")

        set +e
        valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
                 --log-file="$valgrind_output" --error-exitcode=42 \
                 "$binary" -i "$test_file" < /dev/null > /dev/null 2>&1
        local valgrind_exit=$?
        set -e

        if [ $valgrind_exit -eq 42 ]; then
            echo "42" > "$valgrind_exitcode"
        elif [ $valgrind_exit -eq $program_exit ]; then
            echo "0" > "$valgrind_exitcode"
        else
            # Valgrind returned different exit code than program - indicates valgrind error
            echo "$valgrind_exit" > "$valgrind_exitcode"
        fi
    fi
}

process_week() {
    local week="$1"
    local test_dir="testcases/$week"
    local total=0
    local passed=0
    local failed_tests=()
    echo ""
    echo "Running $week..."

    for test_file in "$test_dir"/*.s; do
        if [ ! -f "$test_file" ]; then
            continue
        fi

        total=$((total + 1))
        local test_name=$(basename "$test_file")
        local output_name="${test_name%.s}.out"

        local ci_output="$CI_OUTPUT_DIR/$week/$output_name"
        run_test "$test_file" "$ci_output" "$CI_BIN" "$week"

        local ref_output="$REFERENCE_OUTPUT_DIR/$week/$output_name"
        run_test "$test_file" "$ref_output" "$REFERENCE_BIN" "$week"

        local status="PASS"
        local mem_status="N/A"

        if [ -f "${ci_output%.out}.valgrind.exitcode" ]; then
            local valgrind_exit=$(cat "${ci_output%.out}.valgrind.exitcode")
            if [ "$valgrind_exit" = "42" ]; then
                mem_status="LEAK"
            else
                mem_status="OK"
            fi
        fi

        local outputs_match=true

        if ! diff -q "$ci_output" "$ref_output" > /dev/null 2>&1; then
            outputs_match=false
        fi

        if ! diff -q "${ci_output%.out}.err" "${ref_output%.out}.err" > /dev/null 2>&1; then
            outputs_match=false
        fi

        if ! diff -q "$ci_output.exitcode" "$ref_output.exitcode" > /dev/null 2>&1; then
            outputs_match=false
        fi

        if [ "$outputs_match" = true ]; then
            passed=$((passed + 1))

            rm -f "$ci_output" "${ci_output%.out}.err" "$ci_output.exitcode"
            rm -f "$ref_output" "${ref_output%.out}.err" "$ref_output.exitcode"

            if [ "$mem_status" = "OK" ]; then
                rm -f "${ci_output%.out}.valgrind" "${ci_output%.out}.valgrind.exitcode"
            fi

            if [ "$mem_status" = "N/A" ]; then
                echo "$week/$test_name ... PASS"
            else
                echo "$week/$test_name ... PASS (mem $mem_status)"
            fi
	        else
	            status="FAIL"
	            failed_tests+=("$test_name")

            if [ "$mem_status" = "N/A" ]; then
                echo "$week/$test_name ... FAIL"
            else
                echo "$week/$test_name ... FAIL (mem $mem_status)"
            fi

	            echo "  Reproduce:"
	            echo "    $REFERENCE_BIN -i '$test_file' -o ref.out"
	            echo "    $CI_BIN -i '$test_file' -o ci.out"
	            echo "    diff -u ref.out ci.out"

	            if [ "$mem_status" = "LEAK" ]; then
	                echo "  Valgrind: valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=42 $CI_BIN -i '$test_file'"
	            fi
	        fi
    done

    local percentage=0
    if [ $total -gt 0 ]; then
        percentage=$((passed * 100 / total))
    fi
    echo ""
    echo "$week: $passed/$total ($percentage%)"

    echo "$total $passed ${failed_tests[*]}" >&3
}

echo "CI Test Runner"
mkdir -p "$CI_OUTPUT_DIR"/{week2,week3,week4}
mkdir -p "$REFERENCE_OUTPUT_DIR"/{week2,week3,week4}

total_tests=0
total_passed=0
all_failed=()

if [ $# -eq 1 ]; then
    if [[ "$1" =~ ^[234]$ ]]; then
        target_week="week$1"
    else
        target_week="$1"
    fi
else
    target_week="all"
fi

case "$target_week" in
    "week2"|"2")
        weeks=("week2")
        ;;
    "week3"|"3")
        weeks=("week2" "week3")
        ;;
    "week4"|"4")
        weeks=("week2" "week3" "week4")
        ;;
    "all")
        weeks=("week2" "week3" "week4")
        ;;
    *)
        echo "Invalid week: $target_week"
        exit 1
        ;;
esac

for week in "${weeks[@]}"; do
    temp_summary=$(mktemp)
    
    exec 3>"$temp_summary"
    process_week "$week"
    exec 3>&-
    
    read week_total week_passed week_failed < "$temp_summary"
    rm -f "$temp_summary"

    total_tests=$((total_tests + week_total))
    total_passed=$((total_passed + week_passed))

    if [ -n "$week_failed" ]; then
        for test in $week_failed; do
            all_failed+=("$week/$test")
        done
    fi
done

if [ $total_tests -gt 0 ]; then
    percentage=$((total_passed * 100 / total_tests))
    echo ""
    echo "Overall: $total_passed/$total_tests ($percentage%)"
else
    echo "No tests found!"
fi

if [ ${#all_failed[@]} -ne 0 ]; then
    echo "Failed:"
    for test in "${all_failed[@]}"; do
        echo "  $test"
    done
fi


if [ $total_passed -eq $total_tests ]; then
    exit 0
else
    exit 1
fi

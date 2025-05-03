#!/bin/bash

run_cpp() {
    local start_range="$1"
    local end_range="$2"
    ORIGINAL_PWD=$(pwd)
    echo "Starting script in: ${ORIGINAL_PWD}"

    # --- Step 1: Run Pre-compiled C++ Code (within build directory) ---
    echo "--- [Step 1] Running Pre-compiled C++ Query ---"

    # Check if build directory exists
    if [ ! -d "${BUILD_DIR}" ]; then
        echo "Error: Build directory '${BUILD_DIR}' not found."
        echo "Please ensure the C++ code is compiled."
        exit 1
    fi

    EXECUTABLE_RELATIVE_PATH="./${CPP_EXECUTABLE}"
    OUTPUT_TSV_IN_BUILD="${CPP_OUTPUT_TSV}"

    echo "Changing directory to '${BUILD_DIR}'..."
    # directory changes
    pushd "${BUILD_DIR}" > /dev/null
    echo "Current directory: $(pwd)"


    # Check if the executable exists inside the build directory
    if [ ! -f "${EXECUTABLE_RELATIVE_PATH}" ]; then
        echo "Error: Pre-compiled executable not found at '$(pwd)/${EXECUTABLE_RELATIVE_PATH}'."
        echo "Please compile the C++ code first (e.g., cd build && cmake .. && make)"
        popd > /dev/null # Go back before exiting
        exit 1
    fi

    RUN_CPP_CMD="${EXECUTABLE_RELATIVE_PATH} ${CPP_COMMAND} ${cpp_start_range} ${cpp_end_range} ${CPP_BUFFER_SIZE} ${CPP_TEST_MODE}"
    echo "Executing C++ (inside build dir): ${RUN_CPP_CMD}"
    ${RUN_CPP_CMD} # Run the C++ command

    # Check if C++ output file was created *inside build directory*
    if [ ! -f "${OUTPUT_TSV_IN_BUILD}" ]; then
        echo "Error: C++ execution did not produce the expected output file '${OUTPUT_TSV_IN_BUILD}' in '$(pwd)'."
        popd > /dev/null # Go back before exiting
        exit 1
    fi
    echo "C++ execution finished. Output created: '$(pwd)/${OUTPUT_TSV_IN_BUILD}'"

}

run_bash_query_and_compare() {
    local start_range="$1"
    local end_range="$2"

    # Change back to the original directory
    echo "Changing back to original directory '${ORIGINAL_PWD}'..."
    popd > /dev/null # Suppress popd output
    echo "Current directory: $(pwd)"

    echo "--- C++ execution step complete. ---"
    echo

    echo "--- [Step 2] Running PostgreSQL Script (${POSTGRESQL_BASHFILE}) ---"

    if [ ! -f "${POSTGRESQL_BASHFILE}" ]; then
        echo "Error: PostgreSQL script not found at '${POSTGRESQL_BASHFILE}'."
        exit 1
    fi

    RUN_POSTGRESQL="${POSTGRESQL_BASHFILE} query ${start_range} ${end_range}"
    echo "Executing: ${RUN_POSTGRESQL}"
    ${RUN_POSTGRESQL} 

    if [ ! -f "${POSTGRESQL_OUTPUT_FILE}" ]; then
        echo "Error: PostgreSQL script did not produce the expected output file '${POSTGRESQL_OUTPUT_FILE}'."
        exit 1
    fi
    echo "--- PostgreSQL script finished. ---"
    echo
}

main() {
    set -e

    BUILD_DIR="build"
    CPP_EXECUTABLE="db_engine"

    CPP_COMMAND="run_query"
    
    CPP_BUFFER_SIZE="20"
    CPP_TEST_MODE="test" 
    CPP_OUTPUT_TSV="cpp_join_out.tsv"

    POSTGRESQL_BASHFILE="./test/postgres_command.sh"
    POSTGRESQL_OUTPUT_FILE="postgreSQL_output.tsv"

    local cpp_start_range="$1"
    local cpp_end_range="$2"

    run_cpp "${cpp_start_range}" "${cpp_end_range}"
    run_bash_query_and_compare "${cpp_start_range}" "${cpp_end_range}"
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  main "$@" # Pass command-line arguments to main
fi
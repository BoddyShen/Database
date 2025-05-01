#!/bin/bash
DB_NAME="imdb-test"
PSQL="psql -d $DB_NAME"

create_and_load() {
    $PSQL -f create_and_load.sql
    exit 0
}

query() {
    if [ $# -ne 3 ]; then
        echo "input: \$start_range \$end_range" >&2
        exit 1
    fi

    start_range="$2"
    end_range="$3"

    echo "$start_range $end_range "

    query="SELECT title, name
    FROM Movies, WorkedOn, People
    WHERE title >= '$start_range' AND title <= '$end_range' AND category = 'director' AND Movies.movieId = WorkedOn.movieId AND WorkedOn.personId = People.personId;"

    $PSQL -c "$query"
    exit 0
}

command="$1"


case "$command" in
    create_and_load)
        create_and_load "$@"
        ;;
    query)
        query "$@"
        ;;
    *)
        echo "Error: Unknown command '$command'" >&2
        exit 1
        ;;
esac

exit $?


# ./postgres_command.sh create_and_load
# ./postgres_command.sh query 'A' 'C~'
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

    # query1="SELECT movieId, title
    # FROM Movies
    # WHERE title COLLATE \"C\" >= '$start_range'
    # AND title COLLATE \"C\" <= '$end_range';"
    # $PSQL -c "$query1"

    # query2="SELECT movieId, personId, category
    # FROM WorkedOn
    # WHERE category = 'director'"
    # $PSQL -c "$query2"

    # query3="SELECT Movies.movieId, Movies.title, WorkedOn.movieId, WorkedOn.personId
    # FROM Movies, WorkedOn
    # WHERE title COLLATE \"C\" >= '$start_range'
    # AND title COLLATE \"C\" <= '$end_range' AND category = 'director' AND Movies.movieId = WorkedOn.movieId;" 

    # query3="SELECT Movies.movieId, Movies.title, WorkedOn.movieId, WorkedOn.personId
    # FROM Movies, WorkedOn
    # WHERE title >= '$start_range'
    # AND title <= '$end_range' AND category = 'director' AND Movies.movieId = WorkedOn.movieId;" 

    # $PSQL -c "$query3"

    query="SELECT title, name
    FROM Movies, WorkedOn, People
    WHERE title >= '$start_range' AND title <= '$end_range' AND category = 'director' AND Movies.movieId = WorkedOn.movieId AND WorkedOn.personId = People.personId;"

    # $PSQL -c "$query"
    psql_out="postgreSQL_output.tsv"
    psql -d "$DB_NAME" -A -F $'\t' -P footer=off -o "$psql_out" -c "$query"
    echo "→ wrote $psql_out"

    cpp_out="./build/cpp_join_out.tsv"
    psql_out="postgreSQL_output.tsv"

    # sort them (locale‑neutral C sort)
    sort "$cpp_out"   > sorted_cpp.tsv
    sort "$psql_out"  > sorted_psql.tsv

    if diff -u sorted_cpp.tsv sorted_psql.tsv > /dev/null; then
        echo "Congratulation! The two are the same!"
    else
        echo "Differences found!"
    fi

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
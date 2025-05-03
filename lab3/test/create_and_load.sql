DROP TABLE IF EXISTS WorkedOn;
DROP TABLE IF EXISTS Movies;
DROP TABLE IF EXISTS People;

CREATE TEMP TABLE Movie_Temp (
    tconst VARCHAR(10),
    titleType VARCHAR(20),
    primaryTitle TEXT,
    originalTitle TEXT,
    isAdult BOOLEAN,
    startYear INTEGER,
    endYear INTEGER,
    runtimeMinutes INTEGER,
    genres TEXT
);

CREATE TEMP TABLE People_Temp (
    nconst VARCHAR(10),
    primaryName TEXT,
    birthYear INTEGER,
    deathYear INTEGER,
    primaryProfession TEXT,
    knownForTitles TEXT  
);

CREATE TEMP TABLE WorkedOn_Temp (
    tconst VARCHAR(10),
    ordering INTEGER,
    nconst VARCHAR(10),
    category VARCHAR(20),
    job TEXT,
    characters TEXT
);

CREATE TABLE Movies (
    movieId VARCHAR(9) PRIMARY KEY,
    title VARCHAR(30) NOT NULL
);

CREATE TABLE People (
    personId VARCHAR(10) PRIMARY KEY,
    name VARCHAR(105) NOT NULL
);

CREATE TABLE WorkedOn (
    movieId VARCHAR(9) NOT NULL,
    personId VARCHAR(10) NOT NULL,
    category VARCHAR(20) NOT NULL,

    PRIMARY KEY (movieId, personId, category)
    -- FOREIGN KEY (movieId) REFERENCES Movies(movieId) ON DELETE CASCADE,
    -- FOREIGN KEY (personId) REFERENCES People(personId) ON DELETE CASCADE
);

\COPY Movie_Temp FROM '../movie_clean100000.tsv' WITH (FORMAT CSV, DELIMITER E'\t', HEADER TRUE, NULL '\N', ENCODING 'UTF8');

INSERT INTO Movies (movieId, title)
SELECT tconst, LEFT(primaryTitle, 30) FROM Movie_Temp;

\COPY People_Temp FROM '../people_clean100000.tsv' WITH (FORMAT CSV, DELIMITER E'\t', HEADER TRUE, NULL '\N', ENCODING 'UTF8');
INSERT INTO People (personId, name)
SELECT nconst, primaryName FROM People_Temp;


\COPY WorkedOn_Temp FROM '../workedon_clean100000.tsv' WITH (FORMAT CSV, DELIMITER E'\t', HEADER TRUE, NULL '\N', ENCODING 'UTF8');

INSERT INTO WorkedOn (movieId, personId, category)
SELECT tconst, nconst, category FROM WorkedOn_Temp
ON CONFLICT (movieId, personId, category) DO NOTHING;

DROP TABLE Movie_Temp;
DROP TABLE People_Temp;
DROP TABLE WorkedOn_Temp;

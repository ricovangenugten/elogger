CREATE USER elogger WITH PASSWORD 'elogger';
CREATE DATABASE elogger;
ALTER DATABASE elogger OWNER TO elogger;
GRANT ALL ON DATABASE elogger TO elogger;
CREATE TABLE pulses (id serial PRIMARY KEY UNIQUE,time timestamp);

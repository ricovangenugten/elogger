#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <libpq-fe.h>
#include <wiringPi.h>


#define SENSOR_PIN 0
#define ELOGGER_LAST_POINT 1
#define ELOGGER_ALL_POINTS 2

// Postgres connection is global, so interrupt can also reach it
PGconn *conn;
int new_point;

static void exit_nicely() {
    PQfinish(conn);
    exit(1);
}

void show_points(int which_points) {

    PGresult *res;
    int i;

    // Query
    if (which_points == ELOGGER_ALL_POINTS) {
        res = PQexec(conn, "SELECT id, time FROM pulses;");
    } else {
        res = PQexec(conn, "SELECT id, time FROM pulses ORDER BY id DESC LIMIT 1;");
    }

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Query failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }

    if (which_points == ELOGGER_ALL_POINTS) {
        // first, print out the attribute names
        printf("%-5s %-10s\n", PQfname(res, 0), PQfname(res, 1));
    }

    // next, print out the rows
    for (i = 0; i < PQntuples(res); i++) {
        printf("%-5s %-10s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1));
    }

    PQclear(res);

}

void add_point() {

    PGresult *res;

    // Query
    res = PQexec(conn, "INSERT INTO pulses (time) VALUES (now());");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Query failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }
    PQclear(res);

    new_point = 1;

}

void sigint_handler() {

  fprintf(stdout, "SIGINT received, closing database connection\n");
  PQfinish(conn);
  exit(1);

}

int main(int argc, char **argv) {

    new_point = 0;

    signal(SIGINT, &sigint_handler);

    // Make a connection to the database
    conn = PQconnectdb("dbname=elogger user=elogger password=elogger");

    // Check to see that the backend connection was successfully made
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        exit_nicely(conn);
    }

    show_points(ELOGGER_ALL_POINTS);

    wiringPiSetup();

    if (wiringPiISR(SENSOR_PIN, INT_EDGE_RISING, &add_point)) {
        fprintf(stderr, "Could not attach interrupt\n");
    }

    for (;;) {
       delay(1000);
       //add_point();
       if (new_point == 1) {
         new_point = 0;
         show_points(ELOGGER_LAST_POINT);
       }
    }

    return 0;
}

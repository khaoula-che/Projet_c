#include <stdlib.h>
#include "bdd.h"

PGconn *connexion_db() {
    const char *data = "host=localhost dbname=Projet_c user=postgres password=azerty";
    PGconn *db = PQconnectdb(data);
    if (PQstatus(db) != CONNECTION_OK) {
        fprintf(stderr, "Erreur de connexion : %s\n", PQerrorMessage(db));
        PQfinish(db);
        exit(EXIT_FAILURE);
    }
    PQexec(db, "SET client_encoding TO 'UTF8'");
    return db;
}


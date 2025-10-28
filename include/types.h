#ifndef TYPES_H
#define TYPES_H
#include <stddef.h>
#define STAT_LEN 20
#define NOM_LEN 100
#define nombre_jours 7

typedef struct{
        char nom_client[STAT_LEN];
        int date_debut;
        int date_fin;
}client;

typedef struct {
    char status[NOM_LEN];
    char nom_occupant[STAT_LEN];
    int day;
}jours;


#endif
#ifndef PLANIFICATION_H
#define PLANIFICATION_H
#define nombre_jours 7
#include "enregistrement.h"

    /*typedef struct {
        char status[20];
        char nom_occupant[100];
        int day;
    }jours;

    typedef struct{
        char nom_client[100];
        int date_debut;
        int date_fin;
    }client;*/

    //client Recupération_infos_client();

    //static void vider_entree(void);

    jours table_jours[nombre_jours];

    //remplir_la_table_jours(table_jours,nombres_jours); 
    void remplir_table_jours(jours table_jours[]);

    // fonction d'affichage du planning
    void affichage_planning(jours table_jours[], client tab[], int taille_tab);

    // fonction de prolongement de séjour
    void prolongement(client tab[], int taille_tab);

    // fontion de planification de retard
    void retard(client tab[], int taille_tab);

    //vider le buffer du clavier
    //static void vider_entree(void);
#endif
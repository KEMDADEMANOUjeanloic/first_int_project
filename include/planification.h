#ifndef PLANIFICATION_H
#define PLANIFICATION_H

#include "types.h"

    extern jours table_jours[nombre_jours];

    //remplir_la_table_jours(table_jours,nombres_jours); 
    void remplir_table_jours(jours table_jours[],int taille);

    // fonction d'affichage du planning
    void affichage_planning(jours table_jours[], client tab[], int taille_tab);

    // fonction de prolongement de séjour
    void prolongement(client tab[], int taille_tab);

    // fontion de planification de retard
    void retard(client **tab, int *taille_tab, client clientDeLaFile[], int *tailleDeLaFile);
    //void retard(client tab[], int taille_tab);

   
#endif
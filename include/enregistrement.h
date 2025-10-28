#ifndef ENREGISTREMENT_H
#define ENREGISTREMENT_H
#include "planification.h"
    
    /*prototype de recuperation des infor sur les clients*/
    typedef struct{
        char nom_client[100];
        int date_debut;
        int date_fin;
    }client;
    typedef struct {
        char status[20];
        char nom_occupant[100];
        int day;
    }jours;

    client Recupération_infos_client();

    //remplir_la_table_jours(table_jours,nombres_jours); 
    //void remplir_table_jours(jours table_jours[]);

    // fonction d'affichage du planning
    //void affichage_planning(jours table_jours[], client tab[], int taille_tab);

    // prototypes des fonctions tableaux de liste de reservation et de la file d'attente  

    void fonction_reservation(client **reserver,int * taille_reserver, client temp, int *comteur);

    void file_d_attente(client **attente, int * taille_attente, client temp1);
    
    //fonction d'annulation de reservation
    void annulation(client tab[], int *taille_tab, client tab1[], int *tab1_taille);

#endif
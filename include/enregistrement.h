#ifndef ENREGISTREMENT_H
#define ENREGISTREMENT_H

#include "types.h"
    
    /*prototype de recuperation des infor sur les clients*/

    client Recupération_infos_client(void);

    void affiche(client tab[], int taille);

    // prototypes des fonctions tableaux de liste de reservation et de la file d'attente  

    void fonction_reservation(client **reserver,int * taille_reserver, client temp, int *comteur);

    void file_d_attente(client **attente, int * taille_attente, client temp1);
    
    //fonction d'annulation de reservation
    void annulation(client tab[], int *taille_tab, client tab1[], int *tab1_taille);

#endif
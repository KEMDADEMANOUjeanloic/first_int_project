#include <stdio.h>
#include <stdlib.h>
#include "planification.h"
#include "enregistrement.h"
#include "interface.h"





static void vider_entree(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

int main(){

    int test,valeur, r = 0, a = 0;
    int compteur;
    client clientelle;
    client *reservations = NULL;
    client *la_file = NULL;

    //chargement des données
    reservations = charger_reservations(&r, FICHIER_RESERVATIONS);
    la_file = charger_reservations(&a, FICHIER_ATTENTE);

    remplir_table_jours(table_jours,nombre_jours);
    //affichage_planning(table_jours,reservations,r);
    
    do{
        //printf("\n=== CHOISIS L'ACTION A MENNER ===\n");
        //printf("----------------------------------\n | 0 ---> reservation. \n | 1 ---> annulation.\n | 2 ---> prolongement.\n | 3 ---> annonce d'un retard.\n | 4 ---> quitter;\n----------------------------------\n");
        afficher_menu();
        test = scanf("%d",&valeur);
        if (test != 1 || valeur<0 || valeur>4) vider_entree();
        while (test != 1 || valeur<0 || valeur>4){
            printf("\n ERREUR. entrez une valeur entre [0;4] :");
            test = scanf("%d",&valeur);
            if (test != 1 || valeur<0 || valeur>4) vider_entree();
        }
        affichage_planning(table_jours,reservations,r);
        if(valeur == 0){
            clientelle = Recupération_infos_client();
            fonction_reservation(&reservations,&r,clientelle, &compteur);
            if (compteur == 0){
                file_d_attente(&la_file, &a, clientelle);
                affiche(la_file,a);
            } 
        }
        if (valeur == 1){
            annulation(reservations,&r,la_file,&a);
            affichage_planning(table_jours,reservations,r);
        }
        if (valeur == 2){
            prolongement(reservations,r);
            affichage_planning(table_jours,reservations,r);
        }
        if (valeur == 3){
            retard(reservations,r);
            affichage_planning(table_jours,reservations,r);
        }
    }while(valeur != 4);
    
    affiche(la_file,a);
    
    affichage_planning(table_jours,reservations,r);

    //sauvegarde des données
    sauvegarder_reservations(reservations, r, FICHIER_RESERVATIONS);
    sauvegarder_reservations(la_file, a, FICHIER_ATTENTE);

    free(reservations);
    free(la_file);
    return 0;
}
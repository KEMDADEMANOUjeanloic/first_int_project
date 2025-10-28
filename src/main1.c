#include <stdio.h>
#include <stdlib.h>
#include "planification.h"
#include "enregistrement.h"

static void vider_entree(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

int main(){
    int test, valeur, i, j, r = 0, a = 0;
    int compteur;
    client clientelle;
    client *reservations = NULL;
    client *la_file = NULL;

    remplir_table_jours(table_jours,nombre_jours);

    for (i = 0; i < 3; i++){
        clientelle = Recupération_infos_client();
        fonction_reservation(&reservations,&r,clientelle, &compteur);
        if (compteur == 0){
            file_d_attente(&la_file, &a, clientelle);
        }

        printf("\n*********** repondre par '0' pour -NON- et par '1' pour -OUI- *************");
        printf("\n---> Voulez vous annuler une reservation ? :  ");
        test = scanf("%d", &valeur);
        if (test != 1 || (valeur<0 || valeur>1)) vider_entree();
        while (test != 1 || (valeur<0 || valeur>1)){
            printf("\n ERREUR !! saisir un entier entre [0;1] .");
            test = scanf("%d", &valeur);
            if (test != 1 || (valeur<0 || valeur>1)) vider_entree();
        }
        if (valeur == 1){
            annulation(reservations,&r,la_file,&a);
            affichage_planning(table_jours,reservations,r);
        }

        printf("\n---> Voulez vous prolonger votre séjour ? : ");
        test = scanf("%d", &valeur);
        if (test != 1 || (valeur<0 || valeur>1)) vider_entree();
        while (test != 1 || (valeur<0 || valeur>1)){
            printf("\n ERREUR !! saisir un entier entre [0;1] .");
            test = scanf("%d", &valeur);
            if (test != 1 || (valeur<0 || valeur>1)) vider_entree();
        }
        if (valeur == 1){
            prolongement(reservations,r);
            affichage_planning(table_jours,reservations,r);
        }

        printf("\n---> auriez-vous un retard ? : ");
        test = scanf("%d", &valeur);
        if (test != 1 || (valeur<0 || valeur>1)) vider_entree();
        while (test != 1 || (valeur<0 || valeur>1)){
            printf("\n ERREUR !! saisir un entier entre [0;1] .");
            test = scanf("%d", &valeur);
            if (test != 1 || (valeur<0 || valeur>1)) vider_entree();
        }
        if (valeur == 1){
            retard(reservations,r);
            affichage_planning(table_jours,reservations,r);
        }
    }
    
    affiche(la_file,a);
    
    affichage_planning(table_jours,reservations,r);
    free(reservations);
    free(la_file);
    return 0;
}
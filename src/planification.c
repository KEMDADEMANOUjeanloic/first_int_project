#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "enregistrement.h"
#include "planification.h"

jours table_jours[nombre_jours];

static void vider_entree(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

//rempir_table_jours(table_jours,nombres_jours); 
void remplir_table_jours(jours table_jours[], int taille){
    int i = 0;
    for (i = 0; i < taille; i++){
        strncpy(table_jours[i].status,"LIBRE",sizeof(table_jours[i].status)-1);
        table_jours[i].status[sizeof(table_jours[i].status)-1] = '\0';
        table_jours[i].nom_occupant[0] = '\0';
        table_jours[i].day = (i + 1);
    }
}

// fonction d'affichage du planning
void affichage_planning(jours table_jours[], client tab[], int taille_tab){
    int j = 0;
    int i = 0;
    if (table_jours == NULL || nombre_jours <= 0) return;
    if (tab == NULL && taille_tab > 0) return;

    remplir_table_jours(table_jours, nombre_jours);

    for (j = 0; j < taille_tab; j++){
        if( (tab[j].date_debut < 1) || (tab[j].date_debut > nombre_jours) || (tab[j].date_fin < tab[j].date_debut) || (tab[j].date_fin > nombre_jours)){
            continue;
        }
        for (i = tab[j].date_debut; i <= tab[j].date_fin; i++){
            int indx = i - 1;
            strncpy(table_jours[indx].status, "OCCUPÉ",sizeof(table_jours[indx].status)-1);
            table_jours[indx].status[sizeof(table_jours[indx].status)-1] = '\0';
            strncpy(table_jours[indx].nom_occupant, tab[j].nom_client, sizeof(table_jours[indx].nom_occupant)-1);
            table_jours[indx].nom_occupant[sizeof(table_jours[indx].nom_occupant)-1] = '\0';
            table_jours[indx].day = i;
        }
    }
    int k = 0;
    printf("\n============== planning de la chambre #10 ==============\n");
    printf("========================================================\n");
    printf("jour\t status\t         nom_occupant\n");
    for (k = 0; k < nombre_jours; k++){
        printf("========================================================\n");
        printf("|%2d\t |%-10s\t |%s\n", table_jours[k].day, table_jours[k].status, table_jours[k].nom_occupant);
    }
    printf("========================================================\n");
}

//fonction d'annulation de reservation
void  annulation(client tab[], int *taille_tab, client tab1[], int *tab1_taille){
    int date_int;
    int date_out, test;
    printf("\nentrez la date de début de la reservation à annuler : ");
    test = scanf("%d",&date_int);
    if (test != 1 || date_int<1 || date_int>nombre_jours ) vider_entree();
    while (test != 1 || date_int<1 || date_int>nombre_jours){
        printf("\n ERREUR. entrez une valeur entre [1;%d] :",nombre_jours);
        test = scanf("%d",&date_int);
        if (test != 1 || date_int<1 || date_int>nombre_jours ) vider_entree();
    }
    printf("\nentrez la date de fin de la reservation à annuler : ");
    test = scanf("%d",&date_out);
    if (test != 1 || date_out<1 || date_out>nombre_jours ) vider_entree();
    while (test != 1 || date_out<1 || date_out>nombre_jours){
        printf("\n ERREUR. entrez une valeur entre [1;%d] :",nombre_jours);
        test = scanf("%d",&date_out);
        if (test != 1 || date_out<1 || date_out>nombre_jours ) vider_entree();
    }
    int i, j, k, l, q, conflit, somme = 0;
    for (i = 0; i < *taille_tab; i++){
        if (date_int == tab[i].date_debut && date_out == tab[i].date_fin){
            for (k = 0; k < *tab1_taille; k++){
                if ((date_int == tab1[k].date_debut || date_int < tab1[k].date_debut) && (date_out == tab1[k].date_fin || date_out > tab1[k].date_fin)){
                    tab[i] = tab1[k];
                    printf("\n'%s' vous venez d'etres réserver suite a une annulation d'un client\n",tab1[k].nom_client);
                    (*tab1_taille)--;
                    for (l = k; l < (*tab1_taille); l++){
                        tab1[l] = tab1[l + 1];
                    }
                    return;
                }
                for (q = tab1[k].date_debut; q <= tab1[k].date_debut; q++){
                    if (strcmp(table_jours[tab1[k].date_debut - 1].status, "OCCUPÉ") == 0){
                        conflit = 1;
                        break;
                    }
                }
                if (conflit != 1){
                    tab[i] = tab1[k];
                    printf("\n'%s' vous venez d'etres réserver suite a une annulation d'un client\n",tab1[k].nom_client);
                    (*tab1_taille)--;
                    for (l = k; l < (*tab1_taille); l++){
                        tab1[l] = tab1[l + 1];
                    }
                    return;
                }
                
            }
            (*taille_tab)--;
            for (j = i; j < (*taille_tab); j++){
                tab[j] = tab[j + 1];
            }
            return;
        }
        somme += 1;
    }
    if (somme == (*taille_tab)){
        printf("\nRESERVATION NON TROUVÉ !!!!\n");
    }
}

// fonction de prolongement de séjour
void prolongement(client tab[], int taille_tab){
    char nom[50];
    int date_prol;
    int found = -1;
    int fin = 0;
    
    printf("\nEntrez la votre nom de reservation : ");
    vider_entree();
    if (fgets(nom, sizeof(nom), stdin) == NULL){
        printf("\n ERREUR de saisie de nom");
        vider_entree();
        return;
    }
    nom[strcspn(nom, "\n")] = '\0';

    int test = 0;
    printf("\nvotre date de fin de prolongement : ");
    test = scanf("%d",&date_prol);
    if (test != 1) vider_entree();
    while  ( test != 1){
        printf("\n(!!ERREUR!!) entrez une valeur entrez 1 et %d : ", nombre_jours);
        test = scanf("%d",&date_prol);
        if (test != 1) vider_entree();
    }
    
    if ((date_prol < 1) || (date_prol > nombre_jours)){
        printf("\n=== Se jour ne figure pas sur notre planning ===");
        return;
    }
    for(int i = 0; i < taille_tab; i++){
        if (strcmp(nom,tab[i].nom_client) == 0){
            found = i;
            break;
        }
    }
    if(found == -1){ 
        printf("\n === aucune reservation sous le nom de ( %s )", nom);
        return;
    }
    fin = tab[found].date_fin;
    if (date_prol <= fin){
        printf("\n=== votre nouvelle doit etre superieure a votre date fin !!! ===");
        return;
    }
    int start_end = fin + 1;
    for (int j = 0; j < taille_tab; j++){
        if (j == found) continue;
        if (!((tab[j].date_fin < start_end ) || (date_prol < tab[j].date_debut))){
            printf("\n=== ''DÉSOLÉ'' se prolongement n'est pas possible !! ===");
            return;
        }
    }
    tab[found].date_fin = date_prol;
    printf("\n=== Votre séjour a été prolongé de %d jour(s). Nouvelle date de fin = %d ===\n", date_prol - fin, date_prol);

}

// fontion de planification de retard
void retard(client **tab, int *taille_tab, client clientDeLaFile[], int *tailleDeLaFile){
    int found = -1;
    char nom[50];
    int new_date;

    printf("\n=== Entrez votre nom de reservation : ");
    vider_entree();
    fgets(nom, sizeof(nom), stdin);
    nom[strcspn(nom, "\n")] = '\0';

    printf("\n=== Entrez votre nouvelle date de debut : ");
    while (scanf("%d", &new_date) != 1) {
        vider_entree();
        printf("Date invalide, recommencez : ");
    }

    if (new_date < 1 || new_date > nombre_jours) {
        printf("\n=== Jour invalide ===");
        return;
    }

    // Recherche du client 
    for (int i = 0; i < *taille_tab; i++) {
        if (strcmp(nom, (*tab)[i].nom_client) == 0) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        printf("\n=== Reservation introuvable ===");
        return;
    }

    if (new_date <= (*tab)[found].date_debut) {
        printf("\nNouvelle date invalide");
        return;
    }

    (*tab)[found].date_debut = new_date;
    printf("\n=== Retard applique avec succes ===\n");

    // Vérifier si quelqu’un de la file peut être planifié 
    for (int k = 0; k < *tailleDeLaFile; k++) {
        int conflit = 0;

        for (int j = 0; j < *taille_tab; j++) {
            if (!((*tab)[j].date_fin < clientDeLaFile[k].date_debut ||
                  clientDeLaFile[k].date_fin < (*tab)[j].date_debut)) {
                conflit = 1;
                break;
            }
        }

        if (!conflit) {
            client *tmp = realloc(*tab, (*taille_tab + 1) * sizeof(client));
            if (!tmp) {
                printf("\nErreur mémoire");
                return;
            }

            *tab = tmp;
            (*tab)[*taille_tab] = clientDeLaFile[k];
            (*taille_tab)++;

            // Supprimer de la file 
            for (int l = k; l < *tailleDeLaFile - 1; l++)
                clientDeLaFile[l] = clientDeLaFile[l + 1];

            (*tailleDeLaFile)--;

            printf("\n'%s' a été planifié depuis la file d'attente\n",
                   clientDeLaFile[k].nom_client);
            break;
        }
    }
}






#include "planification.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "enregistrement.h"

//enregistrement des client

static void vider_entree(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

client Recupération_infos_client(){
    client client_i;
    int test;
    printf("\n=====nouveau enregistrement==\n");
    printf("\nEntrez votre nom : ");
    test = scanf("%49s",client_i.nom_client);
    
    printf("\nvotre date de début : ");
    test = scanf("%d",&client_i.date_debut);
    if (test != 1 || client_i.date_debut<1 || client_i.date_debut>nombre_jours ) vider_entree();
    while (test != 1 || client_i.date_debut<1 || client_i.date_debut>nombre_jours){
        printf("\n ERREUR. entrez une valeur entre [1;7] :");
        test = scanf("%d",&client_i.date_debut);
        if (test != 1 || client_i.date_debut<1 || client_i.date_debut>nombre_jours ) vider_entree();
    }

    printf("\nvotre date de fin : ");
    test = scanf("%d",&client_i.date_fin);
    if (test != 1 || client_i.date_fin<1 || client_i.date_fin>nombre_jours ) vider_entree();
    while (test != 1 || client_i.date_fin<1 || client_i.date_fin>nombre_jours){
        printf("\n ERREUR. entrez une valeur entre [1;7] :");
        test = scanf("%d",&client_i.date_fin);
        if (test != 1 || client_i.date_fin<1 || client_i.date_fin>nombre_jours ) vider_entree();
    }
    return client_i;
}

// tableaux de liste de reservation et de la file d'attente  

void fonction_reservation(client **reserver,int *taille_reserver, client temp, int *comteur){
    //client temp = Recupération_infos_client();
    int i = 0;
    for (i = 0; i < (*taille_reserver); i++){
        client *exist = &((*reserver)[i]);
        if (!(exist->date_fin < temp.date_debut || temp.date_fin < exist->date_debut)){
            affichage_planning(table_jours,*reserver, *taille_reserver);
            printf("\nDÉSOLÉ!!! VOTRE SEJOUR EST ENCONFLIT AVEC UN DE NOS CLIENTS .\n");
            printf("\nVOULLEZ VOUS MODIFIER ? !!!!!( 0 pour NON et 1 pour OUI )!!!! : \n");
            int modifier;
            scanf("%d", &modifier);
            if (modifier == 1){
                client temporaire = Recupération_infos_client(); 
                fonction_reservation(reserver, taille_reserver,temporaire, comteur);
                return;
            }else{
                *comteur = 0;
                return;
            }
            
        }
    }
    size_t new_count = (size_t)(*taille_reserver) + 1;
    client *tmp = realloc(*reserver, new_count *sizeof(client));
    if (tmp == NULL) {
        fprintf(stderr,"\nErreur d'allocation mémoire !\n");
        (*taille_reserver)--;
        exit(EXIT_FAILURE); //on arrête complètement le programme (utile si tu veux forcer l’arrêt en cas d’erreur mémoire grave).
    }
    *reserver = tmp;
    (*reserver)[(*taille_reserver)] = temp;
    (*taille_reserver)++;
    *comteur = 1;
    
}

void file_d_attente(client **attente, int *taille_attente, client temp1){
    //client temp1 = Recupération_infos_client();
    int j = 0;
    for (j = 0; j < (*taille_attente); j++){
        client *exist =&((*attente)[j]);
        if (strcmp(exist->nom_client, temp1.nom_client) == 0){
            return;
        }
    }
    size_t new_count = (size_t)(*taille_attente) + 1;
    client * tmp1 = realloc(*attente, new_count * sizeof(client));
    if (tmp1 == NULL) {
        fprintf(stderr,"\nErreur d'allocation mémoire !\n");
        (*taille_attente)--;
        exit(EXIT_FAILURE); //on arrête complètement le programme (utile si tu veux forcer l’arrêt en cas d’erreur mémoire grave).
    }
    *attente = tmp1;
    (*attente)[(*taille_attente)] = temp1;
    (*taille_attente)++; 
} 
 
void affiche(client tab[], int taille){
    for (int j = 0; j < taille; j++){
        //printf("clients reserver : \n");
        printf("\n==============\n");
        printf("nom_clients: %s\n", tab[j].nom_client);
        printf("date_début: %d\n", tab[j].date_debut);
        printf("date_fin: %d\n", tab[j].date_fin);
        printf("==============\n");
    }
}



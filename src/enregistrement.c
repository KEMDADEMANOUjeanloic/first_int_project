#include "enregistrement.h"
#include "planification.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//enregistrement des client

client Recupération_infos_client(){
    client client_i;
    printf("\nEntrez votre nom : ");
    scanf("%s",client_i.nom_client);
    printf("\nvotre date de début : ");
    scanf("%d", &client_i.date_debut);
    printf("\nvotre date de fin : ");
    scanf("%d", &client_i.date_fin);
    return client_i;
}

// tableaux de liste de reservation et de la file d'attente  

void fonction_reservation(client **reserver,int * taille_reserver, client temp, int *comteur){
    //client temp = Recupération_infos_client();
    int i = 0;
    for (i = 0; i < (*taille_reserver); i++){
        client * exist = &((*reserver)[i]);
        if (!(exist->date_fin < temp.date_debut || temp.date_fin < exist->date_debut)){
            printf("DÉSOLÉ!!! VOTRE SEJOUR EST ENCONFLIT AVEC UN DE NOS CLIENTS .\n");
            printf("VOULLEZ VOUS MODIFIER ? !!!!!( 0 pour NON et 1 pour OUI )!!!! : ");
            affichage_planning(table_jours,reserver, taille_reserver);
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
    client * tmp = realloc(* reserver, new_count * sizeof(client));
    if (tmp == NULL) {
        fprintf(stderr,"Erreur d'allocation mémoire !\n");
        (*taille_reserver)--;
        exit(EXIT_FAILURE); //on arrête complètement le programme (utile si tu veux forcer l’arrêt en cas d’erreur mémoire grave).
    }
    *reserver = tmp;
    (* reserver)[(*taille_reserver)] = temp;
    (*taille_reserver)++;
    *comteur = 1;
    
}

void file_d_attente(client **attente, int * taille_attente, client temp1){
    //client temp1 = Recupération_infos_client();
    int j = 0;
    for (j = 0; j < (*taille_attente); j++){
        client * exist =&((*attente)[j]);
        if (exist->nom_client == temp1.nom_client){
            return;
        }
    }
    size_t new_count = (size_t)(*taille_attente) + 1;
    client * tmp1 = realloc(* attente, new_count * sizeof(client));
    if (tmp1 == NULL) {
        fprintf(stderr,"Erreur d'allocation mémoire !\n");
        (*taille_attente)--;
        exit(EXIT_FAILURE); //on arrête complètement le programme (utile si tu veux forcer l’arrêt en cas d’erreur mémoire grave).
    }
    *attente = tmp1;
    (* attente)[(*taille_attente)] = temp1;
    (*taille_attente)++; 
} 
 




/*int main(){
    int i = 0;
    int j = 0;
    int r = 0;
    int a = 0;
    int compteur;
    client clientelle;
    client *reservations = NULL;
    client *la_file = NULL;
    for (i = 0; i < 3; i++){
        clientelle = Recupération_infos_client();
        fonction_reservation(&reservations,&r,clientelle, &compteur);
        if (compteur == 0){
            file_d_attente(&la_file, &a, clientelle);
        }
    }
    for (j = 0; j < (r); j++){
        printf("clients reserver : \n");
        printf("==============\n");
        printf("nom_clients: %s\n", reservations[j].nom_client);
        printf("date_début: %d\n", reservations[j].date_début);
        printf("date_fin: %d\n", reservations[j].date_fin);
        printf("==============\n");
    }
    for (j = 0; j < (a); j++){
        printf("client de la file : \n");
        printf("==============\n");
        printf("nom_clients: %s\n", la_file[j].nom_client);
        printf("date_début: %d\n", la_file[j].date_début);
        printf("date_fin: %d\n", la_file[j].date_fin);
        printf("==============");
    }
    free(reservations);
    free(la_file);
    return 0;
}*/



#include <stdio.h>
#include <stdlib.h>
#include "enregistrement.h"
#include "planification.h"

int main(){
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
        printf("date_début: %d\n", reservations[j].date_debut);
        printf("date_fin: %d\n", reservations[j].date_fin);
        printf("==============\n");
    }
    for (j = 0; j < (a); j++){
        printf("client de la file : \n");
        printf("==============\n");
        printf("nom_clients: %s\n", la_file[j].nom_client);
        printf("date_début: %d\n", la_file[j].date_debut);
        printf("date_fin: %d\n", la_file[j].date_fin);
        printf("==============");
    }
    free(reservations);
    free(la_file);
    return 0;
}
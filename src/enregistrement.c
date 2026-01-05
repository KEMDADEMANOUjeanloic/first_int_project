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

void afficher_menu() {      
        printf("\n|======================================|\n");
        printf("|  GESTION CHAMBRE HÔTEL #10           |\n");
        printf("|======================================|\n");
        printf("| 0 ---> Nouvelle réservation          |\n");
        printf("| 1 ---> Annuler une réservation       |\n");
        printf("| 2 ---> Prolonger un séjour           |\n");
        printf("| 3 ---> Déclarer un retard            |\n");
        printf("| 4 ---> Quitter                       |\n");
        printf("|======================================|\n");
        printf("Choisis l'action à mennée! :");
        
}

client Recupération_infos_client(){
    client client_i;
    int test;
    printf("\n===== nouveau enregistrement =====\n");
    printf("\nEntrez votre nom : ");
    test = scanf("%49s",client_i.nom_client);
    
    printf("\nvotre date de début : ");
    test = scanf("%d",&client_i.date_debut);
    if (test != 1 || client_i.date_debut<1 || client_i.date_debut>nombre_jours ) vider_entree();
    while (test != 1 || client_i.date_debut<1 || client_i.date_debut>nombre_jours){
        printf("\n ERREUR. entrez une valeur entre [1;%d] :",nombre_jours);
        test = scanf("%d",&client_i.date_debut);
        if (test != 1 || client_i.date_debut<1 || client_i.date_debut>nombre_jours ) vider_entree();
    }

    printf("\nvotre date de fin : ");
    test = scanf("%d",&client_i.date_fin);
    if (test != 1 || client_i.date_fin<1 || client_i.date_fin>nombre_jours ) vider_entree();
    while (test != 1 || client_i.date_fin<1 || client_i.date_fin>nombre_jours || client_i.date_fin < client_i.date_debut){
        printf("\n ERREUR. entrez une valeur entre [1;%d] et 'date de fin' doit etres superieure à 'date de début' :",nombre_jours);
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
            printf("\nDÉSOLÉ!!! VOTRE SÉJOUR EST EN CONFLIT AVEC UN DE NOS CLIENTS .\n");
            printf("\nVOULLEZ-VOUS MODIFIER ? !!!!!( 0 pour NON et 1 pour OUI )!!!! : \n");
            int modifier;
            scanf("%d", &modifier);
            if (modifier == 1){
                client temporaire = Recupération_infos_client(); 
                fonction_reservation(reserver, taille_reserver,temporaire, comteur);
                return;
            }else{
                *comteur = 0;
                printf("Vous avez été placer en file d'attente !");
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
    affichage_planning(table_jours,*reserver,*taille_reserver);
    printf("Votre réservation à été ajouter!");
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
 
// affichage de la file d'attente
void affiche(client tab[], int taille){
    for (int j = 0; j < taille; j++){
        printf("clients en attente : \n");
        printf("\n==============\n");
        printf("nom_clients: %s\n", tab[j].nom_client);
        printf("date_début: %d\n", tab[j].date_debut);
        printf("date_fin: %d\n", tab[j].date_fin);
        printf("==============\n");
    }
}

//Sauvegarde des clients dans un fichier binaire. 
void sauvegarder_reservations(client *tab, int taille, const char *nom_fichier) {
    FILE *f = fopen(nom_fichier, "wb");
    if (f == NULL) {
        perror("Erreur lors de l'ouverture du fichier pour l'écriture");
        return;
    }

    // Écrire d'abord la taille du tableau
    fwrite(&taille, sizeof(int), 1, f);

    // Écrire ensuite le tableau complet
    fwrite(tab, sizeof(client), taille, f);

    fclose(f);
}


//Charge un tableau de clients à partir d'un fichier binaire.
client *charger_reservations(int *taille, const char *nom_fichier) {
    FILE *f = fopen(nom_fichier, "rb");
    if (f == NULL) {
        // Le fichier n'existe pas encore (premier lancement), on initialise à zéro.
        *taille = 0;
        return NULL; 
    }

    // Vérification de la taille du fichier pour s'assurer qu'il n'est pas vide
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Un fichier valide doit avoir au moins la taille d'un int (pour stocker 'taille')
    if (file_size < (long)sizeof(int)) {
        fprintf(stderr, "Avertissement : Le fichier %s est trop petit ou vide.\n", nom_fichier);
        *taille = 0;
        fclose(f);
        return NULL;
    }

    // Lire la taille du tableau
    if (fread(taille, sizeof(int), 1, f) != 1) {
        fprintf(stderr, "Erreur lors de la lecture de la taille dans %s\n", nom_fichier);
        *taille = 0;
        fclose(f);
        return NULL;
    }

    if (*taille == 0) {
        fclose(f);
        return NULL;
    }

    // Allouer la mémoire pour le tableau
    client *tab = (client *)malloc(*taille * sizeof(client));
    if (tab == NULL) {
        perror("Erreur d'allocation mémoire au chargement");
        *taille = 0;
        fclose(f);
        return NULL;
    }

    // Lire le tableau
    if (fread(tab, sizeof(client), *taille, f) != (size_t)*taille) {
        fprintf(stderr, "Erreur lors de la lecture des données de client dans %s\n", nom_fichier);
        free(tab);
        *taille = 0;
        tab = NULL;
    }

    fclose(f);
    return tab;
}
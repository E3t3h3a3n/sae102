#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

// taille du serpent
#define TAILLE 10
// dimensions du plateau
#define LARGEUR_PLATEAU 80
#define HAUTEUR_PLATEAU 40
#define MILIEUX_PLATEAUX_X 40
#define MILIEUX_PLATEAUX_Y 20
#define PLATEAUX_HAUT 1
// position initiale de la tête du serpent
#define X_INITIAL 40
#define Y_INITIAL 20
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// temporisation entre deux déplacements du serpent (en microsecondes)
#define ATTENTE 199999
// caractères pour représenter le serpent
#define CORPS 'X'
#define TETE 'O'
//touche d'arret de jeux
#define STOP 'a'
// caractères pour les éléments du plateau
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'
#define NB_PAVES 6

// définition d'un type pour le plateau : tPlateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux
// dimensions et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles
// ne sont jamais utilisées)
typedef char tPlateau[LARGEUR_PLATEAU + 1][HAUTEUR_PLATEAU + 1];

//definition d'un type pour les positions ddes différents portails du jeux
typedef int tPortail[4][2];

int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5}; //tableau des position y de toutes les pommes du tableau
int lesPommesY[NB_POMMES] = { 8, 39, 2, 2, 5, 39, 33, 38, 35, 2}; //tableaux des position x de toites les pommes du tableau
int lesPavesX[NB_PAVES] = {3, 74, 3, 74, 38, 38}; //positions y du coin droit haut des pavés
int lesPavesY[NB_PAVES] = {3, 3, 34, 34, 21, 15}; //position x du coin droit haut des pavés
tPortail portail = {{40, 1}, {40, 40}, {80, 20}, {1, 20}}; //tableaux de tableaux des position x,y des portails du plateau de jeux haut bas droite gauche
int distHaut, distBas, distGauche, distDroite, d; //définition des distances des différentes possibilités de chemin  
int distMini; //distance minimal que le serpent choisit
int chemin; //numéros du chemin a prendre

void initPlateau(tPlateau plateau); //procédure qui initialise le plateaux de jeux
void dessinerPlateau(tPlateau plateau);//procédure qui affiche le plateaux de jeux
void ajouterPomme(tPlateau plateau, int numPomme); //procédure qui ajoute une pomme sur le plateaux de jeux
void afficher(int, int, char); //afficher un carcatere à une position x, y 
void effacer(int x, int y); //efface un carcatere à une position x,y
void dessinerSerpent(int lesX[], int lesY[]); //procédure qui dessine un serpent 
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau,
                bool *collision, bool *pomme, int numPomme, int *nbMvmt,
                int *ch); //fais progresser le serpent dans le plateaux de jeux
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();

int main() {
    clock_t begin = clock(); //démarre le chronos
    // 2 tableaux contenant les positions des éléments qui constituent le serpent
    int lesX[TAILLE]; //position x des éléments du serpent
    int lesY[TAILLE]; //position y des éléments du serpent

    // représente la touche frappée par l'utilisateur : touche de direction ou
    // pour l'arrêt
    char touche = 'd';

    // direction courante du serpent (HAUT, BAS, GAUCHE ou DROITE)
    char direction;

    // le plateau de jeu
    tPlateau lePlateau;

    bool collision = false;
    bool gagne = false;
    bool pommeMangee = false;

    // compteur de pommes mangées
    int nbPommes = 0;

    int nbMvmt = 0;

    // initialisation de listDirecta position du serpent : positionnement de la
    // tête en (X_INITIAL, Y_INITIAL), puis des anneaux à sa gauche
    for (int i = 0; i < TAILLE; i++) {
        lesX[i] = X_INITIAL - i;
        lesY[i] = Y_INITIAL;
    }

    // mise en place du plateau
    initPlateau(lePlateau);
    system("clear");
    dessinerPlateau(lePlateau);

    srand(time(NULL));
    ajouterPomme(lePlateau, nbPommes);

    // initialisation : le serpent se dirige vers la DROITE
    dessinerSerpent(lesX, lesY);
    disable_echo();
    direction = 'd';
    touche = 'd';
    //calcule des différentes distances que le serpent peut emprunter 
    //distance si le serpent prends le portails haut
    distHaut =
        (abs(portail[0][0] - lesX[0]) + abs(portail[0][1] - lesY[0])) +
        (abs(lesPommesX[0] - portail[1][0]) + abs(lesPommesY[0] - portail[1][1]));
    //distance si le serpent prends le portails bas
    distBas =
        (abs(portail[1][0] - lesX[0]) + abs(portail[1][1] - lesY[0])) +
        (abs(lesPommesX[0] - portail[0][0]) + abs(lesPommesY[0] - portail[0][1]));
    //distance si le serpent prends le portails gauche
    distGauche =
        (abs(portail[2][0] - lesX[0]) + abs(portail[2][1] - lesY[0])) +
        (abs(lesPommesX[0] - portail[3][0]) + abs(lesPommesY[0] - portail[3][1]));
    //distance si le serpent prends le postails droit
    distDroite =
        (abs(portail[3][0] - lesX[0]) + abs(portail[3][1] - lesY[0])) +
        (abs(lesPommesX[0] - portail[2][0]) + abs(lesPommesY[0] - portail[2][1]));
    //dist si le serpent prends la distance direct sans portail
    d = abs(lesPommesX[0] - lesX[0]) + abs(lesPommesY[0] - lesY[0]);
    //calcul de la distance minimal entre toute les possibilités
    distMini =
        fmin(d, fmin(distDroite, fmin(distGauche, fmin(distHaut, distBas))));
    //recherche du chemin associé à la plus faible distance
    if (distMini == distHaut) {
        chemin = 1;
    } else if (distMini == distBas) {
        chemin = 2;
    } else if (distMini == distGauche) {
        chemin = 3;
    } else if (distMini == distDroite) {
        chemin = 4;
    } else if (distMini == d) {
        chemin = 0;
    }
    // boucle de jeu. Arret si touche STOP, si collision avec une bordure ou
    // si toutes les pommes sont mangées
    do {
        progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee,
                nbPommes, &nbMvmt, &chemin);
        if (pommeMangee) {
            nbPommes++;
            gagne = (nbPommes == NB_POMMES);
            if (!gagne) {
                ajouterPomme(lePlateau, nbPommes); //ajout d'une pomme sur le terrain
                //recalcul de la meilleur distance avec la nouvel position de la pomme
                distHaut =
                    (abs(portail[0][0] - lesX[0]) + abs(portail[0][1] - lesY[0])) +
                    (abs(lesPommesX[nbPommes] - portail[1][0]) +
                    abs(lesPommesY[nbPommes] - portail[1][1]));
                distBas =
                    (abs(portail[1][0] - lesX[0]) + abs(portail[1][1] - lesY[0])) +
                    (abs(lesPommesX[nbPommes] - portail[0][0]) +
                    abs(lesPommesY[nbPommes] - portail[0][1]));
                distGauche =
                    (abs(portail[2][0] - lesX[0]) + abs(portail[2][1] - lesY[0])) +
                    (abs(lesPommesX[nbPommes] - portail[3][0]) +
                    abs(lesPommesY[nbPommes] - portail[3][1]));
                distDroite =
                    (abs(portail[3][0] - lesX[0]) + abs(portail[3][1] - lesY[0])) +
                    (abs(lesPommesX[nbPommes] - portail[2][0]) +
                    abs(lesPommesY[nbPommes] - portail[2][1]));
                d = abs(lesPommesX[nbPommes] - lesX[0]) +
                    abs(lesPommesY[nbPommes] - lesY[0]);
                //recalcul de la plus petite distance
                distMini = fmin(
                    d, fmin(distDroite, fmin(distGauche, fmin(distHaut, distBas))));
                if (distMini == distHaut) {
                chemin = 1;
                } else if (distMini == distBas) {
                chemin = 2;
                } else if (distMini == distGauche) {
                chemin = 3;
                } else if (distMini == distDroite) {
                chemin = 4;
                } else if (distMini == d) {
                chemin = 0;
                }
                pommeMangee = false;
            }
        }
        usleep(ATTENTE);
        if (kbhit()) {
            touche = getchar();
        }
    } while (touche != STOP && !gagne);
    enable_echo();
    gotoxy(1, HAUTEUR_PLATEAU + 1);
    clock_t end = clock();
    double tmpsCPU = ((end - begin) * 1.0) / CLOCKS_PER_SEC;
    printf(" Temps CPU = %.3f secondes\n", tmpsCPU);
    printf("Nombres de mouvements: %d\n", nbMvmt);
    return EXIT_SUCCESS;
}

/************************************************/
/*		FONCTIONS ET PROCEDURES DU JEU 			*/
/************************************************/
void initPlateau(tPlateau plateau) {
  // Initialisation du plateau avec des espaces (VIDE)
  for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
    for (int j = 1; j <= HAUTEUR_PLATEAU; j++) {
      plateau[i][j] = VIDE;
    }
  }

  // Mise en place des bordures avec un trou au centre
  for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
    for (int j = 1; j <= HAUTEUR_PLATEAU; j++) {
      // Si c'est une bordure (première ou dernière ligne, ou première ou
      // dernière colonne)
      if (i == 1 || i == LARGEUR_PLATEAU || j == 1 || j == HAUTEUR_PLATEAU) {
        // Vérification pour mettre un trou unique au centre de chaque bordure
        if ((i == LARGEUR_PLATEAU / 2 && (j == 1 || j == HAUTEUR_PLATEAU)) ||
            (j == HAUTEUR_PLATEAU / 2 && (i == 1 || i == LARGEUR_PLATEAU))) {
          plateau[i][j] = VIDE; // Trou unique sur la bordure
        } else {
          plateau[i][j] = BORDURE; // Sinon, bordure normale
        }
      }
    }
  }
  for(int i = 0; i < 6; i++ ){
    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 5; y++){
        plateau[lesPavesX[i] + y][lesPavesY[i] + x] = BORDURE;
        }
    }
  }
}
void dessinerPlateau(tPlateau plateau) {
    // affiche eà l'écran le contenu du tableau 2D représentant le plateau
    for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
        for (int j = 1; j <= HAUTEUR_PLATEAU; j++) {
        afficher(i, j, plateau[i][j]);
        }
    }
}

void ajouterPomme(tPlateau plateau, int numPomme) {
    plateau[lesPommesX[numPomme]][lesPommesY[numPomme]] = POMME;
    afficher(lesPommesX[numPomme], lesPommesY[numPomme], POMME);
}

void afficher(int x, int y, char car) {
    gotoxy(x, y);
    printf("%c", car);
    gotoxy(1, 1);
}

void effacer(int x, int y) {
    gotoxy(x, y);
    printf(" ");
    gotoxy(1, 1);
}

void dessinerSerpent(int lesX[], int lesY[]) {
    // affiche les anneaux puis la tête
    for (int i = 1; i < TAILLE; i++) {
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0], TETE);
}

void progresser(int lesX[], int lesY[], char direction, tPlateau plateau,
                bool *collision, bool *pomme, int numPomme, int *nbMvmt,
                int *ch) {
    // efface le dernier élément avant d'actualiser la position de tous les
    // élémentds du serpent avant de le  redessiner et détecte une
    // collision avec une pomme ou avec une bordure
    effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);

    int dx = lesPommesX[numPomme] - lesX[0]; //distance tete du serpent-pomme en x
    int dy = lesPommesY[numPomme] - lesY[0]; //distance tete du serpent-pomme en y
    bool valide = true; //booléen qui valide la prochaine position du serpent
    bool porte = false; //booléen qui valide le passage d'un portail
    int numPorte = 0; //indice du numéros de la porte
    int indice = 1; //indice du parcour de tout les morceaux du serpent 
    int i = 1;
    int distA = 0;
    int distB = 0;

    //incrémentation de tout les éléments du corps du serpent 
    for (int i = TAILLE - 1; i > 0; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }

//////////////////////////////////////////////////////////////////
////////////////CHEMIN VERS LE PORTAIL HAUT///////////////////////
//////////////////////////////////////////////////////////////////
    if (*ch == 1) {
        dx = portail[0][0] - lesX[0]; //calcul de la distance tete-portail du haut en x
        dy = portail[0][1] - lesY[0]; //calcul de la distance tet-portail du haut en y

        //condition pour savoir si la position x est correcte
        if (abs(dx) != 0) {
            //condition si la distance x est supérieur à 0
            if (dx > 0) {
                //verification de la prochaine position du serpent
                while(valide && indice < 10) {
                    //test du contact tete-corps du serpent
                    if (lesX[0] + 1 == lesX[indice] && lesY[0] == lesY[indice]) {
                        valide = false;//position invalide sile serpent se rentre dedans
                    }
                    indice++;
                }
                //position suivante validé
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]+1][lesY[0]] == BORDURE ){
                        //analyse de la distance bas
                        while(plateau[lesX[0]+1][lesY[0]+i] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]+1][lesY[0]-i] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            if(lesY[1] == (lesY[0] + 1)){
                                lesY[0] -= 1; //déplacement vers le haut
                            }
                            else{
                                lesY[0] += 1; //déplacement vers le bas
                            } 
                            (*nbMvmt)++; 
                        }
                        else{
                            if(lesY[1] == (lesY[0] - 1)){
                                lesY[0] += 1; //déplacement vers le haut
                            }
                            else{
                                lesY[0] -= 1; //déplacement vers le bas
                            }
                            (*nbMvmt)++;
                        }
                    }
                    else{
                        lesX[0] += 1; //déplacement sur le droite
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                }
                //si le serpent rencontre son corps
                else {
                    //test si un mouvment est possible vers le bas
                    if (plateau[lesX[0]][lesY[0]+1] == BORDURE) {
                        //test si le mouvment est possible vers le haut
                        if(plateau[lesX[0]][lesY[0]-1] == BORDURE){
                            lesX[0] -=1; //deux option impossible donc le serpent recul
                        }
                        else{
                            lesY[0] -= 1; //déplacement vers le haut
                        }
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                    //si le déplacement ne rencontre aucune bordure
                    else {
                        lesY[0] += 1; //déplacement vers le bas
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    }
                }
            }
            //si distence x est inférieur à 0
            else {
                //verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    //test du contact tete-corps du serpent
                    if (lesX[0] - 1 == lesX[indice] && lesY[0] == lesY[indice]) {
                        valide = false;//position invalide sile serpent se rentre dedans
                    }
                    indice++;
                }
                //si la position est valide
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]-1][lesY[0]] == BORDURE ){
                        //analyse de la distance bas
                        while(plateau[lesX[0]-1][lesY[0]+i] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]-1][lesY[0]-i] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            lesY[0] += 1; //déplacement vers le bas
                        }
                        else{
                            lesY[0] -= 1;//déplacment vers le haut
                        }
                    }
                    else{
                        lesX[0] -= 1; //déplacement sur le droite
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                } 
                //le serpent rencontre une partie de son corps
                else {
                    //test sur la prochain mouvement vers le haut
                    if (plateau[lesX[0]][lesY[0] - 1] == BORDURE) {
                        //test sur le prochain mouvement vers le bas
                        if(plateau[lesX[0]][lesY[0] + 1] == BORDURE){
                            lesX[0] += 1;
                        }
                        else{
                            lesY[0] += 1;
                        }
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                    //mouvement vers le haut possible
                    else {
                        lesY[0] -= 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                }
            }
        }
        //si la position x du serpent est correcte pour rentrer dans le portail 
        else {
            while (valide && indice < 10) {
                if (lesX[0] == lesX[indice] && lesY[0] - 1 == lesY[indice]) {
                    valide = false;
                }
                indice++;
            }
            if (valide == true){
                //test si la prochine case est un mur ou pas
                if(plateau[lesX[0]][lesY[0]-1] == BORDURE ){
                    //analyse de la distance droite
                    while(plateau[lesX[0]+i][lesY[0]-1] == BORDURE){
                        distA += 1;
                        i++;
                    }
                    i = 1;
                    //analyse de la distance gauche
                    while(plateau[lesX[0]-i][lesY[0]-1] == BORDURE){
                        distB += 1;
                        i++;
                    }
                    //choix de la distance la plus courte
                    if(distA <= distB){
                        lesX[0] += 1; //déplacement vers la droite
                    }
                    else{
                        lesX[0] -= 1;//déplacment vers la gauche
                    }
                }
                else{
                    lesY[0] -= 1; //déplacement vers le haut
                    (*nbMvmt)++; //incrémentation du nombre de mouvement
                } 
            } 
            //si la position est invalide
            else {
                //si le prochaine mouvement rencontre une bordure
                if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                    lesX[0] -= 1;
                    (*nbMvmt)++;//incrémentation du nombre de mouvement
                } 
                else {
                    lesX[0] += 1;
                    (*nbMvmt)++;//incrémentation du nombre de mouvement
                }
            }
        }   
    }
//////////////////////////////////////////////////////////////////
////////////////CHEMIN VERS LE PORTAIL BAS////////////////////////
//////////////////////////////////////////////////////////////////
    else if (*ch == 2) {
        dx = portail[1][0] - lesX[0];
        dy = portail[1][1] - lesY[0];
        if (abs(dx) != 0) {
            if (dx > 0) {
                while (valide && indice < 10) {
                    if (lesX[0] + 1 == lesX[indice] && lesY[0] == lesY[indice]) {
                        valide = false;
                    }
                    indice++;
                }
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]+1][lesY[0]] == BORDURE ){
                        //analyse de la distance bas
                        while(plateau[lesX[0]+1][lesY[0]+i] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]+1][lesY[0]-i] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            if(lesY[1] == lesY[0]+1){
                                lesY[0] -= 1;
                            }
                            else{ 
                                lesY[0] += 1; //déplacement vers le bas
                            }
                        }
                        else{
                            if(lesY[1] == lesY[0]-1){
                                lesY[0] += 1;
                            }
                            else{ 
                                lesY[0] -= 1; //déplacement vers le bas
                            }
                        }
                    }
                    else{
                        lesX[0] += 1; //déplacement sur le droite
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                } 
                //si la position est invalide
                else {
                    //si le prochaine mouvement rencontre une bordure
                    if (plateau[lesX[0]][lesY[0] + 1] == BORDURE) {
                        if(plateau[lesX[0]][lesY[0] - 1] == BORDURE)
                            lesX[0] -= 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    } 
                    else {
                        lesY[0] += 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                }
            } 
            //x est supérieur a 0
            else {
                while (valide && indice < 10) {
                    if (lesX[0] - 1 == lesX[indice] && lesY[0] == lesY[indice]) {
                        valide = false;
                    }
                    indice++;
                }
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]-1][lesY[0]] == BORDURE ){
                        //analyse de la distance bas
                        while(plateau[lesX[0]-1][lesY[0]+i] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]-1][lesY[0]-i] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            if(lesY[1] == lesY[0]+1){
                                lesY[0] -= 1;
                            }
                            else{ 
                                lesY[0] += 1; //déplacement vers le bas
                            }
                        }
                        else{
                            if(lesY[1] == lesY[0]-1){
                                lesY[0] += 1;
                            }
                            else{ 
                                lesY[0] -= 1; //déplacement vers le bas
                            }
                        }
                    }
                    else{
                        lesX[0] -= 1; //déplacement sur le droite
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                } 
                //si la position est invalide
                else {
                    //si le prochaine mouvement rencontre une bordure
                    if (plateau[lesX[0]][lesY[0] + 1] == BORDURE) {
                        if(plateau[lesX[0]][lesY[0] - 1] == BORDURE)
                            lesX[0] += 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    } 
                    else {
                        lesY[0] -= 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                }
            }
        } 
        else {
            while (valide && indice < 10) {
                if (lesX[0] == lesX[indice] && lesY[0] + 1 == lesY[indice]) {
                    valide = false;
                }
                indice++;
            }
            if (valide == true){
                //test si la prochine case est un mur ou pas
                if(plateau[lesX[0]][lesY[0]+1] == BORDURE ){
                    //analyse de la distance bas
                    while(plateau[lesX[0]+i][lesY[0]+1] == BORDURE){
                        distA += 1;
                        i++;
                    }
                        i = 1;
                    //analyse de la distance haut
                    while(plateau[lesX[0]-i][lesY[0]+1] == BORDURE){
                        distB += 1;
                        i++;
                    }
                    //choix de la distance la plus courte
                    if(distA <= distB){
                        lesX[0] += 1; //déplacement vers le bas
                    }
                    else{
                        lesX[0] -= 1;//déplacment vers le haut
                    }
                }
                else{
                    lesY[0] += 1; //déplacement sur le droite
                    (*nbMvmt)++; //incrémentation du nombre de mouvement
                } 
            }         
        }
    } 
//////////////////////////////////////////////////////////////////
////////////////CHEMIN VERS LE PORTAIL DROIT/////////////////////
//////////////////////////////////////////////////////////////////
    else if (*ch == 3) {
        dx = portail[2][0] - lesX[0];
        dy = portail[2][1] - lesY[0];
        if (abs(dy) != 0) {
            if (dy > 0) {
                while (valide && indice < 10) {
                    if (lesX[0] == lesX[indice] && lesY[0] + 1 == lesY[indice]) {
                        valide = false;
                    }
                    indice++;
                }
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]][lesY[0]+1] == BORDURE ){
                        //analyse de la distance bas
                        while(plateau[lesX[0]+i][lesY[0]+1] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]-i][lesY[0]+1] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            if(!(lesX[1] == (lesX[0] + 1))){
                                lesX[0] -= 1;
                            }
                            else{
                                lesX[0] += 1; //déplacement vers le bas
                            }
                        }
                        else{
                            if(lesX[1] == (lesX[0] - 1)){
                                lesX[0] += 1;
                            }
                            else{
                                lesX[0] -= 1; //déplacement vers le bas
                            }
                        }
                    }
                    else{
                        lesY[0] += 1; //déplacement sur le droite
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                } 
                //si la position est invalide
                else {
                    //si le prochaine mouvement rencontre une bordure
                    if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                        if(plateau[lesX[0] - 1][lesY[0]] == BORDURE){
                            lesY[0] -= 1;
                        }
                        else{
                            lesX[0] -=1;
                        }
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    } 
                    else {
                        lesX[0] += 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                }
            } 
            else {
                while (valide && indice < 10) {
                    if(lesX[0] == lesX[indice] && lesY[0] - 1 == lesY[indice]){
                        valide = false;
                    }
                    indice++;
                }
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]][lesY[0]-1] == BORDURE ){
                        //analyse de la distance bas
                        while(plateau[lesX[0]+i][lesY[0]-1] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]-i][lesY[0]-1] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            if(lesX[1] == lesX[0]+1){
                                lesX[0] -= 1;
                            }
                            else{
                                lesX[0] += 1; //déplacement vers le bas
                            }
                        }
                        else{
                            if(lesX[1] == lesX[0]-1){
                                lesX[0] += 1;
                            }
                            else{
                                lesX[0] -= 1; //déplacement vers le bas
                            }
                        }
                    }
                    else{
                        lesY[0] -= 1; //déplacement sur le droite
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                }
                else {
                    //si le prochaine mouvement rencontre une bordure
                    if (plateau[lesX[0]+1][lesY[0]] == BORDURE) {
                        if(plateau[lesX[0]-1][lesY[0]] == BORDURE){
                            lesY[0] +=1;
                        }
                        else{
                            lesX[0] -= 1;
                        }
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                    else{
                        lesX[0] += 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                }
            }
        }
        else {
            while (valide && indice < 10) {
                if (lesX[0] + 1 == lesX[indice] && lesY[0] == lesY[indice]) {
                    valide = false;
                }
                indice++;
            }
            if (valide == true){
                //test si la prochine case est un mur ou pas
                if(plateau[lesX[0]+1][lesY[0]] == BORDURE ){
                    //analyse de la distance bas
                    while(plateau[lesX[0]+1][lesY[0]+i] == BORDURE){
                        distA += 1;
                        i++;
                    }
                    i = 1;
                    //analyse de la distance haut
                    while(plateau[lesX[0]+1][lesY[0]-i] == BORDURE){
                        distB += 1;
                        i++;
                    }
                    //choix de la distance la plus courte
                    if(distA <= distB){
                        if(lesY[1] == lesY[0]+1){
                            lesY[0] -= 1;
                        }
                        else{
                            lesY[0] += 1; //déplacement vers le bas
                        }
                    }
                    else{
                        if(lesY[1] == lesY[0]-1){
                            lesY[0] += 1;
                        }
                        else{
                            lesY[0] -= 1; //déplacement vers le bas
                        }
                    }
                }
                else {
                    //si le prochaine mouvement rencontre une bordure
                    if (plateau[lesX[0]+1][lesY[0]] == BORDURE) {
                        if(plateau[lesX[0]-1][lesY[0]] == BORDURE){
                            lesY[0] +=1;
                        }
                        else{
                            lesX[0] -= 1;
                        }
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                    else{
                        lesX[0] += 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                } 
            }
        }
    }
//////////////////////////////////////////////////////////////////
////////////////CHEMIN VERS LE PORTAIL GAUCHE//////////////////////
//////////////////////////////////////////////////////////////////
    else if (*ch == 4) {
        dx = portail[3][0] - lesX[0];
        dy = portail[3][1] - lesY[0];
        if (abs(dy) != 0) {
            if (dy > 0) {
                while (valide && indice < 10) {
                    if (lesX[0] == lesX[indice] && lesY[0] + 1 == lesY[indice]) {
                        valide = false;
                    }
                    indice++;
                }
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]][lesY[0]+1] == BORDURE ){
                        //analyse de la distance bas
                        while(plateau[lesX[0]+i][lesY[0]+1] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]-i][lesY[0]+1] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            if(lesX[1] == (lesX[0] + 1)){
                                lesX[0] -= 1;
                            }
                            else{
                                lesX[0] += 1; //déplacement vers le bas
                            }
                        }
                        else{
                            if(lesX[1] == (lesX[0] - 1)){
                                lesX[0] += 1;
                            }
                            else{
                                lesX[0] -= 1; //déplacement vers le bas
                            }
                        }
                    }
                    else{
                        lesY[0] += 1; //déplacement sur le droite
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                }
                //mouvement vers le bas impossible
                else {
                    //si le prochaine mouvement rencontre une bordure
                    if (plateau[lesX[0]+1][lesY[0]] == BORDURE) {
                        if(plateau[lesX[0]-1][lesY[0]] == BORDURE){
                            lesY[0] -=1;
                        }
                        else{
                            lesX[0] -= 1;
                        }
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                    else{
                        lesX[0] += 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                }
            } 
            else {
                while (valide && indice < 10) {
                    if (lesX[0] == lesX[indice] && lesY[0] - 1 == lesY[indice]) {
                        valide = false;
                    }
                    indice++;
                }
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]][lesY[0]-1] == BORDURE ){
                        //analyse de la distance bas
                        while(plateau[lesX[0]+i][lesY[0]-1] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]-i][lesY[0]-1] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            valide =  true;
                            indice = 1;
                            while (valide && indice < 10) {
                                if (lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]) {
                                    valide = false;
                                }
                                indice++;
                            }
                            if(valide){
                                lesX[0] += 1;
                            }
                            else{
                                lesX[0] -= 1;
                            }
                        }
                        else{
                            valide =  true;
                            indice = 1;
                            while (valide && indice < 10) {
                                if (lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]) {
                                    valide = false;
                                }
                                indice++;
                            }
                            if(valide){
                                lesX[0] -= 1;
                            }
                            else{
                                lesX[0] += 1;
                            }
                        }
                    }
                    else{
                        lesY[0] -= 1; //déplacement sur le droite
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                } 
                else {
                    //si le prochaine mouvement rencontre une bordure
                    if (plateau[lesX[0]+1][lesY[0]] == BORDURE) {
                        if(plateau[lesX[0]-1][lesY[0]] == BORDURE){
                            lesY[0] +=1;
                        }
                        else{
                            lesX[0] -= 1;
                        }
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                    else{
                        lesX[0] += 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                }
            }
        } 
        else {
            while (valide && indice < 10) {
                if (lesX[0] - 1 == lesX[indice] && lesY[0] == lesY[indice]) {
                    valide = false;
                }
                indice++;
            }
            if (valide == true){
                //test si la prochine case est un mur ou pas
                if(plateau[lesX[0]-1][lesY[0]] == BORDURE ){
                    //analyse de la distance bas
                    while(plateau[lesX[0]-1][lesY[0]+i] == BORDURE){
                        distA += 1;
                        i++;
                    }
                    i = 1;
                    //analyse de la distance haut
                    while(plateau[lesX[0]-1][lesY[0]-i] == BORDURE){
                        distB += 1;
                        i++;
                    }
                    //choix de la distance la plus courte
                    if(distA <= distB){
                        if(lesY[1] == (lesY[0] + 1)){
                                lesY[0] -= 1;
                            }
                        else{
                            lesY[0] += 1; //déplacement vers le bas
                        }
                    }
                    else{
                        if(lesY[1] == (lesY[0] - 1)){
                                lesY[0] += 1;
                            }
                        else{
                            lesY[0] -= 1; //déplacement vers le bas
                        }
                    }
                }
                else{
                    lesX[0] -= 1; //déplacement sur la gauche
                    (*nbMvmt)++; //incrémentation du nombre de mouvement
                } 
            } 
            else {
                if (plateau[lesX[0]][lesY[0] + 1] == BORDURE) {
                    lesY[0] -= 1;
                    (*nbMvmt)++;//incrémentation du nombre de mouvement
                } 
                else {
                    lesY[0] += 1;
                    (*nbMvmt)++;//incrémentation du nombre de mouvement
                } 
            } 
        }
    } 
//////////////////////////////////////////////////////////////////
////////////////CHEMIN DIRECT VERS LA POMME///////////////////////
//////////////////////////////////////////////////////////////////
    else if (*ch == 0) {
        dx = lesPommesX[numPomme] - lesX[0];
        dy = lesPommesY[numPomme] - lesY[0];
        if (abs(dx) > abs(dy)){
            //position de la pomme part rapport a la tete du serpent
            if (dx > 0) {
                while (valide && indice < 10) {
                    if (lesX[0] + 1 == lesX[indice] && lesY[0] == lesY[indice]) {
                        valide = false;
                    }
                    indice++;
                }
                //si la position est valide
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]+1][lesY[0]] == BORDURE ){
                        //analyse de la distance bas
                        while(plateau[lesX[0]+1][lesY[0]+i] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]+1][lesY[0]-i] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            if(lesY[1] == (lesY[0] + 1)){
                                lesY[0] -= 1;
                            }
                            else{
                                lesY[0] += 1; //déplacement vers le bas
                            }
                        }
                        else{
                            if(lesY[1] == (lesY[0] - 1)){
                                lesY[0] += 1;
                            }
                            else{
                                lesY[0] -= 1; //déplacement vers le bas
                            }
                        }
                    }
                    else{
                        lesX[0] += 1; //déplacement sur le droite
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                } 
                else {
                    //si le prochaine mouvement rencontre une bordure
                    if (plateau[lesX[0]][lesY[0]-1] == BORDURE) {
                        if(plateau[lesX[0]][lesY[0]+1] == BORDURE){
                            lesX[0] -=1;
                        }
                        else{
                            lesY[0] += 1;
                        }
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                    else{
                        lesY[0] += 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                }
            } 
            else {

                while (valide && indice < 10) {
                    if (lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]) {
                        valide = false;
                    }
                    indice++;
                }
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]-1][lesY[0]] == BORDURE ){

                        //analyse de la distance bas
                        while(plateau[lesX[0]-1][lesY[0]+i] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]-1][lesY[0]-i] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            if(lesY[1] == (lesY[0] + 1)){
                                lesY[0] -= 1;
                            }
                            else{
                                lesY[0] += 1; //déplacement vers le bas
                            }
                        }
                        else{
                            if(lesY[1] == (lesY[0] - 1)){
                                lesY[0] += 1;
                            }
                            else{
                                lesY[0] -= 1; //déplacement vers le bas
                            }
                        }
                    }
                    else{
                        lesX[0] -= 1; //déplacement sur le droite
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                }
                else {
                    if (plateau[lesX[0]][lesY[0]-1] == BORDURE) {
                        if(plateau[lesX[0]][lesY[0]+1] == BORDURE){
                            lesX[0] +=1;
                        }
                        else{
                            lesY[0] += 1;
                        }
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                    else{
                        lesY[0] -= 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                }
            }
        }
        else {
            if (dy > 0) {
                while (valide && indice < 10) {
                    if (lesX[0] == lesX[indice] && lesY[0] + 1 == lesY[indice]) {
                        valide = false;
                    }
                    indice++;
                }
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]][lesY[0]+1] == BORDURE ){
                        //analyse de la distance bas
                        while(plateau[lesX[0]+i][lesY[0]+1] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]-i][lesY[0]+1] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            if(lesX[1] == lesX[0] + 1){
                                lesX[0] -= 1;
                            }
                            else{
                                lesX[0] += 1;
                            }
                        }
                        else{
                            if(lesX[1] == lesX[0] - 1){
                                lesX[0] += 1;
                            }
                            else{
                                lesX[0] -= 1;
                            }

                        }
                    }
                    else{
                        lesY[0] += 1; //déplacement sur le droite
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    }
                } 
                else {
                    if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                        lesX[0] -= 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    } 
                    else {
                        lesX[0] += 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                }
            } 
            else {
                while (valide && indice < 10) {
                    if (lesX[0] == lesX[indice] && lesY[0] - 1 == lesY[indice]) {
                        valide = false;
                    }
                    indice++;
                }
                if (valide == true){
                    //test si la prochine case est un mur ou pas
                    if(plateau[lesX[0]][lesY[0]-1] == BORDURE ){
                        //analyse de la distance bas
                        while(plateau[lesX[0]+i][lesY[0]-1] == BORDURE){
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        //analyse de la distance haut
                        while(plateau[lesX[0]-i][lesY[0]-1] == BORDURE){
                            distB += 1;
                            i++;
                        }
                        //choix de la distance la plus courte
                        if(distA <= distB){
                            valide = true;
                            indice = 1;
                            while (valide && indice < 10) {
                                if (lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]) {
                                    valide = false;
                                }
                                indice++;
                            }
                            if(valide){
                                lesX[0] +=1;
                            }
                            else{
                                lesX[0] -= 1;
                            }
                        }
                        else{
                            valide = true;
                            indice = 1;
                            while (valide && indice < 10) {
                                if (lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]) {
                                    valide = false;
                                }
                                indice++;
                            }
                            if(valide){
                                lesX[0] -=1;
                            }
                            else{
                                lesX[0] += 1;
                            }
                        }
                    }
                    else{
                        lesY[0] -= 1; //déplacement vers le haut
                        (*nbMvmt)++; //incrémentation du nombre de mouvement
                    } 
                } 
                else {
                    if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                        lesX[0] -= 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    } 
                    else {
                        lesX[0] += 1;
                        (*nbMvmt)++;//incrémentation du nombre de mouvement
                    }
                }
            }
        }
    }
    //parcour des diiférente position des portes
    //test si le serpent est dans l'une d'elle 
    while (!porte && numPorte < 4) {
        if (lesX[0] == portail[numPorte][0] && lesY[0] == portail[numPorte][1]) {
            porte = true;
            if (*ch == 1) {
                lesX[0] = portail[numPorte + 1][0];
                lesY[0] = portail[numPorte + 1][1] - 1;
            } 
            else if (*ch == 2) {
                lesX[0] = portail[numPorte - 1][0];
                lesY[0] = portail[numPorte - 1][1] + 1;
            }   
            else if (*ch == 3) {
                lesX[0] = portail[numPorte + 1][0] + 1;
                lesY[0] = portail[numPorte + 1][1];
            } 
            else if (*ch == 4) {
                lesX[0] = portail[numPorte - 1][0] - 1;
                lesY[0] = portail[numPorte - 1][1];
            }
            *ch = 0;
        }
        numPorte++;
    }

  if (plateau[lesX[0]][lesY[0]] == POMME) {
    plateau[lesX[0]][lesY[0]] = VIDE;
    *pomme = true;
  }

  dessinerSerpent(lesX, lesY);
}

/************************************************/
/*				 FONCTIONS UTILITAIRES 			*/
/************************************************/
void gotoxy(int x, int y) { printf("\033[%d;%df", y, x); }

int kbhit() {
    // la fonction retourne :
    // 1 si un caractere est present
    // 0 si pas de caractere présent
    int unCaractere = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    // mettre le terminal en mode non bloquant
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    // restaurer le mode du terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        unCaractere = 1;
    }
    return unCaractere;
}

// Fonction pour désactiver l'echo
void disable_echo() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Désactiver le flag ECHO
    tty.c_lflag &= ~ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

// Fonction pour réactiver l'echo
void enable_echo() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Réactiver le flag ECHO
    tty.c_lflag |= ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}
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
#define X_INITIAL_1 40
#define Y_INITIAL_1 13
#define X_INITIAL_2 40
#define Y_INITIAL_2 27
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// temporisation entre deux déplacements du serpent (en microsecondes)
#define ATTENTE 199999
// caractères pour représenter le serpent
#define CORPS 'X'
#define TETE1 '1'
//touche d'arret de jeux
#define STOP 'a'
// caractères pour les éléments du plateau
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'
#define NB_PAVES 6
#define TETE2 '2'

// définition d'un type pour le plateau : tPlateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux
// dimensions et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles
// ne sont jamais utilisées)
typedef char tPlateau[LARGEUR_PLATEAU + 1][HAUTEUR_PLATEAU + 1];

//definition d'un type pour les positions ddes différents portails du jeux
typedef int tPortail[4][2];

int lesPavesX[NB_PAVES] = { 4, 73, 4, 73, 38, 38};
int lesPavesY[NB_PAVES] = { 4, 4, 33, 33, 14, 22};
int lesPommesX[NB_POMMES] = {40, 75, 78, 2, 9, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = {20, 38, 2, 2, 5, 38, 32, 38, 32, 2};
tPortail portail = {{40, 1}, {40, 40}, {80, 20}, {1, 20}}; //tableaux de tableaux des position x,y des portails du plateau de jeux haut bas droite gauche
int distHaut1, distBas1, distGauche1, distDroite1, d1; //définition des distances des différentes possibilités de chemin  
int distMini1; //distance minimal que le serpent choisit
int distHaut2, distBas2, distGauche2, distDroite2, d2; //définition des distances des différentes possibilités de chemin  
int distMini2; //distance minimal que le serpent choisit
int chemin1; //numéros du chemin a prendre
int chemin2; //numéros du chemin a prendre

void initPlateau(tPlateau plateau); //procédure qui initialise le plateaux de jeux
void dessinerPlateau(tPlateau plateau);//procédure qui affiche le plateaux de jeux
void ajouterPomme(tPlateau plateau, int numPomme); //procédure qui ajoute une pomme sur le plateaux de jeux
void afficher(int x, int y, char); //afficher un carcatere à une position x, y 
void effacer(int x, int y); //efface un carcatere à une position x,y
void dessinerSerpent1(int lesX[], int lesY[]); //procédure qui dessine un serpent 
void dessinerSerpent2(int lesX[], int lesY[]);
void progresser1(int lesX[], int lesX2[], int lesY[], int lesY2[], char direction, tPlateau plateau,
                bool *collision, bool *pomme, int numPomme, int *nbMvmt,
                int *ch); //fais progresser le serpent dans le plateaux de jeux
void progresser2(int lesX[], int lesX2[], int lesY[], int lesY2[], char direction, tPlateau plateau,
                bool *collision, bool *pomme, int numPomme, int *nbMvmt,
                int *ch);
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();

int main() {
    clock_t begin = clock(); //démarre le chronos
    // 2 tableaux contenant les positions des éléments qui constituent le serpent
    int lesX1[TAILLE]; //position x des éléments du serpent
    int lesY1[TAILLE]; //position y des éléments du serpent
    int lesX2[TAILLE]; //position x des éléments du serpent
    int lesY2[TAILLE]; //position y des éléments du serpent

    // représente la touche frappée par l'utilisateur : touche de direction ou
    // pour l'arrêt
    char touche = 'd';

    // direction courante du serpent (HAUT, BAS, GAUCHE ou DROITE)
    char direction1, direction2;

    // le plateau de jeu
    tPlateau lePlateau;

    bool collision = false;
    bool gagne = false;
    bool pommeMangee1 = false;
    bool pommeMangee2 = false;

    // compteur de pommes mangées
    int nbPommes1 = 0;

    int nbMvmt1 = 0;

    int nbPommes2 = 0;

    int nbMvmt2 = 0;

    int nbPommes = 0;
    // initialisation de listDirecta position du serpent : positionnement de la
    // tête en (X_INITIAL, Y_INITIAL), puis des anneaux à sa gauche
    for (int i = 0; i < TAILLE; i++) {
        lesX1[i] = X_INITIAL_1 - i;
        lesY1[i] = Y_INITIAL_1;
    }

    for (int i = 0; i < TAILLE; i++) {
        lesX2[i] = X_INITIAL_2 + i;
        lesY2[i] = Y_INITIAL_2;
    }

    // mise en place du plateau
    initPlateau(lePlateau);
    system("clear");
    dessinerPlateau(lePlateau);

    srand(time(NULL));
    ajouterPomme(lePlateau, nbPommes);

    // initialisation : le serpent se dirige vers la DROITE
    dessinerSerpent1(lesX1, lesY1);
    dessinerSerpent2(lesX2, lesY2);
    disable_echo();

    direction1 = 'd';
    direction2 = 'g';
    touche = 'd';
    //calcule des différentes distances que le serpent peut emprunter 
    //distance si le serpent prends le portails haut
    distHaut1 =
        (abs(portail[0][0] - lesX1[0]) + abs(portail[0][1] - lesY1[0])) +
        (abs(lesPommesX[0] - portail[1][0]) + abs(lesPommesY[0] - portail[1][1]));
    //distance si le serpent prends le portails bas
    distBas1 =
        (abs(portail[1][0] - lesX1[0]) + abs(portail[1][1] - lesY1[0])) +
        (abs(lesPommesX[0] - portail[0][0]) + abs(lesPommesY[0] - portail[0][1]));
    //distance si le serpent prends le portails gauche
    distGauche1 =
        (abs(portail[2][0] - lesX1[0]) + abs(portail[2][1] - lesY1[0])) +
        (abs(lesPommesX[0] - portail[3][0]) + abs(lesPommesY[0] - portail[3][1]));
    //distance si le serpent prends le postails droit
    distDroite1 =
        (abs(portail[3][0] - lesX1[0]) + abs(portail[3][1] - lesY1[0])) +
        (abs(lesPommesX[0] - portail[2][0]) + abs(lesPommesY[0] - portail[2][1]));
    //dist si le serpent prends la distance direct sans portail
    d1 = abs(lesPommesX[0] - lesX1[0]) + abs(lesPommesY[0] - lesY1[0]);
    //calcul de la distance minimal entre toute les possibilités
    distMini1 =
        fmin(d1, fmin(distDroite1, fmin(distGauche1, fmin(distHaut1, distBas1))));

    distHaut2 =
        (abs(portail[0][0] - lesX2[0]) + abs(portail[0][1] - lesY2[0])) +
        (abs(lesPommesX[0] - portail[1][0]) + abs(lesPommesY[0] - portail[1][1]));
    //distance si le serpent prends le portails bas
    distBas2 =
        (abs(portail[1][0] - lesX2[0]) + abs(portail[1][1] - lesY2[0])) +
        (abs(lesPommesX[0] - portail[0][0]) + abs(lesPommesY[0] - portail[0][1]));
    //distance si le serpent prends le portails gauche
    distGauche2 =
        (abs(portail[2][0] - lesX2[0]) + abs(portail[2][1] - lesY2[0])) +
        (abs(lesPommesX[0] - portail[3][0]) + abs(lesPommesY[0] - portail[3][1]));
    //distance si le serpent prends le postails droit
    distDroite2 =
        (abs(portail[3][0] - lesX2[0]) + abs(portail[3][1] - lesY2[0])) +
        (abs(lesPommesX[0] - portail[2][0]) + abs(lesPommesY[0] - portail[2][1]));
    //dist si le serpent prends la distance direct sans portail
    d2 = abs(lesPommesX[0] - lesX2[0]) + abs(lesPommesY[0] - lesY2[0]);
    //calcul de la distance minimal entre toute les possibilités
    distMini2 =
        fmin(d2, fmin(distDroite2, fmin(distGauche2, fmin(distHaut2, distBas2))));
    //recherche du chemin associé à la plus faible distance

    if (distMini1 == distHaut1) {
        chemin1 = 1;
    } else if (distMini1 == distBas1) {
        chemin1 = 2;
    } else if (distMini1 == distGauche1) {
        chemin1 = 3;
    } else if (distMini1 == distDroite1) {
        chemin1 = 4;
    } else if (distMini1 == d1) {
        chemin1 = 0;
    }

    if (distMini2 == distHaut2) {
        chemin2 = 1;
    } else if (distMini2 == distBas2) {
        chemin2 = 2;
    } else if (distMini2 == distGauche2) {
        chemin2 = 3;
    } else if (distMini2 == distDroite2) {
        chemin2 = 4;
    } else if (distMini2 == d2) {
        chemin2 = 0;
    }
    // boucle de jeu. Arret si touche STOP, si collision avec une bordure ou
    // si toutes les pommes sont mangées
    do {
        progresser1(lesX1, lesX2, lesY1, lesY2, direction1, lePlateau, &collision, &pommeMangee1,
                nbPommes, &nbMvmt1, &chemin1);
        progresser2(lesX2, lesX1, lesY2, lesY1, direction2, lePlateau, &collision, &pommeMangee2,
                nbPommes, &nbMvmt2, &chemin2);
        if (pommeMangee1) {
            nbPommes1++;
            nbPommes = nbPommes1 + nbPommes2;
            gagne = (nbPommes == NB_POMMES);
            if (!gagne) {
                ajouterPomme(lePlateau, nbPommes1 + nbPommes2); //ajout d'une pomme sur le terrain
                //recalcul de la meilleur distance avec la nouvel position de la pomme
                distHaut1 =
                    (abs(portail[0][0] - lesX1[0]) + abs(portail[0][1] - lesY1[0])) +
                    (abs(lesPommesX[nbPommes] - portail[1][0]) +
                    abs(lesPommesY[nbPommes] - portail[1][1]));
                distBas1 =
                    (abs(portail[1][0] - lesX1[0]) + abs(portail[1][1] - lesY1[0])) +
                    (abs(lesPommesX[nbPommes] - portail[0][0]) +
                    abs(lesPommesY[nbPommes] - portail[0][1]));
                distGauche1 =
                    (abs(portail[2][0] - lesX1[0]) + abs(portail[2][1] - lesY1[0])) +
                    (abs(lesPommesX[nbPommes] - portail[3][0]) +
                    abs(lesPommesY[nbPommes] - portail[3][1]));
                distDroite1 =
                    (abs(portail[3][0] - lesX1[0]) + abs(portail[3][1] - lesY1[0])) +
                    (abs(lesPommesX[nbPommes] - portail[2][0]) +
                    abs(lesPommesY[nbPommes] - portail[2][1]));
                d1 = abs(lesPommesX[nbPommes] - lesX1[0]) +
                    abs(lesPommesY[nbPommes] - lesY1[0]);
                //recalcul de la plus petite distance
                distMini1 = fmin(d1, fmin(distDroite1, fmin(distGauche1, fmin(distHaut1, distBas1))));

                if (distMini1 == distHaut1) {
                chemin1 = 1;
                } else if (distMini1 == distBas1) {
                chemin1 = 2;
                } else if (distMini1 == distGauche1) {
                chemin1 = 3;
                } else if (distMini1 == distDroite1) {
                chemin1 = 4;
                } else if (distMini1 == d1) {
                chemin1 = 0;
                }

                distHaut2 =
                    (abs(portail[0][0] - lesX2[0]) + abs(portail[0][1] - lesY2[0])) +
                    (abs(lesPommesX[nbPommes] - portail[1][0]) +
                    abs(lesPommesY[nbPommes] - portail[1][1]));
                distBas2 =
                    (abs(portail[1][0] - lesX2[0]) + abs(portail[1][1] - lesY2[0])) +
                    (abs(lesPommesX[nbPommes] - portail[0][0]) +
                    abs(lesPommesY[nbPommes] - portail[0][1]));
                distGauche2 =
                    (abs(portail[2][0] - lesX2[0]) + abs(portail[2][1] - lesY2[0])) +
                    (abs(lesPommesX[nbPommes] - portail[3][0]) +
                    abs(lesPommesY[nbPommes] - portail[3][1]));
                distDroite2 =
                    (abs(portail[3][0] - lesX2[0]) + abs(portail[3][1] - lesY2[0])) +
                    (abs(lesPommesX[nbPommes] - portail[2][0]) +
                    abs(lesPommesY[nbPommes] - portail[2][1]));
                d2 = abs(lesPommesX[nbPommes] - lesX2[0]) +
                    abs(lesPommesY[nbPommes] - lesY2[0]);
                //recalcul de la plus petite distance
                distMini2 = fmin(d2, fmin(distDroite2, fmin(distGauche2, fmin(distHaut2, distBas2))));

                if (distMini2 == distHaut2) {
                chemin2 = 1;
                } else if (distMini2 == distBas2) {
                chemin2 = 2;
                } else if (distMini2 == distGauche2) {
                chemin2 = 3;
                } else if (distMini2 == distDroite2) {
                chemin2 = 4;
                } else if (distMini2 == d2) {
                chemin2 = 0;
                }
                pommeMangee1 = false;
            }
        }

        if (pommeMangee2) {
            nbPommes2++;
            nbPommes = nbPommes1 + nbPommes2;
            gagne = (nbPommes == NB_POMMES);
            if (!gagne) {
                ajouterPomme(lePlateau, nbPommes); //ajout d'une pomme sur le terrain
                //recalcul de la meilleur distance avec la nouvel position de la pomme
                distHaut2 =
                    (abs(portail[0][0] - lesX2[0]) + abs(portail[0][1] - lesY2[0])) +
                    (abs(lesPommesX[nbPommes] - portail[1][0]) +
                    abs(lesPommesY[nbPommes] - portail[1][1]));
                distBas2 =
                    (abs(portail[1][0] - lesX2[0]) + abs(portail[1][1] - lesY2[0])) +
                    (abs(lesPommesX[nbPommes] - portail[0][0]) +
                    abs(lesPommesY[nbPommes] - portail[0][1]));
                distGauche2 =
                    (abs(portail[2][0] - lesX2[0]) + abs(portail[2][1] - lesY2[0])) +
                    (abs(lesPommesX[nbPommes] - portail[3][0]) +
                    abs(lesPommesY[nbPommes] - portail[3][1]));
                distDroite2 =
                    (abs(portail[3][0] - lesX2[0]) + abs(portail[3][1] - lesY2[0])) +
                    (abs(lesPommesX[nbPommes] - portail[2][0]) +
                    abs(lesPommesY[nbPommes] - portail[2][1]));
                d2 = abs(lesPommesX[nbPommes] - lesX2[0]) +
                    abs(lesPommesY[nbPommes] - lesY2[0]);
                //recalcul de la plus petite distance
                distMini2 = fmin(d2, fmin(distDroite2, fmin(distGauche2, fmin(distHaut2, distBas2))));

                if (distMini2 == distHaut2) {
                chemin2 = 1;
                } else if (distMini2 == distBas2) {
                chemin2 = 2;
                } else if (distMini2 == distGauche2) {
                chemin2 = 3;
                } else if (distMini2 == distDroite2) {
                chemin2 = 4;
                } else if (distMini2 == d2) {
                chemin2 = 0;
                }
                distHaut1 =
                    (abs(portail[0][0] - lesX1[0]) + abs(portail[0][1] - lesY1[0])) +
                    (abs(lesPommesX[nbPommes] - portail[1][0]) +
                    abs(lesPommesY[nbPommes] - portail[1][1]));
                distBas1 =
                    (abs(portail[1][0] - lesX1[0]) + abs(portail[1][1] - lesY1[0])) +
                    (abs(lesPommesX[nbPommes] - portail[0][0]) +
                    abs(lesPommesY[nbPommes] - portail[0][1]));
                distGauche1 =
                    (abs(portail[2][0] - lesX1[0]) + abs(portail[2][1] - lesY1[0])) +
                    (abs(lesPommesX[nbPommes] - portail[3][0]) +
                    abs(lesPommesY[nbPommes] - portail[3][1]));
                distDroite1 =
                    (abs(portail[3][0] - lesX1[0]) + abs(portail[3][1] - lesY1[0])) +
                    (abs(lesPommesX[nbPommes] - portail[2][0]) +
                    abs(lesPommesY[nbPommes] - portail[2][1]));
                d1 = abs(lesPommesX[nbPommes] - lesX1[0]) +
                    abs(lesPommesY[nbPommes] - lesY1[0]);
                //recalcul de la plus petite distance
                distMini1 = fmin(d1, fmin(distDroite1, fmin(distGauche1, fmin(distHaut1, distBas1))));

                if (distMini1 == distHaut1) {
                chemin1 = 1;
                } else if (distMini1 == distBas1) {
                chemin1 = 2;
                } else if (distMini1 == distGauche1) {
                chemin1 = 3;
                } else if (distMini1 == distDroite1) {
                chemin1 = 4;
                } else if (distMini1 == d1) {
                chemin1 = 0;
                }
                pommeMangee2 = false;
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
    printf("Nombres de mouvements du serpent 1: %d\n", nbMvmt1);
    printf("Nombres de mouvements du serpent 2: %d\n", nbMvmt2);
    printf("Nombres de pommes mangé par le serpent 1: %d\n", nbPommes1);
    printf("Nombres de pommes mangé par le serpent 2: %d\n", nbPommes2);
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

void dessinerSerpent1(int lesX[], int lesY[]) {
    // affiche les anneaux puis la tête
    for (int i = 1; i < TAILLE; i++) {
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0], TETE1);
}

void dessinerSerpent2(int lesX[], int lesY[]) {
    // affiche les anneaux puis la tête
    for (int i = 1; i < TAILLE; i++) {
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0], TETE2);
}

void progresser1(int lesX[],int lesX2[] , int lesY[], int lesY2[], char direction, tPlateau plateau,
                bool *collision, bool *pomme, int numPomme, int *nbMvmt,
                int *ch) {
    // efface le dernier élément avant d'actualiser la position de tous les
    // élémentds du serpent avant de le  redessiner et détecte une
    // collision avec une pomme ou avec une bordure
    if(lesX[TAILLE - 1] != lesX2[0] || lesY[TAILLE - 1] != lesY2[0]){
        effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);
    }

    int dx = lesPommesX[numPomme] - lesX[0]; // distance tete du serpent-pomme en// x
    int dy = lesPommesY[numPomme] - lesY[0]; // distance tete du serpent-pomme en// y
    bool valide = true; // booléen qui valide la prochaine position du serpent
    bool porte = false; // booléen qui valide le passage d'un portail
    bool gauche = true;
    bool haut = true;
    bool bas = true; // booléen qui valide la prochaine position du serpent
    bool droit = true;
    int numPorte = 0;   // indice du numéros de la porte
    int indice = 1;     // indice du parcour de tout les morceaux du serpent
    int indice2 = 0;
    int i = 1;
    int distA = 0;
    int distB = 0;

    // incrémentation de tout les éléments du corps du serpent
    for (int i = TAILLE - 1; i > 0; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }

    //////////////////////////////////////////////////////////////////
    ////////////////CHEMIN VERS LE PORTAIL HAUT///////////////////////
    //////////////////////////////////////////////////////////////////

    if (*ch == 1) {
        dx = portail[0][0] - lesX[0]; // calcul de la distance tete-portail du haut en x
        dy = portail[0][1] - lesY[0]; // calcul de la distance tet-portail du haut en y
        // condition pour savoir si la position x est correcte
        if (abs(dx) != 0) {
            // condition si la distance x est supérieur à 0
            if (dx > 0) {
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide si le serpent se rentre dedans
                    }
                    indice++; // incrémentation de l'indice
                    indice2++;
                }
                // position suivante validé
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                        // analyse de la distance vers le bas
                        while (plateau[lesX[0] + 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance vers le haut
                        while (plateau[lesX[0] + 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le bas
                            if (valide) {
                                lesY[0] += 1;
                            }
                            // poition invalide déplacement vers le haut
                            else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le haut
                            if (valide) {
                                lesY[0] -= 1;
                            }
                            // position invalide déplacement vers le bas
                            else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si le serpent rencontre son corps
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(bas){
                        lesY[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                } 
            }
            // si distence x est inférieur à 0
            else {
                indice = 1;
                indice2 = 0;
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide sile serpent se rentre dedans
                    }
                    indice++;
                    indice2++;
                }
                // si la position est valide
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] - 1][lesY[0]] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] - 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] += 1;
                            } 
                            else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] -= 1;
                            } 
                            else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // le serpent rencontre une partie de son corps
                else {
                    indice = 1;
                    droit = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(droit && indice < 10){
                        if((plateau[lesX[0]+1][lesY[0]] == BORDURE) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(bas){
                        lesY[0]++;
                    }

                    else if(haut){
                        lesY[0]--;
                    }
                }
            }
        }
        // si la position x du serpent est correcte pour rentrer dans le portail
        else {
            indice = 1;
            indice2 = 0;
            while (valide && indice < 10) {
                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                    valide = false;
                }
                indice++;
                indice2++;
            }
            if (valide == true) {
                // test si la prochine case est un mur ou pas
                if (plateau[lesX[0]][lesY[0] - 1] == BORDURE) {
                    // analise de la distance droite
                    while (plateau[lesX[0] + i][lesY[0] - 1] == BORDURE) {
                        distA += 1;
                        i++;
                    }
                    i = 1;
                    // analise de la distance gauche
                    while (plateau[lesX[0] - i][lesY[0] - 1] == BORDURE) {
                        distB += 1;
                        i++;
                    }
                    // choix de la distance la plus courte
                    if (distA <= distB) {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesX[0] += 1;
                        } 
                        else {
                            lesX[0] -= 1;
                        }
                        (*nbMvmt)++;
                    } 
                    else {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesX[0] -= 1;
                        } else {
                            lesX[0] += 1;
                        }
                        (*nbMvmt)++;
                    }
                } 
                else {
                    lesY[0] -= 1; // déplacement vers le haut
                    (*nbMvmt)++;  // incrémentation du nombre de mouvement
                }
            }
            // si la position est invalide
            else {
                indice = 1;
                gauche = true;
                bas = true;
                droit = true;
                // verification de la prochaine position du serpent
                while (droit && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                        droit = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(bas && indice < 10){
                    if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                        bas = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(gauche && indice < 10){
                    if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                        gauche = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                if(droit){
                    lesX[0]++;
                }
                else if(gauche){
                    lesX[0]--;
                }
                else if(bas){
                    lesY[0]++;
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
            // condition si la distance x est supérieur à 0
            if (dx > 0) {
                indice = 1;
                indice = 0;
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide si le serpent se rentre dedans
                    }
                    indice++; // incrémentation de l'indice
                    indice2++;
                }
                // position suivante validé
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                        // analyse de la distance vers le bas
                        while (plateau[lesX[0] + 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance vers le haut
                        while (plateau[lesX[0] + 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le bas
                            if (valide) {
                                lesY[0] += 1;
                            }
                            // poition invalide déplacement vers le haut
                            else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le haut
                            if (valide) {
                                lesY[0] -= 1;
                            }
                            // position invalide déplacement vers le bas
                            else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si le serpent rencontre son corps
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(bas){
                        lesY[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            }
            // si distence x est inférieur à 0
            else {
                indice = 1;
                indice2 = 0;
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide sile serpent se rentre dedans
                    }
                    indice++;
                    indice2++;
                }
                // si la position est valide
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] - 1][lesY[0]] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] - 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] += 1;
                            } else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] -= 1;
                            } else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // le serpent rencontre une partie de son corps
                else {
                    indice = 1;
                    droit = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(droit && indice < 10){
                        if((plateau[lesX[0]+1][lesY[0]] == BORDURE) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(bas){
                        lesY[0]++;
                    }

                    else if(haut){
                        lesY[0]--;
                    }
                }
            }   
        }
        // si la position x du serpent est correcte pour rentrer dans le portail
        else {
            indice = 1;
            indice2 = 0;
            while (valide && indice < 10) {
                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                    valide = false;
                }
                indice++;
                indice2++;
            }
            if (valide == true) {
                // test si la prochine case est un mur ou pas
                if (plateau[lesX[0]][lesY[0] - 1] == BORDURE) {
                    // analise de la distance droite
                    while (plateau[lesX[0] + i][lesY[0] - 1] == BORDURE) {
                        distA += 1;
                        i++;
                    }
                    i = 1;
                    // analise de la distance gauche
                    while (plateau[lesX[0] - i][lesY[0] - 1] == BORDURE) {
                        distB += 1;
                        i++;
                    }
                    // choix de la distance la plus courte
                    if (distA <= distB) {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesX[0] += 1;
                        } 
                        else {
                            lesX[0] -= 1;
                        }
                        (*nbMvmt)++;
                    } 
                    else {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesX[0] -= 1;
                        } else {
                            lesX[0] += 1;
                        }
                        (*nbMvmt)++;
                    }
                } 
                else {
                    lesY[0] -= 1; // déplacement vers le haut
                    (*nbMvmt)++;  // incrémentation du nombre de mouvement
                }
            }
            // si la position est invalide
            else {
                indice = 1;
                gauche = true;
                haut = true;
                droit = true;
                // verification de la prochaine position du serpent
                while (droit && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                        droit = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(haut && indice < 10){
                    if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                        haut = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(gauche && indice < 10){
                    if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                        gauche = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                if(droit){
                    lesX[0]++;
                }
                else if(gauche){
                    lesX[0]--;
                }
                else if(haut){
                    lesY[0]--;
                }
            }
        }
    }

//////////////////////////////////////////////////////////////////
////////////////CHEMIN VERS LE PORTAIL DROIT//////////////////////
//////////////////////////////////////////////////////////////////

    else if (*ch == 3) {
        dx = portail[3][0] - lesX[0];
        dy = portail[3][1] - lesY[0];
        if (abs(dy) != 0) {
            if (dy > 0) {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] + 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] + 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] + 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesY[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si la position est invalide
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    droit = true;
                    // verification de la prochaine position du serpent
                    while (droit && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                            droit = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            } 
            else {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] - 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] - 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] - 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesY[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                } 
                else {
                    indice = 1;
                    gauche = true;
                    bas = true;
                    droit = true;
                    // verification de la prochaine position du serpent
                    while (droit && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                            droit = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(bas && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(bas){
                        lesY[0]++;
                    }
                }
            }
        } 
        else {
            indice = 1;
            indice2 = 0;
            while (valide && indice < 10) {
                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                    valide = false;
                }
                indice++;
                indice2++;
            }
            if (valide == true) {
                // test si la prochine case est un mur ou pas
                if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                    // analise de la distance bas
                    while (plateau[lesX[0] + 1][lesY[0] + i] == BORDURE) {
                        distA += 1;
                        i++;
                    }
                    i = 1;
                    // analise de la distance haut
                    while (plateau[lesX[0] + 1][lesY[0] - i] == BORDURE) {
                        distB += 1;
                        i++;
                    }
                    // choix de la distance la plus courte
                    if (distA <= distB) {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesY[0] += 1;
                        } 
                        else {
                            lesY[0] -= 1;
                        }
                        (*nbMvmt)++;
                    } 
                    else {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesY[0] -= 1;
                        } else {
                            lesY[0] += 1;
                        }
                        (*nbMvmt)++;
                    }
                }
                else{
                    lesX[0] +=1;
                } 
            }            
            else {
                indice = 1;
                gauche = true;
                haut = true;
                bas = true;
                // verification de la prochaine position du serpent
                while (bas && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                        bas = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(haut && indice < 10){
                    if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                        haut = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(gauche && indice < 10){
                    if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                        gauche = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                if(bas){
                    lesY[0]++;
                }
                else if(gauche){
                    lesX[0]--;
                }
                else if(haut){
                    lesY[0]--;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////
    ////////////////CHEMIN VERS LE PORTAIL GAUCHE/////////////////////
    //////////////////////////////////////////////////////////////////

    else if (*ch == 4) {
        dx = portail[2][0] - lesX[0];
        dy = portail[2][1] - lesY[0];
        if (abs(dy) != 0) {
            if (dy > 0) {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] + 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] + 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] + 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesY[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si la position est invalide
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    droit = true;
                    // verification de la prochaine position du serpent
                    while (droit && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                            droit = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            } 
            else {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] - 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] - 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] - 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } 
                            else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } 
                            else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesY[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                } 
                else {
                    indice = 1;
                    droit = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(droit && indice < 10){
                        if((plateau[lesX[0]+1][lesY[0]] == BORDURE) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(bas){
                        lesY[0]++;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            }
        } 
        else {
            indice = 1;
            indice2 = 0;
            while (valide && indice < 10) {
                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                valide = false;
                }
                indice++;
                indice2++;
            }
            if (valide == true) {
                // test si la prochine case est un mur ou pas
                if (plateau[lesX[0] - 1][lesY[0]] == BORDURE) {
                    // analise de la distance bas
                    while (plateau[lesX[0] + 1][lesY[0] + i] == BORDURE) {
                        distA += 1;
                        i++;
                    }
                    i = 1;
                    // analise de la distance haut
                    while (plateau[lesX[0] - 1][lesY[0] - i] == BORDURE) {
                        distB += 1;
                        i++;
                    }
                    // choix de la distance la plus courte
                    if (distA <= distB) {
                        valide = true;
                        indice = 1;
                        indice = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesY[0] += 1;
                        } else {
                            lesY[0] -= 1;
                        }
                        (*nbMvmt)++;
                    } 
                    else {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesY[0] -= 1;
                        } else {
                            lesY[0] += 1;
                        }
                        (*nbMvmt)++;
                    }
                } 
                else {
                    lesX[0] -= 1; 
                }
            }
            else {
                indice = 1;
                droit = true;
                haut = true;
                bas = true;
                // verification de la prochaine position du serpent
                while (bas && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                        bas = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(haut && indice < 10){
                    if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                        haut = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(droit && indice < 10){
                    if((plateau[lesX[0]+1][lesY[0]] == BORDURE) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))){
                        gauche = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                if(droit){
                    lesX[0]++;
                }
                else if(bas){
                    lesY[0]++;
                }
                else if(haut){
                    lesY[0]--;
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
        if (abs(dx) > abs(dy)) {
            if (dx > 0) {
                indice = 1;
                indice2 = 0;
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide si le serpent se rentre dedans
                    }
                    indice++; // incrémentation de l'indice
                    indice2++;
                }
                // position suivante validé
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                        // analyse de la distance vers le bas
                        while (plateau[lesX[0] + 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance vers le haut
                        while (plateau[lesX[0] + 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le bas
                            if (valide) {
                                lesY[0] += 1;
                            }
                            // poition invalide déplacement vers le haut
                            else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        }
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le haut
                            if (valide) {
                                lesY[0] -= 1;
                            }
                            // position invalide déplacement vers le bas
                            else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si le serpent rencontre son corps
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(bas){
                        lesY[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            }
            // si distence x est inférieur à 0
            else {
                indice = 1;
                indice2 = 0;
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide sile serpent se rentre dedans
                    }
                    indice++;
                    indice2++;
                }
                // si la position est valide
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] - 1][lesY[0]] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] - 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] += 1;
                            } else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        }
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] -= 1;
                            } else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // le serpent rencontre une partie de son corps
                else {
                    indice = 1;
                    droit = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(droit && indice < 10){
                        if((plateau[lesX[0]+1][lesY[0]] == BORDURE) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(bas){
                        lesY[0]++;
                    }
                    else if(droit){
                        lesX[0]++;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            }
        } 
        else {
            if (dy > 0) {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] + 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] + 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] + 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesY[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si la position est invalide
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    droit = true;
                    // verification de la prochaine position du serpent
                    while (droit && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                            droit = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            } 
            else {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] - 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] - 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] - 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))){
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                            lesY[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                else {
                    indice = 1;
                    gauche = true;
                    bas = true;
                    droit = true;
                    // verification de la prochaine position du serpent
                    while (droit && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                            droit = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(bas && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(bas){
                        lesY[0]++;
                    }
                }
            }
        }
    }

    // parcour des diiférente position des portes
    // test si le serpent est dans l'une d'elle

    while (!porte && numPorte < 4) {
        if (lesX[0] == portail[numPorte][0] && lesY[0] == portail[numPorte][1]) {
        porte = true;
        if (*ch == 1) {
            lesX[0] = portail[numPorte + 1][0];
            lesY[0] = portail[numPorte + 1][1] - 1;
        } else if (*ch == 2) {
            lesX[0] = portail[numPorte - 1][0];
            lesY[0] = portail[numPorte - 1][1] + 1;
        } else if (*ch == 3) {
            lesX[0] = portail[numPorte + 1][0] + 1;
            lesY[0] = portail[numPorte + 1][1];
        } else if (*ch == 4) {
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

    dessinerSerpent1(lesX, lesY);
}


void progresser2(int lesX[],int lesX2[] , int lesY[], int lesY2[], char direction, tPlateau plateau,
                bool *collision, bool *pomme, int numPomme, int *nbMvmt,
                int *ch) {
    // efface le dernier élément avant d'actualiser la position de tous les
    // élémentds du serpent avant de le  redessiner et détecte une
    // collision avec une pomme ou avec une bordure
    if(lesX[TAILLE - 1] != lesX2[0] || lesY[TAILLE - 1] != lesY2[0]){
        effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);
    }

    int dx = lesPommesX[numPomme] - lesX[0]; // distance tete du serpent-pomme en// x
    int dy = lesPommesY[numPomme] - lesY[0]; // distance tete du serpent-pomme en// y
    bool valide = true; // booléen qui valide la prochaine position du serpent
    bool porte = false; // booléen qui valide le passage d'un portail
    bool gauche = true;
    bool haut = true;
    bool bas = true; // booléen qui valide la prochaine position du serpent
    bool droit = true;
    int numPorte = 0;   // indice du numéros de la porte
    int indice = 1;     // indice du parcour de tout les morceaux du serpent
    int indice2 = 0;
    int i = 1;
    int distA = 0;
    int distB = 0;

    // incrémentation de tout les éléments du corps du serpent
    for (int i = TAILLE - 1; i > 0; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }

    //////////////////////////////////////////////////////////////////
    ////////////////CHEMIN VERS LE PORTAIL HAUT///////////////////////
    //////////////////////////////////////////////////////////////////

    if (*ch == 1) {
        dx = portail[0][0] - lesX[0]; // calcul de la distance tete-portail du haut en x
        dy = portail[0][1] - lesY[0]; // calcul de la distance tet-portail du haut en y
        // condition pour savoir si la position x est correcte
        if (abs(dx) != 0) {
            // condition si la distance x est supérieur à 0
            if (dx > 0) {
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide si le serpent se rentre dedans
                    }
                    indice++; // incrémentation de l'indice
                    indice2++;
                }
                // position suivante validé
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                        // analyse de la distance vers le bas
                        while (plateau[lesX[0] + 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance vers le haut
                        while (plateau[lesX[0] + 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le bas
                            if (valide) {
                                lesY[0] += 1;
                            }
                            // poition invalide déplacement vers le haut
                            else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le haut
                            if (valide) {
                                lesY[0] -= 1;
                            }
                            // position invalide déplacement vers le bas
                            else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si le serpent rencontre son corps
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(bas){
                        lesY[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                } 
            }
            // si distence x est inférieur à 0
            else {
                indice = 1;
                indice2 = 0;
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide sile serpent se rentre dedans
                    }
                    indice++;
                    indice2++;
                }
                // si la position est valide
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] - 1][lesY[0]] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] - 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] += 1;
                            } 
                            else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] -= 1;
                            } 
                            else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // le serpent rencontre une partie de son corps
                else {
                    indice = 1;
                    droit = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(droit && indice < 10){
                        if((plateau[lesX[0]+1][lesY[0]] == BORDURE) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(bas){
                        lesY[0]++;
                    }

                    else if(haut){
                        lesY[0]--;
                    }
                }
            }
        }
        // si la position x du serpent est correcte pour rentrer dans le portail
        else {
            indice = 1;
            indice2 = 0;
            while (valide && indice < 10) {
                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                    valide = false;
                }
                indice++;
                indice2++;
            }
            if (valide == true) {
                // test si la prochine case est un mur ou pas
                if (plateau[lesX[0]][lesY[0] - 1] == BORDURE) {
                    // analise de la distance droite
                    while (plateau[lesX[0] + i][lesY[0] - 1] == BORDURE) {
                        distA += 1;
                        i++;
                    }
                    i = 1;
                    // analise de la distance gauche
                    while (plateau[lesX[0] - i][lesY[0] - 1] == BORDURE) {
                        distB += 1;
                        i++;
                    }
                    // choix de la distance la plus courte
                    if (distA <= distB) {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesX[0] += 1;
                        } 
                        else {
                            lesX[0] -= 1;
                        }
                        (*nbMvmt)++;
                    } 
                    else {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesX[0] -= 1;
                        } else {
                            lesX[0] += 1;
                        }
                        (*nbMvmt)++;
                    }
                } 
                else {
                    lesY[0] -= 1; // déplacement vers le haut
                    (*nbMvmt)++;  // incrémentation du nombre de mouvement
                }
            }
            // si la position est invalide
            else {
                indice = 1;
                gauche = true;
                bas = true;
                droit = true;
                // verification de la prochaine position du serpent
                while (droit && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                        droit = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(bas && indice < 10){
                    if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                        bas = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(gauche && indice < 10){
                    if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                        gauche = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                if(droit){
                    lesX[0]++;
                }
                else if(gauche){
                    lesX[0]--;
                }
                else if(bas){
                    lesY[0]++;
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
            // condition si la distance x est supérieur à 0
            if (dx > 0) {
                indice = 1;
                indice = 0;
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide si le serpent se rentre dedans
                    }
                    indice++; // incrémentation de l'indice
                    indice2++;
                }
                // position suivante validé
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                        // analyse de la distance vers le bas
                        while (plateau[lesX[0] + 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance vers le haut
                        while (plateau[lesX[0] + 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le bas
                            if (valide) {
                                lesY[0] += 1;
                            }
                            // poition invalide déplacement vers le haut
                            else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le haut
                            if (valide) {
                                lesY[0] -= 1;
                            }
                            // position invalide déplacement vers le bas
                            else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si le serpent rencontre son corps
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(bas){
                        lesY[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            }
            // si distence x est inférieur à 0
            else {
                indice = 1;
                indice2 = 0;
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide sile serpent se rentre dedans
                    }
                    indice++;
                    indice2++;
                }
                // si la position est valide
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] - 1][lesY[0]] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] - 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] += 1;
                            } else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] -= 1;
                            } else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // le serpent rencontre une partie de son corps
                else {
                    indice = 1;
                    droit = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(droit && indice < 10){
                        if((plateau[lesX[0]+1][lesY[0]] == BORDURE) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(bas){
                        lesY[0]++;
                    }

                    else if(haut){
                        lesY[0]--;
                    }
                }
            }   
        }
        // si la position x du serpent est correcte pour rentrer dans le portail
        else {
            indice = 1;
            indice2 = 0;
            while (valide && indice < 10) {
                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                    valide = false;
                }
                indice++;
                indice2++;
            }
            if (valide == true) {
                // test si la prochine case est un mur ou pas
                if (plateau[lesX[0]][lesY[0] - 1] == BORDURE) {
                    // analise de la distance droite
                    while (plateau[lesX[0] + i][lesY[0] - 1] == BORDURE) {
                        distA += 1;
                        i++;
                    }
                    i = 1;
                    // analise de la distance gauche
                    while (plateau[lesX[0] - i][lesY[0] - 1] == BORDURE) {
                        distB += 1;
                        i++;
                    }
                    // choix de la distance la plus courte
                    if (distA <= distB) {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesX[0] += 1;
                        } 
                        else {
                            lesX[0] -= 1;
                        }
                        (*nbMvmt)++;
                    } 
                    else {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesX[0] -= 1;
                        } else {
                            lesX[0] += 1;
                        }
                        (*nbMvmt)++;
                    }
                } 
                else {
                    lesY[0] -= 1; // déplacement vers le haut
                    (*nbMvmt)++;  // incrémentation du nombre de mouvement
                }
            }
            // si la position est invalide
            else {
                indice = 1;
                gauche = true;
                haut = true;
                droit = true;
                // verification de la prochaine position du serpent
                while (droit && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                        droit = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(haut && indice < 10){
                    if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                        haut = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(gauche && indice < 10){
                    if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                        gauche = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                if(droit){
                    lesX[0]++;
                }
                else if(gauche){
                    lesX[0]--;
                }
                else if(haut){
                    lesY[0]--;
                }
            }
        }
    }

//////////////////////////////////////////////////////////////////
////////////////CHEMIN VERS LE PORTAIL DROIT//////////////////////
//////////////////////////////////////////////////////////////////

    else if (*ch == 3) {
        dx = portail[3][0] - lesX[0];
        dy = portail[3][1] - lesY[0];
        if (abs(dy) != 0) {
            if (dy > 0) {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] + 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] + 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] + 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesY[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si la position est invalide
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    droit = true;
                    // verification de la prochaine position du serpent
                    while (droit && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                            droit = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(gauche){
                        lesX[0]--;
                    }
                    else if(droit){
                        lesX[0]++;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            } 
            else {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] - 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] - 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] - 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesY[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                } 
                else {
                    indice = 1;
                    gauche = true;
                    bas = true;
                    droit = true;
                    // verification de la prochaine position du serpent
                    while (droit && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                            droit = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(bas && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(bas){
                        lesY[0]++;
                    }
                }
            }
        } 
        else {
            indice = 1;
            indice2 = 0;
            while (valide && indice < 10) {
                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                    valide = false;
                }
                indice++;
                indice2++;
            }
            if (valide == true) {
                // test si la prochine case est un mur ou pas
                if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                    // analise de la distance bas
                    while (plateau[lesX[0] + 1][lesY[0] + i] == BORDURE) {
                        distA += 1;
                        i++;
                    }
                    i = 1;
                    // analise de la distance haut
                    while (plateau[lesX[0] + 1][lesY[0] - i] == BORDURE) {
                        distB += 1;
                        i++;
                    }
                    // choix de la distance la plus courte
                    if (distA <= distB) {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesY[0] += 1;
                        } 
                        else {
                            lesY[0] -= 1;
                        }
                        (*nbMvmt)++;
                    } 
                    else {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesY[0] -= 1;
                        } else {
                            lesY[0] += 1;
                        }
                        (*nbMvmt)++;
                    }
                }
                else{
                    lesX[0] +=1;
                } 
            }            
            else {
                indice = 1;
                gauche = true;
                haut = true;
                bas = true;
                // verification de la prochaine position du serpent
                while (bas && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                        bas = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(haut && indice < 10){
                    if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                        haut = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(gauche && indice < 10){
                    if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                        gauche = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                if(bas){
                    lesY[0]++;
                }
                else if(gauche){
                    lesX[0]--;
                }
                else if(haut){
                    lesY[0]--;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////
    ////////////////CHEMIN VERS LE PORTAIL GAUCHE/////////////////////
    //////////////////////////////////////////////////////////////////

    else if (*ch == 4) {
        dx = portail[2][0] - lesX[0];
        dy = portail[2][1] - lesY[0];
        if (abs(dy) != 0) {
            if (dy > 0) {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] + 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] + 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] + 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesY[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si la position est invalide
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    droit = true;
                    // verification de la prochaine position du serpent
                    while (droit && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                            droit = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            } 
            else {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] - 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] - 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] - 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } 
                            else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } 
                            else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesY[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                } 
                else {
                    indice = 1;
                    droit = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(droit && indice < 10){
                        if((plateau[lesX[0]+1][lesY[0]] == BORDURE) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(bas){
                        lesY[0]++;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            }
        } 
        else {
            indice = 1;
            indice2 = 0;
            while (valide && indice < 10) {
                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                valide = false;
                }
                indice++;
                indice2++;
            }
            if (valide == true) {
                // test si la prochine case est un mur ou pas
                if (plateau[lesX[0] - 1][lesY[0]] == BORDURE) {
                    // analise de la distance bas
                    while (plateau[lesX[0] + 1][lesY[0] + i] == BORDURE) {
                        distA += 1;
                        i++;
                    }
                    i = 1;
                    // analise de la distance haut
                    while (plateau[lesX[0] - 1][lesY[0] - i] == BORDURE) {
                        distB += 1;
                        i++;
                    }
                    // choix de la distance la plus courte
                    if (distA <= distB) {
                        valide = true;
                        indice = 1;
                        indice = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesY[0] += 1;
                        } else {
                            lesY[0] -= 1;
                        }
                        (*nbMvmt)++;
                    } 
                    else {
                        valide = true;
                        indice = 1;
                        indice2 = 0;
                        while (valide && indice < 10) {
                            if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                valide = false;
                            }
                            indice++;
                            indice2++;
                        }
                        if (valide) {
                            lesY[0] -= 1;
                        } else {
                            lesY[0] += 1;
                        }
                        (*nbMvmt)++;
                    }
                } 
                else {
                    lesX[0] -= 1; 
                }
            }
            else {
                indice = 1;
                droit = true;
                haut = true;
                bas = true;
                // verification de la prochaine position du serpent
                while (bas && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                        bas = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(haut && indice < 10){
                    if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                        haut = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                indice = 1;
                while(droit && indice < 10){
                    if((plateau[lesX[0]+1][lesY[0]] == BORDURE) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))){
                        gauche = false;
                    }
                    indice++; // incrémentation de l'indice
                }
                if(droit){
                    lesX[0]++;
                }
                else if(bas){
                    lesY[0]++;
                }
                else if(haut){
                    lesY[0]--;
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
        if (abs(dx) > abs(dy)) {
            if (dx > 0) {
                indice = 1;
                indice2 = 0;
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide si le serpent se rentre dedans
                    }
                    indice++; // incrémentation de l'indice
                    indice2++;
                }
                // position suivante validé
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] + 1][lesY[0]] == BORDURE) {
                        // analyse de la distance vers le bas
                        while (plateau[lesX[0] + 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance vers le haut
                        while (plateau[lesX[0] + 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le bas
                            if (valide) {
                                lesY[0] += 1;
                            }
                            // poition invalide déplacement vers le haut
                            else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        }
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            // test du contact avec un élément de corps du serpent
                            while (valide && indice < 10) {
                                // test du contact tete-corps du serpent
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++; // incrémentation de l'indice
                                indice2++;
                            }
                            // position valide déplacemnt vers le haut
                            if (valide) {
                                lesY[0] -= 1;
                            }
                            // position invalide déplacement vers le bas
                            else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si le serpent rencontre son corps
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(bas){
                        lesY[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            }
            // si distence x est inférieur à 0
            else {
                indice = 1;
                indice2 = 0;
                // verification de la prochaine position du serpent
                while (valide && indice < 10) {
                    // test du contact tete-corps du serpent
                    if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                        valide = false; // position invalide sile serpent se rentre dedans
                    }
                    indice++;
                    indice2++;
                }
                // si la position est valide
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0] - 1][lesY[0]] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] - 1][lesY[0] + i] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - 1][lesY[0] - i] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                                    valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] += 1;
                            } else {
                                lesY[0] -= 1;
                            }
                            (*nbMvmt)++;
                        }
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesY[0] -= 1;
                            } else {
                                lesY[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesX[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // le serpent rencontre une partie de son corps
                else {
                    indice = 1;
                    droit = true;
                    haut = true;
                    bas = true;
                    // verification de la prochaine position du serpent
                    while (bas && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]][lesY[0]+1] == BORDURE)) || ((lesX[0] == lesX[indice] && lesY[0]+1 == lesY[indice]))) {
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(droit && indice < 10){
                        if((plateau[lesX[0]+1][lesY[0]] == BORDURE) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(bas){
                        lesY[0]++;
                    }
                    else if(droit){
                        lesX[0]++;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            }
        } 
        else {
            if (dy > 0) {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] + 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] + 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] + 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] + 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] + 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                        lesY[0] += 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                // si la position est invalide
                else {
                    indice = 1;
                    gauche = true;
                    haut = true;
                    droit = true;
                    // verification de la prochaine position du serpent
                    while (droit && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                            droit = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(haut && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            haut = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(haut){
                        lesY[0]--;
                    }
                }
            } 
            else {
                indice = 1;
                indice2 = 0;
                while (valide && indice < 10) {
                    if (((lesX[0] == lesX[indice]) && (lesY[0] - 1 == lesY[indice])) || ((lesX[0] == lesX2[indice2]) && (lesY[0] - 1 == lesY2[indice2]))) {
                        valide = false;
                    }
                    indice++;
                    indice2++;
                }
                if (valide == true) {
                    // test si la prochine case est un mur ou pas
                    if (plateau[lesX[0]][lesY[0] - 1] == BORDURE) {
                        // analise de la distance bas
                        while (plateau[lesX[0] + i][lesY[0] - 1] == BORDURE) {
                            distA += 1;
                            i++;
                        }
                        i = 1;
                        // analise de la distance haut
                        while (plateau[lesX[0] - i][lesY[0] - 1] == BORDURE) {
                            distB += 1;
                            i++;
                        }
                        // choix de la distance la plus courte
                        if (distA <= distB) {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] + 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] + 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))){
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] += 1;
                            } else {
                                lesX[0] -= 1;
                            }
                            (*nbMvmt)++;
                        } 
                        else {
                            valide = true;
                            indice = 1;
                            indice2 = 0;
                            while (valide && indice < 10) {
                                if (((lesX[0] - 1 == lesX[indice]) && (lesY[0] == lesY[indice])) || ((lesX[0] - 1 == lesX2[indice2]) && (lesY[0] == lesY2[indice2]))) {
                                valide = false;
                                }
                                indice++;
                                indice2++;
                            }
                            if (valide) {
                                lesX[0] -= 1;
                            } else {
                                lesX[0] += 1;
                            }
                            (*nbMvmt)++;
                        }
                    } 
                    else {
                            lesY[0] -= 1; // déplacement sur le droite
                        (*nbMvmt)++;  // incrémentation du nombre de mouvement
                    }
                }
                else {
                    indice = 1;
                    gauche = true;
                    bas = true;
                    droit = true;
                    // verification de la prochaine position du serpent
                    while (droit && indice < 10) {
                        // test du contact tete-corps du serpent
                        if (((plateau[lesX[0]+1][lesY[0]] == BORDURE)) || ((lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]))) {
                            droit = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(bas && indice < 10){
                        if((plateau[lesX[0]][lesY[0]-1] == BORDURE) || ((lesX[0] == lesX[indice] && lesY[0]-1 == lesY[indice]))){
                            bas = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    indice = 1;
                    while(gauche && indice < 10){
                        if((plateau[lesX[0]-1][lesY[0]] == BORDURE) || ((lesX[0]-1 == lesX[indice] && lesY[0] == lesY[indice]))){
                            gauche = false;
                        }
                        indice++; // incrémentation de l'indice
                    }
                    if(droit){
                        lesX[0]++;
                    }
                    else if(gauche){
                        lesX[0]--;
                    }
                    else if(bas){
                        lesY[0]++;
                    }
                }
            }
        }
    }

    // parcour des diiférente position des portes
    // test si le serpent est dans l'une d'elle

    while (!porte && numPorte < 4) {
        if (lesX[0] == portail[numPorte][0] && lesY[0] == portail[numPorte][1]) {
        porte = true;
        if (*ch == 1) {
            lesX[0] = portail[numPorte + 1][0];
            lesY[0] = portail[numPorte + 1][1] - 1;
        } else if (*ch == 2) {
            lesX[0] = portail[numPorte - 1][0];
            lesY[0] = portail[numPorte - 1][1] + 1;
        } else if (*ch == 3) {
            lesX[0] = portail[numPorte + 1][0] + 1;
            lesY[0] = portail[numPorte + 1][1];
        } else if (*ch == 4) {
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

    dessinerSerpent2(lesX, lesY);
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
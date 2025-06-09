#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

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
#define ATTENTE 200000
// caractères pour représenter le serpent
#define CORPS 'X'
#define TETE 'O'
// touches de direction ou d'arrêt du jeu
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define STOP 'a'
// caractères pour les éléments du plateau
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'

// définition d'un type pour le plateau : tPlateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux
// dimensions et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles
// ne sont jamais utilisées)
typedef char tPlateau[LARGEUR_PLATEAU + 1][HAUTEUR_PLATEAU + 1];
typedef int tPortail[4][2];

int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = {8, 39, 2, 2, 5, 39, 33, 38, 35, 2};
tPortail portail = {{40, 1},{40, 40}, {80, 20}, {1, 20}};
int distHaut, distBas, distGauche, distDroite, d;
int distMini;
int chemin;
int passage;

void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau, int numPomme);
void afficher(int, int, char);
void effacer(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau,
                bool *collision, bool *pomme, int numPomme, int *nbMvmt, int *ch);
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();

int main() {
  clock_t begin = clock();
  // 2 tableaux contenant les positions des éléments qui constituent le serpent
  int lesX[TAILLE];
  int lesY[TAILLE];

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
  direction = DROITE;
  touche = DROITE;
  distHaut = (abs(portail[0][0] - lesX[0]) + abs(portail[0][1] - lesY[0])) + (abs(lesPommesX[0] - portail[1][0]) + abs(lesPommesY[0] - portail[1][1]));
  distBas = (abs(portail[1][0] - lesX[0]) + abs(portail[1][1] - lesY[0])) + (abs(lesPommesX[0] - portail[0][0]) + abs(lesPommesY[0] - portail[0][1]));
  distGauche = (abs(portail[2][0] - lesX[0]) + abs(portail[2][1] - lesY[0])) + (abs(lesPommesX[0] - portail[3][0]) + abs(lesPommesY[0] - portail[3][1]));
  distDroite = (abs(portail[3][0] - lesX[0]) + abs(portail[3][1] - lesY[0])) + (abs(lesPommesX[0] - portail[2][0]) + abs(lesPommesY[0] - portail[2][1]));
  d = abs(lesPommesX[0] - lesX[0]) + abs(lesPommesY[0] - lesY[0]);
  distMini = fmin(d, fmin(distDroite,fmin(distGauche,fmin(distHaut, distBas))));
  if(distMini == distHaut){
    chemin = 1;
  }
  else if(distMini == distBas){
    chemin = 2;
  }
  else if(distMini == distGauche){
    chemin = 3;
  }
  else if(distMini == distDroite){
    chemin = 4;
  }
  else if(distMini == d){
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
        ajouterPomme(lePlateau, nbPommes);
        distHaut = (abs(portail[0][0] - lesX[0]) + abs(portail[0][1] - lesY[0])) + (abs(lesPommesX[nbPommes] - portail[1][0]) + abs(lesPommesY[nbPommes] - portail[1][1]));
        distBas = (abs(portail[1][0] - lesX[0]) + abs(portail[1][1] - lesY[0])) + (abs(lesPommesX[nbPommes] - portail[0][0]) + abs(lesPommesY[nbPommes] - portail[0][1]));
        distGauche = (abs(portail[2][0] - lesX[0]) + abs(portail[2][1] - lesY[0])) + (abs(lesPommesX[nbPommes] - portail[3][0]) + abs(lesPommesY[nbPommes] - portail[3][1]));
        distDroite = (abs(portail[3][0] - lesX[0]) + abs(portail[3][1] - lesY[0])) + (abs(lesPommesX[nbPommes] - portail[2][0]) + abs(lesPommesY[nbPommes] - portail[2][1]));
        d = abs(lesPommesX[nbPommes] - lesX[0]) + abs(lesPommesY[nbPommes] - lesY[0]);
        distMini = fmin(d, fmin(distDroite,fmin(distGauche,fmin(distHaut, distBas))));
        if(distMini == distHaut){
          chemin = 1;
          passage = 1;
        }
        else if(distMini == distBas){
          chemin = 2;
          passage = 2;
        }
        else if(distMini == distGauche){
          chemin = 3;
          passage = 3;
        }
        else if(distMini == distDroite){
          chemin = 4;
          passage = 4;
        } 
        else if(distMini == d){
          chemin = 0;
          passage = 5;
        }
        distMini = fmin(d, fmin(distDroite,fmin(distGauche,fmin(distHaut, distBas))));
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
  printf("%d haut,%d bas, %d gauche, %d droite, %d direct\n %d mini, %d num chemin\n %d passage", distHaut, distBas, distGauche, distDroite, d, distMini, chemin, passage);

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
            // Si c'est une bordure (première ou dernière ligne, ou première ou dernière colonne)
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




void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme, int numPomme, int *nbMvmt, int *ch) {
  // efface le dernier élément avant d'actualiser la position de tous les
  // élémentds du serpent avant de le  redessiner et détecte une
  // collision avec une pomme ou avec une bordure
    effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);

    int dx = lesPommesX[numPomme] - lesX[0];
    int dy = lesPommesY[numPomme] - lesY[0];
    bool valide = true;
    bool porte = false;
    int i = 0;
    int indice = 1;

    for (int i = TAILLE - 1; i > 0; i--){
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }

    if(*ch == 1){
      dx = portail[0][0] - lesX[0];
      dy = portail[0][1] - lesY[0];
      if (abs(dx) != 0){
        if(dx > 0){
          while(valide && indice < 10){
            if(lesX[0] + 1 == lesX[indice] && lesY[0] == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide == true){
            lesX[0] += 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0]][lesY[0] + 1] == BORDURE){
              lesY[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesY[0] += 1;
              (*nbMvmt)++;
            }
          }

        } 
        else{
          while(valide && indice < 10){
            if(lesX[0] - 1 == lesX[indice] && lesY[0] == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide == true){
            lesX[0] -= 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0]][lesY[0] + 1] == BORDURE){
              lesY[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesY[0] += 1;
              (*nbMvmt)++;
            }
          }

        }

      }
      else{
        if(dy > 0){
          while(valide && indice < 10){
            if(lesX[0] == lesX[indice] && lesY[0] + 1 == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide ==  true){
            lesY[0] += 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0] + 1][lesY[0]] == BORDURE){
              lesX[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesX[0] += 1;
              (*nbMvmt)++;
            }
          }
        } 
        else{
          while(valide && indice < 10){
            if(lesX[0] == lesX[indice] && lesY[0] - 1 == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide == true){
            lesY[0] -= 1;
            (*nbMvmt)++;

          }
          else{
            if(plateau[lesX[0] + 1][lesY[0]] == BORDURE){
              lesX[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesX[0] += 1;
              (*nbMvmt)++;
            }
          }
        }    
      }
    }
    else if(*ch == 2){
      dx = portail[1][0] - lesX[0];
      dy = portail[1][1] - lesY[0];
      if (abs(dx)!=0){
        if(dx > 0){
          while(valide && indice < 10){
            if(lesX[0] + 1 == lesX[indice] && lesY[0] == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide == true){
            lesX[0] += 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0]][lesY[0] + 1] == BORDURE){
              lesY[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesY[0] += 1;
              (*nbMvmt)++;
            }
          }
        } 
        else{
          while(valide && indice < 10){
            if(lesX[0] - 1 == lesX[indice] && lesY[0] == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide == true){
            lesX[0] -= 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0]][lesY[0] + 1] == BORDURE){
              lesY[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesY[0] += 1;
              (*nbMvmt)++;
            }
          }
        }

      }
      else{
        if(dy > 0){
          while(valide && indice < 10){
            if(lesX[0] == lesX[indice] && lesY[0] + 1 == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide ==  true){
            lesY[0] += 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0] + 1][lesY[0]] == BORDURE){
              lesX[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesX[0] += 1;
              (*nbMvmt)++;
            }
          }
        } 
        else{
          while(valide && indice < 10){
            if(lesX[0] == lesX[indice] && lesY[0] - 1 == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide == true){
            lesY[0] -= 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0] + 1][lesY[0]] == BORDURE){
              lesX[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesX[0] += 1;
              (*nbMvmt)++;
            }
          }
        }    
      }
    }
    else if(*ch == 3){
      dx = portail[2][0] - lesX[0];
      dy = portail[2][1] - lesY[0];
      if (abs(dy) !=0){
        if(dy > 0){
          while(valide && indice < 10){
            if(lesX[0] == lesX[indice] && lesY[0] + 1== lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide == true){
            lesY[0] += 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0] + 1][lesY[0]] == BORDURE){
              lesX[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesX[0] += 1;
              (*nbMvmt)++;
            }
          }
        } 
        else{
          while(valide && indice < 10){
            if(lesX[0] == lesX[indice] && lesY[0] - 1 == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide == true){
            lesY[0] -= 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0] + 1][lesY[0]] == BORDURE){
              lesX[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesX[0] += 1;
              (*nbMvmt)++;
            }
          }
        }

      }
      else{
        if(dx > 0){
          while(valide && indice < 10){
            if(lesX[0]+1 == lesX[indice] && lesY[0] == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide ==  true){
            lesX[0] += 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0]][lesY[0]+ 1] == BORDURE){
              lesY[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesY[0] += 1;
              (*nbMvmt)++;
            }
          }
        } 
        else{
          while(valide && indice < 10){
            if(lesX[0] - 1 == lesX[indice] && lesY[0] == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide == true){
            lesX[0] -= 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0]][lesY[0]] == BORDURE){
              lesY[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesY[0] += 1;
              (*nbMvmt)++;
            }
          }
        }    
      }
    }
    else if(*ch == 4){
      dx = portail[3][0] - lesX[0];
      dy = portail[3][1] - lesY[0];
      if (abs(dy) !=0){
        if(dy > 0){
          while(valide && indice < 10){
            if(lesX[0] == lesX[indice] && lesY[0] + 1 == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide ==  true){
            lesY[0] += 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0] + 1][lesY[0]] == BORDURE){
              lesX[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesX[0] += 1;
              (*nbMvmt)++;
            }
          }
        } 
        else{
          while(valide && indice < 10){
            if(lesX[0] == lesX[indice] && lesY[0] - 1 == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide == true){
            lesY[0] -= 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0] + 1][lesY[0]] == BORDURE){
              lesX[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesX[0] += 1;
              (*nbMvmt)++;
            }
          }
        }    
      }
      else{
        if(dx > 0){
          while(valide && indice < 10){
              if(lesX[0] + 1 == lesX[indice] && lesY[0] == lesY[indice]){
                valide = false; 
              }
              indice++;
          }
          if(valide == true){
            lesX[0] += 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0]][lesY[0] + 1] == BORDURE){
              lesY[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesY[0] += 1;
              (*nbMvmt)++;
            }
          }
        }
        else{
          while(valide && indice < 10){
            if(lesX[0] - 1 == lesX[indice] && lesY[0] == lesY[indice]){
              valide = false; 
            }
            indice++;
          }
          if(valide == true){
            lesX[0] -= 1;
            (*nbMvmt)++;
          }
          else{
            if(plateau[lesX[0]][lesY[0] + 1] == BORDURE){
              lesY[0] -= 1;
              (*nbMvmt)++;
            }
            else{
              lesY[0] += 1;
              (*nbMvmt)++;
            }
          }
        }
      }
    }
    else if(*ch == 0){
    if (abs(dx) > abs(dy)){
      if(dx > 0){
        while(valide && indice < 10){
          if(lesX[0] + 1 == lesX[indice] && lesY[0] == lesY[indice]){
            valide = false; 
          }
          indice++;
        }
        if(valide == true){
          lesX[0] += 1;
          (*nbMvmt)++;
        }
        else{
          if(plateau[lesX[0]][lesY[0] + 1] == BORDURE){
            lesY[0] -= 1;
            (*nbMvmt)++;
          }
          else{
            lesY[0] += 1;
            (*nbMvmt)++;
          }
        }
      } 
      else{
        while(valide && indice < 10){
          if(lesX[0] - 1 == lesX[indice] && lesY[0] == lesY[indice]){
            valide = false; 
          }
          indice++;
        }
        if(valide == true){
          lesX[0] -= 1;
          (*nbMvmt)++;
        }
        else{
          if(plateau[lesX[0]][lesY[0] + 1] == BORDURE){
            lesY[0] -= 1;
            (*nbMvmt)++;
          }
          else{
            lesY[0] += 1;
            (*nbMvmt)++;
          }
        }
      }

    }
    else{
      if(dy > 0){
        while(valide && indice < 10){
          if(lesX[0] == lesX[indice] && lesY[0] + 1 == lesY[indice]){
            valide = false; 
          }
          indice++;
        }
        if(valide ==  true){
          lesY[0] += 1;
          (*nbMvmt)++;
        }
        else{
          if(plateau[lesX[0] + 1][lesY[0]] == BORDURE){
            lesX[0] -= 1;
            (*nbMvmt)++;
          }
          else{
            lesX[0] += 1;
            (*nbMvmt)++;
          }
        }
      } 
      else{
        while(valide && indice < 10){
          if(lesX[0] == lesX[indice] && lesY[0] - 1 == lesY[indice]){
            valide = false; 
          }
          indice++;
        }
        if(valide == true){
          lesY[0] -= 1;
          (*nbMvmt)++;
        }
        else{
          if(plateau[lesX[0] + 1][lesY[0]] == BORDURE){
            lesX[0] -= 1;
            (*nbMvmt)++;
          }
          else{
            lesX[0] += 1;
            (*nbMvmt)++;
          }
        }
      }    
    }
  }

    while(!porte && i < 4){
      if(lesX[0] == portail[i][0] && lesY[0] == portail[i][1]){
        porte = true;
        if(*ch == 1){
          lesX[0] = portail[i+1][0];
          lesY[0] = portail[i+1][1] - 1;
        }
        else if(*ch == 2){
          lesX[0] = portail[i-1][0];
          lesY[0] = portail[i-1][1] + 1;
        }
        else if(*ch == 3){
          lesX[0] = portail[i+1][0] + 1;
          lesY[0] = portail[i+1][1];
        }
        else if(*ch == 4){
          lesX[0] = portail[i-1][0] - 1;
          lesY[0] = portail[i-1][1];
        }
        *ch = 0;
      }
      i++;
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
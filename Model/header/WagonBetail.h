#ifndef WAGONBETAIL_H_
#define WAGONBETAIL_H_

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include "Vecteur.h"

class WagonBetail{

    public:
        WagonBetail();
        void assembler();
        void dessiner();
        void deplacer(float x, float y, float z);
        void orienter(float x, float y, float z);
        Vecteur getMilieuRouesAvant();
        Vecteur getMilieuRouesArriere();
        float getLongueurTrain();
        float getLargeurTrain();
        
        Vecteur direction;

        Vecteur position;

    private:
        const float largeurTrain=8;
        const float longueurTrain=25;
        const float hauteurTrain=10;
        Vecteur positionRoueAvant;
        Vecteur positionRoueArriere;
};

#endif
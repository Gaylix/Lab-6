 /**
 * @file   main.c, projet Lab4
 * @author Nicolas Gosselin
 * @date   Septembre 2019
 * @brief    
 * Environnement:
 *     Développement: MPLAB X IDE (version 5.05)
 *     Compilateur: XC8 (version 2.00)
 *     Matériel: Carte démo noire avec PIC 18F45K20
 */

 /****************** Liste des INCLUDES ****************************************/
#include <xc.h>
#include <stdbool.h>  // pour l'utilisation du type bool
#include "serie.h"
#include <conio.h>
#include "Lcd4Lignes.c"
#include "Lcd4Lignes.h"
#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/********************** CONSTANTES *******************************************/
#define _XTAL_FREQ 1000000 //Constante utilisée par __delay_ms(x). Doit = fréq interne du uC
//#define DELAI_TMR0 0x0001 // Vitesse du jeu (temps de déplacement des aliens):  0x0BDC = 2s, 0x85EE = 1s
#define NB_LIGNE 4  //afficheur LCD 4x20
#define NB_COL 20
#define AXE_X 7  //canal analogique de l'axe x
#define AXE_Y 6
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define TUILE 1 //caractère cgram d'une tuile
#define MINE 2 //caractère cgram d'une mine
#define NB_MINES 4

/********************** PROTOTYPES *******************************************/
void initialisation(void);
void deplace(int* x, int* y);
bool demine(char x, char y);
char getAnalog(char canal);
void initTabVue(void);
void rempliMines(int nb);
void metToucheCombien(void);
char calculToucheCombien(int ligne, int colonne);
void enleveTuilesAutour(char x, char y);
bool gagne(int* pMines);
void afficheTabVue(void);
void afficheTabMines(void);
void metDrapeau(char x, char y);


/****************** VARIABLES GLOBALES ****************************************/
char m_tabMines[NB_LIGNE][NB_COL+1]; //Tableau contenant les mines, les espaces et les chiffres
char m_tabVue[NB_LIGNE][NB_COL+1];  //Tableau des caractères affichés au LCD


/*               ***** PROGRAMME PRINCPAL *****                             */
void main(void)
{
    /*********variables locales du main********/
    int posX = 0;           // variable pour se souvenir de la position en X du curseur
    int posY = 0;           // variable pour se souvenir de la position en Y du curseur
    int nbMines = NB_MINES;
    
    
    /******** début du code********************/
    initialisation();
    lcd_init();
    init_serie();
    lcd_putMessage("LAB6 Nicolas");
    __delay_ms(1000);
    lcd_effaceAffichage();
    
    initTabVue();
    rempliMines(nbMines);
    metToucheCombien();
    afficheTabVue();
    

    

    
    posX = (NB_COL / 2);        // sers à initialiser la position de départ du curseur au milieu de l'affichage
    posY = (NB_LIGNE / 2);      // sers à initialiser la position de départ du curseur au milieu de l'affichage
    
    while(1)
    {
        deplace(&posX,&posY);      
        
        
        if(PORTBbits.RB0 == false)
        {
            metDrapeau(posX,posY);
        }
        
        if(PORT_SW == false)
        {
            if((demine(posX,posY) == false) || (gagne(&nbMines) == true))
            {
                afficheTabMines();
                __delay_ms(500);
                while(PORT_SW == true)
                {
                    
                }
                
                initTabVue();
                rempliMines(nbMines);
                metToucheCombien();
                afficheTabVue();
                posX = (NB_COL / 2);        // sers à initialiser la position de départ du curseur au milieu de l'affichage
                posY = (NB_LIGNE / 2);      // sers à initialiser la position de départ du curseur au milieu de l'affichage
            }
        }
         
        __delay_ms(100);
    
    }
    
}

/**
 * @brief Si la manette est vers la droite ou la gauche, on déplace le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(int* x, int* y)
{
    
    char amHereX;
    char amHereY;
    int posMemY = *y;
    int posMemX = *x;
    
    amHereX = getAnalog(AXE_X);
    amHereY = getAnalog(AXE_Y);
    
    
    if((amHereX < 99) && (amHereX >= 0))
    {
        *x = *x - 1;
        if(*x <= 0)
        {
            *x = NB_COL;
        }
    }else if((amHereX > 155) && (amHereX <= 255))
    {
        *x = *x + 1;
        if(*x > NB_COL)
        {
            *x = 1;
        }
    }
    
    if((amHereY < 99) && (amHereY >= 0))
    {
        *y = *y - 1;
        if(*y <= 0)
        {
            *y = NB_LIGNE;
        }
    }else if((amHereY > 155) && (amHereY <= 255))
    {
        *y = *y + 1;
        if(*y > NB_LIGNE)
        {
            *y = 1;
        }
    }
    
    
    posMemY = *y;
    posMemX = *x;
    lcd_gotoXY(posMemX,posMemY);
    
    
}
 

/*
 * @brief Dévoile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derrière les tuiles (m_tabMines).
 * Utilise enleveTuileAutour().
 * @param char x, char y Les positions X et y sur l'afficheur LCD
 * @return faux s'il y avait une mine, vrai sinon
 */
bool demine(char x, char y)
{
    if(m_tabMines[y-1][x-1] == ' ')
    {
        m_tabVue[y-1][x-1] = m_tabMines[y-1][x-1];
        enleveTuilesAutour(x,y);
        afficheTabVue();
        return true;
    }else if(m_tabMines[y-1][x-1] != 2)
    {
        m_tabVue[y-1][x-1] = m_tabMines[y-1][x-1];
        afficheTabVue();
        return true;
    }else if(m_tabMines[y-1][x-1] == 2)
    {
        return false;
    }
}


void metDrapeau(char x, char y)
{
    if(m_tabVue[y-1][x-1] == 1)
    {
        m_tabVue[y-1][x-1] = 3;
        afficheTabVue();
    }else if(m_tabVue[y-1][x-1] == 3)
    {
        m_tabVue[y-1][x-1] = 1;
        afficheTabVue();
    }
}


/*
 * @brief Rempli le tableau m_tabVue avec le caractère spécial (définie en CGRAM
 *  du LCD) TUILE. Met un '\0' à la fin de chaque ligne pour faciliter affichage
 *  avec lcd_putMessage().
 * @param rien
 * @return rien
 */
void initTabVue(void)
{
    
    for(int i = 0; i < NB_LIGNE; i ++)
    {
        for(int j = 0; j < NB_COL; j ++)
        {
            m_tabVue[i][j] = 1;
        }
    }
    
}


void afficheTabVue(void)
{
    
    for(int i = 0; i < NB_LIGNE; i ++)
    {
        lcd_gotoXY(1,i+1);
        lcd_putMessage(m_tabVue[i]);
        /*
        for(int j = 0; j < NB_COL; j ++)
        {
            lcd_gotoXY(j+1,i+1);
            lcd_ecritChar(m_tabVue[i][j]);
        }
        */
    }
}

void afficheTabMines(void)
{
    
    for(int i = 0; i < NB_LIGNE; i ++)
    {
        lcd_gotoXY(1,i+1);
        lcd_putMessage(m_tabMines[i]);
        /*
        for(int j = 0; j < NB_COL; j ++)
        {
            lcd_gotoXY(j+1,i+1);
            lcd_ecritChar(m_tabMines[i][j]);
        }
        */
    }
}
 
/*
 * @brief Rempli le tableau m_tabMines d'un nombre (nb) de mines au hasard.
 *  Les cases vides contiendront le code ascii d'un espace et les cases avec
 *  mine contiendront le caractère MINE défini en CGRAM.
 * @param int nb, le nombre de mines à mettre dans le tableau 
 * @return rien
 */
void rempliMines(int nb)
{
    
    int randX = 0;
    int randY = 0;
    
    for(int i = 0; i < NB_LIGNE; i ++)
    {
        for(int j = 0; j < NB_COL; j ++)
        {
            m_tabMines[i][j] = ' ';
        }
    }
    
    
    for(int i = 0; i < nb; i ++)
    {
        do{
            randX = (rand() % NB_COL);
            randY = (rand() % NB_LIGNE);
            
        }while(m_tabMines[randY][randX] != ' ');
        
        m_tabMines[randY][randX] = 2;
    }
    
    
}
 
/*
 * @brief Rempli le tableau m_tabMines avec le nombre de mines que touche la case.
 * Si une case touche à 3 mines, alors la méthode place le code ascii de 3 dans
 * le tableau. Si la case ne touche à aucune mine, la méthode met le code
 * ascii d'un espace.
 * Cette méthode utilise calculToucheCombien(). 
 * @param rien
 * @return rien
 */
void metToucheCombien(void)
{
    
    for(int i = 0; i < NB_LIGNE; i ++)
    {
        for(int j = 0; j < NB_COL; j ++)
        {
            if(m_tabMines[i][j] != 2)
            {
                m_tabMines[i][j] = calculToucheCombien(i,j);
            }
        }
    }
    //rappelZT = m_tabMines[0][3];
}
 
/*
 * @brief Calcul à combien de mines touche la case. Cette méthode est appelée par metToucheCombien()
 * @param int ligne, int colonne La position dans le tableau m_tabMines a vérifier
 * @return char nombre. Le nombre de mines touchées par la case
 */
char calculToucheCombien(int ligne, int colonne)
{
    // code ASCII de 0 = 48
    int compte = 0;
    
    for(int i = -1; i <= 1; i ++)
    {
        if(((ligne + i) != -1) && ((ligne + i) != (NB_LIGNE)))      // vérifie si la ligne présente est à l'intérieur de la matrice
        {
            for(int j = -1; j <= 1; j ++)
            {
                if(((colonne + j) != -1) && ((colonne + j) != (NB_COL)))        // vérifie si la colonne présente est à l'intérieur de la matrice
                {
                    if(m_tabMines[ligne + i][colonne + j] == 2)     // vérifie si la case sélectionnée est une mine
                    {
                        compte ++;
                    }
                }
            }
        }
    }
    
    if(compte == 0)
    {
        return 32;      // si la case ne touche à aucune mine, renvoie le code ASCII de 0
    }else
    {
        return (compte + 48);       // si la case touche à une ou plusieures mines, renvoie le code ASCII du chiffre correspondant
    }
    
}

 
/*
 * @brief Dévoile les cases non minées autour de la tuile reçue en paramètre.
 * Cette méthode est appelée par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y)
{
    for(int i = -1; i <= 1; i ++)
    {
        for(int j = -1; j <= 1; j ++)
        {
            m_tabVue[y+i-1][x+j-1] = m_tabMines[y+i-1][x+j-1];      // dévoile la case sous la tuile sélectionnée
        }
    }
}
 
/*
 * @brief Vérifie si gagné. On a gagné quand le nombre de tuiles non dévoilées
 * est égal au nombre de mines. On augmente de 1 le nombre de mines si on a 
 * gagné.
 * @param int* pMines. Le nombre de mine.
 * @return vrai si gagné, faux sinon
 */
bool gagne(int *pMines)
{
    int compte = 0;
    int nombreMines = 0;
    
    nombreMines = (*pMines);
    
    for(int i = 0; i < NB_LIGNE; i ++)
    {
        for(int j = 0; j < NB_COL; j ++)
        {
            if((m_tabVue[i][j] == 1) || (m_tabVue[i][j] == 3))      // compte le nombre de tuiles et de drapeaux sur l'affichage
            {
                compte ++;
            }
        }
    }
    
    
    
    if(compte == nombreMines)       // vérifie si le nombre de tuiles et de drapeaux est égal au nombre de mines 
    {
        *pMines = *pMines + 1;      // augmente le nombre de mines pour la prochaine partie
        return true;                // pour dire que la partie est gagnée
    }else
    {
        return false;               // pour dire que la partie n'est pas encore gagnée
    }
}

/*
 * @brief Lit le port analogique. 
 * @param Le no du port à lire
 * @return La valeur des 8 bits de poids forts du port analogique
 */
char getAnalog(char canal)
{ 
    ADCON0bits.CHS = canal;
    __delay_us(1);  
    ADCON0bits.GO_DONE = 1;  //lance une conversion
    while (ADCON0bits.GO_DONE == 1) //attend fin de la conversion
        ;
    return  ADRESH; //retourne seulement les 8 MSB. On laisse tomber les 2 LSB de ADRESL
}

/*
 * @brief Fait l'initialisation des différents regesitres et variables.
 * @param Aucun
 * @return Aucun
 */
void initialisation(void)
{
    TRISD = 0; //Tout le port D en sortie
 
    ANSELH = 0;  // RB0 à RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
 
    ANSEL = 0;  // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie
 
    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
 
   //Configuration du port analogique
    ANSELbits.ANS7 = 1;  //A7 en mode analogique
 
    ADCON0bits.ADON = 1; //Convertisseur AN à on
	ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
 
    ADCON2bits.ADFM = 0; //Alignement à gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB à gauche dans ADRESL)
    ADCON2bits.ACQT = 0;//7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0;//6; //Fosc/64 (Fréquence pour la conversion la plus longue possible)
 
    
    /**************Timer 0*****************/
    /*
    T0CONbits.TMR0ON    = 1;
    T0CONbits.T08BIT    = 0; // mode 16 bits
    T0CONbits.T0CS      = 0;
    T0CONbits.PSA       = 0; // prescaler enabled
    T0CONbits.T0PS      = 0b010; // 1:8 pre-scaler
    TMR0 = DELAI_TMR0;
    INTCONbits.TMR0IE   = 1;  // timer 0 interrupt enable
    INTCONbits.TMR0IF   = 0; // timer 0 interrupt flag
    INTCONbits.PEIE = 1; //permet interruption des périphériques
    INTCONbits.GIE = 1;  //interruptions globales permises
    */
}


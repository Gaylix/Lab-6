 /**
 * @file   main.c, projet Lab4
 * @author Nicolas Gosselin
 * @date   D�cembre 2019
 * @brief  code pour recr�er le jeu d�mineur. Place un certain nombre de mines
  *        qui est augment� de 1 apr�s chaque victoire. v�rifie ensuite chaque 
  *        case et met un chiffre correspondant au nombre de mines qui sont 
  *        autour (met une case vide si ce nombre est 0). L'affichage est 
  *        ensuite rempli de tuiles. On peux se d�placer avec une manette et 
  *        d�miner avec la m�me manette. Il est aussi possible de placer un 
  *        drapeau aux endroits o� nous pensons qu'il y as une mine. Lorsqu'une
  *        tuile est d�min�e, si elle ne contient pas un chiffre ni une mine, 
  *        les cases autours sont alors d�min�es. Si la case d�min�e est un 
  *        chiffre, seulement la case s�l�ctionn�e sera d�voil�e. Si la case 
  *        s�l�ctionn�e est une mine, toutes les mines et chiffres seront 
  *        d�voil�s et la partie est alors perdue. La partie est gagn�e lorsque 
  *        toutes les cases sauf les mines sont d�voil�es.
 * Environnement:
 *     D�veloppement: MPLAB X IDE (version 5.05)
 *     Compilateur: XC8 (version 2.00)
 *     Mat�riel: Carte d�mo noire avec PIC 18F45K20
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
#define _XTAL_FREQ 1000000 //Constante utilis�e par __delay_ms(x). Doit = fr�q interne du uC
//#define DELAI_TMR0 0x0001 // Vitesse du jeu (temps de d�placement des aliens):  0x0BDC = 2s, 0x85EE = 1s
#define NB_LIGNE 4  //afficheur LCD 4x20
#define NB_COL 20
#define AXE_X 7  //canal analogique de l'axe x
#define AXE_Y 6
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define TUILE 1 //caract�re cgram d'une tuile
#define MINE 2 //caract�re cgram d'une mine
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
char m_tabVue[NB_LIGNE][NB_COL+1];  //Tableau des caract�res affich�s au LCD


/*               ***** PROGRAMME PRINCPAL *****                             */
void main(void)
{
    /*********variables locales du main********/
    int posX = 0;           // variable pour se souvenir de la position en X du curseur
    int posY = 0;           // variable pour se souvenir de la position en Y du curseur
    int nbMines = NB_MINES;
    
    
    /******** d�but du code********************/
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
    

    

    
    posX = (NB_COL / 2);        // sers � initialiser la position de d�part du curseur au milieu de l'affichage
    posY = (NB_LIGNE / 2);      // sers � initialiser la position de d�part du curseur au milieu de l'affichage
    
    while(1)
    {
        deplace(&posX,&posY);      
        
        
        if(PORTBbits.RB0 == false)      // v�rifie si le bouton RB0 est enfonc�
        {
            metDrapeau(posX,posY);
        }
        
        if(PORT_SW == false)            // v�rifie si le bouton de la manette est enfonc�
        {
            if((demine(posX,posY) == false) || (gagne(&nbMines) == true))       // v�rifie si la partie as �t� gagn�e ou perdue
            {
                afficheTabMines();
                __delay_ms(500);
                while(PORT_SW == true)      // attend que le bouton soit rel�ch�
                {
                    
                }
                
                initTabVue();               // r�initialise la matrice m_tabVue
                rempliMines(nbMines);       // r�initialise la matrice m_tabMines
                metToucheCombien();         // r�initialise les cases qui touchent aux mines
                afficheTabVue();
                posX = (NB_COL / 2);        // sers � initialiser la position de d�part du curseur au milieu de l'affichage
                posY = (NB_LIGNE / 2);      // sers � initialiser la position de d�part du curseur au milieu de l'affichage
            }
        }
         
        __delay_ms(100);
    
    }
    
}

/**
 * @brief Si la manette est vers la droite ou la gauche, on d�place le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(int* x, int* y)
{
    
    char amHereX;       // variable pour se souvenir de la position en x de la manette
    char amHereY;       // variable pour se souvenir de la position en y de la manette
    int posMemY = *y;
    int posMemX = *x;
    
    amHereX = getAnalog(AXE_X);     // prise de la position en x de la manette 
    amHereY = getAnalog(AXE_Y);     // prise de la position en y de la manette
    
    
    if((amHereX < 99) && (amHereX >= 0))        // v�rifie si la manette est vers la gauche
    {
        *x = *x - 1;        // d�place le curseur d'une case vers la gauche
        if(*x <= 0)         // si le curseur sors de l'affichage, on le fait r�apparaitre de l'autre c�t�
        {
            *x = NB_COL;
        }
    }else if((amHereX > 155) && (amHereX <= 255))       // v�rifie si la manette est vers la droite
    {
        *x = *x + 1;        // d�place le curseur d'une case vers la droite
        if(*x > NB_COL)     // si le curseur sors de l'affichage, on le fait r�apparaitre de l'autre c�t�
        {
            *x = 1;
        }
    }
    
    if((amHereY < 99) && (amHereY >= 0))        // v�rifie si la manette est vers le bas
    {
        *y = *y - 1;        // d�place le curseur d'une case vers le bas
        if(*y <= 0)     // si le curseur sors de l'affichage, on le fait r�apparaitre de l'autre c�t�
        {
            *y = NB_LIGNE;
        }
    }else if((amHereY > 155) && (amHereY <= 255))       // v�rifie si la manette est vers le haut
    {
        *y = *y + 1;            // d�place le curseur d'une case vers le haut
        if(*y > NB_LIGNE)       // si le curseur sors de l'affichage, on le fait r�apparaitre de l'autre c�t�
        {
            *y = 1;
        }
    }
    
    
    posMemY = *y;
    posMemX = *x;
    lcd_gotoXY(posMemX,posMemY);        // d�place le curseur dans la direction d�sir�e
    
    
}
 

/*
 * @brief D�voile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derri�re les tuiles (m_tabMines).
 * Utilise enleveTuileAutour().
 * @param char x, char y Les positions X et y sur l'afficheur LCD
 * @return faux s'il y avait une mine, vrai sinon
 */
bool demine(char x, char y)
{
    if(m_tabMines[y-1][x-1] == ' ')     // v�rifie si la case s�l�ctionn�e est une case vide
    {
        m_tabVue[y-1][x-1] = m_tabMines[y-1][x-1];      // d�voile la case s�l�ctionn�e
        enleveTuilesAutour(x,y);                        // d�voile toutes les cases autour
        afficheTabVue();                                // affiche la nouvelle matrice m_tabVue
        return true;                                    // retourne vrai pour indiquer que la case s�l�ctionn�e n'est pas une mine
    }else if(m_tabMines[y-1][x-1] != 2)     // v�rifie si la case est un nombre
    {
        m_tabVue[y-1][x-1] = m_tabMines[y-1][x-1];      // d�voile la case s�l�ctionn�e
        afficheTabVue();                                // affiche la nouvelle matrice m_tabVue
        return true;                                    // retourne vrai pour indiquer que la case s�l�ctionn�e n'est pas une mine
    }else if(m_tabMines[y-1][x-1] == 2)     // v�rifie si la case s�l�ctionn�e est une mine
    {
        return false;       // retourne faux pour indiquer qu'une mine as �t� d�voil�e
    }
}

/*
 * @brief met un drapeau sur la tuile s�l�ctionn�e ou enl�ve un drapeau si il y
 * en as d�ja un
 * @param rien
 * @return rien
 */
void metDrapeau(char x, char y)
{
    if(m_tabVue[y-1][x-1] == 1)     // v�rifie si la case s�l�ctionn�e est une tuile
    {
        m_tabVue[y-1][x-1] = 3;     // si c'est le cas, elle est remplac�e par un drapeau
        afficheTabVue();            // affiche imm�diatement la nouvelle matrice m_tabVue
    }else if(m_tabVue[y-1][x-1] == 3)   // si la case n'est pas une tuile, on v�rifie si c'est un drapeau
    {
        m_tabVue[y-1][x-1] = 1;         // si c'est le cas, il est remplac� par une tuile
        afficheTabVue();                // affiche imm�diatement la nouvelle matrice m_tabVue
    }
}


/*
 * @brief initialise la matrice m_tabVue avec une tuile dans chaque case
 * @param rien
 * @return rien
 */
void initTabVue(void)
{
    
    for(int i = 0; i < NB_LIGNE; i ++)
    {
        for(int j = 0; j < NB_COL; j ++)
        {
            m_tabVue[i][j] = 1;     // met la valeur d'une tuile dans la case s�l�ctionn�e
        }
    }
    
}

/*
 * @brief affiche la matrice m_tabVue
 * @param rien
 * @return rien
 */
void afficheTabVue(void)
{
    
    for(int i = 0; i < NB_LIGNE; i ++)
    {
        lcd_gotoXY(1,i+1);              // vas � la ligne s�l�ctionn�e sur l'affichage 
        lcd_putMessage(m_tabVue[i]);    // affiche toute la ligne s�l�ctionn�e
    }
}

/*
 * @brief affiche la matrice m_tabMines
 * @param rien
 * @return rien
 */
void afficheTabMines(void)
{
    
    for(int i = 0; i < NB_LIGNE; i ++)
    {
        lcd_gotoXY(1,i+1);                  // vas � la ligne s�l�ctionn�e sur l'affichage 
        lcd_putMessage(m_tabMines[i]);      // affiche toute la ligne s�l�ctionn�e
    }
}
 
/*
 * @brief Rempli le tableau m_tabMines d'un nombre (nb) de mines au hasard.
 *  Les cases vides contiendront le code ascii d'un espace et les cases avec
 *  mine contiendront le caract�re MINE d�fini en CGRAM.
 * @param int nb, le nombre de mines � mettre dans le tableau 
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
            m_tabMines[i][j] = ' ';         // vide toutes les cases de la matrice m_tabMines
        }
    }
    
    
    for(int i = 0; i < nb; i ++)        // effectue la boucle suivante pour chaque mine
    {
        do{
            randX = (rand() % NB_COL);
            randY = (rand() % NB_LIGNE);
            
        }while(m_tabMines[randY][randX] != ' ');        // cherche des cases al�atoirement jusqu'� trouver une case o� il n'y as pas d�ja une mine
        
        m_tabMines[randY][randX] = 2;       // met une mine dans la case
    }
    
    
}
 
/*
 * @brief Rempli le tableau m_tabMines avec le nombre de mines que touche la case.
 * Si une case touche � 3 mines, alors la m�thode place le code ascii de 3 dans
 * le tableau. Si la case ne touche � aucune mine, la m�thode met le code
 * ascii d'un espace.
 * Cette m�thode utilise calculToucheCombien(). 
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
                m_tabMines[i][j] = calculToucheCombien(i,j);        // appele la fonction calculToucheCombien si la case s�l�ctionn�e n'est pas une mine
            }
        }
    }
}
 
/*
 * @brief Calcul � combien de mines touche la case. Cette m�thode est appel�e par metToucheCombien()
 * @param int ligne, int colonne La position dans le tableau m_tabMines a v�rifier
 * @return char nombre. Le nombre de mines touch�es par la case
 */
char calculToucheCombien(int ligne, int colonne)
{
    // code ASCII de 0 = 48
    int compte = 0;
    
    for(int i = -1; i <= 1; i ++)
    {
        if(((ligne + i) != -1) && ((ligne + i) != (NB_LIGNE)))      // v�rifie si la ligne pr�sente est � l'int�rieur de la matrice
        {
            for(int j = -1; j <= 1; j ++)
            {
                if(((colonne + j) != -1) && ((colonne + j) != (NB_COL)))        // v�rifie si la colonne pr�sente est � l'int�rieur de la matrice
                {
                    if(m_tabMines[ligne + i][colonne + j] == 2)     // v�rifie si la case s�lectionn�e est une mine
                    {
                        compte ++;
                    }
                }
            }
        }
    }
    
    if(compte == 0)
    {
        return 32;      // si la case ne touche � aucune mine, renvoie le code ASCII de 0
    }else
    {
        return (compte + 48);       // si la case touche � une ou plusieures mines, renvoie le code ASCII du chiffre correspondant
    }
    
}

 
/*
 * @brief D�voile les cases non min�es autour de la tuile re�ue en param�tre.
 * Cette m�thode est appel�e par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y)
{
    for(int i = -1; i <= 1; i ++)
    {
        for(int j = -1; j <= 1; j ++)
        {
            m_tabVue[y+i-1][x+j-1] = m_tabMines[y+i-1][x+j-1];      // d�voile la case sous la tuile s�lectionn�e
        }
    }
}
 
/*
 * @brief V�rifie si gagn�. On a gagn� quand le nombre de tuiles non d�voil�es
 * est �gal au nombre de mines. On augmente de 1 le nombre de mines si on a 
 * gagn�.
 * @param int* pMines. Le nombre de mine.
 * @return vrai si gagn�, faux sinon
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
    
    
    
    if(compte == nombreMines)       // v�rifie si le nombre de tuiles et de drapeaux est �gal au nombre de mines 
    {
        *pMines = *pMines + 1;      // augmente le nombre de mines pour la prochaine partie
        return true;                // pour dire que la partie est gagn�e
    }else
    {
        return false;               // pour dire que la partie n'est pas encore gagn�e
    }
}

/*
 * @brief Lit le port analogique. 
 * @param Le no du port � lire
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
 * @brief Fait l'initialisation des diff�rents regesitres et variables.
 * @param Aucun
 * @return Aucun
 */
void initialisation(void)
{
    TRISD = 0; //Tout le port D en sortie
 
    ANSELH = 0;  // RB0 � RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
 
    ANSEL = 0;  // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie
 
    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
 
   //Configuration du port analogique
    ANSELbits.ANS7 = 1;  //A7 en mode analogique
 
    ADCON0bits.ADON = 1; //Convertisseur AN � on
	ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
 
    ADCON2bits.ADFM = 0; //Alignement � gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB � gauche dans ADRESL)
    ADCON2bits.ACQT = 0;//7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0;//6; //Fosc/64 (Fr�quence pour la conversion la plus longue possible)
 
    
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
    INTCONbits.PEIE = 1; //permet interruption des p�riph�riques
    INTCONbits.GIE = 1;  //interruptions globales permises
    */
}


/********************************************************************************************************/
/********************************************************************************************************/
/***********************************TUTORAT SYSTEMES IMA*************************************************/
/***********************************Amaury KNOCKAERT - Thomas CUNIN**************************************/
/***********************************PROGRAMME PC*************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/



/* Le programme utilise la librairie "ncurses", cette librairie permet de créer une fenêtre au sein d'un terminal.
Nous l'avons utilisée pour permettre d'avoir une interface correcte tout en utilisant des boucles infinies*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libusb-1.0/libusb.h>
#include <ncurses.h>
#include <string.h>

#define IDVENDOR 0x045e		//On définit l'ID du vendeur et l'ID du produit
#define IDPRODUCT 0x028f	//Nos IDs correspondent à une manette de Xbox360 de chez Microsoft
#define ESC 27			//Code de la touche ECHAP
#define ALLUME_LED	49	//Code décimal pour '0'
#define ETEINT_LED	48	//Code décimal pour '1'
#define TIMEOUT		10 	//Durée maximale en ms de bloquage de la fonction libusb_interrupt_transfer
#define	SIZE		500

#define	D3	'2'
#define D4	'4'
#define D5	'8'
#define D6	'0'
#define JS	'1'

/*Codage des touches :
D3 (droite) - '2'
D4 (haut) - '4'
D5 (bas) - '8'
D6 (gauche)- '0'
bouton Joystick - '1'
*/

//On crée une liste de donnée pour enregistrer l'adresse des Endpoints
struct liste{
	uint8_t valeur[SIZE];
	int fin;
};





void init_liste(struct liste *liste) //On initialise toutes les cases de la liste à 0
{
	int i = 0;
	for(i = 0; i < 500;i++)
		liste->valeur[i] = 0;
}

int recuperationArduino(libusb_context *context,libusb_device **deviceArduino, libusb_device_handle ** handleArduino) 	//On cherche l'arduino dans les périphériques
{
	libusb_device **list;
	int exist = 0;
	ssize_t count=libusb_get_device_list(context,&list); //On récupère la liste d'appareils USB connectés et leur nombre.
	if(count<0)
	{
		perror("libusb_get_device_list"); 
		exit(-1);
	}
	ssize_t i=0;
	for(i=0;i<count;i++)	//On parcourt la liste des appareils
	{
		libusb_device *device=list[i];	//on prend l'appareil d'indice i de la liste
		struct libusb_device_descriptor desc;	//structure du descripteur de l'appareil
		int status=libusb_get_device_descriptor(device,&desc);	//On récupère le descripteurs de l'appareil
		
		if(status!=0) //Si le descripteur de l'appareil a bien été enregistré dans desc
			continue;
		if(desc.idVendor == IDVENDOR && desc.idProduct == IDPRODUCT)	//Si c'est l'appareil recherché
		{
			*deviceArduino = device;	//Alors on le récupère
			exist = 1;
		}
	}

	libusb_free_device_list(list,1);	//On libère la liste des appareils

	if(exist)
	{
		int open = libusb_open(*deviceArduino,handleArduino); 	//On récupère le handle de l'arduino
		if(open!=0)
		{
			perror("libusb_open");
			exit(-1);
		}
		return open;
	}else{
		return -1;	
	}
}


int recuperationConfig(libusb_device *deviceArduino,struct libusb_config_descriptor **configDesc)	//On récupère la configuration de l'appareil
{
	int iGetC = libusb_get_config_descriptor(deviceArduino,0,configDesc);
	if(iGetC!=0)
	{
		perror("libusb_get_config_descriptor");
		exit(-1);
	}
	
	return iGetC;
}


struct liste * configPeriph(struct libusb_config_descriptor *configDesc, struct libusb_device_handle *handleArduino)	//Retourne l'adresse des Endpoints
{

	//Quelques compteurs
	int i,j,k,status;
	int x = 0;

	
	//On crée la liste
	struct liste *interruptEndpoint = NULL;
	interruptEndpoint = malloc(sizeof(struct liste));
	init_liste(interruptEndpoint);
	interruptEndpoint->fin = 0;

	//On défini la configuration récupérée comme configuration utilisée
	status=libusb_set_configuration(handleArduino,configDesc->bConfigurationValue);
	if(status!=0)
	{
		perror("libusb_set_configuration"); 
		exit(-1); 
	}

	//Libération des interfaces du périph, du noyau linux
	for(i = 0;configDesc->bNumInterfaces > i;i++)
	{

		for(j = 0;configDesc->interface[i].num_altsetting > j;j++)
		{
			struct libusb_interface_descriptor interface = configDesc->interface[i].altsetting[j];
			if(libusb_kernel_driver_active(handleArduino,interface.bInterfaceNumber))
			{
				status=libusb_detach_kernel_driver(handleArduino,interface.bInterfaceNumber);
				if(status!=0)
				{
					perror("libusb_detach_kernel_driver"); 
					exit(-1);
				}
			}
		}
		
		
	}

	//Boucle de récupération des adresses des Endpoints
	for(i = 0;configDesc->bNumInterfaces > i;i++)	//On parcourt les interfaces
	{

		for(j = 0;configDesc->interface[i].num_altsetting > j;j++)
		{
			struct libusb_interface_descriptor interface = configDesc->interface[i].altsetting[j];
			libusb_claim_interface(handleArduino,interface.bInterfaceNumber);
			if(status!=0)
			{
				perror("libusb_claim_interface"); 
				exit(-1);
			}
		
			printf("\nInterface id = %d n°%d",interface.bInterfaceNumber,i); 
			for(k=0;k<interface.bNumEndpoints;k++)	//On parcourt les endpoints
			{
				if((interface.endpoint->bmAttributes & 0x03) == LIBUSB_TRANSFER_TYPE_INTERRUPT)	//On ne récupère que les Endpoint de type interrupt
				{
					printf("\n\tendpoint : %d		\n",interface.endpoint->bEndpointAddress);
					interruptEndpoint->valeur[x] = interface.endpoint->bEndpointAddress;
					interruptEndpoint->fin++;
					x++;
					break;			
				}
				
			}
			
		}
		
		
	}
	return interruptEndpoint;
	
}

void fermeture(libusb_device *deviceArduino, libusb_device_handle *handleArduino)	//On ferme le port USB
{
	int status,i,j;
	struct libusb_config_descriptor *activeConfigDesc;
	status = libusb_get_active_config_descriptor(deviceArduino,&activeConfigDesc);
	if(status!=0)
	{
		perror("libusb_get_active_config_descriptor"); 
		exit(-1);
	}
	
	//On libère les interfaces de la configuration active
	for(i = 0;activeConfigDesc->bNumInterfaces > i;i++)
	{
		for(j = 0;activeConfigDesc->interface[i].num_altsetting > j;j++)
		{
			struct libusb_interface_descriptor interface = activeConfigDesc->interface[i].altsetting[j];
			status=libusb_release_interface(handleArduino,interface.bInterfaceNumber);
			if(status!=0){ perror("libusb_release_interface"); exit(-1); }
			//printf("Fermeture de l'interface id = %d n°%d\n",interface.bInterfaceNumber,i);
		}
	}
	
	libusb_close(handleArduino);	//On ferme le périphérique
}

void initWindow()	//On initialise la fenêtre
{
    initscr();
    nodelay(stdscr, TRUE); //getchar() est non-bloquant
    noecho();	//On n'affiche pas le caractère de getchar()
    clear();    // Efface le contenu de la fenêtre (donc l'ancien message)
    curs_set(0);	//On efface le curseur
    attron(A_BOLD);	//Les 2 chaînes suivante seront en gras
    mvprintw(0, 0, "Moniteur USB pour Manette Arduino");
    mvprintw(1,0,"Auteurs : Amaury KNOCKAERT - Thomas CUNIN");
    attroff(A_BOLD);	//Enlève l'option gras
    mvprintw(2,0,"Appuyez sur la touche ECHAP pour quitter le programme.");
    mvprintw(3,0,"Le programme affiche ce qu'envoie la manette. Pour envoyer un message vers la carte Arduino il vous suffit d'appuyer sur une touche (sauf ECHAP).");
    mvprintw(4,0,"La LED s'allume lors de l'envoi de '1' et s'éteint pour '0'.");
    refresh();	//rafraîchit la fenêtre pour afficher les lignes déclarée précédemment.
}


char *touche(unsigned char code, char *touche)	//Convertie le code touche en chaîne affichable
{
	switch(code)
	{
		case JS:
			strcpy(touche,"Bouton joystick");
		break;
		case D3:
			strcpy(touche,"Droite (D3)");
		break;
		case D4:
			strcpy(touche,"Haut (D4)");
		break;
		case D5:
			strcpy(touche,"Bas (D5)");
		break;
		case D6:
			strcpy(touche,"Gauche (D6)");
		break;
		default:
			strcpy(touche,"Combinaison de touche");
		break;		
	}	
	return touche;
}

int main()
{

	//La variable status est une variable de test de fonctionnement des fonctions
	libusb_context *context;
	int status=libusb_init(&context);
	if(status!=0)
	{
		perror("libusb_init"); 
		exit(-1);
	}
	
	libusb_device *deviceArduino;
	libusb_device_handle *handleArduino;
	struct libusb_config_descriptor *configDesc;
	

	
	//On récupère l'arduino
	if(recuperationArduino(context,&deviceArduino, &handleArduino) != -1)
	{	//Si la manette est branchée et reconnue

		//On récupère la config
		recuperationConfig(deviceArduino,&configDesc);
	
	
		//On ouvre le périphérique
		status=libusb_open(deviceArduino,&handleArduino);
		if(status!=0){ perror("libusb_open"); exit(-1); }


		//On réclame les interfaces, on liste les endpoints et on récupère leurs adresses (indiquant aussi leur direction de transfert)
		struct liste *endpointAddresses;
		endpointAddresses = configPeriph(configDesc,handleArduino);
	
		//On crée la fenêtre
		initWindow();


		unsigned char *RX;
		RX = malloc(sizeof(unsigned char));
	

		int *transferred;
		transferred = malloc(sizeof(int));
		unsigned char car;
	
		char toucheDecode[SIZE];
	
		//Boucle infinie du programme
		while(1)
		{	//On réceptionne depuis le 16u2
			status = libusb_interrupt_transfer(handleArduino,endpointAddresses->valeur[1],RX,1,transferred,TIMEOUT);
			if(status == 0)	//Si un message est reçu
			{
				move(8, 0); //On se place en début de ligne
				clrtoeol(); //On efface la ligne précédent
				refresh();
				mvprintw(8,0,"Bouton appuyé : %s",touche(*RX,toucheDecode));
				refresh();
			}

			car = getch();
			if(car == ESC)	//On appuie sur ECHAP, on quitte la boucle
				break;
			if(car == ALLUME_LED)
			{	//On envois vers le 16u2
				libusb_interrupt_transfer(handleArduino,endpointAddresses->valeur[0],(unsigned char*)&car,1,transferred,TIMEOUT);
				mvprintw(9,0,"LED allumé");
				refresh();
			}
			if(car == ETEINT_LED)
			{	//On envois vers le 16u2
				libusb_interrupt_transfer(handleArduino,endpointAddresses->valeur[0],(unsigned char*)&car,1,transferred,TIMEOUT);
				mvprintw(9,0,"LED éteinte");
				refresh();
			}
		}



		//Zone de débuggage, visible dans le terminal après destruction de l a fenêtre
	
		printf("\nEndpoint OUT adresse : %d\n",endpointAddresses->valeur[0]);
		printf("\nEndpoint IN adresse : %d\n",endpointAddresses->valeur[1]);
		printf("\nTaille tableau adresse endpoints : %d\n",endpointAddresses->fin);
		free(RX);
		free(transferred);
		free(endpointAddresses);
		fermeture(deviceArduino,handleArduino);



		endwin();	//On détruit la fenêtre
	}else{
		printf("\nErreur : la manette USB n'a pas été trouvée ou reconnue\n");
	}
		libusb_exit(context);
		return 0;
}



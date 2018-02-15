PROJET TUTORAT SYSTEME : BUS USB
AUTEURS :	Amaury KNOCKAERT
		Thomas CUNIN


La manette apparaît en tant que : Manette Xbox360 Microsoft Corp.


					--Programme PC--

Le programme PC utilise la bilbiothèque ncurses qui permet de gérer une fenêtre dans un terminal.
Il suffit de le lancer et vous avez directement accès à la lecture des touches de l'arduino 
ainsi qu'à l'envoi de signaux ASCII '1' et '0' pour allumer la LED

					--Protocole USB--
Les programmes s'envoient 1 octets lors d'un transfert. Pour l'état des boutons, l'octet contient l'état de tous les boutons.
Il est codé par l'atmega328p avant d'être envoyé au 16u2.
Pour la LED, l'envoi du caractère '1' allume la LED et '0' l'éteint.

					--Protocole Série--
Les fonctions série fournie par LUFA sont utilisé côté atmega16u2. Pour l'atmega328p nous reçevons (la commande LED) par interruption
et envoyons l'état des boutons par interruption et scrutation.
Un unique octet est envoyé à chaque transfert.


----------------------------------DEPENDANCES--------------------------------

AVR-GCC,AVR-DUDE,AVR-LIBC
executez : sudo apt-get install gcc-avr avr-libc avrdude

LIBUSB-1.0
executez : sudo apt-get install libusb-1.0-0-dev

NCURSES
executez : sudo apt-get install libncurses-dev

LUFA:
http://www.fourwalledcubicle.com/LUFA.php

---------------------------------INSTALLATION---------------------------------

Placez le dossier LUFA dans le dossier Atmega16u2, à côté du dossier PAD.

----------------------------------UTILISATION---------------------------------



*****************Compilation***************************
La compilation de chacun des 3 programmes s'effectue en executant la commande "make" dans leur dossier respectif (Atmega16u2/PAD,Atmega328p et PC)




*****************Transfert des programmes**************
1. Commencez par transférer le programme de l'Atmega328P en executant "make upload" dans le dossier "Atmega328p".
2. Réinitialisez l'atmega16u2 en court-circuitant les broches "reset" et "ground"
3. Puis transférez le programme vers l'Atmega16u2 en executant dans le dossier Atmega16u2/PAD/, la commande "make dfu". Débranchez et rebranchez l'arduino.




*****************Execution du projet******************
- Branchez la carte sur un port USB.
- Placez-vous en tant que root dans le dossier "PC" et executez le programme usbPC.exe.





****************Réinitialiser l'atmega16u2************
1. Téléchargez le programme disponible à l'adresse :
	https://raw.githubusercontent.com/arduino/Arduino/master/hardware/arduino/avr/firmwares/atmegaxxu2/arduino-usbserial/Arduino-usbserial-uno.hex
2. Réinitialisez l'atmega16u2 (expliqué précédement).
3. Executez dans l'ordre :
	dfu-programmer atmega16u2 erase
	dfu-programmer atmega16u2 flash <programme de réinitialisation>.hex
	dfu-programmer atmega16u2 reset
4. Débranchez et rebranchez l'arduino.


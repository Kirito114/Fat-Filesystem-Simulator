
/*
**  main.c
**
**  Utilisation des fonctions du mini SGF.
**
**  04/04/2007
*/

#include <stdio.h>
#include <stdlib.h>

#include "sgf-disk.h"
#include "sgf-fat.h"
#include "sgf-dir.h"
#include "sgf-io.h"

int main() {
	OFILE* file;
	int c;
	
	init_sgf();
	
	printf("\nListing du disque\n\n");
	list_directory();

	/*Test de seek*/

	/*
	file = sgf_open("essai.txt", READ_MODE);
	int i = 8;
	sgf_seek(file, i);
	while ((c = sgf_getc(file)) > 0) {
		i+=8;
		sgf_seek(file,i);
		printf("char:");
		putchar(c);
		printf("\n");
	}
	sgf_close(file);
	*/

	/*Test de l ecriture*/
	/*file = sgf_open("essai.txt", WRITE_MODE);
	sgf_puts(file, "Ceci est un petit texte ui occupe\n");
	sgf_puts(file, "quelques blocs sur ce disque fictif.\n");
	sgf_puts(file, "Le bloc faisant 128 octets, il faut\n");
	sgf_puts(file, "que je remplisse pour utiliser\n");
	sgf_puts(file, "Encore un peu de texte pour tester si ça marche ou pas il faut savoir si ça fonctionne ou pas\n");
	sgf_puts(file, "Encore un peu de texte pour tester si ça marche ou pas il faut savoir si ça fonctionne ou pas\n");
	sgf_close(file);*/

	/*Test du append*/
	file = sgf_open("essai.txt", WRITE_MODE);
	sgf_close(file);
	unsigned i;
	printf("Wrote in write_mode\n");
	for(i=0;i<500;i++){
		file = sgf_open("essai.txt", APPEND_MODE);
		sgf_putc(file, 97+i%26);
		sgf_close(file);
	}
	file = sgf_open("essai.txt", READ_MODE);

	while((c=sgf_getc(file)) != -1){
		putchar('A');
	}
	sgf_close(file);
	
	
	return (EXIT_SUCCESS);
}


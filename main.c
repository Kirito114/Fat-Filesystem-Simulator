
/*
**  main.c
**
**  Utilisation des fonctions du mini SGF.
**
**  04/04/2007
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /*Question 6*/

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
	
	/*file = sgf_open("essai.txt", READ_MODE);
	int i = 8;
	sgf_seek(file, i);
	while ((c = sgf_getc(file)) > 0) {
		i+=8;
		if(sgf_seek(file,i) == -1) break;
		printf("char:");
		putchar(c);
		printf("\n");
	}
	sgf_close(file);*/




	

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

	/*file = sgf_open("essai.txt", WRITE_MODE);
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
	sgf_close(file);*/





	/*Test du sgf_write*/

	file = sgf_open("essai.txt", WRITE_MODE);
	char data[BLOCK_SIZE*500];
	memset(data,97,sizeof(data));
	data[sizeof(data)-1] = '\n';
	sgf_write(file, data, sizeof(data));
	sgf_close(file);
	sgf_open("essai.txt", READ_MODE);
	while((c=sgf_getc(file)) != -1){
		putchar(c);
	}
	struct DiskStats diskStats = getDiskStats();
	printf("%d EOF block(s)\n", diskStats.nb_eof_blocks);
	printf("%d INODE block(s)\n", diskStats.nb_inode_blocks);
	printf("%d RESERVED block(s)\n", diskStats.nb_reserved_blocks);
	printf("%d data block(s)\n", diskStats.nb_data_blocks);
	printf("%d free block(s) left\n", diskStats.nb_free_blocks);
	printf("%f kib(s) left, %d byte(s) left\n", diskStats.nb_free_bytes/1024.0, diskStats.nb_free_bytes);
	displayFatMap();
	sgf_close(file);
	
	
	return (EXIT_SUCCESS);
}


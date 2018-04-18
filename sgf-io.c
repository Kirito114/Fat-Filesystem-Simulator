
/*
**  sgf-io.c
**
**  fonctions de lecture/écriture (de caractères et de blocs)
**  dans un fichier ouvert.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> /*Question 6*/

#include "sgf-disk.h"
#include "sgf-data.h"
#include "sgf-fat.h"
#include "sgf-dir.h"
#include "sgf-io.h"



/**********************************************************************
 *
 *  FONCTIONS DE LECTURE DANS UN FICHIER
 *
 *********************************************************************/

/**********************************************************************
 Lire dans le "buffer" le bloc logique "nubloc" dans le fichier
 ouvert "file".
 
 ATTENTION: Faites en sorte de ne pas recommencer le chaînage à
            partir du bloc 0 si cela n'est pas utile. Pour éviter ce
            parcours vous pouvez ajouter un champ à la structure OFILE
            qui vous donne l'adresse physique du bloc courant.
 *********************************************************************/

void sgf_read_bloc(OFILE* file, int nubloc)
{
    assert(nubloc < (file->length+BLOCK_SIZE-1)/BLOCK_SIZE);

    /* Working first version */

    int adr;    

    adr = file->first;

    while(nubloc-->0){
        assert(adr>0);
        adr = get_fat(adr);
    }
    read_block(adr, &file->buffer);

    /* Working second version, not well tested though */

    /*if(nubloc == file->currentBlocNum) return;
    int crtBlocNum;
    int adr;
    if(nubloc > file->currentBlocNum && file->currentBlocNum != -1){
        crtBlocNum = file->currentBlocNum;
        adr = file->currentBlocAdr;
    }else{
        crtBlocNum = 0;
        adr = file->first;
    }

    while(nubloc-- > crtBlocNum){
        assert(adr>0);
        adr = get_fat(adr);
    }

    read_block(adr, &file->buffer);
    file->currentBlocNum = nubloc;
    file->currentBlocAdr = adr;*/
}


/**********************************************************************
 Lire un caractère dans un fichier ouvert. Cette fonction renvoie
 -1 si elle trouve la fin du fichier.
 *********************************************************************/

int sgf_getc(OFILE* file)
    {
    int c;
    
    assert (file->mode == READ_MODE);
    
    /* détecter la fin de fichier */
    if (file->ptr >= file->length)
        return (-1);

    /* si le buffer est vide, le remplir */
    if ((file->ptr % BLOCK_SIZE) == 0)
    {
        sgf_read_bloc(file, file->ptr / BLOCK_SIZE);
    }
    /* Recupere le caractere courant */
    c = file->buffer[ (file->ptr % BLOCK_SIZE) ];
    file->ptr ++;
    return (c);
    }



/**********************************************************************
 *
 *  FONCTIONS D'ECRITURE DANS UN FICHIER
 *
 *********************************************************************/

/**********************************************************************
 Ajouter le bloc contenu dans le tampon au fichier ouvert décrit
 par "f".
 *********************************************************************/

int sgf_append_block(OFILE* f)
{
    TBLOCK b;
    int adr;

    if(f->mode == WRITE_MODE){
        /* Si on est en mode ecriture, on cherche un nouveau bloc libre,
        on verifie bien qu un bloc libre etait disponible, puis on indique que
        ce nouveau bloc est le dernier bloc du fichier. Ensuite si ce nouveau bloc
        est le premier bloc du fichier on met l adresse du premier bloc et du dernier
        bloc a adr sinon on indique que l ancien dernier bloc pointer desormais sur le nouveau
        dernier bloc et que le dernier bloc est le nouveau bloc */
        adr = alloc_block();
        if(adr < 0) return -1;
        write_block(adr, &f->buffer);
        set_fat(adr, FAT_EOF);
        if(f->first == FAT_EOF)
            f->first = f->last = adr;
        else{
            set_fat(f->last, adr);
            f->last = adr;
        } 
    }else if(f->mode == APPEND_MODE){
        /*Si on est en mode append, on ecrase le dernier bloc avec le buffer complete puis on passe en write mode*/
        write_block(f->last, &f->buffer);
        f->mode = WRITE_MODE;
    }
    /* On met a jour la longueur du fichier en memoire et les informations de l inode sur le disque */
    f->length = f->ptr;
    b.inode.length = f->ptr;
    b.inode.first = f->first;
    b.inode.last = f->last;
    write_block(f->inode, &b.data);

    return 0;
}


/**********************************************************************
 Ecrire le caractère "c" dans le fichier ouvert décrit par "file".
 *********************************************************************/

int sgf_putc(OFILE* file, char  c)
{
    assert (file->mode == WRITE_MODE || file->mode == APPEND_MODE);

    /*On insere le caractere dans le buffer*/
    file->buffer[(file->ptr % BLOCK_SIZE)] = c;
    file->ptr++;

    /*On test si le buffer est plein dans quel cas on append le block (ou ecrase suivant le mode)*/
    if((file->ptr % BLOCK_SIZE) == 0){
        printf("[sgf_putc] : Buffer va etre appended\n");
        if(sgf_append_block(file) < 0){
            return -1;
        }
    }

    return 0;
}


/**********************************************************************
 Écrire la chaîne de caractère "s" dans un fichier ouvert en écriture
 décrit par "file".
 *********************************************************************/

void sgf_puts(OFILE* file, char* s)
    {
    assert (file->mode == WRITE_MODE);
    /* On ajoute un caractere au buffer tant que l on a pas atteint la fin de la chaine de caractere */
    for (; (*s != '\0'); s++) {
        sgf_putc(file, *s);
        }
    }



/**********************************************************************
 *
 *  FONCTIONS D'OUVERTURE, DE FERMETURE ET DE DESTRUCTION.
 *
 *********************************************************************/

/************************************************************
 Détruire un fichier.
 ************************************************************/

void sgf_remove(int adr_inode)
{
    TBLOCK b;
    int adr, suivant;
    read_block(adr_inode, &b.data);

    /*Working version might fail if file has no data block yet*/

    /*adr = b.inode.first;
    do{
        suivant = get_fat(adr);
        set_fat(adr, FAT_FREE);
        adr = suivant;
    }while(suivant != FAT_EOF);*/

    /*Second working version just in case file has no data block yet*/
    /*On met tout les blocs du fichier a Free les rendant a nouveau disponible sur le disque*/
    suivant = adr = b.inode.first;
    while(suivant != FAT_EOF){
        suivant = get_fat(adr);
        set_fat(adr, FAT_FREE);
        adr = suivant;
    }
    /*Puis on supprime l inode du disque*/
    set_fat(adr_inode, FAT_FREE);
    /*On affiche des informations sur le disque*/
    printf("FAT State:\n");
    printf("    NB_FREE_BLOCKS : %d\n",get_free_fat_blocks_count());
}


/************************************************************
 Ouvrir un fichier en écriture seulement (NULL si échec).
 ************************************************************/

static  OFILE*  sgf_open_write(const char* nom)
{
    int inode, oldinode;
    OFILE* file;
    TBLOCK b;

    /* Rechercher un bloc libre sur disque */
    inode = alloc_block();
    assert (inode >= 0);

    /* Allouer une structure OFILE */
    file = malloc(sizeof(struct OFILE));
    if (file == NULL) return (NULL);
    
    /* préparer un inode vers un fichier vide */
    b.inode.length = 0;
    b.inode.first  = FAT_EOF;
    b.inode.last   = FAT_EOF;

    /* sauver ce inode */
    write_block(inode, &b.data);
    set_fat(inode, FAT_INODE);

    /* mettre a jour le repertoire */
    oldinode = add_inode(nom, inode);
    if (oldinode > 0) sgf_remove(oldinode);
    
    file->length  = 0;
    file->first   = FAT_EOF;
    file->last    = FAT_EOF;
    file->inode   = inode;
    file->mode    = WRITE_MODE;
    file->ptr     = 0;

    return (file);
}


/************************************************************
 Ouvrir un fichier en lecture seulement (NULL si échec).
 ************************************************************/

static  OFILE*  sgf_open_read(const char* nom)
{
    int inode;
    OFILE* file;
    TBLOCK b;
    
    /* Chercher le fichier dans le répertoire */
    inode = find_inode(nom);
    if (inode < 0) return (NULL);
    
    /* lire le inode */
    read_block(inode, &b.data);
    
    /* Allouer une structure OFILE */
    file = malloc(sizeof(struct OFILE));
    if (file == NULL) return (NULL);
    
    file->length  = b.inode.length;
    file->first   = b.inode.first;
    file->last    = b.inode.last;
    file->inode   = inode;
    file->mode    = READ_MODE;
    file->ptr     = 0;
    file->currentBlocNum = -1;
    file->currentBlocAdr = -1;
    
    return (file);
}

/************************************************************
 Ouvrir un fichier en append (NULL si échec).
 ************************************************************/

static  OFILE*  sgf_open_append(const char* nom)
{
    int inode;
    OFILE* file;
    TBLOCK b;
    
    /* Chercher le fichier dans le répertoire */
    inode = find_inode(nom);
    if (inode < 0) return (NULL);
    
    /* lire le inode */
    read_block(inode, &b.data);
    
    /* Allouer une structure OFILE */
    file = malloc(sizeof(struct OFILE));
    if (file == NULL) return (NULL);
    
    file->length  = b.inode.length;
    file->first   = b.inode.first;
    file->last    = b.inode.last;
    file->inode   = inode;
    file->mode    = APPEND_MODE;
    file->ptr     = file->length;

    /*Si un bloc est incomplet on le charge sinon on passe directement en mode ecriture*/
    if((file->length%BLOCK_SIZE) != 0){
        printf("[sgf_open_append] : reading incomplete block\n");
        read_block(file->last, &file->buffer);
        printf("[sgf_open_append] : sucessfully read incomplete block\n");
    }else{
        file->mode = WRITE_MODE;
    }
    
    return (file);
}


/************************************************************
 Ouvrir un fichier (NULL si échec).
 ************************************************************/

OFILE* sgf_open (const char* nom, int mode)
    {
    switch (mode)
        {
        case READ_MODE:  return sgf_open_read(nom);
        case WRITE_MODE: return sgf_open_write(nom);
        case APPEND_MODE: return sgf_open_append(nom);
        default:         return (NULL);
        }
    }


/************************************************************
 Fermer un fichier ouvert.
 ************************************************************/

int sgf_close(OFILE* file)
{
    /* Cette fonction s assure que toutes les donnees dans le buffer n ayant pas encore ete ecrites sur le disque le sont a present */
    if(file->mode ==  WRITE_MODE || file->mode == APPEND_MODE){
        if((file->ptr % BLOCK_SIZE) != 0){
            /*Le append block modifie deja l inode*/
            if(sgf_append_block(file) < 0){
                return -1;
            }
        }
    }

    free(file);
    file = NULL;

    return 0;
}


/**********************************************************************
 initialiser le SGF
 *********************************************************************/

void init_sgf (void)
    {
    init_sgf_disk();
    init_sgf_fat();
    }


/**********************************************************************
 * Réalise le déplacement du pointeur ptr en lecture
 *********************************************************************/
    
int sgf_seek (OFILE* f, int pos){
    assert(f->mode == READ_MODE);
    /*Position hors des bornes, on indique une erreur*/
    if(pos < 0 || pos > (f->length - 1))
        return -1;
    /*Load block if buffer is empty, especially if sgf_seek is used before sgf_getc*/
    if ((f->ptr % BLOCK_SIZE) == 0)
    {
        printf("sgf_seek loaded block into buffer\n");
        sgf_read_bloc(f, f->ptr / BLOCK_SIZE);
    }
    /*La nouvelle position est un multiple de BLOCK_SIZE, la fonction sgf_getc s occupera du chargement du bloc*/
    if(pos%BLOCK_SIZE == 0){
        f->ptr = pos;
        printf("sgf_seek : getchar will load block\n");
        return 0;
    }
    /*La nouvelle position se trouve dans le meme bloc, on change uniquement la position du pointeur dans le fichier*/
    if(pos/BLOCK_SIZE == f->ptr/BLOCK_SIZE){
        f->ptr = pos;
        printf("sgf_seek : same block\n");
        return 0;
    }

    /*Derniers recours on charge le nouveau bloc vers le buffer*/
    sgf_read_bloc(f, pos/BLOCK_SIZE);
    f->ptr = pos;
    printf("sgf_seek : load block\n");

    return 0;
}

int sgf_write(OFILE* f, char *data, int size){
    /*Only allow sgf_write with write mode and append mode*/
    assert(f->mode == WRITE_MODE || f->mode == APPEND_MODE);

    /*Check weather or not disk space is large enough to fit new data*/
    unsigned freeBlocksCount = get_free_fat_blocks_count();
    if(size >= freeBlocksCount*BLOCK_SIZE){
        fprintf(stderr, "[sgf_write] : Not enough space left to write desired data block\n");
        return -1;
    }

    /*Iterate while all the data has not yet been written*/

    /*First version*/
    /*unsigned writtenBytes = 0;
    while(writtenBytes < size){
        f->buffer[f->ptr%BLOCK_SIZE] = *(data+writtenBytes++);
        f->ptr++;
        if((f->ptr%BLOCK_SIZE) == 0){
            sgf_append_block(f);
        }
    }*/

    /*Second Version with memcpy*/
    /* Write as long as all bytes have not yet been written to the disk, this function either writes BLOCK_SIZE per BLOCK_SIZE or fills left space
    in the buffer in append mode or just writes left bytes to the buffer depending on the situation */
    unsigned writtenBytes = 0;
    while(writtenBytes < size){
        unsigned amountToWrite = ((BLOCK_SIZE - (f->ptr%BLOCK_SIZE)) >= (size-writtenBytes)) ? (size-writtenBytes) : BLOCK_SIZE - (f->ptr%BLOCK_SIZE);
        memcpy(f->buffer+(f->ptr%BLOCK_SIZE), data+writtenBytes, amountToWrite);
        writtenBytes += amountToWrite;
        f->ptr += amountToWrite;
        printf("[sgf_write] Progress : %d bytes of %d\n",writtenBytes,size);
        if((f->ptr%BLOCK_SIZE) == 0){
            sgf_append_block(f);
        }
    }

    return 0;
}
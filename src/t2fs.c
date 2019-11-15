/**
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"
#include "../include/t2disk.h"

typedef struct mbr
{
    unsigned short int versao;
    unsigned short int tam_setor;
    unsigned short int inicioTabelaParticoes;
    unsigned short int qntParticoes;

    int endPrimeiroBlocoPartZero;
    int endUltimoBlocoPartZero;
    char nomePartZero[24];

    int endPrimeiroBlocoPartUm;
    int endUltimoBlocoPartUm;
    char nomePartUm[24];

    int endPrimeiroBlocoPartDois;
    int endUltimoBlocoPartDois;
    char nomePartDois[24];

    int endPrimeiroBlocoPartTres;
    int endUltimoBlocoPartTres;
    char nomePartTres[24];

} mbr_data;

mbr_data mbrData;

int readMbr()
{




    char buffer[SECTOR_SIZE] = {'c'};
    read_sector(0, buffer);
    int i = 0;
    int j = 0;


    for (i = 0; i < 256; i ++)
    {
        for (j = 0; j < 8; j ++)
        {
            printf("%d", !!((buffer[i] << j) & 0x80));
        }
        printf("\n");
    }

    printf("\n\n\n");
    unsigned int mask = 0;
    unsigned int shift = 0;

    for (i = 0; i < 2; i ++)
    {
        if (i == 0)
        {
            mask = 255;
            shift = 0;
        }
        else
        {
            mask = 65280;
            shift = 8;
        }
        mbrData.versao = mbrData.versao | ((4294967295 & buffer[i]) << shift);
    }

    for (i = 2; i < 4; i ++)
    {
        if (i == 2)
        {
            mask = 255;
            shift = 0;
        }
        else
        {
            mask = 65280;
            shift = 8;
        }
        mbrData.tam_setor = mbrData.tam_setor | ((4294967295 & buffer[i]) << shift);
    }


    for (i = 4; i < 6; i ++)
    {
        if (i == 4)
        {
            mask = 255;
            shift = 0;
        }
        else
        {
            mask = 65280;
            shift = 8;
        }
        mbrData.inicioTabelaParticoes = mbrData.inicioTabelaParticoes | ((4294967295 & buffer[i]) << shift);
    }


    for (i = 6; i < 8; i ++)
    {
        if (i == 6)
        {
            mask = 255;
            shift = 0;
        }
        else
        {
            mask = 65280;
            shift = 8;
        }
        mbrData.qntParticoes = mbrData.qntParticoes | ((4294967295 & buffer[i]) << shift);
    }


    for (i = 8; i < 12; i ++)
    {
        if (i == 8)
        {
            mask = 255;
            shift = 0;
        }
        else if (i == 9)
        {
            mask = 65280;
            shift = 8;
        }
        else if (i == 10)
        {
            mask = 16711680;
            shift = 16;
        }
        else
        {
            mask = 4278190080;
            shift = 24;
        }
        mbrData.endPrimeiroBlocoPartZero = mbrData.endPrimeiroBlocoPartZero | ((4294967295 & buffer[i]) << shift);
        mbrData.endPrimeiroBlocoPartUm = mbrData.endPrimeiroBlocoPartUm | ((4294967295 & buffer[i + 32]) << shift);
        mbrData.endPrimeiroBlocoPartDois = mbrData.endPrimeiroBlocoPartDois | ((4294967295 & buffer[i + 64]) << shift);
        mbrData.endPrimeiroBlocoPartTres = mbrData.endPrimeiroBlocoPartTres | ((4294967295 & buffer[i + 96]) << shift);
    }



    for (i = 12; i < 16; i ++)
    {
        if (i == 12)
        {
            mask = 255;
            shift = 0;
        }
        else if (i == 13)
        {
            mask = 65280;
            shift = 8;
        }
        else if (i == 14)
        {
            mask = 16711680;
            shift = 16;
        }
        else
        {
            mask = 4278190080;
            shift = 24;
        }
        mbrData.endUltimoBlocoPartZero = mbrData.endUltimoBlocoPartZero | ((4294967295 & buffer[i]) << shift);
        mbrData.endUltimoBlocoPartUm = mbrData.endUltimoBlocoPartUm | ((4294967295 & buffer[i+32]) << shift);
        mbrData.endUltimoBlocoPartDois = mbrData.endUltimoBlocoPartDois | ((4294967295 & buffer[i+64]) << shift);
        mbrData.endUltimoBlocoPartTres = mbrData.endUltimoBlocoPartTres | ((4294967295 & buffer[i+96]) << shift);
    }



    for(i = 0; i < 24; i++){
        mbrData.nomePartZero[i] = buffer[16+i];
        mbrData.nomePartUm[i] = buffer[48+i];
        mbrData.nomePartDois[i] = buffer[80+i];
        mbrData.nomePartTres[i] = buffer[112+i];
    }

    printf("VERSAO = %d\n", mbrData.versao);
    printf("TAM_SETOR = %d\n", mbrData.tam_setor);
    printf("INICIO TABELA = %d\n", mbrData.inicioTabelaParticoes);
    printf("NUMERO DE PAR TICOES = %d\n", mbrData.qntParticoes);

    printf("\nNOME PART ZERO %s\n", mbrData.nomePartZero);
    printf("ENDERECO PRIMEIRO BLOCO PART ZERO = %d\n", mbrData.endPrimeiroBlocoPartZero);
    printf("ENDERECO ULTIMO BLOCO PART ZERO = %d\n", mbrData.endUltimoBlocoPartZero);

    printf("\nNOME PART UM %s\n", mbrData.nomePartUm);
    printf("ENDERECO PRIMEITO BLOCO PART UM = %d\n", mbrData.endPrimeiroBlocoPartUm);
    printf("ENDERECO ULTIMO BLOCO PART UM = %d\n", mbrData.endUltimoBlocoPartUm);

    printf("\nNOME PART DOIS %s\n", mbrData.nomePartDois);
    printf("ENDERECO PRIMEIRO BLOCO PART DOIS = %d\n", mbrData.endPrimeiroBlocoPartDois);
    printf("ENDERECO ULTIMO BLOCO PART DOIS = %d\n", mbrData.endUltimoBlocoPartDois);

    printf("\nNOME PART TRES %s\n", mbrData.nomePartTres);
    printf("ENDERECO PRIMEIRO BLOCO PART TRES = %d\n", mbrData.endPrimeiroBlocoPartTres);
    printf("ENDERECO ULTIMO BLOCO PART TRES = %d\n", mbrData.endUltimoBlocoPartTres);

    return 0;

}
int readSuper()
{
    unsigned char buffer[SECTOR_SIZE];
    if(read_sector(0, buffer) != 0)
    {
        printf("Error: Failed reading sector 0!\n");
        return -1;
    }
    strncpy(Super.id, (char*)buffer, 4);
    Super.version = *( (DWORD*)(buffer + 4) );
    Super.superblockSize = *( (WORD*)(buffer + 6) );
    Super.freeBlocksBitmapSize = *( (WORD*)(buffer + 8) );
    Super.freeInodeBitmapSize = *( (WORD*)(buffer + 10) );
    Super.inodeAreaSize = *( (WORD*)(buffer + 12) );
    Super.blockSize = *( (WORD*)(buffer + 14) );
    Super.diskSize = *( (DWORD*)(buffer + 16) );
    return 0;
}

/*-----------------------------------------------------------------------------
Função:	Informa a identificação dos desenvolvedores do T2FS.
-----------------------------------------------------------------------------*/
int identify2 (char *name, int size)
{
    int i = 0;
    char names[71] = {0};

    strcpy (names, "Demetrio Boeira - 297693\nGustavo Lorentz - 287681\nPedro Weber - 287678\n");

    for (i=0; i < size; i++)
    {
        name[i] = names[i];
    }

    if (strcmp(names, name) == 0)
    {
        return 0;
    }


    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Formata logicamente uma partição do disco virtual t2fs_disk.dat para o sistema de
		arquivos T2FS definido usando blocos de dados de tamanho
		corresponde a um múltiplo de setores dados por sectors_per_block.
-----------------------------------------------------------------------------*/
int format2(int partition, int sectors_per_block)
{

    for(i = 0; i < Super.diskSize; i++)
        setBitmap2(BITMAP_DADOS, i, 0);

    for ( i = 0; i < Super.SuperSize; i++)
        setBitmap2(BITMAP_DADOS, i, 1);

    for ( i = 0; i < Super.freeBlockBitmapSize; i++)
        setBitmap2(BITMAP_DADOS, Super.SuperSize + i, 1);

    for ( i =0; i < Super.freeInodeBitmapSize; i++)
        setBitmap2(BITMAP_DADOS, Super.SuperSize + Super.freeBlocksBitmapSize + i, 1);

    for ( i = 0; i< Super.inodeAreaSize; i++)
        setBitmap2(BITMAP_DADOS, Super.SuperSize + Super.freeBlocksBitmapSize + Super.freeInodeBitmapSize + i, 1);

    for (i = 0; i < Super.inodeAreaSize * iNodesPerSector * Super.blockSize; i++)
        setBitmap2(BITMAP_INODE, i, 0);

    setBitmap2(BITMAP_INODE, 0, 1);

}

/*-----------------------------------------------------------------------------
Função:	Monta a partição indicada por "partition" no diretório raiz
-----------------------------------------------------------------------------*/
int mount(int partition)
{

}

/*-----------------------------------------------------------------------------
Função:	Desmonta a partição atualmente montada, liberando o ponto de montagem.
-----------------------------------------------------------------------------*/
int unmount(void)
{
    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para criar um novo arquivo no disco e abrí-lo,
		sendo, nesse último aspecto, equivalente a função open2.
		No entanto, diferentemente da open2, se filename referenciar um
		arquivo já existente, o mesmo terá seu conteúdo removido e
		assumirá um tamanho de zero bytes.
-----------------------------------------------------------------------------*/
FILE2 create2 (char *filename)
{
    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para remover (apagar) um arquivo do disco.
-----------------------------------------------------------------------------*/
int delete2 (char *filename)
{
    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função que abre um arquivo existente no disco.
-----------------------------------------------------------------------------*/
FILE2 open2 (char *filename)
{
    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para fechar um arquivo.
-----------------------------------------------------------------------------*/
int close2 (FILE2 handle)
{
    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para realizar a leitura de uma certa quantidade
		de bytes (size) de um arquivo.
-----------------------------------------------------------------------------*/
int read2 (FILE2 handle, char *buffer, int size)
{
    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para realizar a escrita de uma certa quantidade
		de bytes (size) de  um arquivo.
-----------------------------------------------------------------------------*/
int write2 (FILE2 handle, char *buffer, int size)
{
    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função que abre um diretório existente no disco.
-----------------------------------------------------------------------------*/
DIR2 opendir2 (void)
{
    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para ler as entradas de um diretório.
-----------------------------------------------------------------------------*/
int readdir2 (DIRENT2 *dentry)
{
    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para fechar um diretório.
-----------------------------------------------------------------------------*/
int closedir2 (void)
{
    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para criar um caminho alternativo (softlink)
-----------------------------------------------------------------------------*/
int sln2 (char *linkname, char *filename)
{
    return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para criar um caminho alternativo (hardlink)
-----------------------------------------------------------------------------*/
int hln2(char *linkname, char *filename)
{
    return -1;
}




/**
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"
#include "../include/t2disk.h"
typedef struct t2fs_superbloco superbloco;
typedef struct mbr
{
    WORD versao;
    WORD tam_setor;
    WORD inicioTabelaParticoes;
    WORD qntParticoes;

    DWORD endPrimeiroBlocoPartZero;
    DWORD endUltimoBlocoPartZero;
    BYTE nomePartZero[24];

    DWORD endPrimeiroBlocoPartUm;
    DWORD endUltimoBlocoPartUm;
    BYTE nomePartUm[24];

    DWORD endPrimeiroBlocoPartDois;
    DWORD endUltimoBlocoPartDois;
    BYTE nomePartDois[24];

    DWORD endPrimeiroBlocoPartTres;
    DWORD endUltimoBlocoPartTres;
    BYTE nomePartTres[24];

} mbr_data;

mbr_data mbrData;
superbloco Super;

int roundUp(float x){
    if (x - (int) x > 0){
        return (int) x + 1;
    }

    return (int) x;
}

int logMbr()
{
    read_sector(0, (BYTE *)&mbrData);

    printf("VERSAO = %d\n", mbrData.versao);
    printf("TAM_SETOR = %d\n", mbrData.tam_setor);
    printf("INICIO TABELA = %d\n", mbrData.inicioTabelaParticoes);
    printf("NUMERO DE PARTICOES = %d\n", mbrData.qntParticoes);

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
int readSuper(int partition)
{
    int sectorToRead = 0;
    int i = 0;
    if (partition == 0){
        sectorToRead = mbrData.endPrimeiroBlocoPartZero;
    }
    else if (partition == 1){
        sectorToRead = mbrData.endPrimeiroBlocoPartUm;
    }
    else if (partition == 2){
        sectorToRead = mbrData.endPrimeiroBlocoPartDois;
    }
    else if (partition == 3){
        sectorToRead = mbrData.endPrimeiroBlocoPartTres;
    }

    printf(" \nSECTOR TO READ = %d\n", sectorToRead);

    BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
    if(read_sector(sectorToRead, buffer) != 0)
    {
        printf("Error: Failed reading sector 0!\n");
        return -1;
    }

    memcpy(&Super, buffer, sizeof(superbloco));

    printf("SUPER VERSION = %d\n", (int) Super.version);
    printf("SUPER ID = ");
    for (i = 0; i < 4; i++){
        printf("%c", Super.id[i]);

    }
    printf("\n");
    printf("SUPER BLOCKSIZE = %d\n", (int) Super.superblockSize);
    printf("SUPER BITMAP SIZE = %d\n", (int) Super.freeBlocksBitmapSize);
    printf("SUPER INODEBITMAP SIZE = %d\n", (int) Super.freeInodeBitmapSize);
    printf("SUPER INODEAREA SIZE = %d\n", (int) Super.inodeAreaSize);

    return 0;
}
//
///*-----------------------------------------------------------------------------
//Função:	Informa a identificação dos desenvolvedores do T2FS.
//-----------------------------------------------------------------------------*/
//int identify2 (char *name, int size)
//{
//    int i = 0;
//    char names[71] = {0};
//
//    strcpy (names, "Demetrio Boeira - 297693\nGustavo Lorentz - 287681\nPedro Weber - 287678\n");
//
//    for (i=0; i < size; i++)
//    {
//        name[i] = names[i];
//    }
//
//    if (strcmp(names, name) == 0)
//    {
//        return 0;
//    }
//
//
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Formata logicamente uma partição do disco virtual t2fs_disk.dat para o sistema de
//		arquivos T2FS definido usando blocos de dados de tamanho
//		corresponde a um múltiplo de setores dados por sectors_per_block.
//-----------------------------------------------------------------------------*/
int format2(int partition, int sectors_per_block)
{
//    readSuper(partition);
    int i = 0;
    strcpy(Super.id, "T2FS\0");
    Super.version = 32306;
    Super.superblockSize = 1;
    Super.blockSize = sectors_per_block;
    int firstSector = 0;
    if (partition == 0){
        Super.diskSize = mbrData.endUltimoBlocoPartZero - mbrData.endPrimeiroBlocoPartZero + 1;
        firstSector = mbrData.endPrimeiroBlocoPartZero;

    }
    else if (partition == 1){
        Super.diskSize = mbrData.endUltimoBlocoPartUm - mbrData.endPrimeiroBlocoPartUm + 1;
        firstSector = mbrData.endPrimeiroBlocoPartUm;
    }
    else if (partition == 2){
        Super.diskSize = mbrData.endUltimoBlocoPartDois - mbrData.endPrimeiroBlocoPartDois + 1;
        firstSector = mbrData.endPrimeiroBlocoPartDois;
    }
    else if (partition == 3){
        Super.diskSize = mbrData.endUltimoBlocoPartTres - mbrData.endPrimeiroBlocoPartTres + 1;
        firstSector = mbrData.endPrimeiroBlocoPartTres;
    }
    printf("DISK SIZE = %d e BLOCK SIZE = %d\n", Super.diskSize, Super.blockSize);
    Super.inodeAreaSize = roundUp(0.1 * Super.diskSize);

    Super.freeInodeBitmapSize = roundUp( (float) ( (float) Super.inodeAreaSize / ( (float) SECTOR_SIZE * 8.0 * (float) Super.blockSize)));
    Super.freeBlocksBitmapSize = roundUp( (float) (0.9 * (float) Super.diskSize / ((float) SECTOR_SIZE * 8.0 * (float) Super.blockSize)));

//    printf("RESULTADO DA CONTA = %.20f\n", (Super.inodeAreaSize / (SECTOR_SIZE * 8.0 * Super.blockSize)));
//    printf("INodeAreaSize = %d e FreeINodeBitmapSize = %d e BlockSize = %d\n", Super.inodeAreaSize, Super.freeInodeBitmapSize, Super.blockSize );
    int iNodesPerSector = roundUp(Super.inodeAreaSize / (Super.freeInodeBitmapSize * Super.blockSize));

    for(i = 0; i < Super.diskSize; i++)
        setBitmap2(BITMAP_DADOS, i, 0);

    for ( i = 0; i < Super.superblockSize; i++)
        setBitmap2(BITMAP_DADOS, i, 1);

    for ( i = 0; i < Super.freeBlocksBitmapSize; i++)
        setBitmap2(BITMAP_DADOS, Super.superblockSize + i, 1);

    for ( i =0; i < Super.freeInodeBitmapSize; i++)
        setBitmap2(BITMAP_DADOS, Super.superblockSize + Super.freeBlocksBitmapSize + i, 1);

    for ( i = 0; i< Super.inodeAreaSize; i++)
        setBitmap2(BITMAP_DADOS, Super.superblockSize + Super.freeBlocksBitmapSize + Super.freeInodeBitmapSize + i, 1);

    for (i = 0; i < Super.inodeAreaSize * iNodesPerSector * Super.blockSize; i++)
        setBitmap2(BITMAP_INODE, i, 0);

    setBitmap2(BITMAP_INODE, 0, 1);

    BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
    memcpy(buffer, &Super, sizeof(superbloco));

    write_sector(firstSector, buffer);
}
//
///*-----------------------------------------------------------------------------
//Função:	Monta a partição indicada por "partition" no diretório raiz
//-----------------------------------------------------------------------------*/
//int mount(int partition)
//{
//
//}
//
///*-----------------------------------------------------------------------------
//Função:	Desmonta a partição atualmente montada, liberando o ponto de montagem.
//-----------------------------------------------------------------------------*/
//int unmount(void)
//{
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para criar um novo arquivo no disco e abrí-lo,
//		sendo, nesse último aspecto, equivalente a função open2.
//		No entanto, diferentemente da open2, se filename referenciar um
//		arquivo já existente, o mesmo terá seu conteúdo removido e
//		assumirá um tamanho de zero bytes.
//-----------------------------------------------------------------------------*/
//FILE2 create2 (char *filename)
//{
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para remover (apagar) um arquivo do disco.
//-----------------------------------------------------------------------------*/
//int delete2 (char *filename)
//{
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Função que abre um arquivo existente no disco.
//-----------------------------------------------------------------------------*/
//FILE2 open2 (char *filename)
//{
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para fechar um arquivo.
//-----------------------------------------------------------------------------*/
//int close2 (FILE2 handle)
//{
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para realizar a leitura de uma certa quantidade
//		de bytes (size) de um arquivo.
//-----------------------------------------------------------------------------*/
//int read2 (FILE2 handle, char *buffer, int size)
//{
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para realizar a escrita de uma certa quantidade
//		de bytes (size) de  um arquivo.
//-----------------------------------------------------------------------------*/
//int write2 (FILE2 handle, char *buffer, int size)
//{
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Função que abre um diretório existente no disco.
//-----------------------------------------------------------------------------*/
//DIR2 opendir2 (void)
//{
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para ler as entradas de um diretório.
//-----------------------------------------------------------------------------*/
//int readdir2 (DIRENT2 *dentry)
//{
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para fechar um diretório.
//-----------------------------------------------------------------------------*/
//int closedir2 (void)
//{
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para criar um caminho alternativo (softlink)
//-----------------------------------------------------------------------------*/
//int sln2 (char *linkname, char *filename)
//{
//    return -1;
//}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para criar um caminho alternativo (hardlink)
//-----------------------------------------------------------------------------*/
//int hln2(char *linkname, char *filename)
//{
//    return -1;
//}
//
//
//

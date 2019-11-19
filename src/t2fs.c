/**
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"
#include "../include/t2disk.h"
#include "../include/linked.h"

int mountedPartition;
typedef struct t2fs_superbloco superbloco;
typedef struct t2fs_inode inode;
typedef struct t2fs_record record;
typedef struct Node node;

node* current_files;

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
int initialized = 0;

int initT2FS ()
{
    if (initialized == 0)
    {
        if (read_sector(0, (BYTE *)&mbrData) != 0)
        {
            printf("Error while reading MBR\n");
            return -1;
        }
        initialized = 1;
    }

    return 0;
}

int roundUp(float x)
{
    if (x - (int) x > 0)
    {
        return (int) x + 1;
    }

    return (int) x;
}

int logMbr()
{
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
    if (partition == 0)
    {
        sectorToRead = mbrData.endPrimeiroBlocoPartZero;
    }
    else if (partition == 1)
    {
        sectorToRead = mbrData.endPrimeiroBlocoPartUm;
    }
    else if (partition == 2)
    {
        sectorToRead = mbrData.endPrimeiroBlocoPartDois;
    }
    else if (partition == 3)
    {
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
    for (i = 0; i < 4; i++)
    {
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
int identify2 (char *name, int size)
{
    initT2FS();

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
//
///*-----------------------------------------------------------------------------
//Função:	Formata logicamente uma partição do disco virtual t2fs_disk.dat para o sistema de
//		arquivos T2FS definido usando blocos de dados de tamanho
//		corresponde a um múltiplo de setores dados por sectors_per_block.
//-----------------------------------------------------------------------------*/
int format2(int partition, int sectors_per_block)
{
    initT2FS();

    int j = 0;
    int i = 0;
    strcpy(Super.id, "T2FS");
    Super.version = 32306;
    Super.superblockSize = 1;
    Super.blockSize = sectors_per_block;
    int firstSector = 0;

    if (partition == 0)
    {
        Super.diskSize = mbrData.endUltimoBlocoPartZero - mbrData.endPrimeiroBlocoPartZero + 1;
        firstSector = mbrData.endPrimeiroBlocoPartZero;
    }
    else if (partition == 1)
    {
        Super.diskSize = mbrData.endUltimoBlocoPartUm - mbrData.endPrimeiroBlocoPartUm + 1;
        firstSector = mbrData.endPrimeiroBlocoPartUm;
    }
    else if (partition == 2)
    {
        Super.diskSize = mbrData.endUltimoBlocoPartDois - mbrData.endPrimeiroBlocoPartDois + 1;
        firstSector = mbrData.endPrimeiroBlocoPartDois;
    }
    else if (partition == 3)
    {
        Super.diskSize = mbrData.endUltimoBlocoPartTres - mbrData.endPrimeiroBlocoPartTres + 1;
        firstSector = mbrData.endPrimeiroBlocoPartTres;
    }
//
//    printf("\nDISK SIZE = %d e BLOCK SIZE = %d\n", Super.diskSize, Super.blockSize);
//    printf("FIRST SECTOR = %d\n", firstSector);

    float rawInodeAreaSize = 0.1f * ((float)Super.diskSize);
    Super.inodeAreaSize = roundUp(rawInodeAreaSize);

    int numberOfInodes = roundUp((float)(((float)(Super.inodeAreaSize * SECTOR_SIZE * sectors_per_block)) / 32.0f));

    int inodeBitmapSizeBytes = roundUp((float)numberOfInodes / 8.0f);
    float rawfreeInodeBitmapSizeBlocks = inodeBitmapSizeBytes / (float)(sectors_per_block * SECTOR_SIZE);

    Super.freeInodeBitmapSize = roundUp(rawfreeInodeBitmapSizeBlocks);

    int numberOfBlocks = roundUp(Super.diskSize);

    int blocksBitmapSizeBytes = roundUp((float)numberOfBlocks / 8.0f);
    float rawfreeBlocksBitmapSizeBlocks = blocksBitmapSizeBytes / (float)(sectors_per_block * SECTOR_SIZE);

    Super.freeBlocksBitmapSize = roundUp(rawfreeBlocksBitmapSizeBlocks);

    BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);

//
//    printf("\nInode Area in bytes = %d\nInode size in bytes = 32\nNumber of Inodes = %d\nInode bitmap size in bytes = %d\nInode bitmap size in blocks = %d\n", Super.inodeAreaSize * SECTOR_SIZE * sectors_per_block, numberOfInodes, inodeBitmapSizeBytes, Super.freeInodeBitmapSize);
//    printf("\n\nBlocks Area in bytes = %d\nBlock size in bytes = %d\nNumber of Blocks = %d\nBlocks bitmap size in bytes = %d\nBlocks bitmap size in blocks = %d\n", numberOfBlocks * sectors_per_block * SECTOR_SIZE, Super.blockSize * SECTOR_SIZE, numberOfBlocks, blocksBitmapSizeBytes, Super.freeBlocksBitmapSize);

    int iNodesPerSector = roundUp((float)(((float) Super.inodeAreaSize) / ((float) Super.freeInodeBitmapSize * (float) Super.blockSize)));

    memcpy(buffer, &Super, sizeof(superbloco));
    write_sector(firstSector, buffer);

    openBitmap2(firstSector);

    //zera tudo
    for(i = 0; i < Super.diskSize; i++)
        setBitmap2(BITMAP_DADOS, i, 0);

    //marca blocos usados pro superbloco
    for ( i = 0; i < Super.superblockSize; i++)
        setBitmap2(BITMAP_DADOS, i, 1);

    //marca blocos do bitmap de dados
    for ( i = 0; i < Super.freeBlocksBitmapSize; i++)
        setBitmap2(BITMAP_DADOS, Super.superblockSize + i, 1);

    //marca blocos do bitmap de inodes
    for ( i =0; i < Super.freeInodeBitmapSize; i++)
        setBitmap2(BITMAP_DADOS, Super.superblockSize + Super.freeBlocksBitmapSize + i, 1);

    //marca blocos dos proprios inodes
    for ( i = 0; i< Super.inodeAreaSize; i++)
        setBitmap2(BITMAP_DADOS, Super.superblockSize + Super.freeBlocksBitmapSize + Super.freeInodeBitmapSize + i, 1);


    //                             * Super.blockSize
    for (i = 0; i < numberOfInodes                  ; i++) //usei numberOfInodes ao inves de Super.inodeAreaSize * iNodesPerSector
        setBitmap2(BITMAP_INODE, i, 0);

    closeBitmap2();

//    openBitmap2(firstSector);
//    printf("Bloco livre ao final da format = %d\n", searchBitmap2(BITMAP_DADOS, 0));




//    read_sector(firstSector, buffer);
//
//    DWORD temp;
//    DWORD checksum = 0;
//
//    // MUDAR AQUI: TA LENDO AO CONTRARIO! Deveria ler version concatenado com superblocksize, ta lendo superblocksize concatenado com version, aí os valores saem todo bugados.
//    for(i = 0; i < 5; i++)
//    {
//        memcpy(&temp, &buffer[i * 4], sizeof(DWORD));
//        checksum += temp;
//        printf("TEMP = %u\n", temp);
////            temp = 0;
////        temp = (temp) | ( ( ( *buffer >> i ) & 1) << i );
//    }
//
//    printf("\nCHECKSUM = %d\n", ~checksum);
//
//    Super.Checksum = ~ checksum;
//
//    memcpy(buffer, &Super, sizeof(superbloco));
//
//    write_sector(firstSector, buffer);


}
//
///*-----------------------------------------------------------------------------
//Função:	Monta a partição indicada por "partition" no diretório raiz
//-----------------------------------------------------------------------------*/
int mount(int partition)
{
    int initialBlock = 0;

    if (partition == 0)
    {
        initialBlock = mbrData.endPrimeiroBlocoPartZero;
    }
    else if (partition == 1)
    {
        initialBlock = mbrData.endPrimeiroBlocoPartUm;
    }
    else if (partition == 2)
    {
        initialBlock = mbrData.endPrimeiroBlocoPartDois;
    }
    else if (partition == 3)
    {
        initialBlock = mbrData.endPrimeiroBlocoPartTres;
    }

    BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
    read_sector(initialBlock, buffer);
    memcpy(&Super, buffer, sizeof(superbloco));

//    printf("Initial block na mount %d\n", initialBlock);

    int firstBlockBitmapFreeblocks = initialBlock + Super.superblockSize;
    int firstBlockBitmapInodes = firstBlockBitmapFreeblocks + Super.freeBlocksBitmapSize;
    int firstBlockInodeArea = firstBlockBitmapInodes + Super.freeInodeBitmapSize;
    int firstBlockBlocksArea = firstBlockInodeArea + Super.inodeAreaSize;

    int firstSectorBitmapFreeblocks = firstBlockBitmapFreeblocks * Super.blockSize;
    int firstSectorBitmapInodes = firstBlockBitmapInodes * Super.blockSize;
    int firstSectorInodeArea = firstBlockInodeArea * Super.blockSize;
    int firstSectorBlocksArea = firstBlockBlocksArea * Super.blockSize;

//    printf("\nPrimeiro bloco do bitmap de blocos: %d\nPrimeiro setor do bitmap de blocos: %d\n\n", firstBlockBitmapFreeblocks, firstSectorBitmapFreeblocks);
//    printf("Primeiro bloco do bitmap de Inodes: %d\nPrimeiro setor do bitmap de Inodes: %d\n\n", firstBlockBitmapInodes, firstSectorBitmapInodes);
//    printf("Primeiro bloco da area de inodes: %d\nPrimeiro setor da area de inodes: %d\n\n", firstBlockInodeArea, firstSectorInodeArea);
//    printf("Primeiro bloco da area de dados: %d\nPrimeiro setor da area de dados: %d\n\n", firstBlockBlocksArea, firstSectorBlocksArea);


    openBitmap2(initialBlock);

    int freeInodeBit = searchBitmap2(BITMAP_INODE, 0);
    int freeBlockBit = searchBitmap2(BITMAP_DADOS, 0);

    inode iNodeDir;

    iNodeDir.blocksFileSize = 0;
    iNodeDir.bytesFileSize = 45;

    iNodeDir.dataPtr[0] = firstBlockBlocksArea + freeBlockBit;
    setBitmap2(BITMAP_DADOS, freeBlockBit, 1);

    freeBlockBit = searchBitmap2(BITMAP_DADOS, 0);

    iNodeDir.dataPtr[1] = firstBlockBlocksArea + freeBlockBit;
    setBitmap2(BITMAP_DADOS, freeBlockBit, 1);

    freeBlockBit = searchBitmap2(BITMAP_DADOS, 0);

    iNodeDir.singleIndPtr = firstBlockBlocksArea + freeBlockBit;
    setBitmap2(BITMAP_DADOS, freeBlockBit, 1);

    freeBlockBit = searchBitmap2(BITMAP_DADOS, 0);

    iNodeDir.doubleIndPtr = firstBlockBlocksArea + freeBlockBit;
    setBitmap2(BITMAP_DADOS, freeBlockBit, 1);

    freeBlockBit = searchBitmap2(BITMAP_DADOS, 0);


    iNodeDir.RefCounter = 0;
    iNodeDir.reservado = 0;


    memcpy(buffer, &iNodeDir, sizeof(iNodeDir));
    write_sector(firstSectorInodeArea, buffer);
    setBitmap2(BITMAP_INODE, 0, 1);

    mountedPartition = partition;
    closeBitmap2();

//    openBitmap2(initialBlock);

//    printf("Primeiro livre depois da mount: %d\n", searchBitmap2(BITMAP_DADOS, 0));


    return 0;
}
//
///*-----------------------------------------------------------------------------
//Função:	Desmonta a partição atualmente montada, liberando o ponto de montagem.
//-----------------------------------------------------------------------------*/
int unmount(void)
{
    if (mountedPartition != -1)
    {
        mountedPartition = -1;
    }
    else
    {
        printf("Can't unmount because there is no partition mounted\n");
        return -1;
    }

    return 0;
}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para criar um novo arquivo no disco e abrí-lo,
//		sendo, nesse último aspecto, equivalente a função open2.
//		No entanto, diferentemente da open2, se filename referenciar um
//		arquivo já existente, o mesmo terá seu conteúdo removido e
//		assumirá um tamanho de zero bytes.
//-----------------------------------------------------------------------------*/
FILE2 create2 (char *filename)
{
    //a ideia eh passar por todos os blocos de dados do inode do diretorio, olhando pra ver se o nome é igual ao passado
    //assim tu descobre se tem que criar do zero
//
//    inode tempINode;
//    record tempRecord;
//    inode currentInode;
//    record currentRecord;
//    int superblockSectorsuperblockSector =0;
//
//    if (partition == 0)
//    {
//        superblockSector = mbrData.endPrimeiroBlocoPartZero;
//    }
//    else if (partition == 1)
//    {
//        superblockSector = mbrData.endPrimeiroBlocoPartUm;
//    }
//    else if (partition == 2)
//    {
//        superblockSector = mbrData.endPrimeiroBlocoPartDois;
//    }
//    else if (partition == 3)
//    {
//        superblockSector = mbrData.endPrimeiroBlocoPartTres;
//    }
//
//    read_sector(superblockSector, &Super);
//
//    int firstBlockBitmapFreeblocks = superblockSector + 1;
//    int firstSectorBitmapInodes = firstBlockBitmapFreeblocks + Super.freeBlocksBitmapSize;
//    int firstSectorInodeArea = firstSectorBitmapInodes + Super.freeInodeBitmapSize;
//    int firstBlockBlocksArea = firstSectorInodeArea + Super.inodeAreaSize;
//
//    int freeInodeBit = searchBitmap2(BITMAP_INODE, 0);
//    int freeBlockBit = searchBitmap2(BITMAP_DADOS, 0);

//    //skip dir's iNode
//    int currentInodeIndex = 1;
//    while (currentInodeIndex < freeInodeBit){
//
//        read_sector();
//        memcpy(currentInode
//
//        currentInodeIndex += 1
//
//    }



}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para remover (apagar) um arquivo do disco.
//-----------------------------------------------------------------------------*/
int delete2 (char *filename)
{
    return -1;
}
//
///*-----------------------------------------------------------------------------
//Função:	Função que abre um arquivo existente no disco.
//-----------------------------------------------------------------------------*/
FILE2 open2 (char *filename)
{

}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para fechar um arquivo.
//-----------------------------------------------------------------------------*/
int close2 (FILE2 handle)
{
    return -1;
}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para realizar a leitura de uma certa quantidade
//		de bytes (size) de um arquivo.
//-----------------------------------------------------------------------------*/
int read2 (FILE2 handle, char *buffer, int size)
{
    return -1;
}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para realizar a escrita de uma certa quantidade
//		de bytes (size) de  um arquivo.
//-----------------------------------------------------------------------------*/
int write2 (FILE2 handle, char *buffer, int size)
{
    return -1;
}
//
///*-----------------------------------------------------------------------------
//Função:	Função que abre um diretório existente no disco.
//-----------------------------------------------------------------------------*/
DIR2 opendir2 (void)
{
    inode iNodeDir;
    record tempRecord;

    int initialBlock = 0;

    if (mountedPartition == 0)
    {
        initialBlock = mbrData.endPrimeiroBlocoPartZero;
    }
    else if (mountedPartition == 1)
    {
        initialBlock = mbrData.endPrimeiroBlocoPartUm;
    }
    else if (mountedPartition == 2)
    {
        initialBlock = mbrData.endPrimeiroBlocoPartDois;
    }
    else if (mountedPartition == 3)
    {
        initialBlock = mbrData.endPrimeiroBlocoPartTres;
    }

//    printf("Initial block na open %d\n", initialBlock);

    BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
    read_sector(initialBlock, buffer);
    memcpy(&Super, buffer, sizeof(superbloco));

    int firstBlockBitmapFreeblocks = initialBlock + Super.superblockSize;
    int firstBlockBitmapInodes = firstBlockBitmapFreeblocks + Super.freeBlocksBitmapSize;
    int firstBlockInodeArea = firstBlockBitmapInodes + Super.freeInodeBitmapSize;
    int firstBlockBlocksArea = firstBlockInodeArea + Super.inodeAreaSize;

    int firstSectorBitmapFreeblocks = firstBlockBitmapFreeblocks * Super.blockSize;
    int firstSectorBitmapInodes = firstBlockBitmapInodes * Super.blockSize;
    int firstSectorInodeArea = firstBlockInodeArea * Super.blockSize;
    int firstSectorBlocksArea = firstBlockBlocksArea * Super.blockSize;

//    printf("\nPrimeiro bloco do bitmap de blocos: %d\nPrimeiro setor do bitmap de blocos: %d\n\n", firstBlockBitmapFreeblocks, firstSectorBitmapFreeblocks);
//    printf("Primeiro bloco do bitmap de Inodes: %d\nPrimeiro setor do bitmap de Inodes: %d\n\n", firstBlockBitmapInodes, firstSectorBitmapInodes);
//    printf("Primeiro bloco da area de inodes: %d\nPrimeiro setor da area de inodes: %d\n\n", firstBlockInodeArea, firstSectorInodeArea);
//    printf("Primeiro bloco da area de dados: %d\nPrimeiro setor da area de dados: %d\n\n", firstBlockBlocksArea, firstSectorBlocksArea);

    read_sector(firstSectorInodeArea, buffer);
    memcpy(&iNodeDir, buffer, sizeof(iNodeDir));

    int direct1 = iNodeDir.dataPtr[0];
    int direct2 = iNodeDir.dataPtr[1];
    int simpleIndirect = iNodeDir.singleIndPtr;
    int doubleIndirect = iNodeDir.doubleIndPtr;

    printf("DIRETOS          : %d e %d\n", direct1, direct2);
    printf("INDIRETOS (S e D): %d e %d\n", simpleIndirect, doubleIndirect);

    current_files = createLinkedList();

}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para ler as entradas de um diretório.
//-----------------------------------------------------------------------------*/
int readdir2 (DIRENT2 *dentry)
{
    return -1;
}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para fechar um diretório.
//-----------------------------------------------------------------------------*/
int closedir2 (void)
{
    return -1;
}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para criar um caminho alternativo (softlink)
//-----------------------------------------------------------------------------*/
int sln2 (char *linkname, char *filename)
{
    return -1;
}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para criar um caminho alternativo (hardlink)
//-----------------------------------------------------------------------------*/
int hln2(char *linkname, char *filename)
{
    return -1;
}
//
//
//

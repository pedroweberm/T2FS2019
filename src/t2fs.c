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


int recordsToRead;
int recordsRead;
node* files_in_dir;
node* opened_files;
int dir_is_open = 0;
DWORD dirIOPointer;
DWORD current_handle = 0;
DWORD firstValidEntry = 0;
int firstValidEntrySet = 0;
int initialBlock = 0;
int num_opened = 0;

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

int round(float x)
{
    if (x - (int) x >= 0.5)
    {
        return (int) x + 1;
    }

    return (int) x;
}

int floor(float x)
{
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
    int firstBlock = 0;

    if (partition == 0)
    {
        Super.diskSize = mbrData.endUltimoBlocoPartZero - mbrData.endPrimeiroBlocoPartZero + 1;
        firstBlock = mbrData.endPrimeiroBlocoPartZero;
    }
    else if (partition == 1)
    {
        Super.diskSize = mbrData.endUltimoBlocoPartUm - mbrData.endPrimeiroBlocoPartUm + 1;
        firstBlock = mbrData.endPrimeiroBlocoPartUm;
    }
    else if (partition == 2)
    {
        Super.diskSize = mbrData.endUltimoBlocoPartDois - mbrData.endPrimeiroBlocoPartDois + 1;
        firstBlock = mbrData.endPrimeiroBlocoPartDois;
    }
    else if (partition == 3)
    {
        Super.diskSize = mbrData.endUltimoBlocoPartTres - mbrData.endPrimeiroBlocoPartTres + 1;
        firstBlock = mbrData.endPrimeiroBlocoPartTres;
    }
//
//    printf("\nDISK SIZE = %d e BLOCK SIZE = %d\n", Super.diskSize, Super.blockSize);
//    printf("FIRST SECTOR = %d\n", initialBlock);

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

    int firstSector = firstBlock * Super.blockSize;
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

    openBitmap2(firstSector);
    printf("Bloco livre ao final da format = %d\n", searchBitmap2(BITMAP_DADOS, 0));




//    read_sector(initialBlock, buffer);
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
//    write_sector(initialBlock, buffer);


}
//
///*-----------------------------------------------------------------------------
//Função:	Monta a partição indicada por "partition" no diretório raiz
//-----------------------------------------------------------------------------*/
int mount(int partition)
{
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

//    int firstSector = initialBlock * Super.blockSize;
//
    BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
//    read_sector(initialBlock, buffer);
//    memcpy(&Super, buffer, sizeof(superbloco));

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


    openBitmap2(initialBlock * Super.blockSize);

    int freeInodeBit = searchBitmap2(BITMAP_INODE, 0);
    int freeBlockBit = searchBitmap2(BITMAP_DADOS, 0);

    inode iNodeDir;

    iNodeDir.blocksFileSize = 0;
    iNodeDir.bytesFileSize = 0;

//    iNodeDir.dataPtr[0] = firstBlockBlocksArea + freeBlockBit;
    iNodeDir.dataPtr[0] = -1;
//    setBitmap2(BITMAP_DADOS, freeBlockBit, 1);

//    freeBlockBit = searchBitmap2(BITMAP_DADOS, 0);

//    iNodeDir.dataPtr[1] = firstBlockBlocksArea + freeBlockBit;
    iNodeDir.dataPtr[1] = -1;
//    setBitmap2(BITMAP_DADOS, freeBlockBit, 1);

//    freeBlockBit = searchBitmap2(BITMAP_DADOS, 0);

//    iNodeDir.singleIndPtr = firstBlockBlocksArea + freeBlockBit;
    iNodeDir.singleIndPtr = -1;
//    setBitmap2(BITMAP_DADOS, freeBlockBit, 1);

//    freeBlockBit = searchBitmap2(BITMAP_DADOS, 0);

//    iNodeDir.doubleIndPtr = firstBlockBlocksArea + freeBlockBit;
    iNodeDir.doubleIndPtr = -1;
//    setBitmap2(BITMAP_DADOS, freeBlockBit, 1);

//    freeBlockBit = searchBitmap2(BITMAP_DADOS, 0);

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

int initializeBlock(DWORD block, DWORD value)
{
    int sectorToRead = block * Super.blockSize;

    BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);

    int i = 0;
    int j = 0;
    for (i = 0; i < Super.blockSize; i++)
    {
        for (j = 0; j < SECTOR_SIZE / 4; j++)
        {
            memcpy(&buffer[j * sizeof(DWORD)], &value, sizeof(DWORD));
        }
        write_sector(sectorToRead + i, buffer);
    }


//
//    for (i = 0; i < Super.blockSize; i++)
//    {
//        read_sector(sectorToRead + i, buffer);
//        for (j = 0; j < SECTOR_SIZE / 4; j++)
//        {
//            if (j % sizeof(DWORD) == 0)
//            {
//                printf("\n");
//            }
//            printf(" %d", buffer[i * SECTOR_SIZE + j]);
//        }
//    }
}

int writeRecToDir(record newRecord)
{
    inode iNodeDir;

    BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);

    int firstBlockBitmapFreeblocks = initialBlock + Super.superblockSize;
    int firstBlockBitmapInodes = firstBlockBitmapFreeblocks + Super.freeBlocksBitmapSize;
    int firstBlockInodeArea = firstBlockBitmapInodes + Super.freeInodeBitmapSize;
    int firstBlockBlocksArea = firstBlockInodeArea + Super.inodeAreaSize;

    int firstSectorBitmapFreeblocks = firstBlockBitmapFreeblocks * Super.blockSize;
    int firstSectorBitmapInodes = firstBlockBitmapInodes * Super.blockSize;
    int firstSectorInodeArea = firstBlockInodeArea * Super.blockSize;
    int firstSectorBlocksArea = firstBlockBlocksArea * Super.blockSize;

    read_sector(firstSectorInodeArea, buffer);
    memcpy(&iNodeDir, buffer, sizeof(iNodeDir));

    int recordsInDir = iNodeDir.bytesFileSize / sizeof(record);
    printf("BYTES FILE SIZE %d\nrecordsInDir = %d\n", iNodeDir.bytesFileSize,  recordsInDir);

    int recordsPerSector = SECTOR_SIZE / sizeof(record);
    int recordsPerBlock = Super.blockSize * recordsPerSector;

    int pointersPerSector = SECTOR_SIZE / sizeof(DWORD);
    int pointersPerBlock = Super.blockSize * pointersPerSector;

    int recordsInDirect1 = recordsPerBlock;
    int recordsInDirect2 = recordsInDirect1 + recordsPerBlock;
    int recordsInSimple  = recordsInDirect2 + pointersPerBlock * (recordsPerBlock);
    int recordsInDouble  = recordsInSimple  + pointersPerBlock * pointersPerBlock * (recordsPerBlock);

    int sectorToRead = 0;
    int indexInSector = 0;

    printf("Records per sectors = %d\nRecords per block = %d\nRecordsD1 = %d\nRecordsD2 = %d\nRecordsInSimple = %d\nRecordsInDouble = %d\n", recordsPerSector, recordsPerBlock, recordsInDirect1, recordsInDirect2, recordsInSimple, recordsInDouble);
    if(recordsInDir < recordsInDirect1)
    {
        printf("Entrei no dataPtr[0]\n");
        if (iNodeDir.dataPtr[0] == -1)
        {
            printf("Era -1\n");
            openBitmap2(initialBlock * Super.blockSize);
            iNodeDir.dataPtr[0] = searchBitmap2(BITMAP_DADOS, 0);

            setBitmap2(BITMAP_DADOS, iNodeDir.dataPtr[0], 1);
            closeBitmap2();

            printf("Agora virou %d\n", iNodeDir.dataPtr[0]);
            iNodeDir.blocksFileSize += 1;

        }
        sectorToRead = iNodeDir.dataPtr[0] * Super.blockSize + initialBlock;
        indexInSector = recordsInDir % recordsPerSector;

        read_sector(sectorToRead, buffer);
        memcpy(&buffer[sizeof(record) * indexInSector], &newRecord, sizeof(record));
        write_sector(sectorToRead, buffer);

        record newRecord2;

        printf("Setor a ser lido = %d\nIndice no setor = %d\n", sectorToRead, indexInSector);

        read_sector(sectorToRead, buffer);
        memcpy(&newRecord2, &buffer[sizeof(record) * indexInSector], sizeof(record));

        printf("Name = %s\n", newRecord2.name);
        iNodeDir.bytesFileSize += sizeof(record);
    }
    else if (recordsInDir < recordsInDirect2)
    {
        printf("Entrei no iNodeDir.dataPtr[1]\n");

        if (iNodeDir.dataPtr[1] == -1)
        {
            printf("Era -1\n");

            openBitmap2(initialBlock * Super.blockSize);
            iNodeDir.dataPtr[1] = searchBitmap2(BITMAP_DADOS, 0);

            setBitmap2(BITMAP_DADOS, iNodeDir.dataPtr[1], 1);
            closeBitmap2();
            iNodeDir.blocksFileSize += 1;
            printf("Agora virou %d\n", iNodeDir.dataPtr[1]);
        }
        sectorToRead = iNodeDir.dataPtr[1] * Super.blockSize + initialBlock;
        indexInSector = (recordsInDir - recordsInDirect1) % recordsPerSector;
        printf("Setor a ser lido = %d\nIndice no setor = %d\n", sectorToRead, indexInSector);


        read_sector(sectorToRead, buffer);
        memcpy(&buffer[sizeof(record) * indexInSector], &newRecord, sizeof(record));
        write_sector(sectorToRead, buffer);

        iNodeDir.bytesFileSize += sizeof(record);

    }
    else if (recordsInDir < recordsInSimple)
    {
        printf("\n\nEntrei no recordsInSimple\n");
        if (iNodeDir.singleIndPtr == -1)
        {
            printf("Era -1 no bloco de ponteros\n");
            openBitmap2(initialBlock * Super.blockSize);
            iNodeDir.singleIndPtr = searchBitmap2(BITMAP_DADOS, 0);
            setBitmap2(BITMAP_DADOS, iNodeDir.singleIndPtr, 1);
            closeBitmap2();
            initializeBlock(iNodeDir.singleIndPtr + initialBlock, -1);
            printf("Agora virou %d no bloco de ponteitos\n", iNodeDir.singleIndPtr);
        }

        int pointerIndex = floor((recordsInDir - recordsInDirect2) / recordsPerBlock); // isso retorna qual BLOCO atual
        int pointerSector = floor(pointerIndex/pointersPerSector);
        int pointerSectorInBlock = (recordsInDir - recordsInDirect2) % recordsPerSector;
        int pointerIndexInSector = pointerIndex % pointersPerSector;

        printf("Indice pointer: %d\nsetor pointer: %d\nindice in sector pointer: %d\n", pointerIndex, pointerSector, pointerIndexInSector);

        int pointer;

        read_sector((iNodeDir.singleIndPtr + initialBlock) * Super.blockSize + pointerSector, buffer);

        memcpy(&pointer, &buffer[pointerIndexInSector * sizeof(DWORD)], sizeof(DWORD));

        printf("Pointer %d\n", pointer);

        if (pointer == -1)
        {
            printf("pointer era -1\n");
            openBitmap2(initialBlock * Super.blockSize);

            DWORD newPointer = searchBitmap2(BITMAP_DADOS, 0);

            initializeBlock(newPointer + initialBlock, -1);
            DWORD newPointerInBlock = newPointer + initialBlock;

            read_sector((iNodeDir.singleIndPtr + initialBlock) * Super.blockSize + pointerSector, buffer);
            memcpy(&buffer[pointerIndexInSector * sizeof(DWORD)], &newPointerInBlock, sizeof(DWORD));

            write_sector((iNodeDir.singleIndPtr + initialBlock) * Super.blockSize + pointerSector, buffer);

            setBitmap2(BITMAP_DADOS, newPointer, 1);

            closeBitmap2();

            iNodeDir.blocksFileSize += 1;

            read_sector((iNodeDir.singleIndPtr + initialBlock) * Super.blockSize + pointerSector, buffer);

            memcpy(&pointer, &buffer[pointerIndexInSector * sizeof(DWORD)], sizeof(DWORD));
            printf("pointer era -1 agora eh %d\n", pointer);

        }

        sectorToRead = (pointer + initialBlock) * Super.blockSize + pointerSector;

        read_sector(sectorToRead, buffer);
        memcpy(&buffer[sizeof(record) * pointerSectorInBlock], &newRecord, sizeof(record));
        write_sector(sectorToRead, buffer);
        iNodeDir.bytesFileSize += sizeof(record);
    }
    else
    {
        printf("\n\nEntrei no recordsInDouble\n");
        if (iNodeDir.doubleIndPtr == -1)
        {
            printf("Era -1 no bloco de ponteros\n");
            openBitmap2(initialBlock * Super.blockSize);
            iNodeDir.doubleIndPtr = searchBitmap2(BITMAP_DADOS, 0);
            setBitmap2(BITMAP_DADOS, iNodeDir.doubleIndPtr, 1);
            closeBitmap2();
            initializeBlock(iNodeDir.doubleIndPtr + initialBlock, -1);
            printf("Agora virou %d no bloco de ponteitos\n", iNodeDir.doubleIndPtr + initialBlock);
        }

        int pointerIndex = floor((recordsInDir - recordsInSimple) / (recordsPerBlock * pointersPerBlock));
        int pointerIndexFstLevel = floor(pointerIndex/pointersPerBlock);
        int pointerIndexSndLevel = pointerIndex % pointersPerBlock;

        int pointerSector = floor(pointerIndex/pointersPerSector);
        int pointerSectorFstLevel = floor(pointerIndexFstLevel / pointersPerSector); // qual setor ler do FIRST LEVEL
        int pointerSectorSndLevel = floor(pointerSector        / Super.blockSize  ); // qual setor ler do SECOND LEVEL
        int pointerSectorBlock = floor((recordsInDir - recordsInSimple) / recordsPerSector); // qual setor ler do SECOND LEVEL

        int pointerIndexInSectorFstLevel = pointerIndexFstLevel % pointersPerSector; // deslocamento dentro do FIRST LEVEL
        int pointerIndexInSectorSndLevel = pointerIndexSndLevel % pointersPerSector; // deslocamento dentro do SECOND LEVEL
        int pointerIndexInSectorBlock    = (recordsInDir - recordsInSimple) % recordsPerSector; // deslocamento dentro do SECOND LEVEL

        printf("pointerIndex: %d\nPointer index fst: %d\nPointer index snd: %d\n", pointerIndex, pointerIndexFstLevel, pointerIndexSndLevel);
        printf("pointerSector: %d\nPointer sector fst: %d\nPointer sector snd: %d\n", pointerSector, pointerSectorFstLevel, pointerSectorSndLevel);
        printf("pointerIndexInSectorFstLevel: %d\npointerIndexInSectorSndLevel: %d\n", pointerIndexInSectorFstLevel, pointerIndexInSectorSndLevel);

        int pointerFstLevel;

        read_sector((iNodeDir.doubleIndPtr + initialBlock) * Super.blockSize + pointerSectorFstLevel, buffer);

        memcpy(&pointerFstLevel, &buffer[pointerIndexInSectorFstLevel * sizeof(DWORD)], sizeof(DWORD));

        printf("Pointer pointerFstLevel %d\n", pointerFstLevel);

        if (pointerFstLevel == -1)
        {
            printf("pointerFstLevel era -1\n");
            openBitmap2(initialBlock * Super.blockSize);
            DWORD newPointer = searchBitmap2(BITMAP_DADOS, 0);
            setBitmap2(BITMAP_DADOS, newPointer, 1);
            closeBitmap2();
            initializeBlock(newPointer + initialBlock, -1);
            DWORD newPointerInBlock = newPointer + initialBlock;

            read_sector((iNodeDir.doubleIndPtr + initialBlock) * Super.blockSize + pointerSectorFstLevel, buffer);
            memcpy(&buffer[pointerIndexInSectorFstLevel * sizeof(DWORD)], &newPointerInBlock, sizeof(DWORD));
            write_sector((iNodeDir.doubleIndPtr + initialBlock) * Super.blockSize + pointerSectorFstLevel, buffer);

            iNodeDir.blocksFileSize += 1;

            read_sector((iNodeDir.doubleIndPtr + initialBlock) * Super.blockSize + pointerSectorFstLevel, buffer);

            memcpy(&pointerFstLevel, &buffer[pointerIndexInSectorFstLevel * sizeof(DWORD)], sizeof(DWORD));
            printf("pointerFstLevel era -1 agora eh %d\n", pointerFstLevel);
//
//                        read_sector((pointerFstLevel + initialBlock) * Super.blockSize + pointerSectorSndLevel, buffer);
//            memcpy(&pointerSndLevel, &buffer[pointerIndexInSectorSndLevel * sizeof(DWORD)], sizeof(DWORD));
//            printf("POINTER SND LEVEL = %d\n", pointerSndLevel);
        }

        int pointerSndLevel;
//        printf("soma = %d\n", (pointerFstLevel + initialBlock) * Super.blockSize + pointerSectorSndLevel);

        read_sector((pointerFstLevel + initialBlock) * Super.blockSize + pointerSectorSndLevel, buffer);

        memcpy(&pointerSndLevel, &buffer[pointerIndexInSectorSndLevel * sizeof(DWORD)], sizeof(DWORD));

        printf("PointerSndLevel %d\n", pointerSndLevel);

        if (pointerSndLevel == -1)
        {
            printf("pointerSndLevel era -1\n");
            openBitmap2(initialBlock * Super.blockSize);
            DWORD newPointer = searchBitmap2(BITMAP_DADOS, 0);
            setBitmap2(BITMAP_DADOS, newPointer, 1);
            closeBitmap2();
            initializeBlock(newPointer + initialBlock, -1);
            DWORD newPointerInBlock = newPointer + initialBlock;

            read_sector((iNodeDir.doubleIndPtr + initialBlock) * Super.blockSize + pointerSectorSndLevel, buffer);
            memcpy(&buffer[pointerIndexInSectorSndLevel * sizeof(DWORD)], &newPointerInBlock, sizeof(DWORD));

            write_sector((iNodeDir.doubleIndPtr + initialBlock) * Super.blockSize + pointerSectorSndLevel, buffer);

            iNodeDir.blocksFileSize += 1;

            read_sector((iNodeDir.doubleIndPtr + initialBlock) * Super.blockSize + pointerSectorSndLevel, buffer);

            memcpy(&pointerSndLevel, &buffer[pointerIndexInSectorSndLevel * sizeof(DWORD)], sizeof(DWORD));
            printf("pointerSndLevel era -1 agora eh %d\n", pointerSndLevel);
        }

        sectorToRead = (pointerSndLevel + initialBlock) * Super.blockSize + pointerSectorBlock;

        read_sector(sectorToRead, buffer);
        memcpy(&buffer[sizeof(record) * pointerIndexInSectorBlock], &newRecord, sizeof(record));
        write_sector(sectorToRead, buffer);
        iNodeDir.bytesFileSize += sizeof(record);
    }

    read_sector(firstSectorInodeArea, buffer);
    memcpy(&buffer[0], &iNodeDir, sizeof(inode));
    write_sector(firstSectorInodeArea, buffer);

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
    node* file = malloc(sizeof(node));
    int fileIndex = 0;


    //a ideia eh passar por todos os blocos de dados do inode do diretorio, olhando pra ver se o nome é igual ao passado
    //assim tu descobre se tem que criar do zero

    if(!dir_is_open)
    {
        return -1;
    }

    fileIndex = getIndex(files_in_dir, filename);
    file = searchList(files_in_dir, fileIndex);

    if (file != NULL)
    {
        BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);

        inode fileiNode;
        // ja exsitia acrquwibo com esase nome

        int iNodeNumber = file->data->inodeNumber;
        int inodes_per_sector = roundUp((SECTOR_SIZE) / 32.0);

        //writeInode(newiNode, availableINODE)
        int firstBlockBitmapFreeblocks = initialBlock + Super.superblockSize;
        int firstBlockBitmapInodes = firstBlockBitmapFreeblocks + Super.freeBlocksBitmapSize;
        int firstBlockInodeArea = firstBlockBitmapInodes + Super.freeInodeBitmapSize;
        int firstBlockBlocksArea = firstBlockInodeArea + Super.inodeAreaSize;

        int firstSectorBitmapFreeblocks = firstBlockBitmapFreeblocks * Super.blockSize;
        int firstSectorBitmapInodes = firstBlockBitmapInodes * Super.blockSize;
        int firstSectorInodeArea = firstBlockInodeArea * Super.blockSize;
        int firstSectorBlocksArea = firstBlockBlocksArea * Super.blockSize;

        int inodeSector = iNodeNumber / inodes_per_sector;
        int inodeIndexInsideSector = iNodeNumber % inodes_per_sector;

        read_sector(firstSectorInodeArea + inodeSector, buffer);
        memcpy(&fileiNode, &buffer[sizeof(inode) * inodeIndexInsideSector], sizeof(inode));

        fileiNode.bytesFileSize = 0;
        fileiNode.blocksFileSize = 0;
        fileiNode.dataPtr[0] = -1;
        fileiNode.dataPtr[1] = -1;
        fileiNode.singleIndPtr = -1;
        fileiNode.doubleIndPtr = -1;

        memcpy(&buffer[sizeof(inode) * inodeIndexInsideSector], &fileiNode, sizeof(inode));
        write_sector(firstSectorInodeArea + inodeSector, buffer);
        return 0;

    }
    else
    {
        record newRecord;
        inode newiNode;

        newRecord.TypeVal = 1;
        strcpy(newRecord.name, filename);

        BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
        BYTE* buffer2 = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
//        int initialSector = initialBlock * Super.blockSize;
//        read_sector(initialSector, buffer);
//        memcpy(&Super, buffer, sizeof(superbloco));

        openBitmap2(initialBlock * Super.blockSize);

        int availableINode = searchBitmap2(BITMAP_INODE, 0);
        newRecord.inodeNumber = availableINode;

        setBitmap2(BITMAP_INODE, availableINode, 1);

        newiNode.blocksFileSize = 0;
        newiNode.bytesFileSize = 0;

        newiNode.dataPtr[0] = -1;
        newiNode.dataPtr[1] = -1;
        newiNode.singleIndPtr = -1;
        newiNode.doubleIndPtr = -1;

        newiNode.RefCounter = 1;

        closeBitmap2();


        int inodes_per_sector = roundUp((SECTOR_SIZE) / 32.0);


        //writeInode(newiNode, availableINODE)
        int firstBlockBitmapFreeblocks = initialBlock + Super.superblockSize;
        int firstBlockBitmapInodes = firstBlockBitmapFreeblocks + Super.freeBlocksBitmapSize;
        int firstBlockInodeArea = firstBlockBitmapInodes + Super.freeInodeBitmapSize;
        int firstBlockBlocksArea = firstBlockInodeArea + Super.inodeAreaSize;

        int firstSectorBitmapFreeblocks = firstBlockBitmapFreeblocks * Super.blockSize;
        int firstSectorBitmapInodes = firstBlockBitmapInodes * Super.blockSize;
        int firstSectorInodeArea = firstBlockInodeArea * Super.blockSize;
        int firstSectorBlocksArea = firstBlockBlocksArea * Super.blockSize;

        int inodeSector = newRecord.inodeNumber / inodes_per_sector;
        int inodeIndexInsideSector = newRecord.inodeNumber % inodes_per_sector;

        read_sector(firstSectorInodeArea + inodeSector, buffer);
        memcpy(&buffer[sizeof(inode) * inodeIndexInsideSector], &newiNode, sizeof(inode));
        write_sector(firstSectorInodeArea + inodeSector, buffer);

        // testado ate aqui


        writeRecToDir(newRecord);


    }
    open2(filename);
    return 0;
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
    node* file = malloc(sizeof(node));
    int fileIndex;

    if (!dir_is_open)
    {
        return -1;
    }

    fileIndex = getIndex(files_in_dir, filename);
    file = searchList(files_in_dir, fileIndex);


    if (file != NULL)
    {
        if (num_opened < 10)
        {
            file->isOpen = 1;
            opened_files = appendToList(opened_files, file);
            num_opened += 1;
            return file->handle;
        }
    }
    return -1;

}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para fechar um arquivo.
//-----------------------------------------------------------------------------*/
int close2 (FILE2 handle)
{
    node* file = malloc(sizeof(node));
    int fileIndex;

    if (!dir_is_open)
    {
        return -1;
    }

    fileIndex = getIndex(files_in_dir, filename);
    file = searchList(files_in_dir, fileIndex);

    if (file != NULL)
    {
        file->isOpen = 0;
        opened_files = removeFromList(opened_files, filename);
        num_opened -= 1;
        return 0;
    }
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

int read_direct(DWORD blockToRead)
{
    if (blockToRead != -1)
    {
        int initialBlock = 0;
        record tempRecord;
        node* tempNode;

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

        BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
//        int initialSector = initialBlock * Super.
//        read_sector(initialBlock, buffer);
//        memcpy(&Super, buffer, sizeof(superbloco));

        int records_per_sector = roundUp(SECTOR_SIZE / sizeof(record));
        //printf("RECORD PER SECTOR = %d\n", records_per_sector); printa 32, ta certo

        int sectorToRead = blockToRead * Super.blockSize;

        int i = 0;
        int j = 0;
        for (i = 0; i < Super.blockSize; i++)
        {
            read_sector(sectorToRead + i, buffer);

            for (j = 0; j < records_per_sector; j++)
            {
                if(recordsRead < recordsToRead)
                {
                    memcpy(&tempRecord, &buffer[j * sizeof(record)], sizeof(record));

                    tempNode = createNode(tempRecord.TypeVal, tempRecord.name, tempRecord.Nao_usado, tempRecord.inodeNumber, 0, current_handle, 0);
                    current_handle += 1;

                    if(appendToList(files_in_dir, tempNode) != 0)
                    {
                        return -1;
                    }

                    recordsRead += 1;
                }
                else
                {
                    printf("Terminei de ler todos os blocos do diretorio.\n");
                    recordsRead = 0;
                    recordsToRead = 0;
                }

            }
            printf("Alcancei o final do setor %d do bloco %d", sectorToRead + i, blockToRead);
        }
        return 0;
    }
    else
    {
        return 0;
    }

}

int read_simple_indirect(DWORD blockToRead)
{
    if (blockToRead != -1)
    {
        int initialBlock = 0;
        record tempRecord;
        node* tempNode;
        DWORD tempBlockToRead = 0;

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

        BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
//        read_sector(initialBlock, buffer);
//        memcpy(&Super, buffer, sizeof(superbloco));

        int pointers_per_sector = roundUp(SECTOR_SIZE / sizeof(DWORD));
        //printf("RECORD PER SECTOR = %d\n", records_per_sector); printa 32, ta certo

        int sectorToRead = blockToRead * Super.blockSize;

        int i = 0;
        int j = 0;

        for (i = 0; i < Super.blockSize; i++)
        {
            read_sector(sectorToRead + i, buffer);

            for (j = 0; j < pointers_per_sector; j++)
            {
                memcpy(&tempBlockToRead, &buffer[j * sizeof(DWORD)], sizeof(DWORD));
                read_direct(tempBlockToRead);
            }
        }
        return 0;
    }
    else
    {
        return 0;
    }

}

int read_double_indirect(DWORD blockToRead)
{
    if (blockToRead != -1)
    {
        int initialBlock = 0;
        record tempRecord;
        node* tempNode;
        DWORD tempBlockToRead = 0;
        DWORD tempPointer = 0;

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

        BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
//        read_sector(initialBlock, buffer);
//        memcpy(&Super, buffer, sizeof(superbloco));

        int pointers_per_sector = roundUp(SECTOR_SIZE / sizeof(DWORD));
        //printf("RECORD PER SECTOR = %d\n", records_per_sector); printa 32, ta certo

        int sectorToRead = blockToRead * Super.blockSize;

        int i = 0;
        int j = 0;

        for (i = 0; i < Super.blockSize; i++)
        {
            read_sector(sectorToRead + i, buffer);

            for (j = 0; j < pointers_per_sector; j++)
            {
                memcpy(&tempPointer, &buffer[j * sizeof(DWORD)], sizeof(DWORD));
                read_simple_indirect(tempPointer);
            }
        }
        return 0;
    }
    else
    {
        return 0;
    }

}
//
///*-----------------------------------------------------------------------------
//Função:	Função que abre um diretório existente no disco.
//-----------------------------------------------------------------------------*/
DIR2 opendir2 (void)
{
    inode iNodeDir;
    record tempRecord;
    dirIOPointer = 0;
//
//    int initialBlock = 0;
//
//    if (mountedPartition == 0)
//    {
//        initialBlock = mbrData.endPrimeiroBlocoPartZero;
//    }
//    else if (mountedPartition == 1)
//    {
//        initialBlock = mbrData.endPrimeiroBlocoPartUm;
//    }
//    else if (mountedPartition == 2)
//    {
//        initialBlock = mbrData.endPrimeiroBlocoPartDois;
//    }
//    else if (mountedPartition == 3)
//    {
//        initialBlock = mbrData.endPrimeiroBlocoPartTres;
//    }

//    printf("Initial block na open %d\n", initialBlock);

    BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
//    read_sector(initialBlock, buffer);
//    memcpy(&Super, buffer, sizeof(superbloco));

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

    files_in_dir = createLinkedList();
    opened_files = createLinkedList();

    recordsToRead = iNodeDir.bytesFileSize / sizeof(record);

    read_direct(direct1);
    read_direct(direct2);
    read_simple_indirect(simpleIndirect);
    read_double_indirect(doubleIndirect);

    dir_is_open = 1;
    return 0;
}

//
///*-----------------------------------------------------------------------------
//Função:	Função usada para ler as entradas de um diretório.
//-----------------------------------------------------------------------------*/
int readdir2 (DIRENT2 *dentry)
{
    inode fileInode;
    DIRENT2 newDentry;
    record* tempRecord;
    if (!dir_is_open)
    {
        return -1;
    }

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

    tempRecord = searchList(files_in_dir, dirIOPointer)->data;

    strcpy(newDentry.name, tempRecord->name);


    BYTE* buffer = (BYTE *) malloc(sizeof(BYTE) * SECTOR_SIZE);
//    read_sector(initialBlock, buffer);
//    memcpy(&Super, buffer, sizeof(superbloco));

    int inodes_per_sector = roundUp((SECTOR_SIZE) / 32.0);

    int firstBlockBitmapFreeblocks = initialBlock + Super.superblockSize;
    int firstBlockBitmapInodes = firstBlockBitmapFreeblocks + Super.freeBlocksBitmapSize;
    int firstBlockInodeArea = firstBlockBitmapInodes + Super.freeInodeBitmapSize;
    int firstBlockBlocksArea = firstBlockInodeArea + Super.inodeAreaSize;

    int firstSectorBitmapFreeblocks = firstBlockBitmapFreeblocks * Super.blockSize;
    int firstSectorBitmapInodes = firstBlockBitmapInodes * Super.blockSize;
    int firstSectorInodeArea = firstBlockInodeArea * Super.blockSize;
    int firstSectorBlocksArea = firstBlockBlocksArea * Super.blockSize;

    int inodeSector = tempRecord->inodeNumber / inodes_per_sector;
    int inodeIndexInsideSector = tempRecord->inodeNumber % inodes_per_sector;

    read_sector(firstSectorInodeArea + inodeSector, buffer);
    memcpy(&fileInode, buffer[sizeof(inode) * inodeIndexInsideSector], sizeof(inode));

    newDentry.fileSize = fileInode.bytesFileSize;
    newDentry.fileType = tempRecord->TypeVal;

    dirIOPointer += 1;

    memcpy(dentry, &newDentry, sizeof(DIRENT2));
}
//
///*-----------------------------------------------------------------------------
//Função:	Função usada para fechar um diretório.
//-----------------------------------------------------------------------------*/
int closedir2 (void)
{
    if (dir_is_open)
    {
        dir_is_open = 0;
        files_in_dir = NULL;
        firstValidEntrySet = 0;
    }
    return 0;
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



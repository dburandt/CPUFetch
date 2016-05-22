
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include "printInternalReg.h"

#define ERROR_RETURN -1
#define SUCCESS 0

FILE *fp;
uint64_t fileSize;
uint64_t missingBytes;
unsigned char buffer[10];
unsigned char iCdBuffer[1];
unsigned char advanceBy;
uint64_t haltCount;
int availBytes;

// Fetch bytes and printReg           
// Parameters: int PC, int buffer length
// Returns: SUCCESS or ERROR_RETURN codes
// Errors: Not enough bytes
int readInstr(uint64_t PC, int insLength) {
    
    unsigned char insBuffer[insLength];
    
    // values for printInternalReg --->
    nibble iCd, iFn = 0;
    int regsValid = 0; // 0 when not both, 1 when at least one reg used.
    nibble rA, rB = 0; // reg value, 15 for F, anything if no reg
    int  valCValid = 0; // 0 for no valC
    uint64_t valC = 0;  // Convert little-endian bytes into int (or 0 for no valC)
    uint8_t byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7 = 0; // valC little-endian
    int64_t valP = 0; // new valP. PC + size(buffer) normally, new target if jp.
    char *instrStr = NULL; // string version of instruction
    // <---
    
    fseek(fp, PC, 0);
    
    fread(insBuffer, sizeof(insBuffer), 1, fp);
    
    
    iCd = (insBuffer[0] >> 4);
    iFn = (insBuffer[0] & 15);
    
    int64_t missingBytes = (PC + insLength) - fileSize;
    
    if (iCd != 0)
        haltCount = 0;
    
    // halt
    if (iCd == 0) {
        haltCount++;
        valP = PC + insLength;
        instrStr = (char[]) {'h', 'a', 'l', 't', '\0'};
        
        // nop
    } else if (iCd == 1) {
        valP = PC + insLength;
        instrStr = (char[]) {'n', 'o', 'p', '\0'};
        
        // cmovXX rA, rB
    } else if (iCd == 2) {
	if (missingBytes < 1){        
	    regsValid = 1;
	}
        valP = PC + insLength;
        if (iFn == 0) {
            instrStr = (char[]) {'r', 'r', 'm', 'o', 'v', 'q', '\0'};
        } else if (iFn == 1) {
            instrStr = (char[]) {'l', 'e', 'm', 'o', 'v', 'q', '\0'};
        } else if (iFn == 2) {
            instrStr = (char[]) {'l', 'm', 'o', 'v', 'q', '\0'};
        } else if (iFn == 3) {
            instrStr = (char[]) {'e', 'm', 'o', 'v', 'q', '\0'};
        } else if (iFn == 4) {
            instrStr = (char[]) {'n', 'e', 'm', 'o', 'v', 'q', '\0'};
        } else if (iFn == 5) {
            instrStr = (char[]) {'g', 'e', 'm', 'o', 'v', 'q', '\0'};
        } else if (iFn == 6) {
            instrStr = (char[]) {'g', 'm', 'o', 'v', 'q', '\0'};
        }
        
        // irmovq V, rB
    } else if (iCd == 3) {
        if (missingBytes < 9){        
	    regsValid = 1;
	}
        valCValid = 2;
        valP = PC + insLength;
        instrStr = (char[]) {'i', 'r', 'm', 'o', 'v', 'q', '\0'};
        
        // rmmovq rA, D(rB)
    } else if (iCd == 4) {
        if (missingBytes < 9){        
	    regsValid = 1;
	}
        valCValid = 2;
        valP = PC + insLength;
        instrStr = (char[]) {'r', 'm', 'm', 'o', 'v', 'q', '\0'};
        
        // mrmovq D(rB), rA
    } else if (iCd == 5) {
        if (missingBytes < 9){        
	    regsValid = 1;
	}
        valCValid = 2;
        valP = PC + insLength;
        instrStr = (char[]) {'m', 'r', 'm', 'o', 'v', 'q', '\0'};
        
        // OPI rA, rB
    } else if (iCd == 6) {
        if (missingBytes < 1){        
	    regsValid = 1;
	}
        valP = PC + insLength;
        if (iFn == 0) {
            instrStr = (char[]) {'a', 'd', 'd', 'q', '\0'};
        } else if (iFn == 1) {
            instrStr = (char[]) {'s', 'u', 'b', 'q', '\0'};
        } else if (iFn == 2) {
            instrStr = (char[]) {'a', 'n', 'd', 'q', '\0'};
        } else if (iFn == 3) {
            instrStr = (char[]) {'x', 'o', 'r', 'q', '\0'};
        } else if (iFn == 4) {
            instrStr = (char[]) {'m', 'u', 'l', 'q', '\0'};
        } else if (iFn == 5) {
            instrStr = (char[]) {'d', 'i', 'v', 'q', '\0'};
        } else if (iFn == 6) {
            instrStr = (char[]) {'m', 'o', 'd', 'q', '\0'};
        }
        // jXX Dest
    } else if (iCd == 7) {
        valCValid = 1;
        valP = PC + insLength;
        if (iFn == 0) {
            instrStr = (char[]) {'j', 'm', 'p', '\0'};
        } else if (iFn == 1) {
            instrStr = (char[]) {'j', 'l', 'e', '\0'};
        } else if (iFn == 2) {
            instrStr = (char[]) {'j', 'l', '\0'};
        } else if (iFn == 3) {
            instrStr = (char[]) {'j', 'e', '\0'};
        } else if (iFn == 4) {
            instrStr = (char[]) {'j', 'n', 'e', '\0'};
        } else if (iFn == 5) {
            instrStr = (char[]) {'j', 'g', 'e', '\0'};
        } else if (iFn == 6) {
            instrStr = (char[]) {'j', 'g', '\0'};
        }
        
        
        // call Dest
    } else if (iCd == 8) {
        valCValid = 1;
        valP = PC + insLength;
        instrStr = (char[]) {'c', 'a', 'l', 'l', '\0'};
        
        // ret
    } else if (iCd == 9) {
        valP = PC + insLength;
        instrStr = (char[]) {'r', 'e', 't', '\0'};
        
        // pushq rA
    } else if (iCd == 10) {
        if (missingBytes < 1){        
	    regsValid = 1;
	}
        valP = PC + insLength;
        instrStr = (char[]) {'p', 'u', 's', 'h', 'q', '\0'};
        
        // popq rA
    } else if (iCd == 11) {
        if (missingBytes < 1){        
	    regsValid = 1;
	}
        valP = PC + insLength;
        instrStr = (char[]) {'p', 'o', 'p', 'q', '\0'};
        
    }
    
    if (regsValid > 0) {
        rA = (insBuffer[1] >> 4);
        rB = (insBuffer[1] & 15);
    }
    

    // valCValid = 1: Start reading bytes from buffer[1] ex. call Dest
    // valCValid = 2: Start reading bytes from buffer[2] ex. irmovq V, rB
    if (valCValid > 0) {
        int c = 1;

        int availBytes = insLength - valCValid - missingBytes;
        
        if (c > availBytes) {
            byte0 = 0;
        } else {
            byte0 = insBuffer[valCValid++];
        }
        c+=1;
        if (c > availBytes) {
            byte1 = 0;
        } else {
            byte1 = insBuffer[valCValid++];
        }
        c+=1;
        if (c > availBytes) {
            byte2 = 0;
        } else {
            byte2 = insBuffer[valCValid++];
        }
        c+=1;
        if (c > availBytes) {
            byte3 = 0;
        } else {
            byte3 = insBuffer[valCValid++];
        }
        c+=1;
        if (c > availBytes) {
            byte4 = 0;
        } else {
            byte4 = insBuffer[valCValid++];
        }
        c+=1;
        if (c > availBytes) {
            byte5 = 0;
        } else {
            byte5 = insBuffer[valCValid++];
        }
        c+=1;
        if (c > availBytes) {
            byte6 = 0;
        } else {
            byte6 = insBuffer[valCValid++];
        }
        c+=1;
        if (c > availBytes) {
            byte7 = 0;
        } else {
            byte7 = insBuffer[valCValid++];
        }
    
        valC = (uint64_t)byte0 | (uint64_t)byte1 << 8 | (uint64_t)byte2 << 16 | (uint64_t)byte3 << 32 | (uint64_t)byte4 << 40 | (uint64_t)byte5 << 48 | (uint64_t)byte6 << 54 | (uint64_t)byte7 << 62;
    }
    
    if (haltCount <= 5) {
        printReg(PC, iCd, iFn, regsValid, rA, rB, valCValid, valC,
                 byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7,
                 valP, instrStr);
    }
    
    if (missingBytes > 0) {
	printf("Memory access error at 0x%016llX, required %x bytes, read %llx bytes.",PC,insLength,missingBytes);
	return ERROR_RETURN;
    }
    return valP;
}

// Helper function to handle invalid iCd or iFn codes
int handleiICdIFn(uint64_t iCd, uint64_t iFn, uint64_t PC) {
    // handle incorrect opcode
    if (iCd > 11) {
        printf("Invalid opcode %llx%llx at 0x%016llX", iCd, iFn, PC);
	return 0;
    }

    // check for an acceptable iFn
    if (iFn == 0) {
        return 1;
    } else if (iCd == 2 || iCd == 6 || iCd == 7) {
        if (iFn < 7) {
            return 1;
        } else {
	    printf("Invalid function code %llx%llx at 0x%016llX", iCd, iFn, PC);
            return 0;
        }
    } else {
	printf("Invalid function code %llx%llx at 0x%016llX", iCd, iFn, PC);
        return 0;
    }
}

// Go to PC
// Parameters: int next PC
// Returns: SUCCESS or ERROR_RETURN codes 
// Errors: Invalid op code or fn code
void readNext(uint64_t PC) {
    
    fseek(fp, PC, 0);
    
    if (PC == fileSize) {
        printf("Normal Termination");
        return;
    }
    
    // get op code
    fread(buffer, sizeof(iCdBuffer), 1, fp);
    
    uint64_t iCd = (buffer[0] >> 4);
    uint64_t iFn = (buffer[0] & 15);
    
    //check for correct iFn

    if (handleiICdIFn(iCd, iFn, PC) == 0) {
        return;
    }
    
    if (iCd == 0) {
        PC = readInstr(PC, 1);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else if (iCd == 1) {
        PC = readInstr(PC, 1);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else if (iCd == 2) {
        PC = readInstr(PC, 2);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else if (iCd == 3) {
        PC = readInstr(PC, 10);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else if (iCd == 4) {
        PC = readInstr(PC, 10);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else if (iCd == 5) {
        PC = readInstr(PC, 10);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else if (iCd == 6) {
        PC = readInstr(PC, 2);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else if (iCd == 7) {
        PC = readInstr(PC, 9);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else if (iCd == 8) {
        PC = readInstr(PC, 9);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else if (iCd == 9) {
        PC = readInstr(PC, 1);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else if (iCd == 10) {
        PC = readInstr(PC, 2);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else if (iCd == 11) {
        PC = readInstr(PC, 2);
        if (PC == ERROR_RETURN) {
            return;
        }
        readNext(PC);
    } else {
        // handle incorrect opcode
        printf("Invalid opcode %llx%llx at 0x%016llX", iCd, iFn, PC);
        return;
    }
    
    return;
}


int main(int argc, char **argv) {
    
    int machineCodeFD = -1;       // File descriptor of file with object code
    uint64_t PC = 0;              // The program counder
    
    // Verify that the command line has an appropriate number
    // of arguments
    
    if (argc < 2 || argc > 3) {
        printf("Usage: %s InputFilename [startingOffset]\n", argv[0]);
        return ERROR_RETURN;
    }
    
    // First argument is the file to open, attempt to open it
    // for reading and verify that the open did occur.
    machineCodeFD = open(argv[1], O_RDONLY);
    
    if (machineCodeFD < 0) {
        printf("Failed to open: %s\n", argv[1]);
        return ERROR_RETURN;
    }
    
    // If there is a 2nd argument present it is an offset so
    // convert it to a value. This offset is the initial value the
    // program counter is to have. The program will seek to that location
    // in the object file and begin fetching instructions from there.
    if (3 == argc) {
        // See man page for strtol() as to why
        // we check for errors by examining errno
        errno = 0;
        PC = strtol(argv[2], NULL, 0);
        if (errno != 0) {
            perror("Invalid offset on command line");
            return ERROR_RETURN;
        }
    }
    //0x%016llX  <- unsigned long long int in hex, min 16 positions, 0 padded
    
    printf("Opened %s, starting offset 0x%016llX\n", argv[1], PC);
    
    fp = fopen(argv[1], "rb");
    
    // get the size of the file
    fseek(fp, 0L, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    
    readNext(PC);
    
    fclose(fp);
    printf("\n\n");
    
    // Start adding your code here and comment out the line the #define EXAMPLESON line
    
//#define EXAMPLESON 1
#ifdef  EXAMPLESON
    
    
    
    // The next few lines are examples of various types of output. In the comments is
    // an instruction, the address it is at and the associated binary code that would
    // be found in the object code file at that address (offset). Your program
    // will read that binary data and then pull it appart just like the fetch stage.
    // Once everything has been pulled apart then a call to printReg is made to
    // have the output printed. Read the comments in printInternalReg.c for what the
    // various arguments are and how they are to be used.
    
    /*************************************************
     irmovq $1, %rsi   0x008: 30f60100000000000000
     ***************************************************/
    
    printReg(8, 3, 0, 1, 15, 6, 1,  1, 01, 0, 0, 0, 0, 0, 0, 0, 8+10, "irmovq");
    
    /*************************************************
     je target   x034: 733f00000000000000     Note target is a label
     
     ***************************************************/
    printReg(0x34, 7, 3, 0, 15, 0xf, 1, 0x3f, 0x3f, 0, 0, 0, 0, 0, 0, 0, 0x34 + 9, "je");
    
    
    /*************************************************
     nop  x03d: 10
     
     ***************************************************/
    printReg(0x3d, 1, 0, 0, 15, 0xf, 0, 0x3f, 0x3f, 0, 0, 0, 0, 0, 0, 0, 0x3e + 1, "nop");
    
    
    
    /*************************************************
     addq %rsi,%rdx  0x03f: 6062
     
     ***************************************************/
    printReg(0x3f, 0, 0, 0, 6, 2, 0, 0x3f, 0x3f, 0, 0, 0, 0, 0, 0, 0, 0x3f + 1, "halt");
    
    
#endif
    
    
    return SUCCESS;
    
}








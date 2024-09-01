/*
    Conventions:
        1. the ETM ID can be configured by writing to ETM registers. In current config, ETM ID for A53 0 is 1, for A53 1 is 2, and so forth
*/


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// requires 2 positional arguments: the number of active ETMs and the input file name
void parse_args(int argc, char *argv[], uint8_t* n_mp) {
    if(argc != 3) {
        // printf("One positional argument required. i.e. the number of active ETMs\n");
        printf("Usage %s <number of active ETMs> <input file name>\n", argv[0]);
        exit(1);
    }
    *n_mp = strtol(argv[1], NULL, 0);
    return;
}

/* 
    The ID starts from 1 to 4. Thus when indexing, the position is id-1.
    The first ETM ID is 1 instead of 0, since potentially, the ID 0 might be reserved 
*/
FILE* id2file(FILE** fps, int id) {
    return fps[id - 1]; 
}

/* 
    frame_buf is 16 bytes long. Only the entire 16bytes are recevied, the deformatting can start meaningfully.
    the cur_id is consistent with ETM ID. 

    cur_id == 0 is reserved to indicate null. This could happen when trace end or flush.
    The data bytes associate with ID 0 are discarded.
*/
void proc_frame(FILE** fps, uint8_t* frame_buf, int* cur_id) {
    int i;
    char aux = frame_buf[15];
    for(i=0; i<8; i++) {
        if ( (frame_buf[i*2] & 0x1) && (aux & (0x1 << i)) ) {
            // new ID and the next byte corresponding to the old ID
            if(i==7) {
                printf("auxiliary fault!\n");
                exit(0);
            }
            fwrite(&frame_buf[i*2 + 1], sizeof(uint8_t), 1, id2file(fps, *cur_id));
            *cur_id = (frame_buf[i*2] & 0xfe) >> 1; 
        } else if ( (frame_buf[i*2] & 0x1) && !(aux & (0x1 << i)) ) {
            // new ID and the next byte corresponding to the new ID
            *cur_id = (frame_buf[i*2] & 0xfe) >> 1;
            if (*cur_id == 0) {
                continue;
            }
            if(i != 7) {
                fwrite(&frame_buf[i*2 + 1], sizeof(uint8_t), 1, id2file(fps, *cur_id));
            }
        } else {
            // Data byte
            if (*cur_id == 0) {
                continue;
            }
            char dat = (frame_buf[i*2] & 0xfe) | ((aux & (0x1 << i)) >> i);
            FILE* tar_fp = id2file(fps, *cur_id);
            fwrite(&dat, sizeof(uint8_t), 1, tar_fp); 
            if(i != 7) {
                fwrite(&frame_buf[i*2 + 1], sizeof(uint8_t), 1, tar_fp); 
            }
        }
    }
}

void dat2out(char* ifname, char* ofname) {
    FILE* f1 = fopen(ifname, "rb");
    FILE* f2 = fopen(ofname, "w");

    uint32_t buf;
    int status;
    while(1) {
        status = fread(&buf, sizeof(uint32_t), 1, f1);
        if (status != 1) {
            break;
        }
        fprintf(f2, "0x%08X\n", buf);
    }
    fclose(f1);
    fclose(f2);
}


// requires two positional arguments: the number of active ETMs and the input file name
int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage %s <number of active ETMs> <input file name>\n", argv[0]);
        exit(1);
    }

    uint8_t n_mp = strtol(argv[1], NULL, 0);
    char* fname = argv[2];

    uint8_t frame_buf[16];
    FILE* fp = fopen(fname, "rb");
    int status;
    int cur_id = -1;

    // parse_args(argc, argv, &n_mp);
    FILE** fps = (FILE**) malloc(sizeof(FILE*) * n_mp);
    int i;
    for(i=0; i<n_mp; i++) {
        char sep_fname[32];
        sprintf(sep_fname, "trc_%u.dat", i);
        fps[i] = fopen(sep_fname, "wb");
    }

    while(1) {
        status = fread(frame_buf, sizeof(frame_buf), 1, fp);
        if (status != 1) {
            break;
        }
        proc_frame(fps, frame_buf, &cur_id);
    }

    for(i=0; i<n_mp; i++) {
        fclose(fps[i]);
        char ifname[32];
        char ofname[32];
        sprintf(ifname, "trc_%u.dat", i);
        sprintf(ofname, "trc_%u.out", i);
        dat2out(ifname, ofname);
    }

    return 0;
}

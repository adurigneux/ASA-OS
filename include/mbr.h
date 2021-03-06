/* ------------------------------
   $Id: mbr.h 7085 2013-10-18 15:37:14Z marquet $
   ------------------------------------------------------------

   MBR definition and manipualtion.
   Philippe Marquet, october 2002
   
*/

#ifndef _MBR_H_
#define _MBR_H_

#define MAX_VOL		8

enum vol_type_e {
    base,
    annex,
    other
};

struct vol_descr_s {
    unsigned int vol_first_cylinder;
    unsigned int vol_first_sector;
    unsigned int vol_n_bloc;
    enum vol_type_e vol_type;
};

struct mbr_descr_s {
    unsigned int mbr_magic; /* MBR_MAGIC */
    unsigned int mbr_n_vol; 
    struct vol_descr_s mbr_vol[MAX_VOL]; /* first mbr_n_vol are in used */
};

extern struct mbr_descr_s mbr;

/* return RETURN_FAILURE in case of a non preexisting mbr */
int load_mbr();
void save_mbr();

int add_vol(unsigned int c, unsigned int s, unsigned int size);
int rmvol(unsigned int i);
int check_vol(unsigned int vol);
void display_mbr();
void check_struc_mbr();

char char_of_vol_type(enum vol_type_e);
enum vol_type_e vol_type_of_char(char);
unsigned int get_block(unsigned int, unsigned int);
unsigned int cylindre_of_bloc(unsigned int, unsigned int);
unsigned int sector_of_bloc(unsigned int, unsigned int);

/*
 * read_bloc()
 * This method reads a bloc from the disk and put it in the memory
 */
void read_bloc(unsigned int, unsigned int, unsigned char*);

/*
 * write_bloc()
 * This method writes a bloc from the memory to the disk
 */
void write_bloc(unsigned int, unsigned int, unsigned char*);

/*
 * read_bloc_n()
 * This method reads a specified number of blocs from the disk and put these blocks in the memory
 */
void read_bloc_n(unsigned int, unsigned int, unsigned char*, unsigned int);

/*
 * write_bloc_n()
 * This method writes a specified number of blocs from the memory to the disk
 */
void write_bloc_n(unsigned int, unsigned int, unsigned char*, unsigned int);

#endif

#ifndef _THREE_D_LUT_H_
#define _THREE_D_LUT_H_
#include <string>
#define CLIP(Val,Max,Min)	(((Val)<(Min))? (Min):(((Val)>(Max))? (Max):(Val)))
#define ROUND_N(value, n)	(((value) + (1 << ((n) - 1))) >> (n))

#define COLOR_RANGE_BITS    8
#define COLOR_RANGE_MAX     ((1<<COLOR_RANGE_BITS)-1)

#define LUT_ADDR_BITS       3
#define LUT_WEIGHT_MASK     ((1<<(COLOR_RANGE_BITS-LUT_ADDR_BITS))-1)
#define LUT_LVL             ((1<<LUT_ADDR_BITS)+1)
#define LUT_CUBE_LEN        ((COLOR_RANGE_MAX+1)/(LUT_LVL-1))

#define RGB_LUT_DATA_BITS   10
#define R_CHANNEL_MASK      0x3ff00000
#define G_CHANNEL_MASK      0xffc00
#define B_CHANNEL_MASK      0x3ff

void tetrahedral_interpolation(unsigned int R_i, unsigned int G_i, unsigned B_i, unsigned int *R_o, unsigned int *G_o, unsigned int *B_o, unsigned int *pt_lut);

void address_decoder(unsigned int R_i, unsigned int G_i, unsigned B_i,  int *case_id, unsigned int addr[4], unsigned int weight[3]);

void data_switch(int case_id, unsigned int v_in_0, unsigned int v_in_1, unsigned int v_in_2, unsigned int v_in_3, unsigned int *v_out_0, unsigned int *v_out_1, unsigned int *v_out_2, unsigned int *v_out_3);

void load_3d_lut(FILE *ptf, unsigned int *pt_lut);
void free_lut_mem(unsigned int *pt_lut);

void load_color_data(FILE *ptf, unsigned int *pt_data);
void generate_3d_lut(unsigned int *pt_data, unsigned int *pt_lut);

#endif

#include "three_d_lut.h"

void tetrahedral_interpolation(unsigned int R_i, unsigned int G_i, unsigned B_i, unsigned int *R_o, unsigned int *G_o, unsigned int *B_o, unsigned int *pt_lut)
{
    int c0, c1, c2, c3;
    int case_id;
    int tmp_m[4], tmp_s[4];
    unsigned int addr[4], weight[3];
    unsigned int v0, v1, v2, v3;
    unsigned int v_minuend[4] = {0};
    unsigned int v_subtrahend[4] = {0};

    address_decoder(R_i, G_i, B_i, &case_id, addr, weight);

    v0 = pt_lut[addr[0]];
    v1 = pt_lut[addr[1]];
    v2 = pt_lut[addr[2]];
    v3 = pt_lut[addr[3]];

    data_switch(case_id, v0 ,v1 ,v2, v3, &v_minuend[0]   , &v_minuend[1]   , &v_minuend[2]   , &v_minuend[3]   );
    data_switch(case_id, v3 ,v0 ,v1, v2, &v_subtrahend[0], &v_subtrahend[1], &v_subtrahend[2], &v_subtrahend[3]);

    //R
    tmp_m[0] = (int)((v_minuend[0]&R_CHANNEL_MASK)>>(RGB_LUT_DATA_BITS+RGB_LUT_DATA_BITS));
    tmp_m[1] = (int)((v_minuend[1]&R_CHANNEL_MASK)>>(RGB_LUT_DATA_BITS+RGB_LUT_DATA_BITS));
    tmp_m[2] = (int)((v_minuend[2]&R_CHANNEL_MASK)>>(RGB_LUT_DATA_BITS+RGB_LUT_DATA_BITS));
    tmp_m[3] = (int)((v_minuend[3]&R_CHANNEL_MASK)>>(RGB_LUT_DATA_BITS+RGB_LUT_DATA_BITS));

    tmp_s[0] = (int)((v_subtrahend[0]&R_CHANNEL_MASK)>>(RGB_LUT_DATA_BITS+RGB_LUT_DATA_BITS));
    tmp_s[1] = (int)((v_subtrahend[1]&R_CHANNEL_MASK)>>(RGB_LUT_DATA_BITS+RGB_LUT_DATA_BITS));
    tmp_s[2] = (int)((v_subtrahend[2]&R_CHANNEL_MASK)>>(RGB_LUT_DATA_BITS+RGB_LUT_DATA_BITS));
    tmp_s[3] = (int)((v_subtrahend[3]&R_CHANNEL_MASK)>>(RGB_LUT_DATA_BITS+RGB_LUT_DATA_BITS));

    c0 =  tmp_m[0]<<(COLOR_RANGE_BITS - LUT_ADDR_BITS);
    c1 = (tmp_m[1] - tmp_s[1]);
    c2 = (tmp_m[2] - tmp_s[2]);
    c3 = (tmp_m[3] - tmp_s[3]);

    *R_o = (c0 + c1*weight[0] + c2*weight[1] + c3*weight[2])>>(COLOR_RANGE_BITS - LUT_ADDR_BITS);

    //G
    tmp_m[0] = (int)((v_minuend[0]&G_CHANNEL_MASK)>>RGB_LUT_DATA_BITS);
    tmp_m[1] = (int)((v_minuend[1]&G_CHANNEL_MASK)>>RGB_LUT_DATA_BITS);
    tmp_m[2] = (int)((v_minuend[2]&G_CHANNEL_MASK)>>RGB_LUT_DATA_BITS);
    tmp_m[3] = (int)((v_minuend[3]&G_CHANNEL_MASK)>>RGB_LUT_DATA_BITS);

    tmp_s[0] = (int)((v_subtrahend[0]&G_CHANNEL_MASK)>>RGB_LUT_DATA_BITS);
    tmp_s[1] = (int)((v_subtrahend[1]&G_CHANNEL_MASK)>>RGB_LUT_DATA_BITS);
    tmp_s[2] = (int)((v_subtrahend[2]&G_CHANNEL_MASK)>>RGB_LUT_DATA_BITS);
    tmp_s[3] = (int)((v_subtrahend[3]&G_CHANNEL_MASK)>>RGB_LUT_DATA_BITS);

    c0 =  tmp_m[0]<<(COLOR_RANGE_BITS - LUT_ADDR_BITS);
    c1 = (tmp_m[1] - tmp_s[1]);
    c2 = (tmp_m[2] - tmp_s[2]);
    c3 = (tmp_m[3] - tmp_s[3]);

    *G_o = (c0 + c1*weight[0] + c2*weight[1] + c3*weight[2])>>(COLOR_RANGE_BITS - LUT_ADDR_BITS);

    //B
    tmp_m[0] = (int)(v_minuend[0]&B_CHANNEL_MASK);
    tmp_m[1] = (int)(v_minuend[1]&B_CHANNEL_MASK);
    tmp_m[2] = (int)(v_minuend[2]&B_CHANNEL_MASK);
    tmp_m[3] = (int)(v_minuend[3]&B_CHANNEL_MASK);

    tmp_s[0] = (int)(v_subtrahend[0]&B_CHANNEL_MASK);
    tmp_s[1] = (int)(v_subtrahend[1]&B_CHANNEL_MASK);
    tmp_s[2] = (int)(v_subtrahend[2]&B_CHANNEL_MASK);
    tmp_s[3] = (int)(v_subtrahend[3]&B_CHANNEL_MASK);

    c0 =  tmp_m[0]<<(COLOR_RANGE_BITS - LUT_ADDR_BITS);
    c1 = (tmp_m[1] - tmp_s[1]);
    c2 = (tmp_m[2] - tmp_s[2]);
    c3 = (tmp_m[3] - tmp_s[3]);

    *B_o = (c0 + c1*weight[0] + c2*weight[1] + c3*weight[2])>>(COLOR_RANGE_BITS - LUT_ADDR_BITS);
}

void address_decoder(unsigned int R_i, unsigned int G_i, unsigned B_i,  int *case_id, unsigned int addr[4], unsigned int weight[3])
{
    unsigned int i_0, j_0, k_0, i_1, j_1, k_1;
    unsigned int di, dj, dk;

    i_0 = R_i>>(COLOR_RANGE_BITS-LUT_ADDR_BITS);
    j_0 = G_i>>(COLOR_RANGE_BITS-LUT_ADDR_BITS);
    k_0 = B_i>>(COLOR_RANGE_BITS-LUT_ADDR_BITS);
    i_1 = i_0 + 1;
    j_1 = j_0 + 1;
    k_1 = k_0 + 1;
    di = R_i&LUT_WEIGHT_MASK;
    dj = G_i&LUT_WEIGHT_MASK;
    dk = B_i&LUT_WEIGHT_MASK;

    if(R_i==COLOR_RANGE_MAX)
    {
        di=LUT_CUBE_LEN;
    }

    if(G_i==COLOR_RANGE_MAX)
    {
        dj=LUT_CUBE_LEN;
    }

    if(B_i==COLOR_RANGE_MAX)
    {
        dk=LUT_CUBE_LEN;
    }

    weight[0] = di;
    weight[1] = dj;
    weight[2] = dk;

    if(di>=dj && dj>dk)
    {
        *case_id = 0;

        //addr 000 100 110 111
        addr[0] = i_0*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_0;
        addr[1] = addr[0] + LUT_LVL*LUT_LVL              ; //addr_0 + LUT_LVL*LUT_LVL             //i_1*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_0
        addr[2] = addr[0] + LUT_LVL*LUT_LVL + LUT_LVL    ; //addr_0 + LUT_LVL*LUT_LVL + LUT_LVL   //i_1*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_0
        addr[3] = addr[0] + LUT_LVL*LUT_LVL + LUT_LVL +1 ; //addr_0 + LUT_LVL*LUT_LVL + LUT_LVL +1//i_1*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_1
    }else if(di>=dk && dk>=dj)
    {
        *case_id = 1;

        //addr 000 100 101 111
        addr[0] = i_0*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_0;
        addr[1] = addr[0] + LUT_LVL*LUT_LVL              ; //addr_0 + LUT_LVL*LUT_LVL             //i_1*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_0
        addr[2] = addr[0] + LUT_LVL*LUT_LVL + 1          ; //addr_0 + LUT_LVL*LUT_LVL + 1         //i_1*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_1
        addr[3] = addr[0] + LUT_LVL*LUT_LVL + LUT_LVL +1 ; //addr_0 + LUT_LVL*LUT_LVL + LUT_LVL +1//i_1*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_1
    }else if(dk>di && di>dj)
    {
        *case_id = 2;

        //addr 000 001 101 111
        addr[0] = i_0*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_0;
        addr[1] = addr[0] + 1                            ; //addr_0 + 1                            //i_0*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_1
        addr[2] = addr[0] + LUT_LVL*LUT_LVL + 1          ; //addr_0 + LUT_LVL*LUT_LVL + 1          //i_1*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_1
        addr[3] = addr[0] + LUT_LVL*LUT_LVL + LUT_LVL +1 ; //addr_0 + LUT_LVL*LUT_LVL + LUT_LVL +1 //i_1*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_1
    }else if(dj>di && di>=dk)
    {
        *case_id = 3;

        //addr 000 010 110 111
        addr[0] = i_0*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_0;
        addr[1] = addr[0] + LUT_LVL                      ; //addr_0 + LUT_LVL                     //i_0*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_0
        addr[2] = addr[0] + LUT_LVL*LUT_LVL + LUT_LVL    ; //addr_0 + LUT_LVL*LUT_LVL + LUT_LVL   //i_1*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_0
        addr[3] = addr[0] + LUT_LVL*LUT_LVL + LUT_LVL +1 ; //addr_0 + LUT_LVL*LUT_LVL + LUT_LVL +1//i_1*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_1
    }else if(dj>=dk && dk>=di)
    {
        *case_id = 4;

        //addr 000 010 011 111
        addr[0] = i_0*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_0;
        addr[1] = addr[0] + LUT_LVL                      ; //addr_0 + LUT_LVL                     //i_0*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_0
        addr[2] = addr[0] + LUT_LVL + 1                  ; //addr_0 + LUT_LVL + 1                 //i_0*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_1
        addr[3] = addr[0] + LUT_LVL*LUT_LVL + LUT_LVL +1 ; //addr_0 + LUT_LVL*LUT_LVL + LUT_LVL +1//i_1*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_1
    }else if(dk>dj && dj>=di)
    {
        *case_id = 5;

        //addr 000 001 011 111
        addr[0] = i_0*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_0;
        addr[1] = addr[0] + 1                            ;//i_0*LUT_LVL*LUT_LVL + j_0*LUT_LVL + k_1; //
        addr[2] = addr[0] + LUT_LVL + 1                  ;//i_0*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_1; //
        addr[3] = addr[0] + LUT_LVL*LUT_LVL + LUT_LVL +1 ; //i_1*LUT_LVL*LUT_LVL + j_1*LUT_LVL + k_1; //
    }
}

void data_switch(int case_id, unsigned int v_in_0, unsigned int v_in_1, unsigned int v_in_2, unsigned int v_in_3, unsigned int *v_out_0, unsigned int *v_out_1, unsigned int *v_out_2, unsigned int *v_out_3)
{
    switch (case_id)
    {
    case (0):
        {
            *v_out_0 = v_in_0;
            *v_out_1 = v_in_1;
            *v_out_2 = v_in_2;
            *v_out_3 = v_in_3;
            break;
        }

    case (1):
        {
            *v_out_0 = v_in_0;
            *v_out_1 = v_in_1;
            *v_out_2 = v_in_3;
            *v_out_3 = v_in_2;
            break;
        }

    case (2):
        {
            *v_out_0 = v_in_0;
            *v_out_1 = v_in_2;
            *v_out_2 = v_in_3;
            *v_out_3 = v_in_1;
            break;
        }

    case (3):
        {
            *v_out_0 = v_in_0;
            *v_out_1 = v_in_2;
            *v_out_2 = v_in_1;
            *v_out_3 = v_in_3;
            break;
        }

    case(4):
        {
            *v_out_0 = v_in_0;
            *v_out_1 = v_in_3;
            *v_out_2 = v_in_1;
            *v_out_3 = v_in_2;
            break;
        }

    case (5):
        {
            *v_out_0 = v_in_0;
            *v_out_1 = v_in_3;
            *v_out_2 = v_in_2;
            *v_out_3 = v_in_1;
            break;
        }
    }
}

void free_lut_mem(unsigned int *pt_lut)
{
    free(pt_lut);
    pt_lut = NULL;
}

void load_3d_lut(FILE *ptf, unsigned int *pt_lut)
{
    int i;

    for(i=0;i<LUT_LVL*LUT_LVL*LUT_LVL;i++)
    {
        fscanf(ptf, "%d", &pt_lut[i]);
    }

}

void load_color_data(FILE *ptf, unsigned int *pt_data)
{
    int i;

    for(i=0;i<LUT_LVL*LUT_LVL*LUT_LVL*3;i++)
    {
        fscanf(ptf, "%d", &pt_data[i]);
    }

    fclose(ptf);
}

void generate_3d_lut(unsigned int *pt_data, unsigned int *pt_lut)
{
    int i;
    unsigned int RGB_10bits, R, G, B;


    for(i=0;i<LUT_LVL*LUT_LVL*LUT_LVL;i++)
    {
        RGB_10bits = 0;

        R = pt_data[i*3+0]<<2;
        G = pt_data[i*3+1]<<2;
        B = pt_data[i*3+2]<<2;

        RGB_10bits = (R<<20)+(G<<10)+B;

        pt_lut[i] = RGB_10bits;
    }
}

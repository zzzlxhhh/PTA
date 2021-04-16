#include <math.h>
#include <string.h>

#include "DSPF_sp_qrd_cn.h"

void part_op(int vBlock_index)
{
    switch (vBlock_index)
    {
    case 0: {
        mov_to_vlr(0xFFFF);
        break;
    }
    case 1: {
        mov_to_vlr(0xFFFE);
        break;
    }
    case 2: {
        mov_to_vlr(0xFFFC);
        break;
    }
    case 3: {
        mov_to_vlr(0xFFF8);
        break;
    }
    case 4: {
        mov_to_vlr(0xFFF0);
        break;
    }
    case 5: {
        mov_to_vlr(0xFFE0);
        break;
    }
    case 6: {
        mov_to_vlr(0xFFC0);
        break;
    }
    case 7: {
        mov_to_vlr(0xFF80);
        break;
    }
    case 8: {
        mov_to_vlr(0xFF00);
        break;
    }
    case 9: {
        mov_to_vlr(0xFE00);
        break;
    }
    case 10: {
        mov_to_vlr(0xFC00);
        break;
    }
    case 11: {
        mov_to_vlr(0xF800);
        break;
    }
    case 12: {
        mov_to_vlr(0xF000);
        break;
    }
    case 13: {
        mov_to_vlr(0xE000);
        break;
    }
    case 14: {
        mov_to_vlr(0xC000);
        break;
    }
    case 15: {
        mov_to_vlr(0x8000);
        break;
    }
    }
}
void part_op_uv(int vBlock_index)
{
    switch (vBlock_index)
    {
    case 15: {
        mov_to_vlr(0x8000);
        break;
    }
    case 14: {
        mov_to_vlr(0x4000);
        break;
    }
    case 13: {
        mov_to_vlr(0x2000);
        break;
    }
    case 12: {
        mov_to_vlr(0x1000);
        break;
    }
    case 11: {
        mov_to_vlr(0x0800);
        break;
    }
    case 10: {
        mov_to_vlr(0x0400);
        break;
    }
    case 9: {
        mov_to_vlr(0x0200);
        break;
    }
    case 8: {
        mov_to_vlr(0x0100);
        break;
    }
    case 7: {
        mov_to_vlr(0x0080);
        break;
    }
    case 6: {
        mov_to_vlr(0x0040);
        break;
    }
    case 5: {
        mov_to_vlr(0x0020);
        break;
    }
    case 4: {
        mov_to_vlr(0x0010);
        break;
    }
    case 3: {
        mov_to_vlr(0x0008);
        break;
    }
    case 2: {
        mov_to_vlr(0x0004);
        break;
    }
    case 1: {
        mov_to_vlr(0x0002);
        break;
    }
    case 0: {
        mov_to_vlr(0x0001);
        break;
    }
    }
}
//更新uv、R主对角元?  也可尝试用svr更新
void update_uv_R(vector float *v, vector float *uv, int vBlock_index, float alpha)
{
    vector float vtmp;
    vtmp = vec_svbcast(alpha);
    part_op_uv(vBlock_index);

    // update uv u[col] = R[col + col * Ncols] + alpha
    uv[0] = vec_add(uv[0], vtmp);

    
    // update R   R[col + col * Ncols] = -alpha;
    v[0] = vec_sub(vtmp,v[0]);

    mov_to_vlr(0xFFFF);
}
float reduce_16(vector float*v)
{
    float array[16];
    M7002_datatrans(v,array,64);
    int i=0;
    float res=0;
    for(i;i<16;i++)
    {
        res+=array[i];
    }
    return res;
}

//计算第col列的模长，顺便更新一部分uv和R的�?
float norm2(vector float *v, vector float *uv, int cv_p, int vBlock_index)
{
    int i;
    float res;
    vector float vtmp, vzero;
    float s = 0;
    vtmp = vec_svbcast(s);
    vzero = vtmp;
	
    part_op(vBlock_index);
    uv[0] = vec_muli(uv[0], vzero);     
    uv[0] = vec_add(uv[0], v[0]);

    vtmp = vec_mula(v[0], v[0], vtmp);
    v[0] = vec_muli(v[0], vzero);
    mov_to_vlr(0xFFFF);

    for (i = 1; i < cv_p; i++)
    {
        vtmp = vec_mula(v[i], v[i], vtmp);
        uv[i] = v[i];
        v[i] =  vzero;
    }
    res=reduce_16(&vtmp);
    return res;
}

/*
vector float norm2(vector float *v, vector float *uv, int cv_p, int vBlock_index)
{
    int i;
    vector float vtmp, vzero;
    float s = 0;
    vtmp = vec_svbcast(s);
    vzero = vtmp;
	
    part_op(vBlock_index);
    uv[0] = vec_muli(uv[0], vzero); //清空�?在使用加法赋�?    
    uv[0] = vec_add(uv[0], v[0]);

    vtmp = vec_mula(v[0], v[0], vtmp);
    v[0] = vec_muli(v[0], vzero);
    mov_to_vlr(0xFFFF);

    for (i = 1; i < cv_p; i++)
    {
        vtmp = vec_mula(v[i], v[i], vtmp);
        v[i] =  vzero;
        uv[i] = v[i];
    }

    return vtmp;
}
*/
float com_norm_sqr(vector float *v1, vector float *v2, int vBlocks, int vBlock_index)
{
    int i;
    float res;

    vector float vtmp;
    float s = 0;
    vtmp = vec_svbcast(s);

    part_op(vBlock_index);
    vtmp = vec_mula(v1[0], v2[0], vtmp);
    mov_to_vlr(0xFFFF);

    for (i = 1; i < vBlocks; i++)
    {
        vtmp = vec_mula(v1[i], v2[i], vtmp);
    }
    
 	res=reduce_16(&vtmp);
    return res;
}
void v_mul_sub(vector float *v1, vector float *v2, float sum, int vBlocks, int vBlock_index)
{
    int i;
    vector float vtmp;
    vtmp = vec_svbcast(sum);

    part_op(vBlock_index);
    v1[0] = vec_mulb(v2[0], vtmp, v1[0]);
    v1[0] = vec_neg(v1[0]);
    mov_to_vlr(0xFFFF);

    for (i = 1; i < vBlocks; i++)
    {
        v1[i] = vec_mulb(v2[i], vtmp, v1[i]);
        v1[i] = vec_neg(v1[i]);
    }
}


//处理16对齐
int DSPF_sp_qrd_48(const int Nrows, const int Ncols, float *A, float *Q, float *R, float *u)
{

    int row, col,i,k,loop_count, num, vBlock_index, trans_index;
    float alpha, scale, scale_tmp, sum,sump, norm_sqr,diag;
    num = Nrows * Ncols;

    // int rv_to_p = Ncols / 16; // vec in a row to process
    // int cv_to_p = Nrows / 16; // vec in a row to  R process
    int Nvecs = Nrows / 16; // R的向量数
    int Ri = -1;            // Vec matrix_index

    // cv=cv_p;
    //
    memset(Q, 0.0, sizeof(float) * num);
    for (row = 0; row < Nrows; row++)
    {
        Q[row + row * Nrows] = 1.0;
    }

    vector float *r = (vector float *)vmalloc(num * 4);
    vector float *uv = (vector float *)vmalloc(Nrows * 4);
    vector float *q = (vector float *)vmalloc(Nrows * Nrows * 4);
    float *darray=(float *)malloc(64);
    vector float castv;
    //转置传输
    trans_index = Ncols * 4 - 4;
    for (col = 0; col < Ncols; col++)
    {
        M7002_datatrans_index(&A[col], &r[col * Nvecs], Nrows, 1, trans_index);
    }
    M7002_datatrans(Q, q, Nrows * Nrows * 4);

    if (Nrows <= Ncols)
    {
        loop_count = Nrows - 2;
    }
    else
    {
        loop_count = Ncols - 1;
    }
    printf("\n\t op_sum:\t");
    for (col = 0; col <= loop_count; col++)
    {
        if (col % 16 == 0)
            Ri++; // 在向量矩阵中的行index
        sum = 0;
        vBlock_index = col % 16;
        sum = norm2(&r[col * Nvecs + Ri], &uv[Ri], Nvecs - Ri, vBlock_index);
        //printf("%f \t", sum);
        if (sum != 0)
        {
            M7002_datatrans(uv,darray,64);
            diag=darray[vBlock_index];
            alpha = sqrt(sum);
            if (diag >= 0)
            {
                alpha = -alpha;
            }
            //
            // u[col] = diag + alpha;
            //通过vpe控制 广播alpha
            update_uv_R(&r[col * Nvecs + Ri], &uv[Ri], vBlock_index, alpha);
            //norm_sqr = com_norm_sqr(uv + Ri, uv + Ri, Nvecs - Ri, vBlock_index);

            // if (alpha * u[col] != 0.0)
            scale_tmp = alpha * (diag + alpha);
            
            if (scale_tmp != 0.0)
            {
                scale = 1 / scale_tmp;
                //printf("%f \t", scale);
                /* R=Q1*R */
                for (i = col + 1; i < Ncols; i++)
                {
                    sum = 0;
                    sum = com_norm_sqr(&r[i * Nvecs + Ri], &uv[Ri], Nvecs - Ri, vBlock_index);
                    sum *= scale;
                    //printf("%f \t", sum);
                    v_mul_sub(&r[i * Nvecs + Ri], &uv[Ri], sum, Nvecs - Ri, vBlock_index);
                }
                /* Q=A*Q1 */
                for (i = 0; i < Nrows; i++)
                {
                    sum = 0;
                    sum = com_norm_sqr(&q[i * Nvecs + Ri], &uv[Ri], Nvecs - Ri, vBlock_index);
                    sum *= scale;
                    if(Ri<1)
                    printf("%f \t",sum);
                    v_mul_sub(&q[i * Nvecs + Ri], &uv[Ri], sum, Nvecs - Ri, vBlock_index);
                }
            } /* if (norm_sqr!=0) */
        }     /* if (sum!=0) */

    } /* for (col=0;col<=loop_count;col++) */
    // M7002_mat_transpose(r, R, Ncols,Nrows, 0);
    //printf("\t\n");
    trans_index = trans_index<<16;
    for (col = 0; col < Ncols; col++)
    {
        M7002_datatrans_index( &r[col * Nvecs],&R[col], Nrows, 1, trans_index);
    }
    M7002_datatrans(q, Q, Nrows * Nrows * 4);
    M7002_datatrans(uv, u, Nrows * 4);
    vfree(r);
    vfree(uv);
    vfree(q);
    return 0;
}


/* ======================================================================= */
/*  End of file:  DSPF_sp_qrd_cn.c                                         */
/* ----------------------------------------------------------------------- */
/*            Copyright (c) 2013 Texas Instruments, Incorporated.          */
/*                           All Rights Reserved.                          */
/* ======================================================================= */

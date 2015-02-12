/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_drv_voc.c
 @brief  : adpcm player codes
 @author : gexueyuan
 @history:
           2015-1-30    gexueyuan    Created file
           ...
******************************************************************************/
#include "cv_osal.h"
#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_INFO
#define MODULE_NAME "voc"
#include "cv_osal_dbg.h"

    
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "assert.h"
#include "cv_cms_def.h"

#define BUFFERSIZE   4096

short buffer_4k_1[BUFFERSIZE]; 
short buffer_4k_2[BUFFERSIZE]; 

unsigned char fin_flag = 0;

static ADPCMState adpcm_state;


static const int indexTable[ 16 ] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
} ;
/* define an array of index adjustments to the step index value */

#define	MAXSTEPINDEX	88
/* define the maximum value to access the top element of stepSizeTable below */

static const int stepSizeTable[ MAXSTEPINDEX + 1 ] = {
	    7,     8,     9,    10,    11,    12,    13,    14,    16,    17,
	   19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
	   50,    55,    60,    66,    73,    80,    88,    97,   107,   118,
	  130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
	  337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
	  876,   963,  1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
	 2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
	 5894,  6484,  7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
} ;


int DecodeADPCMC( int adpcmSample, ADPCMStatePtr decodeStatePtr )
{
    int step ;
    int delta ;
    int predictionAdjustment ;
    
    if( ( adpcmSample < 0 ) || ( adpcmSample > 15 ) ) {
        osal_printf("[DecodeADPCMC] Error in ADPCM sample, aborting.\n\n" ) ;
        /* function name given since intended as internal error for programmer */
        return 0 ;
    }
    
    if( !decodeStatePtr ) {
        osal_printf("[DecodeADPCMC] Error in ADPCMState, aborting.\n\n" ) ;
        /* function name given since intended as internal error for programmer */
        return 0 ;
    }
        
    step = stepSizeTable[ decodeStatePtr->stepIndex ] ;
    
    delta = adpcmSample ;

    decodeStatePtr->stepIndex += indexTable[ delta ] ;
    /* range check the stepIndex */
    if( decodeStatePtr->stepIndex < 0 ) {
        decodeStatePtr->stepIndex = 0 ;
    }
    else if( decodeStatePtr->stepIndex > MAXSTEPINDEX ) {
        decodeStatePtr->stepIndex = MAXSTEPINDEX ;
    }

    /* remove sign from delta - pull to 3-bit */
    if( delta > 7 ) {
        delta -= 8 ;
    }
    
    predictionAdjustment = ( 2*delta + 1 )* step/8 ;
    if( adpcmSample > 7 ) {
        decodeStatePtr->prediction -= predictionAdjustment ;
    }
    else {
        decodeStatePtr->prediction += predictionAdjustment ;
    }
    
    /* range check the prediction so that fits in number of bits */
    if( decodeStatePtr->prediction > ( 1 << ( MAXBITS - 1 ) ) - 1 ) {
        decodeStatePtr->prediction = ( 1 << ( MAXBITS - 1 ) ) - 1 ;
    }
    else if( decodeStatePtr->prediction < -( 1 << ( MAXBITS - 1 ) ) ) {
        decodeStatePtr->prediction = -( 1 << ( MAXBITS - 1 ) ) ;
    }

    return decodeStatePtr->prediction ;
}

int adpcm_de(char *code, short *pcm, int count)
{
    int a,b;
    short p;

    RT_ASSERT((code != NULL)&&(pcm!= NULL)&&(count >0));

    while(count--){
        a = *code ++;
        b = (a>>4)&0x0f;
        p = DecodeADPCMC(b,&adpcm_state);
        *pcm++ = p;
        *pcm++ = p; // expand to stero
        b = (a)&0x0f;
        p = DecodeADPCMC(b,&adpcm_state); 
        *pcm++ = p;
        *pcm++ = p;
    }
    return 0;
}
adpcm_t adpcm_de_process(char *src_c,int sourceFileLen,int channel)
{
    short *buffer_tmp;
    
    adpcm_t audio_pcm;

    uint32_t tick_adpcm_bg,tick_adpcm_fh;
    

    if(channel)
        buffer_tmp = buffer_4k_2;
    else        
        buffer_tmp = buffer_4k_1;
    tick_adpcm_bg = osal_get_systemtime();
    adpcm_de(src_c,buffer_tmp,sourceFileLen);
    tick_adpcm_fh = osal_get_systemtime();


    rt_kprintf("time of decode is %lu\n\n",tick_adpcm_fh - tick_adpcm_bg);
    audio_pcm.Addr = (uint32_t)buffer_tmp;
    audio_pcm.Size = 8*sourceFileLen;


    return audio_pcm;
}

void adpcm_play(char* pBuffer, uint32_t Size)
{
    adpcm_t audio_pcm;

    static unsigned char channel = 0x00;

    uint16_t count_play = 0;

    rt_kprintf("total size of voc is %d\n\n",Size);
    while(Size>0){
    
    count_play++;
    
    if(Size > BUFFERSIZE/4){
        
        audio_pcm = adpcm_de_process(pBuffer,BUFFERSIZE/4,channel);
        Size -= BUFFERSIZE/4;
        pBuffer += BUFFERSIZE/4;
    }
    else{
        audio_pcm = adpcm_de_process(pBuffer,Size,channel);
        channel = 0;
        Size = 0;
        pBuffer += Size;
    }

    
   while(fin_flag){
       };
   
   Pt8211_AUDIO_Play((uint16_t *)(audio_pcm.Addr), audio_pcm.Size);
   
   rt_kprintf("this %d times decode and play,size = %d Byte\nchannel is %d\naddress is %x\n\n",count_play,audio_pcm.Size,channel,audio_pcm.Addr);
   
    channel = ~channel;


    }


}


void rt_adpcm_thread_entry(void *parameter)
{
    vsa_envar_t *p_vsa = (vsa_envar_t *)parameter;

    adpcm_t *audio_pcm;

    while(1){
        
        if(rt_mb_recv(p_vsa->mb_sound,(rt_uint32_t*)&audio_pcm,RT_WAITING_FOREVER) == OSAL_STATUS_SUCCESS){
            adpcm_play((char*)audio_pcm->Addr,audio_pcm->Size);
        }
        
    }



}


void adpcm_init(void)
{
    osal_task_t  *adpcm_thread;
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;
    

    memset(buffer_4k_1,0,BUFFERSIZE);
    memset(buffer_4k_2,0,BUFFERSIZE);

    adpcm_thread = osal_task_create("t-adpcm",
                           rt_adpcm_thread_entry, p_vsa,
                           RT_VSA_THREAD_STACK_SIZE, RT_ADPCM_THREAD_PRIORITY);

    osal_assert(adpcm_thread != NULL);

    rt_kprintf("adpcm inited!!\n\n");

}

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
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_TRACE
#define MODULE_NAME "voc"
#include "cv_osal_dbg.h"

    
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "assert.h"
#include "cv_cms_def.h"


#define BUFFER_COUNT  2
#define BUFFER_SIZE   4096

short buffer_voc[BUFFER_COUNT][BUFFER_SIZE]; 

static uint8_t cursor = 0;
adpcm_t play_pcm_data; 
osal_sem_t   *sem_adpcm;
osal_sem_t   *sem_play,*sem_finish;


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
		OSAL_MODULE_DBGPRT(MODULE_NAME,OSAL_DEBUG_ERROR,"[DecodeADPCMC] Error in ADPCM sample, aborting.\n\n");
        /* function name given since intended as internal error for programmer */
        return 0 ;
    }
    
    if( !decodeStatePtr ) {
		OSAL_MODULE_DBGPRT(MODULE_NAME,OSAL_DEBUG_ERROR,"[DecodeADPCMC] Error in ADPCMState, aborting.\n\n");
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
    
    adpcm_t audio_pcm;

    uint32_t tick_adpcm_bg,tick_adpcm_fh;
    

    tick_adpcm_bg = osal_get_systemtime();
    adpcm_de(src_c,buffer_voc[channel],sourceFileLen);
    tick_adpcm_fh = osal_get_systemtime();


	OSAL_MODULE_DBGPRT(MODULE_NAME,OSAL_DEBUG_TRACE,"time of decode is %lu\n\n",tick_adpcm_fh - tick_adpcm_bg);
    audio_pcm.addr = (uint32_t)buffer_voc[channel];
    audio_pcm.size = 8*sourceFileLen;


    return audio_pcm;
}

void adpcm_play(char* pBuffer, uint32_t Size)
{

    uint16_t count_play = 0;

	OSAL_MODULE_DBGPRT(MODULE_NAME,OSAL_DEBUG_TRACE,"total size of voc is %d\n\n",Size);
    while(Size > 0){ 
        
        osal_sem_take(sem_adpcm,OSAL_WAITING_FOREVER);
        
        count_play++;
        
        if(Size > BUFFER_SIZE/4){
            
            play_pcm_data = adpcm_de_process(pBuffer,BUFFER_SIZE/4,cursor%BUFFER_COUNT);
            Size -= BUFFER_SIZE/4;
            pBuffer += BUFFER_SIZE/4;
        }
        else{
            play_pcm_data = adpcm_de_process(pBuffer,Size,cursor%BUFFER_COUNT);
            Size = 0;
            pBuffer += Size;
        }

	   OSAL_MODULE_DBGPRT(MODULE_NAME,OSAL_DEBUG_TRACE,\
	   		"this %d times decode and play,size = %lu Byte\nchannel is %d\naddress is %x\n\n",\
	   		count_play,play_pcm_data.size,cursor%BUFFER_COUNT,play_pcm_data.addr);
	   cursor++;       
       osal_sem_release(sem_play);
   }
}


void rt_adpcm_thread_entry(void *parameter)
{
    osal_status_t err;
    adpcm_t *p_audio_adpcm;
    vsa_envar_t *p_vsa = (vsa_envar_t *)parameter;
    while(1){
        err = osal_queue_recv(p_vsa->queue_voc,&p_audio_adpcm,RT_WAITING_FOREVER);
        if( err == OSAL_STATUS_SUCCESS){
            adpcm_play((char*)p_audio_adpcm->addr,p_audio_adpcm->size);
            osal_free(p_audio_adpcm);
        }
        else{
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "%s: rt_mq_recv error [%d]\n", __FUNCTION__, err);         
            osal_free(p_audio_adpcm);
        }
    }



}


void adpcm_init(void)
{
    osal_task_t  *adpcm_thread;
    vsa_envar_t *p_vsa = &p_cms_envar->vsa;
    

    adpcm_thread = osal_task_create("t-adpcm",
                           rt_adpcm_thread_entry, p_vsa,
                           RT_VSA_THREAD_STACK_SIZE, RT_ADPCM_THREAD_PRIORITY);

    osal_assert(adpcm_thread != NULL);


}
void release_semadpcm(void)
{

    osal_sem_release(sem_adpcm);
	
    osal_sem_release(sem_finish);

    sound_en(0);//disable
}

void rt_play_thread_entry(void *parameter)
{
	adpcm_t *p_audio_pcm;
    p_audio_pcm = &play_pcm_data;
    
    while(1){
        osal_sem_take(sem_play,OSAL_WAITING_FOREVER);
		osal_sem_take(sem_finish,OSAL_WAITING_FOREVER);
        sound_en(1);//enable
        Pt8211_AUDIO_Play((uint16_t *)(p_audio_pcm->addr), p_audio_pcm->size);
        //sound_en(GPIO_PuPd_UP);
    }


}
void voc_play_init(void)
{


    osal_task_t  *play_thread;
    
    Pt8211_AUDIO_Init(I2S_AudioFreq_8k);
    
    play_thread = osal_task_create("t-play",
                           rt_play_thread_entry, NULL,
                           RT_VSA_THREAD_STACK_SIZE, RT_PLAY_THREAD_PRIORITY);

    osal_assert(play_thread != NULL);


}
void voc_init(void)
{
    
    adpcm_init();

    voc_play_init();

	sem_adpcm = osal_sem_create("sem-adpcm",BUFFER_COUNT);
	osal_assert(sem_adpcm != NULL);

	sem_play = osal_sem_create("sem-play",0);
	osal_assert(sem_play != NULL);

	sem_finish = osal_sem_create("sem-finish",1);
	osal_assert(sem_finish != NULL);

	
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "module initial\n\n");         


}



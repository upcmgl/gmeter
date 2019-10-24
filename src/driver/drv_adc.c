/*
 * drv_adc.c
 *
 * Created: 2018/12/14 星期五 20:58:07
 *  Author: Administrator
 */ 
#include "dev_var.h"
#include "asf.h"
#include "os.h"
#include "drv_adc.h"


typedef enum{
    AD_CHN_LVD, 
    AD_CHN_BAT,
   AD_CHN_TEMP,
    
    AD_CHN_MAX
}tagADC_CHN;

typedef struct{
    INT32U chn;
    INT16U ad_val;//AD value
    INT16U v;
    INT16S data;//data on app-level
    
}tagAD_DATA;

static volatile INT8U adc_chn_now;
static INT16U v_ref = 3300, ad_max = 0xFFF;//mv
tagAD_DATA ad_data[AD_CHN_MAX];

void adc_start(INT8U id);
static INT16S ad_trans_data(INT8U ch);
static INT16S Convert_R2T(INT32U R);
const INT32U APR03[161];

void Task_ad_init()
{
    struct adc_config cfg;
    
    system_peripheral_clock_enable(ID_ADC);
    
    adc_set_writeprotect(ADC, FALSE);
    
    adc_get_config_defaults(&cfg);
    cfg.mck = SYS_FREQ;
    cfg.startup_time = ADC_STARTUP_TIME_15; //为啥改成了15.
    
    adc_init(ADC, &cfg);
    adc_enable_interrupt(ADC, ADC_INTERRUPT_DATA_READY);
    //adc_enable_interrupt(ADC, ADC_INTERRUPT_EOC_0);
    //adc_enable_interrupt(ADC, ADC_INTERRUPT_EOC_1);
    //adc_enable_interrupt(ADC, ADC_INTERRUPT_EOC_3);
    NVIC_EnableIRQ(ADC_IRQn);
    
    adc_set_writeprotect(ADC, TRUE);
    
    pio_set_writeprotect(PIOA, FALSE);
    pio_set_writeprotect(PIOB, FALSE);

    pio_pull_up(PORT_LVD, PIN_LVD, 0);
    pio_pull_down(PORT_LVD, PIN_LVD, 0);
    pio_pull_up(PORT_V_BAT, PIN_V_BAT, 0);
    pio_pull_down(PORT_V_BAT, PIN_V_BAT, 0);
    pio_pull_up(PORT_V_TEMP, PIN_V_TEMP, 0);
    pio_pull_down(PORT_V_TEMP, PIN_V_TEMP, 0);
    
    pio_set_writeprotect(PIOA, TRUE);
    pio_set_writeprotect(PIOB, TRUE);
    
    
    
    ad_data[AD_CHN_LVD].chn = 3;
    
    ad_data[AD_CHN_BAT].chn = 0;
    
    ad_data[AD_CHN_TEMP].chn = 1;
}

static volatile INT8U  ad_is_ready = 1;
void ADC_Handler( void )
{
    ad_data[adc_chn_now].ad_val = ADC->ADC_LCDR;//ADC->ADC_CDR[ad_data[adc_chn_now].chn];
    ad_data[adc_chn_now].data = ad_trans_data(adc_chn_now);
    if(++adc_chn_now >= AD_CHN_MAX)
    {
		ad_is_ready=1;	    
    }
    else
    {
		adc_start(adc_chn_now);
    }
}
INT8U ad_is_rdy(void)
{
	return ad_is_ready;
}
void ad_rdy_clear(void)
{
	ad_is_ready = 0;
}
void adc_start(INT8U id)
{
    adc_set_writeprotect(ADC, FALSE);
    ADC->ADC_CHDR = 0xff;
    ADC->ADC_CHER = 1<< ad_data[id].chn;
    
    ADC->ADC_CR = 0x02;//software enable start
    //adc_set_writeprotect(ADC, TRUE);
}


void Task_ad_start( void )
{
    adc_chn_now = 0;
    adc_start(adc_chn_now);
}

static INT16S ad_trans_data(INT8U ch)
{
    INT32S data;
    if(ch == AD_CHN_LVD)
    {
        data = v_ref * ad_data[AD_CHN_LVD].ad_val/ad_max;
        ad_data[AD_CHN_LVD].v = data;
        data = 132 *data /12;
        
        gSystemInfo.lvd = data;
    }
    else if(ch == AD_CHN_BAT)
    {
        data = v_ref * ad_data[AD_CHN_BAT].ad_val/ad_max;
        ad_data[AD_CHN_BAT].v = data;
        data = data *5/2;
        
        gSystemInfo.bat_mon = data;
    }
    else if( ch == AD_CHN_TEMP)
    {
        data = v_ref * ad_data[AD_CHN_TEMP].ad_val/ad_max;   //V
        ad_data[AD_CHN_TEMP].v = data;
        data = data * 10000 /(v_ref - data);    //R
        data = Convert_R2T(data);   //Temp
        
        gSystemInfo.dev_temp = data;
    }
            
    return data;
}
INT16S drv_adc_get_LVD()
{
    return ad_data[AD_CHN_LVD].data;
}
INT16S drv_adc_get_Vbat()
{
    return ad_data[AD_CHN_BAT].data;
}
INT16S drv_adc_get_Temp()
{
    return ad_data[AD_CHN_TEMP].data;
}


static INT16S Convert_R2T(INT32U R)
{
    int start=0,end=160,mid;
    while(start<=end)
    {
        mid=(start+end)/2;
        if(R<=APR03[mid])
            start=mid+1;
        else
            end=mid-1;
    }
    return (start-40);
}

const INT32U APR03[161]={   //APR03(NTCG163JF103FT1)
188500,178600,169200,160400,152100,     144300,136900,130000,123400,117200,  /* -40~-31 */
111300,105800,100600,95640,90970,   86560,82380,78430,74690,71140,       /* -30~-21 */
67790,64610,61600,58740,56030,      53460,51030,48710,46520,44430,       /* -20~-11 */    
42450,40570,38780,37080,35460,      33930,32460,31070,29750,28490,       /* -10~-1  */    
27280,26140,25050,24010,23020,      22070,21170,20310,19490,18710,17960,   /*  0 ~ 10 */     
17250,16570,15910,15290,14700,      14130,13590,13070,12570,12090,       /*  11~ 20 */   
11640,11200,10780,10380,10000,      9633,9282,8945,8622,8312,            /*  21~ 30 */
8015,7730,7456,7194,6942,           6700,6468,6245,6031,5826,            /*  31~ 40 */
5628,5438,5255,5080,4911,           4749,4592,4442,4297,4158,            /*  41~ 50 */   
4024,3895,3771,3651,3536,           3425,3318,3215,3115,3019,            /*  51~ 60 */ 
2927,2837,2751,2668,2588,           2511,2436,2364,2295,2227,            /*  61~ 70 */
2163,2100,2039,1981,1924,           1869,1817,1765,1716,1668,            /*  71~ 80 */
1622,1577,1534,1492,1451,           1412,1374,1337,1302,1267,            /*  81~ 90 */   
1234,1201,1170,1139,1110,           1081,1054,1027,1001,975,             /*  91~100 */ 
951,927,904,881,860,                838,818,798,779,760,                 /* 101~110 */ 
742,724,707,690,674,                658,643,628,613,599                  /* 111~120 */    
};
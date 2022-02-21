#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include.h"
#include "base_type.h"
#include "FlashEraseWrite.c"	
#include "GSM.c"	
#include "tablouri.h"

#define asteapta 	delay2(0x0f);
#define wP35_0		DioClr(pADI_GP3, BIT5)
#define wP35_1		DioSet(pADI_GP3, BIT5)
#define wP03_0		DioClr(pADI_GP0, BIT3)
#define wP03_1		DioSet(pADI_GP0, BIT3)
#define CSM_0 		DioClr(pADI_GP0, BIT3)
#define CSM_1 		DioSet(pADI_GP0, BIT3)	

extern unsigned int contor_get_number, start_stop=0, nr_timer, ii2, ii3, iii;
extern unsigned long rezultat[128];
extern unsigned char bat_clk, repede_sleep=0, net_stat, count_fail, starts=0, porneste=0, power_up=0, working=0, power_down=0, already_up=0, get_number=0;
extern unsigned char print_number_call=0, dormi=1, network;
extern unsigned char buffer_clk[100], buffer_numar[50], buffer_net[30], suna_adas, get_credit, buffer_get_number[200];
extern unsigned int contor_clk, contor_numar, contor_net;
unsigned char suna_adas2=0, sari=0;
extern unsigned char primit_FTPPUT=0, asteapta_ftpput=0;
extern unsigned char nume_client_test[280], scrie_on_flash[280];
void delay (unsigned int n_val);
void buffer_RAM0_to_main (short page_addr);
void erase_all_memory_Tx (void);

unsigned int data_frame_index, data_frame_T, sampeling, ii4;
unsigned char ADAS_data[5][4], header_B[4];
unsigned char first_byte, page_start=0x00;
char data_ECG_v[256];
char data_RTYU[50];

//extern unsigned char mem_buffer1[1300], contor_mem_buffer1;

void UARTInit(int uiBaud);
void delay2 (unsigned long length);
//void print_time (void);
void delay_0(unsigned long vv);
void wait_for(void);
void power_up_gsm(void);
void check_credit (void);
void set_watchdog(void);
void func_baterie(void);
void func_ceas(void);
void print_status(void);
void send_date_adas(void);
void go_to_sleep (void);
void check_network(void);
void call_power_up(void);
extern void write_FLASH (int page_addr);
extern void read_nume_prenume_FLASH_test(unsigned int ADDR_pagina);
void read_adas(void);
extern void erase_all_memory(void);
void ADAS_config(void);

int main (void)
	{
		
		SysClkCfg(CLKCON_CD_DIV1, CLKCON_CLKMUX_HFOSC, CLKCON_CLKOUT_UCLKCG, XOSCCON_ENABLE_DIS);
		
		set_watchdog();
		test_uart = 0;

// 	P1.2 and P1.3  as output
		DioCfg(pADI_GP1, 0x0050);
		DioOen(pADI_GP1, BIT2 | BIT3);	
		
//		UARTInit(115200);
		UARTInit(57600);
  	printf("\n== Start microcontroler ==\n");

  	UrtIntCfg(COMIEN_ERBFI_EN); 			// enable Rx interrupts
		NVIC_SetPriority(UART_IRQn, 1);  
  	NVIC_EnableIRQ(UART_IRQn);				// setup to receive data using interrupts
	

	pADI_GP0->GPCON &= ~(GP0CON_CON3_MSK | GP0CON_CON6_MSK);
	pADI_GP0->GPCON |=  (GP0CON_CON3_GPIOIRQ1 | GP0CON_CON6_GPIOIRQ2);		
		
  EiCfg(EXTINT1, INT_EN, INT_EDGES); // P0.3
  NVIC_SetPriority(EINT1_IRQn, 1);
  EiClr(EXTINT1);
  NVIC_ClearPendingIRQ(EINT1_IRQn);		


	EiCfg(EXTINT2, INT_EN, INT_EDGES); // P0.6
  NVIC_SetPriority(EINT2_IRQn, 1);
  EiClr(EXTINT2);
  NVIC_ClearPendingIRQ(EINT2_IRQn);			
	NVIC_EnableIRQ(EINT2_IRQn);			// activez intreruperea 	

		
/// timer0 500us
	GptLd (pADI_TM0, 15); 							// Interval of 500us
	GptCfg(pADI_TM0, TCON_CLK_LFOSC, TCON_PRE_DIV1, TCON_ENABLE|TCON_MOD_PERIODIC);
	while (GptSta(pADI_TM0)& TSTA_CON); // wait for sync of TCON write. required because of use of asynchronous clock
	GptClrInt(pADI_TM0,TCLRI_TMOUT);
	while (GptSta(pADI_TM0)& TSTA_CLRI); // wait for sync of TCLRI write. required because of use of asynchronous clock
	NVIC_EnableIRQ(TIMER0_IRQn);
//	NVIC_DisableIRQ(TIMER0_IRQn);
/// end timer0 500us

/// timer1 100ms
	GptLd (pADI_TM1, 13); 							// Interval of 101ms
	GptCfg(pADI_TM1, TCON_CLK_LFOSC, TCON_PRE_DIV256, TCON_ENABLE|TCON_MOD_PERIODIC);
	while (GptSta(pADI_TM1)& TSTA_CON); // wait for sync of TCON write. required because of use of asynchronous clock
	GptClrInt(pADI_TM1,TCLRI_TMOUT);
	while (GptSta(pADI_TM1)& TSTA_CLRI); // wait for sync of TCLRI write. required because of use of asynchronous clock
	NVIC_EnableIRQ(TIMER1_IRQn);
/// end timer 100ms

// SPI mem
	DioCfg(pADI_GP0,GP0CON_CON0_SPI1MISO | GP0CON_CON1_SPI1SCLK | GP0CON_CON2_SPI1MOSI | GP0CON_CON3_GPIOIRQ1);// | GP0CON_CON3_SPI1CS0); 
	DioOen(pADI_GP0,BIT3);
	SpiBaud(pADI_SPI1,4,SPIDIV_BCRST_DIS);
	SpiCfg(pADI_SPI1,SPICON_MOD_TX1RX1,SPICON_MASEN_EN,SPICON_ENABLE_EN|SPICON_CPHA_SAMPLETRAILING|SPICON_CPOL_HIGH| SPICON_TIM_TXWR); 
	DioClr(pADI_GP0, BIT3);	
	delay_0(1000); 
	DioSet(pADI_GP0, BIT3);	
// END SPI mem 

	// P3.2  as output
		DioCfg(pADI_GP3, 0x0550);
		DioOen(pADI_GP3, BIT2 | BIT3 | BIT4 | BIT5);
		DioSet(pADI_GP3, BIT2 | BIT3 | BIT4); DioSet(pADI_GP1, BIT3);
		nr_timer = 0; while(nr_timer<2);	
		DioClr(pADI_GP3, BIT2 | BIT3 | BIT4); DioClr(pADI_GP1, BIT3);
		nr_timer = 0; while(nr_timer<5);	
		DioSet(pADI_GP3, BIT2 | BIT3 | BIT4); DioSet(pADI_GP1, BIT3);
		nr_timer = 0; while(nr_timer<2);	
		DioClr(pADI_GP3, BIT2 | BIT3 | BIT4); DioClr(pADI_GP1, BIT3);
		nr_timer = 0; while(nr_timer<5);	
		DioSet(pADI_GP3, BIT2 | BIT3 | BIT4); DioSet(pADI_GP1, BIT3);
		nr_timer = 0; while(nr_timer<2);	
		DioClr(pADI_GP3, BIT2 | BIT3 | BIT4); DioClr(pADI_GP1, BIT3);
		nr_timer = 0; while(nr_timer<5);
		DioSet(pADI_GP3, BIT2);

			ulSelectPage = 0x1E000;
			uiArraySize = 128;
			ReadFlash(ulSelectPage,uiArraySize);

		iii = 0;
			for(ii2=0; ii2<10; ii2++)
				{
					for(ii3=0; ii3<10; ii3++)
					{
						telefon[ii2][ii3] = rezultat[iii++];
					}				
				}			
			for (ii2=0; ii2 < 10; ii2++)
				{
					telefon[0][ii2] = telefon_cristi[ii2];
				}
	
			for (ii2=0; ii2 < 10; ii2++)
				{
					printf("Telefon: ");
					for (ii3=0; ii3 < 10; ii3++)
						{
							printf("%c",telefon[ii2][ii3]);
						}
					printf("\n");			
				}		
///////////////////////
//				printf("\nSTART ACHIZITIE DATE ADAS1000 :) \n");
//				read_adas();

//				for(ii2=0; ii2<180; ii2++)
//					{
//						read_nume_prenume_FLASH_test(ii2);						
//						for(ii3=0; ii3<14; ii3++)
//						{				
//					printf(" %c%c%c%c", nume_client_test[ii3*18 + 3], nume_client_test[ii3*18 + 4], nume_client_test[ii3*18 + 5], nume_client_test[ii3*18 + 6]);
//					printf(" %c%c%c%c", nume_client_test[ii3*18 + 8], nume_client_test[ii3*18 + 9], nume_client_test[ii3*18 + 10], nume_client_test[ii3*18 + 11]);
//					printf(" %c%c%c%c \n", nume_client_test[ii3*18 + 13], nume_client_test[ii3*18 + 14], nume_client_test[ii3*18 + 15], nume_client_test[ii3*18 + 16]);							
//							for(ii4=0; ii4<100; ii4++);
//						}	
//					}	
//				printf("\nTerminat de citit 180 pagini Flash  :) \n");
				
				
//				printf("\nSTART ACHIZITIE DATE ADAS1000 :) \n");
//				read_adas();

//				for(ii2=0; ii2<180; ii2++)
//					{
//						read_nume_prenume_FLASH_test(ii2);						
//						for(ii3=0; ii3<14; ii3++)
//						{				
//							printf(" %02x%02x", nume_client_test[ii3*18 + 4], nume_client_test[ii3*18 + 5]);
//							printf(" %02x%02x", nume_client_test[ii3*18 + 7], nume_client_test[ii3*18 + 8]);
//							printf(" %02x%02x \r", nume_client_test[ii3*18 + 10], nume_client_test[ii3*18 + 11]);
//							for(ii4=0; ii4<100; ii4++);
//						}	
//					}	
//				printf("\nTerminat de citit 180 pagini Flash  :) \n");
				
				
////				for(ii2=0; ii2<180; ii2++)
////					{
//////						printf("\nPag=%d", ii2);
////						read_nume_prenume_FLASH_test(ii2);						
////						for(ii3=0; ii3<252; ii3++)
////						{
////							printf(" %02x", nume_client_test[ii3]);
////						}	
////					}	
////				printf("\nTerminat de citit 180 pagini Flash  :) \n");	

//////////				while(1);		// ptr. Marius ca sa faca teste
//////////				trimite_pagini_mem(page_start, 180);	// citeste cate 5 pagini odata .... ptr. GPRS
//////////				for(ii2=5; ii2<10; ii2++)
//////////					{
//////////						for(ii3=0; ii3<256; ii3++)
//////////						{
//////////							scrie_on_flash[ii3] = 0x34;
//////////						}	
//////////						write_FLASH(ii2);
//////////					}	
//////////				trimite_pagini_mem(5, 5);	
//////////				printf("\nTERMINAT citirea 3  :) \n");	
				
			
				power_up_gsm(); exit_while=75; 	  while(!(exit_while<=5));	 // wait
				check_network();															// SIM instalat ? 
				WdtClrInt(0xCCCC);													// RESET WATCHDOG!   good to be here ...
				printf("%s\n",com01); delay2(0x000FFFFF);		// AT wake up !!!
				printf("%s\n",com01); delay2(0x000FFFFF);		// AT wake up !!!
				nr_timer = 0; while(nr_timer<3);	
				printf("%s\n",com28);		delay2(0x000FFFFF);		// CPOWD - normal Power_Down
				sari = 1;

	while(1)
		{

			if(!sari) {nr_timer = 0; while(nr_timer<50);	}				// wait for Button
			sari = 0;
			WdtClrInt(0xCCCC);																		// RESET WATCHDOG!   good to be here ...
			
			if(power_up)							// la apasare buton Power_up
					{
						DioSet(pADI_GP3, BIT3);	nr_timer = 0; while(nr_timer<2);	
						DioClr(pADI_GP3, BIT3);	nr_timer = 0; while(nr_timer<2);	
						DioSet(pADI_GP3, BIT3);	nr_timer = 0; while(nr_timer<2);	
						DioClr(pADI_GP3, BIT3);	nr_timer = 0; while(nr_timer<2);	
						DioSet(pADI_GP3, BIT3);	nr_timer = 0; while(nr_timer<2);	
						DioClr(pADI_GP3, BIT3);	nr_timer = 0; while(nr_timer<2);	
						DioSet(pADI_GP3, BIT3);	nr_timer = 0; while(nr_timer<2);	
						DioClr(pADI_GP3, BIT3);	nr_timer = 0; while(nr_timer<2);	
						DioSet(pADI_GP3, BIT3);
						power_up = 0;
						call_power_up();
						working = 1; already_up = 1;
					}
				if(power_down)
						{
							UARTInit(57600);
							printf("\nPower dowsn software\n");
							DioSet(pADI_GP3, BIT4);	nr_timer = 0; while(nr_timer<2);	
							DioClr(pADI_GP3, BIT4);	nr_timer = 0; while(nr_timer<2);	
							DioSet(pADI_GP3, BIT4);	nr_timer = 0; while(nr_timer<2);	
							DioClr(pADI_GP3, BIT4);	nr_timer = 0; while(nr_timer<2);	
							DioSet(pADI_GP3, BIT4);	nr_timer = 0; while(nr_timer<2);	
							DioClr(pADI_GP3, BIT4);	nr_timer = 0; while(nr_timer<2);	
							DioSet(pADI_GP3, BIT4);	nr_timer = 0; while(nr_timer<2);	
							DioClr(pADI_GP3, BIT4);	nr_timer = 0; while(nr_timer<2);	
							DioSet(pADI_GP3, BIT4);							
							already_up = 0;
							power_down = 0;
							nr_timer = 0; while(nr_timer<10);							// 
							printf("%s\n",com28);		delay2(0x000FFFFF);		// CPOWD - normal Power_Down
							nr_timer = 0; while(nr_timer<50);							// 
							DioClr(pADI_GP1, BIT3);
						}

				if(working)								// intru daca am Hibernate+Working
						{
							if(repede_sleep && already_up)			// IMPORTANT -> nu intru in acest IF daca am wake-up (doar SMS sau CALL) !!!!!!!!!!!!!!!!!!!!!
								{
										repede_sleep = 0;
//										printf("\nWhile1\n");
										contor_get_number = 0;
										DioSet(pADI_GP3, BIT2);
//										exit_while=70; while(!(exit_while<=5));		// ????????????????????							
										check_call();	
											
										if((contor_AT > 350) && contor_AT0)		// rutina utila doar in modul "Charging"
												{
													contor_AT = 0; contor_AT0 = 0;
													printf("%s\n",com01); delay2(1000000); printf("%s\n",com01);
												}											
										if(already_up && dormi) check_network();
										if((suna_adas || suna_adas2)&& network)												
											{
												// disable GSM
												printf("%s\n",com28);		delay2(0x000FFFFF);		// CPOWD - normal Power_Down
												nr_timer = 0; while(nr_timer<10);							// 
															DioSet(pADI_GP3,  BIT3 | BIT4);	nr_timer = 0; while(nr_timer<2);	
															DioClr(pADI_GP3,  BIT3 | BIT4);	nr_timer = 0; while(nr_timer<2);	
															DioSet(pADI_GP3,  BIT3 | BIT4);	nr_timer = 0; while(nr_timer<2);	
															DioClr(pADI_GP3,  BIT3 | BIT4);	nr_timer = 0; while(nr_timer<2);	
															DioSet(pADI_GP3,  BIT3 | BIT4);	nr_timer = 0; while(nr_timer<2);	
															DioClr(pADI_GP3,  BIT3 | BIT4);	nr_timer = 0; while(nr_timer<2);	
															DioSet(pADI_GP3,  BIT3 | BIT4);	nr_timer = 0; while(nr_timer<2);	
															DioClr(pADI_GP3,  BIT3 | BIT4);	nr_timer = 0; while(nr_timer<2);
												// citeste Date ADAS
														read_adas();
												// enable GSM																			
														power_up_gsm();	
														nr_timer = 0; 		while(nr_timer<50);	
														primit_ok = 0;												
														printf("%s\n",com01); delay2(0x000FFFFF);		// AT wake up !!!
														printf("%s\n",com01); delay2(0x000FFFFF);		// AT wake up !!!												
												
												suna_adas = 0;
												suna_adas2= 0;
												send_date_adas();
											}

										functie_sms();
								}
							if(already_up && dormi) check_network();			// verifica daca am SIM chiar daca sunt in Hibernate								
						}						
			working = 1;																// nu schimba pozitia! la Initializare NU se executa CREG (check network)
			if(dormi) go_to_sleep();
				
		}// end_while(1)
}// end_main



// Simple Delay routine
void delay2 (unsigned long length) {
	while (length >0)
    	length--;
}

void delay (unsigned int n_val)
	{
	 while (n_val > 0)
	 	{
	 n_val--;
		}
	}

void wait_for(void)
	{
		delay_0(1);
	}

void delay_0(unsigned long vv)
	{
		 do
		 	{
				vv--;
			}
		while (vv>0);
	}


///////////////////////////////////////////////////////////////////////////
// Timer0 handler 
///////////////////////////////////////////////////////////////////////////
void GP_Tmr0_Int_Handler ()
{
  if (GptSta(pADI_TM0)== TSTA_TMOUT) // if timout interrupt
  {
    GptClrInt(pADI_TM0,TCLRI_TMOUT);
		sampeling++;
    while (GptSta(pADI_TM0)& TSTA_CLRI); // wait for sync of TCLRI write. required because of use of asynchronous clock
  }
}	
///////////////////////////////////////////////////////////////////////////
// Timer1 handler 
///////////////////////////////////////////////////////////////////////////
void GP_Tmr1_Int_Handler ()
{
  if (GptSta(pADI_TM1)== TSTA_TMOUT) // if timout interrupt
  {
    GptClrInt(pADI_TM1,TCLRI_TMOUT);
	nr_timer++;
	contor_AT++;
	start_stop++;
	exit_while--;	if(exit_while==0) exit_while=0xFFFF;
    while (GptSta(pADI_TM1)& TSTA_CLRI); // wait for sync of TCLRI write. required because of use of asynchronous clock
  }
}



///////////////////////////////////////////////////////////////////////////
// UART Interrupt handler 
///////////////////////////////////////////////////////////////////////////
void UART_Int_Handler ()
{ 	
	int ucCOMIID0; 
 	ucCOMIID0 = UrtIntSta();		// Read UART Interrupt ID register

  if ((ucCOMIID0 & COMIIR_STA_RXBUFFULL) == COMIIR_STA_RXBUFFULL)	  // Receive buffer full interrupt
  {
	ucRxBuffer[6] = ucRxBuffer[5];
	ucRxBuffer[5] = ucRxBuffer[4];
  ucRxBuffer[4] = ucRxBuffer[3];	
	ucRxBuffer[3] = ucRxBuffer[2];
	ucRxBuffer[2] = ucRxBuffer[1];
  ucRxBuffer[1] = ucRxBuffer[0];
	ucRxBuffer[0]	= UrtRx(0);   //call UrtRd() clears COMIIR_STA_RXBUFFULL
	if(ucRxBuffer[1]=='O' && ucRxBuffer[0]=='K')
		{
			primit_ok=1;
			asteapta;
		}

		if(sms2)
			{
				buffer_get_number[contor_get_number] = ucRxBuffer[0];
				contor_get_number++;
				if(contor_get_number > 106)
					{
						print_number = 1;
						sms2 = 0;
					}		
			}

		if(asteapta_ftpput)
			{
				if(ucRxBuffer[6]=='+' && ucRxBuffer[5]=='F' && ucRxBuffer[4]=='T' && ucRxBuffer[3]=='P' && ucRxBuffer[2]=='P' && ucRxBuffer[1]=='U' && ucRxBuffer[0]=='T')
					{primit_FTPPUT=1;
						asteapta;
					}
			}


	if(receptie) 
		{
			contor_buffer_download++;
			if( (contor_buffer_download >= 1460+36+41) && scade)
				{
					scade = 0;
					contor_buffer_download -= 41;
				}
		}
	if(clip)
		{
			if(ucRxBuffer[3]=='C' && ucRxBuffer[2]=='L' && ucRxBuffer[1]=='I' && ucRxBuffer[0]=='P')
				{			
					contor_numar = 0;
					get_number=1;
					clip = 0;
					asteapta;
				}
		}

	if(get_number) 
		{
			buffer_numar[contor_numar++] = ucRxBuffer[0];
			if(contor_numar >= 25)			
				{
					print_number_call = 1;
					get_number = 0;
					exit_while = 7;		// iesi din while si du-te repede la ATH
				}
		}
	if(get_credit)
		{
			buffer_get_number[contor_get_number++] = ucRxBuffer[0];
			if(contor_get_number > 30)
				{
					print_credit = 1;
				}			
		}
	if(bat_clk) 
		{
			buffer_clk[contor_clk++] = ucRxBuffer[0];
		}
	if(net_stat) 
		{
			buffer_net[contor_net++] = ucRxBuffer[0];
		}	
		
  }
}




void set_watchdog(void)
{
	  	WdtGo(T3CON_ENABLE_DIS);																//
			while(WdtSta() & T3STA_CON); 														// wait for sync of T3CON write						
	    WdtCfg(T3CON_PRE_DIV4096,T3CON_IRQ_DIS,T3CON_PD_DIS);  	// config while timer is disabled 
	    	while(WdtSta() & T3STA_CON); 													// wait for sync of T3CON write 		

//	    WdtLd(1440); 							// load timeout - 3min			
//	    WdtLd(2400); 							// load timeout - 5min			
	    WdtLd(3360); 							// load timeout - 7min		
	
	    	while(WdtSta() & T3STA_LD); 		// wait for sync of T3LD write
			WdtGo(T3CON_ENABLE_EN);  				// start the timer
	    	while(WdtSta() & T3STA_CON); 		// wait for sync of T3CON write
	
	WutCfg(T2CON_MOD_PERIODIC,T2CON_WUEN_EN,T2CON_PRE_DIV32768,T2CON_CLK_LFOSC);  // config while timer is disabled 
  WutLdWr(3,240); // load 4 min.
  WutClrInt(T2CLRI_WUFD); 
  WutCfgInt(T2IEN_WUFD,1); // enables field D interrupt
  NVIC_EnableIRQ(WUT_IRQn);
  WutGo(T2CON_ENABLE_EN);  // start the timer	
}

void UARTInit(int uiBaud)
{
  UrtCfg(0,uiBaud,COMLCR_WLS_8BITS,COMLCR_STOP_DIS);
  // Enable the UART functionality on P1.0\P1.1
  pADI_GP1->GPCON &= ~(GP1CON_CON0_MSK     | GP1CON_CON1_MSK    );
  pADI_GP1->GPCON |=  (GP1CON_CON0_UART0RXD| GP1CON_CON1_UART0TXD);
}

///////////// go_to_sleep begin ///////////////////////////
void go_to_sleep (void)
	{
		WdtClrInt(0xCCCC);					// RESET WATCHDOG!
		
		NVIC_DisableIRQ(UART_IRQn);
//		printf("\ngo to SLEEP\n");
		exit_while=15; while(!(exit_while<=5));
		DioClr(pADI_GP3, BIT2 | BIT3 | BIT4);	
		if (!network) DioSet(pADI_GP3, BIT4);
		
		// Enable the IRQ1 on 
			pADI_GP1->GPCON &= ~(GP1CON_CON0_MSK);
			pADI_GP1->GPCON |=  (GP1CON_CON0_GPIOIRQ4);		
		
		
  EiCfg(EXTINT4, INT_EN, INT_EDGES); // P1.0
  NVIC_SetPriority(EINT4_IRQn, 1);
  EiClr(EXTINT4);
  NVIC_ClearPendingIRQ(EINT4_IRQn);				
	NVIC_EnableIRQ(EINT4_IRQn);			// activez intreruperea 			

		
		PwrCfg(PWRMOD_MOD_HIBERNATE, SCR_SLEEPONEXIT_DIS);
        if (!SWACT_ACT_BBA) 							// not in debug mode
           while (!PWRMOD_WICENACK_BBA); 	// wait until WICENACK is set   
        __DSB();  												// wait for PWRMOD register to be written
        __WFI();  												// Go to sleep! 
	}
///////////// go_to_sleep end /////////////////////////////
	
///////////////////////////////////////////////////////////////////////////
// WUT handler 
///////////////////////////////////////////////////////////////////////////
void WakeUp_Int_Handler(void)
{
	UARTInit(57600);
//	printf("\nWake UP !!!\n");

	NVIC_DisableIRQ(EINT4_IRQn);
	UrtIntCfg(COMIEN_ERBFI_EN); 			// enable Rx interrupts
	NVIC_SetPriority(UART_IRQn, 1);  
	NVIC_EnableIRQ(UART_IRQn);				// setup to receive data using interrupts

	WutClrInt(T2CLRI_WUFD);  
	repede_sleep=0;	
  __DSB();  
  return;
}

/////////////////////////////////////////////////////////////////////////
// External Int1 handler 
/////////////////////////////////////////////////////////////////////////
void Ext_Int4_Handler ()   	// intrerupere externa IRQ4 -> de la UART
{
	UARTInit(57600);
//	printf("\nAm ajuns in intreruperea EXT4\n");

	NVIC_DisableIRQ(EINT4_IRQn);
	UrtIntCfg(COMIEN_ERBFI_EN); 			// enable Rx interrupts
	NVIC_SetPriority(UART_IRQn, 1);  
	NVIC_EnableIRQ(UART_IRQn);				// setup to receive data using interrupts
  EiClr(EXTINT4);

	repede_sleep=1;
}

void Ext_Int2_Handler ()   // BUTON
{
	UARTInit(57600);
//	printf("\nBUTON\n");
	
	if (!starts) 
	{
			start_stop = 0;
			starts = 1;
			repede_sleep=0;
			dormi = 0;
	}
	else 
		{	if (start_stop > 9) 
				{
					printf("\nLUNG = %d\n", start_stop);
					if (already_up)	{power_down=1; DioSet(pADI_GP3, BIT4);}
					else 						{power_up = 1;	DioSet(pADI_GP3, BIT3); DioSet(pADI_GP1, BIT3);} // GSM Initialization
//					suna_adas=0;
				}
			
			else 
				{		printf("\n scurt = %d\n", start_stop);
						if (already_up) 
							{	repede_sleep=1;  
								suna_adas2=1;
								DioSet(pADI_GP3, BIT2); 
							}
				}
			dormi = 1;	
			starts = 0;
			primit_ok=1;		// uneori se mai ramane blocat datorita apasarilor succesive cand se asteapta raspunsul OK de la GSM
			nr_timer = 100;
		}
		
	UrtIntCfg(COMIEN_ERBFI_EN); 			// enable Rx interrupts
	NVIC_SetPriority(UART_IRQn, 1);  
	NVIC_EnableIRQ(UART_IRQn);				// setup to receive data using interrupts
  EiClr(EXTINT2);
}

void call_power_up(void)
{
// turn on GSM Module		
		power_up_gsm();	
		primit_ok = 0;												
		printf("%s\n",com01); delay2(0x000FFFFF);		// AT wake up !!!
		printf("%s\n",com01); delay2(0x000FFFFF);		// AT wake up !!!
		nr_timer = 0;
		while(nr_timer<20);	
		printf("%s\n",com01); while(!primit_ok); primit_ok = 0;	  	delay2(0x000FFFFF);
		printf("%s\n",com01a); while(!primit_ok); primit_ok = 0;	delay2(0x000FFFFF);
		count_fail = 0; exit_while=15;
								do
									{
										printf("%s\n",com20); 	
										exit_while=15; while(!(exit_while<=5));
										count_fail++;
										if(count_fail > 5)
											{
												DioClr(pADI_GP3, BIT2 | BIT3 | BIT4);
//												while(1)																				// mori aici - probleme cu reteaua GSM !!!!!!!!!!!!!!!!!!
												WdtClrInt(0xCCCC);					// RESET WATCHDOG!
												for(ii2=0; ii2<50; ii2++)
												{
														DioTgl(pADI_GP3, BIT4);
														exit_while=7; while(!(exit_while<=5));
												}
											}											
									}
								while(!primit_ok );	 primit_ok = 0;			delay2(0x000FFFFF);
		count_fail = 0; exit_while=15;
								do
									{
										printf("%s\n",com16); 	
										exit_while=15; while(!(exit_while<=5));
										count_fail++;
										if(count_fail > 5)
											{
												DioClr(pADI_GP3, BIT2 | BIT3 | BIT4);
//												while(1)																				// mori aici - probleme cu reteaua GSM !!!!!!!!!!!!!!!!!!
												WdtClrInt(0xCCCC);					// RESET WATCHDOG!
												for(ii2=0; ii2<50; ii2++)
												{
														DioTgl(pADI_GP3, BIT4);
														exit_while=7; while(!(exit_while<=5));
												}
											}											
									}
								while(!primit_ok );	 primit_ok = 0;			delay2(0x000FFFFF);

		printf("%s\n",com22); while(!primit_ok); primit_ok = 0;		delay2(0x000FFFFF);// CSQ
			  
//		func_baterie();
//		func_ceas();
//		print_status();
		contor_AT = 0; contor_AT0 = 1;
		clip = 1;									
		check_sms = 1;
		sms2 = 1;
		contor_get_number = 0;
		repede_sleep=0;		
}



void erase_all_memory_Tx (void)
	{
			unsigned int it=0, q;
			unsigned short block_ADDR_val=0;
			char b1, b2, b3;

				for(it=0;it<30;it++) // erase 100 bloks
					{	 
						DioCfg(pADI_GP0,GP0CON_CON0_SPI1MISO | GP0CON_CON1_SPI1SCLK | GP0CON_CON2_SPI1MOSI); 			// trebuie facut inapoi CLOCK 
							b1=(block_ADDR_val>>4);
							b2=(block_ADDR_val<<4);
							b3=0x00;
							
							DioClr(pADI_GP0, BIT3);
							for(q=0;q<mem_delay;q++);

							
							SpiTx(pADI_SPI1, 0x50);		            
										for(q=0;q<mem_delay;q++);
							SpiTx(pADI_SPI1, b1);		            
									for(q=0;q<mem_delay;q++);
							SpiTx(pADI_SPI1, b2);		            
									for(q=0;q<mem_delay;q++);
							SpiTx(pADI_SPI1, b3);		           
									for(q=0;q<mem_delay;q++);
							
							DioSet(pADI_GP0, BIT3);
							for(q=0;q<mem_delay;q++);

								block_ADDR_val+=1;
								delay_0(150000);
	//							wP01_0();
								DioCfg(pADI_GP0,GP0CON_CON0_SPI1MISO | GP0CON_CON1_GPIO | GP0CON_CON2_SPI1MOSI | GP0CON_CON3_GPIOIRQ1);//		CLOCK devine GPIO						
								DioClr(pADI_GP0, BIT1);
			}	
			DioCfg(pADI_GP0,GP0CON_CON0_SPI1MISO | GP0CON_CON1_SPI1SCLK | GP0CON_CON2_SPI1MOSI); 	
			delay_0(1000000);

	}



void read_adas(void)
{
	unsigned int  im, repeat_read, nr_sample=0, hh, tempe;
	

	erase_all_memory_Tx();
	ADAS_config();
			  //delay(2000);
	for(repeat_read=page_start; repeat_read<180+page_start; repeat_read++)
	{			
			data_frame_index = 0;
			while (data_frame_index <14)	
					{			
							for(hh=0;hh<44;hh++)
								{
										data_RTYU[hh]=0x30;
								}

						nr_sample++;
						data_frame_T=18*data_frame_index;					
						
						data_ECG_v[1 + data_frame_T]=(nr_sample&0xFF00)>>8;
						data_ECG_v[2 + data_frame_T]=(nr_sample&0x00FF);						
					
						// citire reg. 0x11
						SpiCfg(pADI_SPI1,SPICON_MOD_TX1RX1,SPICON_MASEN_EN,SPICON_ENABLE_EN|SPICON_CPHA_SAMPLELEADING|SPICON_CPOL_LOW|SPICON_TIM_TXWR);	
						wP35_0; //CS = 0 -> activare
							 //delay(200);
						SpiTx(pADI_SPI1, 0x40);		            // transmit command or any dummy data
								while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit	

							//delay(200);						

						SpiCfg(pADI_SPI1,SPICON_MOD_TX1RX1,SPICON_MASEN_EN,SPICON_ENABLE_EN|SPICON_CPHA_SAMPLELEADING|SPICON_CPOL_LOW|SPICON_TIM_RXRD);//SAMPLETRAILING 	SAMPLELEADING
							
							
							for(im=0;im<44;im++)
								{
									data_RTYU[im]=SpiRx(pADI_SPI1);
									delay(30);
								}
	
											data_ECG_v[0 + data_frame_T]=0x46;		// F
											data_ECG_v[1 + data_frame_T]=0x46;		// F
// lead1							
											data_ECG_v[2 + data_frame_T]=0x20;		// 
											tempe = (data_RTYU[13]&0xF0)>>4;
											if(tempe > 9) data_ECG_v[3 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[3 + data_frame_T]= tempe + 0x30;
											tempe = (data_RTYU[13]&0x0F);
											if(tempe > 9) data_ECG_v[4 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[4 + data_frame_T]= tempe + 0x30;

											tempe = (data_RTYU[14]&0xF0)>>4;
											if(tempe > 9) data_ECG_v[5 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[5 + data_frame_T]= tempe + 0x30;
											tempe = (data_RTYU[14]&0x0F);
											if(tempe > 9) data_ECG_v[6 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[6 + data_frame_T]= tempe + 0x30;											
// lead2
											data_ECG_v[7 + data_frame_T]=0x20;		// 
											tempe = (data_RTYU[17]&0xF0)>>4;
											if(tempe > 9) data_ECG_v[8 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[8 + data_frame_T]= tempe + 0x30;
											tempe = (data_RTYU[17]&0x0F);
											if(tempe > 9) data_ECG_v[9 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[9 + data_frame_T]= tempe + 0x30;

											tempe = (data_RTYU[18]&0xF0)>>4;
											if(tempe > 9) data_ECG_v[10 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[10 + data_frame_T]= tempe + 0x30;
											tempe = (data_RTYU[18]&0x0F);
											if(tempe > 9) data_ECG_v[11 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[11 + data_frame_T]= tempe + 0x30;	
// lead3
											data_ECG_v[12 + data_frame_T]=0x20;		// 
											tempe = (data_RTYU[21]&0xF0)>>4;
											if(tempe > 9) data_ECG_v[13 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[13 + data_frame_T]= tempe + 0x30;
											tempe = (data_RTYU[21]&0x0F);
											if(tempe > 9) data_ECG_v[14 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[14 + data_frame_T]= tempe + 0x30;

											tempe = (data_RTYU[22]&0xF0)>>4;
											if(tempe > 9) data_ECG_v[15 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[15 + data_frame_T]= tempe + 0x30;
											tempe = (data_RTYU[22]&0x0F);
											if(tempe > 9) data_ECG_v[16 + data_frame_T]= tempe + 0x37;
												else data_ECG_v[16 + data_frame_T]= tempe + 0x30;	

											data_ECG_v[17 + data_frame_T]=0x0D;		//


//											data_ECG_v[3 + data_frame_T]=data_RTYU[16-4];//derivatia I - adresa reg.
//											data_ECG_v[4 + data_frame_T]=data_RTYU[18-4];//derivatia I - val. MSB
//											data_ECG_v[5 + data_frame_T]=data_RTYU[19-4];//derivatia I - val. LSB										
//											data_ECG_v[6 + data_frame_T]=data_RTYU[20-4];//derivatia II - adresa reg.
//											data_ECG_v[7 + data_frame_T]=data_RTYU[22-4];//derivatia II - val. MSB
//											data_ECG_v[8 + data_frame_T]=data_RTYU[23-4];//derivatia II - val. LSB
//											data_ECG_v[9 + data_frame_T]=data_RTYU[24-4];//derivatia II - adresa reg.
//											data_ECG_v[10 + data_frame_T]=data_RTYU[26-4];//derivatia II - val. MSB
//											data_ECG_v[11 + data_frame_T]=data_RTYU[27-4];//derivatia II - val. LSB

//											data_ECG_v[16 + data_frame_T]=0x0d;//
//											data_ECG_v[17 + data_frame_T]=0x0a;//
										 
						wP35_1; // CS = 1 ->dezactivare	
						data_frame_index++;	   
						sampeling = 0; while(sampeling<8);		// wait for another sample = 8*0.5=4ms							
				
				
//				for(hh=16;hh<32;hh++)
//					{
//					if(hh<31)
//				printf("%02x", data_RTYU[hh]);
//					else
//					   printf("%02x\n", data_RTYU[hh]);
//				    }
				} 


				for (ii2=0; ii2 < 255; ii2++)
					{
						//if(ii2<254)
						//printf("%02x", data_ECG_v[ii2]);
							//else
							  //printf("%02x\n", data_ECG_v[ii2]);
						scrie_on_flash[ii2] = data_ECG_v[ii2];
					}	
				write_FLASH(repeat_read);		// fleosch in pagina flash :)		
		} // for()  repeat_read

}

//void ADAS_config (void)
//	{
//	 DioCfg(pADI_GP0,GP0CON_CON0_SPI1MISO | GP0CON_CON1_SPI1SCLK | GP0CON_CON2_SPI1MOSI); 
//  	SpiBaud(pADI_SPI1,0x7,SPIDIV_BCRST_EN);
//	//SpiBaud(pADI_SPI1,0x10,SPIDIV_BCRST_EN);
//  SpiCfg(pADI_SPI1,SPICON_MOD_TX1RX1,SPICON_MASEN_EN,SPICON_ENABLE_EN|SPICON_CPHA_SAMPLELEADING|SPICON_CPOL_LOW|SPICON_TIM_TXWR); 

////again:	
//	//wP35_0();
//	
//	SpiCfg(pADI_SPI1,SPICON_MOD_TX1RX1,SPICON_MASEN_EN,SPICON_ENABLE_EN|SPICON_CPHA_SAMPLELEADING|SPICON_CPOL_LOW|SPICON_TIM_TXWR);	
//////////////////////////////////////////////////////////////////////////////////	
//	//scriere in reg. 1 ECGCTL
//		wP35_0; //CS = 0 -> activare
//	
//	SpiTx(pADI_SPI1, 0x81);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);
//	SpiTx(pADI_SPI1, 0xF8);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit	
//		//delay (200);
//	//SpiTx(pADI_SPI1, 0x00);		            // transmit command or any dummy data
//	SpiTx(pADI_SPI1, 0x04);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);
//	SpiTx(pADI_SPI1, 0xAE);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);	
//		
//		wP35_1; //CS = 1 -> dezactivare


//////////////////////////////////////////////////////////////////////////////////		
//		//scriere in reg. 4 
//		wP35_0; //CS = 0 -> activare
//	
//	SpiTx(pADI_SPI1, 0x84);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);
//	SpiTx(pADI_SPI1, 0x00);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit	
//		//delay (200);
//	SpiTx(pADI_SPI1, 0x0F);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);
//	SpiTx(pADI_SPI1, 0x88);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);	
//		
//		wP35_1; //CS = 1 -> dezactivare			

//////////////////////////////////////////////////////////////////////////////////		
//		//scriere in reg. 5 
//		wP35_0; //CS = 0 -> activare
//	
//	SpiTx(pADI_SPI1, 0x85);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);
//	SpiTx(pADI_SPI1, 0xE0);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit	
//		//delay (200);
//	SpiTx(pADI_SPI1, 0x00);		            // transmit command or any dummy data
////	SpiTx(pADI_SPI1, 0x01);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);
//	SpiTx(pADI_SPI1, 0x0A);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);	
//		
//		wP35_1; //CS = 1 -> dezactivare
//		
//////////////////////////////////////////////////////////////////////////////////		
//		//scriere in reg. 0x0A 
//		wP35_0; //CS = 0 -> activare
//	
//	SpiTx(pADI_SPI1, 0x8A);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);
//	SpiTx(pADI_SPI1, 0x07);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit	
//		//delay (200);
//	SpiTx(pADI_SPI1, 0x90);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);
////	SpiTx(pADI_SPI1, 0x10);	//OBS: cu valoarea 0x10 reprezentarea se face in "electrode forman!!!" de modificat pentru "lead format" !!!
//	SpiTx(pADI_SPI1, 0x00);		            // transmit command or any dummy data
//     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
//		//delay (200);		
//		wP35_1; //CS = 1 -> dezactivare						
//}

void ADAS_config (void)
	{
		unsigned int q;

//		DioCfg(pADI_GP0,GP0CON_CON0_SPI1MISO | GP0CON_CON1_SPI1SCLK | GP0CON_CON2_SPI1MOSI); 
//  	SpiBaud(pADI_SPI1,0x5,SPIDIV_BCRST_EN);
	//SpiBaud(pADI_SPI1,0x10,SPIDIV_BCRST_EN);
//  SpiCfg(pADI_SPI1,SPICON_MOD_TX1RX1,SPICON_MASEN_EN,SPICON_ENABLE_EN|SPICON_CPHA_SAMPLELEADING|SPICON_CPOL_HIGH|SPICON_TIM_TXWR); 
//	for(q=0;q<500;q++);
//again:	
	//wP35_0();
	
////////////////////////////////////////////////////////////////////////////////	
	//scriere in reg. 1 ECGCTL
		wP35_0; //CS = 0 -> activare
		for(q=0;q<50;q++);
	
	SpiTx(pADI_SPI1, 0x81);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0xF8);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit	
		for(q=0;q<50;q++);
	//SpiTx(pADI_SPI1, 0x00);		            // transmit command or any dummy data
	SpiTx(pADI_SPI1, 0x04);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0xAE);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);	
		
		wP35_1; //CS = 1 -> dezactivare
		for(q=0;q<50;q++);

////////////////////////////////////////////////////////////////////////////////		
		//scriere in reg. 4 PACECTL
		wP35_0; //CS = 0 -> activare
	for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0x84);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0x00);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit	
		for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0x0F);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0x88);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);	
		
		wP35_1; //CS = 1 -> dezactivare			
for(q=0;q<50;q++);
////////////////////////////////////////////////////////////////////////////////		
		//scriere in reg. 5 
		wP35_0; //CS = 0 -> activare
	for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0x85);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0xE0);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit	
		for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0x00);		            // transmit command or any dummy data
//	SpiTx(pADI_SPI1, 0x01);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0x0A);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);	
		
		wP35_1; //CS = 1 -> dezactivare
	for(q=0;q<50;q++);	
////////////////////////////////////////////////////////////////////////////////		
		//scriere in reg. 0x0A 
		wP35_0; //CS = 0 -> activare
	for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0x8A);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0x07);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit	
		for(q=0;q<50;q++);
	SpiTx(pADI_SPI1, 0x90);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);
//	SpiTx(pADI_SPI1, 0x10);	//OBS: cu valoarea 0x10 reprezentarea se face in "electrode forman!!!" de modificat pentru "lead format" !!!
	SpiTx(pADI_SPI1, 0x00);		            // transmit command or any dummy data
     	while ((SpiSta(pADI_SPI1) & SPISTA_TX) != SPISTA_TX) ; // wait for data received status bit
		for(q=0;q<50;q++);		
		wP35_1; //CS = 1 -> dezactivare
	for(q=0;q<50;q++);		
}

////////////////////////////////Receptor RF.c//////////////////////////////////
// AUTOR : Marcus Vinícius Marques Costa                                     //
// FUNÇÃO: Decodificar mensagem gerada pelo controle LedDrive                //
// DATA  : 21/mai/2015                                                       //
//         Microcontolador utilizado PIC16F1788                              //
///////////////////////////////////////////////////////////////////////////////

#define  rx_flag   INTCON.INTF   // Flag de interrupção indicando chegada de sinal
#define  est_tmr0  INTCON.TMR0IF // Flag de estouro do TMR0
//#define  lig_tmr2  T2CON.TMR2ON  // Habilita ou desabilita TMR2
//#define  est_tmr2  PIR1.TMR2IF   // Flag de estouro do TMR2
#define  sinal     PortB.f0      // Porta onde o receptor está conectado
#define  lo(param)      ((short *)&param)[0]  //Função p/ ler o 1º byte de um registro
#define  hi(param)      ((short *)&param)[1]  //Função p/ ler o 2º byte de um registro
#define  unib1(param,param2)  ((short *)&param)[1]=((short *)&param2)[0]
#define  unib2(param,param2)  ((short *)&param)[2]=((short *)&param2)[0]

int const     ajuste = 750;    // Valor para ajuste do delay de leitura
unsigned int  endereco, autorizado1, autorizado2;  // Variável que armazena o endereço recebido
unsigned short data_tmp;
unsigned char tecla = 0;       // Variável que armazena o dado recebido
unsigned char botao, porra, prog;     // Indica se o botão prog foi apertado

void Inicia_receptor(){
// Inicia botão PROG
   OPTION_REG.f7 = 0;          //habilita a ativação dos pullups
         WPUC.f0 = 1;          //ativa pullup na porta do botão prog
        trisC.f0 = 1;          //botão PROG como entrada
//configurações do Timer 0:
   OPTION_REG.f5 = 0;          //clock interno (Fosc/4)
   OPTION_REG.f3 = 0;          //ativa o prescaler no Timer0
   OPTION_REG.f2 = 1;          //prescaler 1:256
   OPTION_REG.f1 = 1;          //prescaler 1:256
   OPTION_REG.f0 = 1;          //prescaler 1:256
//configurações do Timer 2:
//   T2CON = 0b01111011;
//     PR2 = 0b11111111;
//configurações da Interrupção INT0 e portB:       
   TRISB.f0        = 1;        // Define portB.f0 como entrada
   ANSELB.f0       = 0;        // Configura portB0 como I/O Digital
   WPUB.f0         = 0;        // Desliga resistor interno de pull-up no portB0
   OPTION_REG.f6   = 1;        // INT0 em borda de subida (OPTION_REG.INTEDG: 1 = Interrupt on rising edge)
   INTCON.INTE     = 1;        // Habilita INT0
// INTCON.GIE      = 1;        // Habilita Global Interruptions
//lê endereço dos controles cadastrados na memória
   data_tmp = Eeprom_Read(0x00); // Lê o 1° byte do 1º endereço permitido
   while(EECON1.RD);             // Aguarda final da leitura
   autorizado1 = data_tmp;
   data_tmp = Eeprom_Read(0x01); // Lê o 2° byte do 1º endereço permitido
   while(EECON1.RD);             // Aguarda final da leitura
   unib1(autorizado1,data_tmp);
   data_tmp = Eeprom_Read(0x02); // Lê o 1° byte do 2º endereço permitido
   while(EECON1.RD);             // Aguarda final da leitura
   autorizado2 = data_tmp;
   data_tmp = Eeprom_Read(0x03); // Lê o 2° byte do 2º endereço permitido
   while(EECON1.RD);             // Aguarda final da leitura
   unib1(autorizado2,data_tmp);
   prog = 0;
   porra=1;
}

unsigned char decodifica_sinal(){
   unsigned char conta;
   endereco=0;                   // reseta a variável que indica o endereço do controle
   for(conta=1; conta<=19; conta++){
      delay_us(ajuste);
	  if(sinal){endereco++ ; while(sinal);}
	  endereco<<=1;
	  tmr0=0;
	  est_tmr0=0;
	  while(!sinal && !est_tmr0);
	  if(est_tmr0) return(0);
//	  est_tmr2=0;
//	  tmr2=0;
//	  lig_tmr2=1;
//	  while(!sinal && !est_tmr2);
//	  if(est_tmr2) return(0);
   }
   tecla=0;                      // zera a variável "tecla" que indica o botão acionado
   for(conta=1; conta<=6; conta++){
      delay_us(ajuste);
	  if(sinal){tecla++ ; while(sinal);}
	  tecla<<=1;
	  tmr0=0;
	  est_tmr0=0;
	  while(!sinal && !est_tmr0 && conta<6);
	  if(est_tmr0)return(0);
//	  est_tmr2=0;
//	  tmr2=0;
//	  lig_tmr2=1;
//	  while(!sinal && !est_tmr2 && conta<6);
//	  if(est_tmr2)return(0);
   }
   endereco>>=1;
   tecla>>=2;
//   return(tecla); // =======>>>>>>>>> para desativar filtro de endereço
   if(prog){
      autorizado2=autorizado1;
	  autorizado1=endereco;
      data_tmp = lo(autorizado1);  // Copia byte0 do addr
      Eeprom_Write(0x00,data_tmp); // Grava na memória EEPROM
      data_tmp = hi(autorizado1);  // Copia byte1 do addr
      Eeprom_Write(0x01,data_tmp); // Grava na memória EEPROM
      data_tmp = lo(autorizado2);  // Copia byte0 do addr
      Eeprom_Write(0x02,data_tmp); // Grava na memória EEPROM
      data_tmp = hi(autorizado2);  // Copia byte1 do addr
      Eeprom_Write(0x03,data_tmp); // Grava na memória EEPROM
	  prog=0;                      // Zera o indicador de programação
      return(0);                   // Encerra leitura da mensagem sem novo dado lido
   }else{
      if(endereco==autorizado1 || endereco==autorizado2) return(tecla);
	  else return(0);
   }
}

void verifica_botao(){
      if(Button(&PORTC,0,1,1)){
         botao = 1;
      }
      if (botao && Button(&PORTC,0,1,0)) {  //ações quando o botão for acionado
         botao=0;
		 prog=3;
      }
}

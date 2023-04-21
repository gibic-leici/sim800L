
#include "main.h"
#include <string.h>
#include <stdio.h>


/* ------------------------------------------------------------------------------*/
/* ------------------------ Funciones de Inicializacion ------------------------ */
/* ------------------------------------------------------------------------------*/

void InitSIM(SIM800* sim, UART_HandleTypeDef * huart,int print)
{

	sim-> huart = huart;
	memset(sim->response, 0, LENGTH);
	memset(sim->txt_last_sms, 0, LEN_TXT);

	InitBufferLargo(sim);

	EnviarComandoAT(sim,"AT+CMGF=1\r\n",print); // Activa modo texto
	EnviarComandoAT(sim,"AT+CNMI=2,1,0,0,0\r\n",print); // Configura recepcion de SMS
												  // Esto es importante : Primer num: el 2 es para que guarde los msj en un buffer si la conexion Uart esta ocupada
												 // Segundo num: el 1 es para que guarde internamente los msj, despues para leerlos hay que usar CMGR. Si esto lo pones en 2 forwardea el msj directamente y no guarda nada

	EnviarComandoAT(sim, "AT+CFUN=1\r\n",print); // Esto me parece que es innecesario... (activa todas las funcionalidades pero creo que esta en 1 por defecto)
}

void InitGPRS(SIM800 *sim,int print)
{
	if(isConnectedGPRS(sim,print) == 0)
	{
		//EnviarComandoAT(sim,"AT+CSTT=\"igprs.claro.com.ar\",\"ctigprs\",\"ctigprs999\"\r\n",print);
		EnviarComandoAT(sim,"AT+CSTT=\"igprs.claro.com.ar\",\"\",\"\"\r\n",print);
			// Esto setea el APN contraseña y usuario de Claro. Sin esto se conecta igual a red celular pero no anda el GPRS

		EnviarComandoAT(sim,"AT+CIICR\r\n",print);	// Inicializa el GPRS


		EnviarComandoAT(sim,"AT+CIFSR\r\n",print);	// Debería imprimir el IP
	}
}

void InitBufferLargo(SIM800 *sim)
{
	strcpy(sim->buffer_largo,"En Argentina naci. Tierra del Diego y Lionel. De los pibes de Malvinas. Que jamas olvidare. No te lo puedo explicar. Porque no vas a entender. Las finales que perdimos. Cuantos anos la llore. Pero eso se termino. Porque en el Maracana. La final con los brazucas. La volvio a ganar papa. Muchachos. Ahora nos volvimos a ilusionar. Quiero ganar la tercera. Quiero ser campeon mundial. Y al Diego. Desde el cielo lo podemos ver. Con Don Diego y La Tota. Alentandolo a Lionel. ");
}

/* ------------------------------------------------------------------------------*/
/* ---------------------------- Funciones generales ---------------------------- */
/* ------------------------------------------------------------------------------*/


void EnviarComandoAT(SIM800* sim, char* comando, int print)
{
	Enviar(sim,comando);
	WaitForAnswer(sim,print);
	HAL_Delay(100);
}

void Enviar(SIM800 *sim, char * cadena)
{
	HAL_UART_Transmit (sim->huart, (uint8_t*) cadena, strlen(cadena),TIMEOUT);
}

void Imprimir_respuesta(SIM800 *sim)
{
	printf("Respuesta: %s\r\n",sim->response);
}

void Borrar_respuesta(SIM800 *sim)
{
	memset(sim->response, 0, LENGTH);
}

int WaitForAnswer(SIM800* sim,int print)
{
	int counter=0;
	Borrar_respuesta(sim);
	while((strlen(sim->response)==0) && (counter < WAIT_MAX / TIMEOUT) )
	{
		counter++;
		HAL_UART_Receive (sim->huart, (uint8_t*) sim->response, LENGTH,TIMEOUT);
	}
	if(print == 1)
	{
		if(counter ==  WAIT_MAX / TIMEOUT)
		{
			printf("Tiempo de espera agotado\r\n");
		}
		else
		{
			Imprimir_respuesta(sim);
		}

	}
	return (counter ==  WAIT_MAX / TIMEOUT)? 0:1;
}

void SerialDebug( UART_HandleTypeDef * huart1 , UART_HandleTypeDef * huart2 , char caracter_finalizacion )
{
	uint8_t rx_byte = 0;

	while(rx_byte != caracter_finalizacion)
	{
		while (HAL_UART_Receive(huart2, &rx_byte, 1, 0) == HAL_OK)
		{
		    HAL_UART_Transmit(huart1, &rx_byte, 1, 0xFFFF); // Forward received byte to UART1
		}
		while (HAL_UART_Receive(huart1, &rx_byte, 1, 0) == HAL_OK)
		{
		    HAL_UART_Transmit(huart2, &rx_byte, 1, 0xFFFF); // Forward received byte to UART2
		}


	}
}

/* ------------------------------------------------------------------------------*/
/* ---------------------------- Funciones de status ---------------------------- */
/* ------------------------------------------------------------------------------*/

void EnviarAT(SIM800* sim)
{
	EnviarComandoAT(sim,"AT\r\n",1);
}

void ConsultarSignal(SIM800* sim)
{
	EnviarComandoAT(sim,"AT+CSQ\r\n",1);
}

void ConsultarEstadoSIM(SIM800* sim)
{
	EnviarComandoAT(sim,"AT+CPIN?\r\n",1);
}

void ListarRedesDisponibles(SIM800* sim)
{
	EnviarComandoAT(sim,"AT+COPS=?\r\n",1);
	WaitForAnswer(sim,1);
}

int isConnected(SIM800 *sim,int print)
{
	// Si esta conectado creg me devuelve 0,1. Espero a la coma y me fijo el siguiente caracter
	EnviarComandoAT(sim,"AT+CREG?\r\n",print);

	int index = 0;
	while( (*(sim->response +index) != ',') && (*(sim->response+index) != '\0') )
	{
		index++;
	}

	return (*(sim->response+index+1) == '1' )?1:0;
}

int isConnectedGPRS(SIM800 *sim,int print)
{
	// Si esta conectado cgatt me devuelve +CGATT: 1. Espero al espacio y me fijo el siguiente caracter
	EnviarComandoAT(sim,"AT+CGATT?\r\n",print);

	int index = 0;
	while( (*(sim->response +index) != 32) && (*(sim->response+index) != '\0') )
	{
		index++;
	}

	return (*(sim->response+index+1) == '1' )?1:0;

}


/* ------------------------------------------------------------------------------*/
/* ------------------------------ Funciones de gprs ----------------------------- */
/* ------------------------------------------------------------------------------*/

void TestGPRS(SIM800* sim,int print)
{
	InitGPRS(sim, 0);

	char cmd [LEN_TXT] = "AT+CIPSTART=\"TCP\",\"httpbin.org\",80\r\n";
	EnviarComandoAT(sim,cmd,print);
	WaitForAnswer(sim, 1); // Aca espero dos veces porque primero va el OK y despues el connect OK

	// HTTP Request:
	char cmd2 [LEN_TXT] = "GET /get HTTP/1.1\r\nHost: httpbin.org\r\nAccept: */* \r\n\r\n";
	char cmd3 [LEN_TXT];
	sprintf(cmd3,"AT+CIPSEND=%d\r\n",strlen(cmd2));
	EnviarComandoAT(sim,cmd3,print);

	EnviarComandoAT(sim,cmd2,print);

	// Esperamos la respuesta y despues cerramos
	WaitForAnswer(sim,1);
	WaitForAnswer(sim,1);

	EnviarComandoAT(sim,"AT+CIPCLOSE\r\n",print);

}

// IMPORTANTE -> Esto anda hasta unos 500 bytes x msj!
void SendTCPtoIP(SIM800* sim, char * msj, char* IP, int port,int print)
{
	InitGPRS(sim, print);

	char cmd [LEN_TXT];
	sprintf(cmd,"AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",IP,port);
	EnviarComandoAT(sim,cmd,print);
	WaitForAnswer(sim, print); // Aca espero dos veces porque primero va el OK y despues el connect OK

	char cmd2 [LEN_TXT] = "AT+CIPSEND\r\n";
	EnviarComandoAT(sim,cmd2,print);

	Enviar(sim,msj);
	Enviar(sim,"\r\n\032");
	WaitForAnswer(sim, print);

	EnviarComandoAT(sim,"AT+CIPCLOSE\r\n",print);

}


/* ------------------------------------------------------------------------------*/
/* ------------------------------ Funciones de sms ----------------------------- */
/* ------------------------------------------------------------------------------*/

void EnviarSMS(SIM800 * sim, char * numero, char* mensaje, int print )
{
	char cmd [LEN_CMD];

	sprintf(cmd,"AT+CMGS=\"%s\"\r\n",numero);

	EnviarComandoAT(sim,cmd,print);

	Enviar(sim,mensaje);
	Enviar(sim,"\r\n\032");

}

void ListarSMS( SIM800 *sim )
{

	EnviarComandoAT(sim,"AT+CMGL=\"ALL\"\r\n",1);
}

void BorrarAllSMS(SIM800 *sim, int print)
{
	EnviarComandoAT(sim,"AT+CMGDA=\"DEL ALL\"\r\n",print);
}

void LeerSMS_i(SIM800 *sim, int i, int print)
{
	char msj [LEN_CMD];
	sprintf(msj,"AT+CMGR=%d\r\n",i);
	EnviarComandoAT(sim,msj,print);
}

// Esto se rompio arreglalo antes de irte para ser feliz
int ListenSMS(SIM800 * sim, UART_HandleTypeDef * huart1 )
{
	Borrar_respuesta(sim);
	uint8_t rx_byte = 0;

	while ( ( strlen(sim->response) == 0 ) )
	{
		HAL_UART_Receive (sim->huart, (uint8_t*) sim->response, LENGTH,TIMEOUT);

		if (__HAL_UART_GET_FLAG(huart1, UART_FLAG_RXNE) == SET)
		{
		    HAL_UART_Receive (huart1, &rx_byte, 1,TIMEOUT); // Leer el dato recibido en UART1
		    if(rx_byte==26)
		    {
		    	return 0;
		    }
		}
	}

	int indice = ExtraerIndiceCMTI(sim->response);
	ExtraerTextoDeSMS(sim,indice,sim->txt_last_sms);
	return 1;

}


int ExtraerIndiceCMTI(char * codigo)
{
	// Forma del codigo -> +CMTI: "SM",5

	int i = 0;
	while(*(codigo+i) !=',' ){i++;}

	return *(codigo+i+1)-48;	// Para pasarlo a entero
}

void ExtraerTextoDeSMS(SIM800 *sim, int indice, char * texto)
{
	LeerSMS_i(sim,indice,0);
	// Extrae el texto del SMS guardado en la posicion i
	// Formato del codigo -> 3,"READ/UNREAD","NUM","","FECHA" MSJ
	// Voy a contar comillas, serían 8

	int comillas = 0;
	int tam_header = 0;
	while(comillas < 8)
	{
		if(*(sim->response + tam_header) == '"') { comillas++; }
		tam_header++;
	}

	int j;

	for (j = 0; j<strlen(sim->response)-tam_header;j++)
	{
		*(texto+j) = *(sim->response + tam_header + j);
	}
	*(texto+j+1)='\0';
}









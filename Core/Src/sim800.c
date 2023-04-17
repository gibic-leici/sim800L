
#include "main.h"
#include <string.h>
#include <stdio.h>


void InitSIM(SIM800* sim, UART_HandleTypeDef * huart){
	sim-> huart = huart;
	EnviarComandoAT(sim,"AT+CMGF=1\r\n"); // Activa modo texto
	EnviarComandoAT(sim,"AT+CNMI=2,1,0,0,0\r\n"); // Configura recepcion de SMS
												  // Esto es importante : Primer num: el 2 es para que guarde los msj en un buffer si la conexion Uart esta ocupada
												  // Segundo num: el 1 es para que guarde internamente los msj, despues para leerlos hay que usar CMGR. Si esto lo pones en 2 forwardea el msj directamente y no guarda nada
	memset(sim->response, 0, LENGTH);
	memset(sim->txt_last_sms, 0, LEN_TXT);
}

void EnviarComandoAT(SIM800* sim, char* comando)
{
	HAL_UART_Transmit (sim->huart, (uint8_t*) comando, strlen(comando),TIMEOUT);
	HAL_UART_Receive (sim->huart, (uint8_t*) sim->response, LENGTH,TIMEOUT);
}


void EnviarAT(SIM800* sim)
{
	EnviarComandoAT(sim,"AT\r\n");
	Imprimir_respuesta(sim,1);
}

void EnviarPuertoSerie(SIM800 *sim)
{
	char msj [50];
	scanf("%s",msj);
	strcat(msj,"\r\n");
	printf("Enviando: %s",msj);
	EnviarComandoAT(sim,msj);
	Imprimir_respuesta(sim,1);
}


void ConsultarSignal(SIM800* sim)
{
	EnviarComandoAT(sim,"AT+CSQ\r\n");
	Imprimir_respuesta(sim,1);
}

void ConsultarEstadoSIM(SIM800* sim)
{
	EnviarComandoAT(sim,"AT+CPIN?\r\n");
	Imprimir_respuesta(sim,1);
}

void ListarRedesDisponibles(SIM800* sim)
{
	char comand [50] = "AT+COPS=?\r\n";
	// No uso la funcion para cambiarle el timeout a ver que onda (es un comando que tarda un toque mas)
	HAL_UART_Transmit (sim->huart, (uint8_t*) comand, strlen(comand),TIMEOUT);
	HAL_UART_Receive (sim->huart, (uint8_t*) sim->response, LENGTH,10000);

	Imprimir_respuesta(sim,1);
}

void EnviarSMS(SIM800 * sim, char * numero, char* mensaje )
{
	EnviarComandoAT(sim,"AT+CMGS=\""); // Manda el msj
	EnviarComandoAT(sim,numero);
	EnviarComandoAT(sim,"\"\r\n");

	EnviarComandoAT(sim,mensaje);
	EnviarComandoAT(sim,"\r\n\032");	// CTRL+Z para terminar
}

void ListarSMS( SIM800 *sim )
{

	EnviarComandoAT(sim,"AT+CMGL=\"ALL\"\r\n");
	Imprimir_respuesta(sim,1);
}

void BorrarAllSMS(SIM800 *sim)
{
	EnviarComandoAT(sim,"AT+CMGDA=\"DEL ALL\"\r\n");
	Imprimir_respuesta(sim,1);
}

void LeerSMS_i(SIM800 *sim, int i, int imprimir)
{
	char msj [50];
	sprintf(msj,"AT+CMGR=%d\r\n",i);
	EnviarComandoAT(sim,msj);
	if(imprimir==1)
	{
		Imprimir_respuesta(sim,1);
	}

}

int ListenSMS(SIM800 * sim)
{
	HAL_UART_Receive (sim->huart, (uint8_t*) sim->response, LENGTH,TIMEOUT);
	if( strlen (sim-> response) != 0 )
	{
		int indice = ExtraerIndiceCMTI(sim->response);
		ExtraerTextoDeSMS(sim,indice,sim->txt_last_sms);
		Borrar_respuesta(sim);
		return 1;
	}
	return 0;
}

void Imprimir_respuesta(SIM800 *sim, int borrar)
{
	printf("Respuesta: %s\r\n",sim->response);

	if(borrar == 1)
	{
		memset(sim->response, 0, LENGTH);
	}
}

void Borrar_respuesta(SIM800 *sim)
{
	memset(sim->response, 0, LENGTH);
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

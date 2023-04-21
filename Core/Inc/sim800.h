/*
 * sim800.h
 *
 *  Created on: 11 abr. 2023
 *      Author: mati9
 */

#ifndef INC_SIM800_H_
#define INC_SIM800_H_

#define LENGTH 2000
#define LEN_TXT 100
#define LEN_CMD 50
#define TIMEOUT 500
#define WAIT_MAX 10000
#define LEN_BUF_LARGO 450

typedef struct{
	UART_HandleTypeDef * huart;
	char response [LENGTH];
	char txt_last_sms [LEN_TXT];
	char buffer_largo [LEN_BUF_LARGO];
} SIM800;


// Inicializacion
void InitSIM(SIM800* sim, UART_HandleTypeDef * huart,int print);
void EnviarAT(SIM800* sim);


// Generales
void EnviarComandoAT(SIM800* sim, char* comando, int print);
void Enviar(SIM800 *sim, char * cadena);
void Imprimir_respuesta(SIM800 *sim); // Imprime y borra
void Borrar_respuesta(SIM800 *sim);
int WaitForAnswer(SIM800* sim,int print);
void Listen(SIM800* sim);
int isConnected(SIM800 *sim,int print);
void SerialDebug( UART_HandleTypeDef * huart1 , UART_HandleTypeDef * huart2 , char caracter_finalizacion );


// Funciones de status
void ListarRedesDisponibles(SIM800* sim);
void ConsultarEstadoSIM(SIM800* sim);
void ConsultarSignal(SIM800* sim);

// Funciones de SMS
void EnviarSMS(SIM800 * sim, char * numero, char* mensaje,int print  );
void ListarSMS( SIM800 *sim );
void BorrarAllSMS(SIM800 *sim,int print);
void LeerSMS_i(SIM800 *sim, int i,int imprimir);
int ListenSMS(SIM800 * sim);
int ExtraerIndiceCMTI(char * codigo);
void ExtraerTextoDeSMS(SIM800 *sim, int indice, char * texto);

// Funciones de GPRS
void InitGPRS(SIM800 *sim,int print);
void TestGPRS(SIM800* sim,int print);
void SendTCPtoIP(SIM800* sim, char * msj, char* IP, int port,int print);




#endif /* INC_SIM800_H_ */

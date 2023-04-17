/*
 * sim800.h
 *
 *  Created on: 11 abr. 2023
 *      Author: mati9
 */

#ifndef INC_SIM800_H_
#define INC_SIM800_H_

#define LENGTH 1000
#define LEN_TXT 50
#define LEN_CMD 50
#define TIMEOUT 1000

typedef struct{
	UART_HandleTypeDef * huart;
	char response [LENGTH];
	char txt_last_sms [LEN_TXT];
} SIM800;


// Inicializacion
void InitSIM(SIM800* sim, UART_HandleTypeDef * huart);
void EnviarAT(SIM800* sim);

// Generales
void EnviarComandoAT(SIM800* sim, char* comando);
void Imprimir_respuesta(SIM800 *sim,int borrar); // Imprime y borra
void Borrar_respuesta(SIM800 *sim);
void EnviarPuertoSerie(SIM800 *sim);

// Funciones de status
void ListarRedesDisponibles(SIM800* sim);
void ConsultarEstadoSIM(SIM800* sim);
void ConsultarSignal(SIM800* sim);

// Funciones de SMS
void EnviarSMS(SIM800 * sim, char * numero, char* mensaje );
void ListarSMS( SIM800 *sim );
void BorrarAllSMS(SIM800 *sim);
void LeerSMS_i(SIM800 *sim, int i,int imprimir);
int ListenSMS(SIM800 * sim);
int ExtraerIndiceCMTI(char * codigo);
void ExtraerTextoDeSMS(SIM800 *sim, int indice, char * texto);



#endif /* INC_SIM800_H_ */

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
import serial
from datetime import datetime
from random import randint, uniform, random

"""
ANTES DE EJECUTAR ESTE SCRIPT
   Instalar pyserial. En la consola de anaconda (Anaconda prompt) ejectutar:
      conda install pyserial
  
EJECUTAR ESTE SCRIPT
   En la consola de anaconda (Anaconda prompt) ejectutar:
      cd path/to/script
      python data_gen_serial.py

-------------------------------------------------------------------------------

INFO de pyserial: http://pyserial.readthedocs.io/en/latest/shortintro.html

possible timeout values:
    1. None: wait forever, block call
    2. 0: non-blocking mode, return immediately
    3. x, x is bigger than 0, float allowed, timeout block call

 configure the serial connections (the parameters differs on the device you are
 connecting to)
 
ser = serial.Serial(
   port = "/dev/ttyUSB0",
   baudrate = 57600,
   parity = serial.PARITY_ODD,
   stopbits = serial.STOPBITS_TWO,
   bytesize = serial.SEVENBITS,
   timeout = 1
)
"""

# Variables globales con valores iniciales ------------------------------------

ahora = datetime.now()
ID0	= '0'                    # ID del dispositivo (String)
ID1	= '1'                    # ID del dispositivo (String)


ID = ID0                               # ID del dispositivo (String)

DATE_GPS	= ahora.strftime("%d-%m-%Y")  # Fecha del GPS (Fecha)
TIME_GPS	= ahora.strftime("%H:%M:%S")  # Hora del GPS	(HH:MM:SS)
LAT_GPS	= -34.620295                  # Latitud del GPS	(String)
LONG_GPS	= -58.371484                  # Longitud del GPS (String)

HS_S	   = 1.2                         # Horas marcha (Float)
HS_P	   = 3.4                         # Horas de parado (Float)
HS_E	   = 0.1                         # Horas de falla (Float)

STATE	   = 0                           # Estado del equipo: 0 = Run, 1 = Stop, 2 = Failure. (String)

PS_MAX	= 12.0                        # Presion de succión máxima (Float)
PS_PROM	= 11.3                        # Presion de succión promedio (Float)
PS_MIN	= 9.4                         # Presion de succión mínima (Float)

PD_MAX	= 16.3                        # Presión de descarga máxima (Float)
PD_PROM	= 16.0                        # Presión de descarga promedio (Float)
PD_MIN	= 9.5                         # Presión de descarga mínima (Float)

DP_MAX	= round(PD_MAX - PS_MAX, 2)   # Diferenca de presion máxima (Float)
DP_PROM	= round(PD_PROM - PS_PROM, 2) # Diferenca de presion promedio (Float)
DP_MIN	= round(PD_MIN - PS_MIN, 2)   # Diferenca de presion mínima (Float)


# Funciones -------------------------------------------------------------------

# Imprime los valores iniciales
def cmd_printInitialValues():

   print( "\nValores iniciales -----------------------------------------------\n" )

   print( 'ID de dispositivo (String):', ID )
   print( '' )

   """
   print( 'Fecha del GPS (Fecha):', DATE_GPS, )
   print( 'Hora del GPS (HH:MM:SS):', TIME_GPS )
   print( 'Latitud del GPS (String):', LAT_GPS	)
   print( 'Longitud del GPS (String):', LONG_GPS )
   print( '' )
   """

   print( 'Horas marcha (Float):', HS_S )
   print( 'Horas parado (Float):', HS_P	) 
   print( 'Horas falla (Float):', HS_E )  
   print( '' )

   print( 'Estado del equipo: 0 = Run, 1 = Stop, 2 = Failure. (String):', STATE )
   print( '' )

   print( 'Presion de succión máxima (Float):', PS_MAX, 'BARg' )
   print( 'Presion de succión promedio (Float):', PS_PROM, 'BARg' )	
   print( 'Presion de succión mínima (Float):', PS_MIN, 'BARg' )
   print( '' )

   print( 'Presión de descarga máxima (Float):', PD_MAX, 'BARg' )
   print( 'Presión de descarga promedio (Float):', PD_PROM, 'BARg' )
   print( 'Presión de descarga mínima (Float):', PD_MIN, 'BARg' )
   print( '' )

   print( 'Diferenca de presion máxima (Float):', DP_MAX, 'BAR' )
   print( 'Diferenca de presion promedio (Float):', DP_PROM, 'BAR' )
   print( 'Diferenca de presion mínima (Float):', DP_MIN, 'BAR' )
   print( '' )

   return

# Imprime el formato de salida
def cmd_printDataOutputFormat():
   print( "Formato de salida -------------------------------------------------\n" )

   #formatoSalida = 'ID|DATE_GPS|TIME_GPS|LAT_GPS|LONG_GPS|HS_S|HS_P|HS_E|STATE|PS_MAX|PS_PROM|PS_MIN|PD_MAX|PD_PROM|PD_MIN|DP_MAX|DP_PROM|DP_MIN'
   formatoSalida = 'ID|HS_S|HS_P|HS_E|STATE|PS_MAX|PS_PROM|PS_MIN|PD_MAX|PD_PROM|PD_MIN|DP_MAX|DP_PROM|DP_MIN'

   print( 'Formato que sale por UART:' )
   print( formatoSalida )
   print( '' )

   formatoSalida = genOutputValues()

   print( 'Ejemplo de salida por UART:' )
   print( formatoSalida )
   print( '' )

   return


# Genera valores aleatorios sobre los valores iniciales
def genDeviceRandomValues():  

   ahora = datetime.now()

   global DATE_GPS # global adelante para poder modificar una variable global
   global TIME_GPS
   global LAT_GPS	
   global LONG_GPS

   global HS_S	   
   global HS_P	   
   global HS_E	   

   global STATE	

   global PS_MAX	
   global PS_PROM	
   global PS_MIN	

   global PD_MAX	
   global PD_PROM	
   global PD_MIN	

   global DP_MAX	
   global DP_PROM	
   global DP_MIN	

   DATE_GPS	= ahora.strftime("%d-%m-%Y")  # Fecha del GPS (Fecha)
   TIME_GPS	= ahora.strftime("%H:%M:%S")  # Hora del GPS	(HH:MM:SS)
   LAT_GPS	= round(-34.620295 + uniform(0,0.000005), 6) # Latitud del GPS (String)
   LONG_GPS	= round(-58.371484 + uniform(0,0.000005), 6) # Longitud del GPS (String)
   
   HS_S = round(1.2 + uniform(0,1), 2) # Horas marcha (Float)
   HS_P = round(3.4 + uniform(0,1), 2) # Horas de parado (Float)
   HS_E = round(0.1 + uniform(0,1), 2) # Horas de falla (Float)
 
   randStateNumber = randint(0,2)
   if randStateNumber == 0:
      randomState	= 'Marcha'
   elif randStateNumber == 1:
      randomState	= 'Parado'      
   else:
      randomState	= 'Falla'  

   #STATE = randomState # Estado del equipo: Marcha, Parado, Falla, etc. (String)
   STATE = randStateNumber # Estado del equipo: Marcha, Parado, Falla, etc. (String)
   
   PS_MAX  = round(12.0 + uniform(0,1), 2) # Presion de succión máxima (Float)
   PS_PROM = round(11.3 + uniform(0,1), 2) # Presion de succión promedio (Float)
   PS_MIN  = round(9.4  + uniform(0,1), 2) # Presion de succión mínima (Float)
   
   PD_MAX  = round(16.3 + uniform(0,1), 2) # Presión de descarga máxima (Float)
   PD_PROM = round(16.0 + uniform(0,1), 2) # Presión de descarga promedio (Float)
   PD_MIN  = round(9.5  + uniform(0,1), 2) # Presión de descarga mínima (Float)
   
   DP_MAX  = round(PD_MAX - PS_MAX, 2)   # Diferenca de presion máxima (Float)
   DP_PROM = round(PD_PROM - PS_PROM, 2) # Diferenca de presion promedio (Float)
   DP_MIN  = round(PD_MIN - PS_MIN, 2)   # Diferenca de presion mínima (Float)

   return


# Genera el String de salida a enviar por UART
"""
def genOutputValues():
   return ID + '|' + DATE_GPS + '|' + TIME_GPS + '|' + str(LAT_GPS) + '|' + str(LONG_GPS) + '|' + str(HS_S) + '|' + str(HS_P) + '|' + str(HS_E) + '|' + str(STATE) + '|' + str(PS_MAX) + '|' + str(PS_PROM) + '|' + str(PS_MIN) + '|' + str(PD_MAX) + '|' + str(PD_PROM) + '|' + str(PD_MIN) + '|' + str(DP_MAX) + '|' + str(DP_PROM) + '|' + str(DP_MIN) + '\r\n'
"""
def genOutputValues():
   return ID + '|' + str(HS_S) + '|' + str(HS_P) + '|' + str(HS_E) + '|' + str(STATE) + '|' + str(PS_MAX) + '|' + str(PS_PROM) + '|' + str(PS_MIN) + '|' + str(PD_MAX) + '|' + str(PD_PROM) + '|' + str(PD_MIN) + '|' + str(DP_MAX) + '|' + str(DP_PROM) + '|' + str(DP_MIN) + '\r\n'


# Enviar dato aleatorio con cirto ID
def sendDataFromID( devID ):
   #print( "Dato enviado --------------------------------------------\n" )
   global ID
   ID = devID
   genDeviceRandomValues()
   command = genOutputValues()
   ser.write(command.encode("ascii"))
   print( 'Dato enviado:' )
   print( command )
   print( '' )
   return


# comando help: Imprime la lista de comandos
def cmd_h():
   print( "Comandos disponibles -----------------------------------------------" )
   print( "   'h' (help) imprime esta lista de comandos." )
   print( "   'q' (quit) Salir del programa." )
   print( "   'v' Imprimir valores iniciales individuales." )
   print( "   'b' Imprimir valores iniciales en formato de salida." )
   print( "   '0' Enviar un dato aleatorio del equipo con ID0." )
   print( "   '1' Enviar un dato aleatorio del equipo con ID1." )
   print( "   'r' (random) Enviar un dato aleatorio de un equipo aleatorio." )
   print( "--------------------------------------------------------------------\n" )
   return

# comando 0: Enviar un dato aleatorio del equipo con ID0.
def cmd_0():
   sendDataFromID( ID0 )
   return

# comando 1: Enviar un dato aleatorio del equipo con ID1.
def cmd_1():
   sendDataFromID( ID1 )
   return

# comando r: Enviar un dato aleatorio del equipo con ID1.
def cmd_r():
   if randint(0,1) == 0:
      IDrandom	= ID0
   else:
      IDrandom	= ID1   
   sendDataFromID( IDrandom )
   return


# Inicializa y abre el puertos serie ------------------------------------------

ser = serial.Serial()

print( "\nconexión al puerto serie ----------------------------------------\n" )

print("Ingrese el puerto serie, ejemplos: /dev/ttyUSB0 , COM1")
print("O bien ingrese 'l' para /dev/ttyUSB0, o 'w' para COM3")

receive = input()

if receive == 'l':
   ser.port = "/dev/ttyUSB0"       # Puerto por defecto para Linux
else:
   if receive == 'w':
      ser.port = "COM3"            # Puerto por defecto para Windows
   else:
      ser.port = receive
   
ser.baudrate = 38400
ser.bytesize = serial.EIGHTBITS    # number of bits per bytes # SEVENBITS
ser.parity = serial.PARITY_NONE    # set parity check: no parity # PARITY_ODD
ser.stopbits = serial.STOPBITS_ONE # number of stop bits # STOPBITS_TWO
# ser.timeout = None                 # block read
ser.timeout = 1                    # non-block read
# ser.timeout = 2                    # timeout block read
ser.xonxoff = False                # disable software flow control
ser.rtscts = False                 # disable hardware (RTS/CTS) flow control
ser.dsrdtr = False                 # disable hardware (DSR/DTR) flow control
ser.writeTimeout = 2               # timeout for write

try: 
   ser.open()
except Exception as e:
   print("Error abriendo puerto serie.\n" + str(e) + '\nFin de programa.')
   exit()

# Si pudo abrir el puerto -----------------------------------------------------

if ser.isOpen():

   print(ser.name + ' abierto.\n')

   try:
      ser.flushInput()  # flush input buffer, discarding all its contents
      ser.flushOutput() # flush output buffer, aborting current output 
                        # and discard all that is in buffer

      cmd_h()           # Imprime la lista de comandos

      # Ciclo infinito hasta comando exit (q) ---------------------------------
      while True: 

         command = ""

         # get keyboard input
         # input = raw_input(">> ")  # for Python 2
         command = input(">> ")      # for Python 3

         if command == 'q':
            print("Puerto cerrado. Se cierra el programa.")
            ser.close()
            exit()

         elif command == 'h':
            cmd_h()

         elif command == 'v':
            cmd_printInitialValues()    # Imprime los valores iniciales

         elif command == 'b':
            cmd_printDataOutputFormat() # Imprime el formato de salida

         elif command == '0':
            cmd_0()

         elif command == '1':
            cmd_1()

         elif command == 'r':
            cmd_r()

         else:
            print("Comando no conocido.")

   except Exception as e1:
      print("error de comunicación." + str(e1))

else:
   print("No se puede abrir el puerto serie.")
   exit()

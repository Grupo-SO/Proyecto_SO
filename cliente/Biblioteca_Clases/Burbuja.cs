﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Biblioteca_Clases
{
    public class Burbuja
    {
        //=========================================================================================================================\\
        //======================================================= ATRIBUTOS =======================================================\\

        // Caracteristicas
        public string nombre = "Oscar";
        public uint velocidad = 7; // Entero sin signo, de 0 a 20. Default 5;
        public int size = 100; // Entero, máximo de 502. Default 100.
        public int color = 1; // Entero de 1 a 8. Default 1 (azul).

        // Posición y velocidad de la burbuja dentro de la ventana del juego
        public float X = 100;
        public float Y = 100;

        // Posición y tamaño de la burbuja en el mapa real del juego
        public float posicion_x = 0;
        public float posicion_y = 0;
    }
}

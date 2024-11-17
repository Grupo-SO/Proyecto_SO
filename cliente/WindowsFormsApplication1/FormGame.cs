using Biblioteca_Clases;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;
using static System.Net.Mime.MediaTypeNames;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using Image = System.Drawing.Image;
using System.Runtime.InteropServices;
using System.Net.Sockets;

namespace WindowsFormsApplication1
{
    public partial class FormGame : Form
    {
        //=========================================================================================================================\\
        //======================================================= ATRIBUTOS =======================================================\\

        Timer clock = new Timer();
        Image fondo = Image.FromFile("fondo_juego.png");
        Image burbuja_sprite;

        // Dimensiones del picturebox que hace de interfaz
        int box_ancho = 0;
        int box_alto = 0;

        Point origen = new Point(); // Punto orígen de coordenadas
        bool cursor_over_box = false; // Detecta si el cursor está dentro o fuera del form

        // Coordenadas del dibujado del fondo
        int fondo_x = 0;
        int fondo_y = 0; 

        // Jugador principal de clase burbuja
        Burbuja jugador = new Burbuja();

        //=========================================================================================================================\\
        //======================================================== MÉTODOS ========================================================\\

        public FormGame(string usuario)
        {
            InitializeComponent();
            jugador.nombre = usuario;
        }

        // Configuración de la velocidad del ratón
        [DllImport("user32.dll", CharSet = CharSet.Auto),]
        public static extern int SystemParametersInfo(uint uiAction, uint uiParam, uint pvParam, uint fWinIni);

        // Load
        private void FormGame_Load(object sender, EventArgs e)
        {
            this.DoubleBuffered = true;

            // Propiedades del jugador
            Random rnd = new Random();
            jugador.color = rnd.Next(1, 9);
            string sprite_model = "burbuja_sprite_" + (jugador.color).ToString() + ".png";
            burbuja_sprite = Image.FromFile(sprite_model);

            // Propiedades de la ventana del juego
            this.MinimumSize = new Size(866, 539);
            this.MaximumSize = new Size(866, 539);
            this.CenterToScreen();

            // Propiedades del picturebox
            fondo_box.BackColor = Color.Transparent;
            origen = fondo_box.Location;
            this.box_ancho = fondo_box.Width;
            this.box_alto = fondo_box.Height;
            this.Controls.Add(this.fondo_box);

            // Propiedades del clock
            clock.Interval = 16; // aprox 60Hz de frecuencia
            clock.Tick += new EventHandler(clock_Tick);
        }

        // Cómputo del movimiento del fondo en la ventana
        private void Movimiento_Fondo(Point cursor)
        {
            int x_cursor = 3 * cursor.X;
            int y_cursor = 3 * cursor.Y;

            // Límites de el mapa
            if (this.cursor_over_box == true)
            {
                if (x_cursor > (2 * this.box_ancho))
                {
                    this.fondo_x = 2 * this.box_ancho;
                }
                else if (x_cursor <= 0)
                {
                    this.fondo_x = 0;
                }
                else
                {
                    this.fondo_x = x_cursor;
                }

                if (y_cursor > (2 * this.box_alto))
                {
                    this.fondo_y = 2 * this.box_alto;
                }
                else if (y_cursor <= 0)
                {
                    this.fondo_y = 0;
                }
                else
                {
                    this.fondo_y = y_cursor;
                }
            }
        }

        // Cómputo del movimiento del sprite jugador en la ventana
        private void Movimiento_Sprite(Point cursor)
        {
            int superficie = 290;

            // Colisión con la superfície
            if (this.cursor_over_box == true)
            {
                if ((this.fondo_y + 85) > (superficie + (jugador.size / 2)))
                {
                    jugador.X = cursor.X - (jugador.size / 2);
                    jugador.Y = cursor.Y - (jugador.size / 2);
                }
                else
                {
                    jugador.X = cursor.X - (jugador.size / 2);
                    jugador.Y = superficie - this.fondo_y;
                }
            }
        }

        // Activa el clock e inicializa el juego 
        private void fondo_box_Click(object sender, EventArgs e)
        {
            clock.Enabled = true;
            SystemParametersInfo(113, 0, jugador.velocidad, 0);
            cursor_over_box = true;
            Cursor.Hide();
        }

        // Evento clock (aprox 60 Hz)
        void clock_Tick(object sender, EventArgs e)
        {
            // Obtención de la posición del cursor en el pictureBox de fondo
            Point cursor = fondo_box.PointToClient(Cursor.Position);
            Movimiento_Fondo(cursor);
            Movimiento_Sprite(cursor);
            Invalidate();
        }

        // Dibuja el fondo y todos los sprites
        private void FormGame_Paint(object sender, PaintEventArgs e)
        {
            // Formato del game tag
            StringFormat tag_formato = new StringFormat();
            tag_formato.LineAlignment = StringAlignment.Center;
            tag_formato.Alignment = StringAlignment.Center;
            Font fuente = this.Font;
            Brush color = Brushes.Black;

            // Dibujado de la sección del mapa
            e.Graphics.TranslateTransform(this.origen.X, this.origen.Y); // Se situa en el picturebox
            e.Graphics.DrawImage(this.fondo,
                new Rectangle(0, 0, this.box_ancho, this.box_alto),
                new Rectangle(this.fondo_x, this.fondo_y, this.box_ancho, this.box_alto),
                GraphicsUnit.Pixel);

            // Dijujado del sprite del jugador y su gametag
            if (clock.Enabled == true)
            {
                Point tag_location = new Point((jugador.X + (jugador.size / 2)), (jugador.Y + jugador.size + 10));
                e.Graphics.DrawImage(burbuja_sprite, jugador.X, jugador.Y, jugador.size, jugador.size);
                e.Graphics.DrawString(jugador.nombre, fuente, color, tag_location.X, tag_location.Y, tag_formato);
            }
        }

        // La velocidad del cursor se adapta a la del juego
        private void fondo_box_MouseEnter(object sender, EventArgs e)
        {
            if (clock.Enabled == true)
            {
                SystemParametersInfo(113, 0, jugador.velocidad, 0);
                cursor_over_box = true;
            }
        }

        // La velocidad del cursor se normaliza
        private void fondo_box_MouseLeave(object sender, EventArgs e)
        {
            if (clock.Enabled == true)
            {
                SystemParametersInfo(113, 0, 10, 0);
                cursor_over_box = false;
            }
        }

    }
}

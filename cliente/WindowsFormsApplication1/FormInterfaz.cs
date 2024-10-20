using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class FormInterfaz : Form
    {
        //=========================================================================================================================\\
        //======================================================= ATRIBUTOS =======================================================\\

        private Socket server; // Socket para la conexión con el servidor
        private string usuario; // Nombre del usuario que juega

        //=========================================================================================================================\\
        //======================================================== MÉTODOS ========================================================\\

        public FormInterfaz(Socket serverSocket, string usuario)
        {
            InitializeComponent();
            this.server = serverSocket; // Inicializar el socket con el proporcionado
            this.usuario = usuario; // Usuario que juega

            // Verificar la conexión al servidor
            if (server != null && server.Connected)
            {
                Console.WriteLine("Conexión establecida con el servidor.");
            }
            else
            {
                Console.WriteLine("No se pudo conectar al servidor.");
                MessageBox.Show("No se pudo conectar al servidor. Verifica la conexión.");
            }
        }

        private void playButton_Click(object sender, EventArgs e)
        {
            // Lógica del juego (comentada para ser implementada más tarde)
            FormGame formGame = new FormGame(usuario);
            formGame.ShowDialog();
            this.Close();
        }

        private void FormInterfaz_Load(object sender, EventArgs e)
        {
            // Cargar la imagen de fondo
            Image background = Image.FromFile("imagen3.png");
            this.BackgroundImage = background;

            // Ajustar el modo de visualización de la imagen
            this.BackgroundImageLayout = ImageLayout.Stretch; // Puedes usar otros modos como Tile, Center, Zoom, etc.
        }

        private async void usuariosConectadosButton_Click(object sender, EventArgs e)
        {
            // Deshabilitar el botón para evitar múltiples clics
            usuariosConectadosButton.Enabled = false;

            try
            {
                // Verificar que el socket esté conectado

                if (server == null || !server.Connected)
                {
                    MessageBox.Show("No hay conexión con el servidor.");
                    return; // Salir de la función si no está conectado
                }

                // Enviar solicitud al servidor
                string mensaje = "LISTAR_USUARIOS"; // Mensaje para solicitar la lista de usuarios
                byte[] msg = Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg); // Enviar el mensaje al servidor

                // Recibir la lista de jugadores
                byte[] buffer = new byte[1024]; // Asegúrate de que este tamaño sea suficiente
                int bytesRecibidos = await Task.Run(() => server.Receive(buffer));
                string respuesta = Encoding.ASCII.GetString(buffer, 0, bytesRecibidos).Trim();

                if (respuesta.StartsWith("USUARIOS_CONECTADOS/"))
                {
                    // Extraer la lista de jugadores de la respuesta
                    string listaJugadores = respuesta.Substring("USUARIOS_CONECTADOS/".Length);
                    string[] jugadores = listaJugadores.Split(',');

                    // Comprobar si hay jugadores
                    if (jugadores.Length == 0)
                    {
                        MessageBox.Show("No hay jugadores conectados.");
                        return;
                    }

                    // Crear un nuevo DataGridView
                    DataGridView dgv = new DataGridView();
                    dgv.AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.Fill; // Ajusta el tamaño de las columnas
                    dgv.AllowUserToAddRows = false; // Evitar la fila para agregar
                    dgv.Columns.Add("Nombre del jugador", "Nombre del jugador");

                    for (int i = 0; i < jugadores.Length; i++) 
                    {
                        dgv.Rows.Add(jugadores[i]);
                    }

                    // Mostrar el DataGridView en un nuevo formulario
                    Form dgvForm = new Form
                    {
                        Size = new Size(400, 300),
                        Text = "Jugadores Conectados"
                    };
                    dgvForm.Controls.Add(dgv);
                    dgvForm.ShowDialog(); 
                }
                else
                {
                    MessageBox.Show("Error al obtener la lista de jugadores.");
                }
            }
            catch (ObjectDisposedException ex)
            {
                MessageBox.Show($"El socket ha sido cerrado: {ex.Message}");
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al comunicar con el servidor: {ex.Message}");
            }
            finally
            {
                // Volver a habilitar el botón
                usuariosConectadosButton.Enabled = true;
            }
        }


    }
}


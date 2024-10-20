using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;

namespace WindowsFormsApplication1
{
    public partial class FormCrearUsuario : Form
    {
        //=========================================================================================================================\\
        //======================================================= ATRIBUTOS =======================================================\\

        private Socket server;

        //=========================================================================================================================\\
        //======================================================== MÉTODOS ========================================================\\

        public FormCrearUsuario(Socket serverSocket)
        {
            InitializeComponent();
            server = serverSocket;  
        }

        private void CrearBDButton_Click(object sender, EventArgs e)
        {
            try
            {
                string mensaje = "CREAR_BD"; // Mensaje al server para que sepa que hacer y enviamos
                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);

                byte[] buffer = new byte[256]; // Recibimos respuesta
                int bytesRecibidos = server.Receive(buffer);
                string respuesta = System.Text.Encoding.ASCII.GetString(buffer, 0, bytesRecibidos).Trim();
                MessageBox.Show("Respuesta del servidor: " + respuesta);

                if (respuesta.Equals("OK", StringComparison.OrdinalIgnoreCase))
                {
                    MessageBox.Show("Base de datos creada correctamente.");
                }
                else
                {
                    MessageBox.Show("Error al crear la base de datos.");
                }
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al cerrar la conexión: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void guardarButton_Click(object sender, EventArgs e)
        {
            try
            {
                string msgUsuario = inserta_usuarioTextBox.Text;
                string msgContraseña = inserta_contraseñaTextBox.Text;
                string mensaje = "ADD_USER/" + msgUsuario + "/" + msgContraseña; // Mensaje al server para que sepa que hacer

                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje); // Enviamos en bytes
                server.Send(msg);

                byte[] buffer = new byte[256]; // Recibimos
                int bytesRecibidos = server.Receive(buffer);
                string respuesta = System.Text.Encoding.ASCII.GetString(buffer, 0, bytesRecibidos).Trim();
                MessageBox.Show("Respuesta del servidor: " + respuesta);

                if (respuesta.Equals("OK", StringComparison.OrdinalIgnoreCase))
                {
                    MessageBox.Show("Usuario añadido correctamente.");
                }
                else
                {
                    MessageBox.Show("Error al añadir usuario.");
                }
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al cerrar la conexión: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void FormCrearUsuario_Load(object sender, EventArgs e)
        {

        }
    }
}

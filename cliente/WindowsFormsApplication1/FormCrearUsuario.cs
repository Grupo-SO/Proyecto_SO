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
                MessageBox.Show("Respuesta del servidor: " + respuesta, "Respuesta del servidor", MessageBoxButtons.OK, MessageBoxIcon.Information);

                if (respuesta.Equals("OK", StringComparison.OrdinalIgnoreCase))
                {
                    MessageBox.Show("Base de datos creada correctamente.", "Operación exitosa", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                else
                {
                    MessageBox.Show("Error al crear la base de datos.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al cerrar la conexión: {ex.Message}", "Error de conexión", MessageBoxButtons.OK, MessageBoxIcon.Error);
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

                if (respuesta.Equals("OK", StringComparison.OrdinalIgnoreCase))
                {
                    MessageBox.Show("Usuario añadido correctamente.", "Operación exitosa", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                else if (respuesta.Equals("Existe", StringComparison.OrdinalIgnoreCase))
                {
                    MessageBox.Show("El usuario ya existe.", "Usuario duplicado", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                }
                else
                {
                    MessageBox.Show("Error al añadir usuario.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al cerrar la conexión: {ex.Message}", "Error de conexión", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void FormCrearUsuario_Load(object sender, EventArgs e)
        {

        }
    }
}


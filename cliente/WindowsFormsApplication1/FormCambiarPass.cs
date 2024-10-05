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
    public partial class FormCambiarPass : Form
    {
        private Socket server;
        public FormCambiarPass()
        {
            InitializeComponent();
        }
        public FormCambiarPass(Socket serverSocket)
        {
            InitializeComponent();
            server = serverSocket;
        }

        private void ConfirmarButton_Click(object sender, EventArgs e)
        {
            if (PlayerCodetextBox2.Text == "7733")
            {
                try
                {
                    string msgUsuario = NewUsertextBox1.Text;                                                            //Preparamos el mensaje concatenando el usuario y la contraseña con un delimitador "/"
                    string msgContraseña = NewPasstextBox3.Text;
                    string mensaje = "CAMBIAR_PASS/" + msgUsuario + "/" + msgContraseña;

                    byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                    server.Send(msg);

                    byte[] buffer = new byte[256];                                                                              //Recibimos respuesta
                    int bytesRecibidos = server.Receive(buffer);
                    string respuesta = System.Text.Encoding.ASCII.GetString(buffer, 0, bytesRecibidos).Trim();
                    MessageBox.Show("Respuesta del servidor: " + respuesta);

                    if (respuesta == "OK")
                    {
                        MessageBox.Show("Contraseña cambiada con éxito");
                        this.Close();
                    }
                    if (respuesta == "ERROR")
                    {
                        MessageBox.Show("Este usuario no existe.");
                    }
                }
                catch (SocketException ex)
                {
                    MessageBox.Show($"Error al cerrar la conexión: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                MessageBox.Show("Codigo de jugador erroneo");
            }
        }
    }
}

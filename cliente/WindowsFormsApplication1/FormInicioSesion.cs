using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;

namespace WindowsFormsApplication1
{
    public partial class FormInicioSesion : Form
    {
        Socket server;                                                                                            //Creamos objeto de la clase Socket (librerias)
        public FormInicioSesion()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            
            IPAddress direc = IPAddress.Parse("83.42.10.116");                                                    //Creamos un IPEndPoint con el ip del servidor y puerto del servidor 
            IPEndPoint ipep = new IPEndPoint(direc, 20518);

            server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);                       //Creamos Socket
            try
            {
                server.Connect(ipep);                                                                                   //Intentamos conectar el socket (verde si funciona)
                MessageBox.Show("Conectado");
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al cerrar la conexión: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
        }

        private void entrarButton_Click(object sender, EventArgs e)
        {
            try
            {
                if (!string.IsNullOrEmpty(usuarioTextBox.Text) && !string.IsNullOrEmpty(contraseñaTextBox.Text))
                {
                    string msgUsuario = usuarioTextBox.Text;                                                            //Preparamos el mensaje concatenando el usuario y la contraseña con un delimitador "/"
                    string msgContraseña = contraseñaTextBox.Text;
                    string mensaje = "AUTENTICAR/" + msgUsuario + "/" + msgContraseña;                                  //Mensaje al server para que sepa que hacer

                    byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);                                          //Enviamos al servidor el usuario (azul)
                    server.Send(msg);
                    this.BackColor = Color.Blue;

                    byte[] buffer = new byte[256];                                                                      //Recibimos la respuesta del servidor
                    int bytesRecibidos = server.Receive(buffer);
                    string respuesta = System.Text.Encoding.ASCII.GetString(buffer, 0, bytesRecibidos);
                    MessageBox.Show("Respuesta recibida del servidor:" + respuesta);

                    if (respuesta == "OK")
                    {
                        server.Shutdown(SocketShutdown.Both);                                                               // Finalizamos la conexión una vez se ha realizado todo
                        server.Close();
                        FormInterfaz formInterfaz = new FormInterfaz();
                        formInterfaz.ShowDialog();
                        this.Close();
                    }
                    if (respuesta == "ERROR")
                    {
                        this.BackColor = Color.Brown;
                        MessageBox.Show("Usuario o contraseña incorrectos. Inténtalo de nuevo.");
                    }
                }
                else
                {
                    MessageBox.Show("Introduce usuario y contraseña");
                }

            }
            catch (SocketException ex) 
            {
                MessageBox.Show($"Error al cerrar la conexión: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
        private void crearusuarioButton_Click(object sender, EventArgs e)
        {
            FormCrearUsuario formCrearUsuario = new FormCrearUsuario(server);                                           //Para no tener que volver a establecer conexion nos la llevamos
            formCrearUsuario.ShowDialog();
        }


        private void contar_usuariosButton_Click(object sender, EventArgs e)
        {
            try
            {
                string mensaje = "CONTAR_USERS";                                                                        //Mensaje al server para que sepa que hacer

                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);                                          //Enviamos al servidor el usuario (azul)
                server.Send(msg);
                this.BackColor = Color.Blue;

                byte[] buffer = new byte[256];                                                                      //Recibimos la respuesta del servidor
                int bytesRecibidos = server.Receive(buffer);
                string respuesta = System.Text.Encoding.ASCII.GetString(buffer, 0, bytesRecibidos);
                MessageBox.Show("Respuesta recibida del servidor:" + respuesta);

                if (respuesta == "OK")
                {
                    return;
                }
                if (respuesta == "ERROR")
                {
                    this.BackColor = Color.Brown;
                    MessageBox.Show("No se han podido contar los usuarios.");
                }

                else
                {
                    MessageBox.Show("Introduce usuario y contraseña");
                }
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al cerrar la conexión: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void pass_olvidadaButton_Click(object sender, EventArgs e)
        {
            FormCambiarPass form1 = new FormCambiarPass(server);
            form1.ShowDialog();
        }
    }
}

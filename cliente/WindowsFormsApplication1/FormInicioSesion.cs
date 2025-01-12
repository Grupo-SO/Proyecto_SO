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
        //=========================================================================================================================\\
        //======================================================= ATRIBUTOS =======================================================\\

        Socket server; // Creamos objeto de la clase Socket (librerias)
        string servidor_ip = "10.4.119.5"; // IP del servidor
        int puerto_tcp = 50012; // Puerto del servidor

        //=========================================================================================================================\\
        //======================================================== MÉTODOS ========================================================\\
        public FormInicioSesion()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            IPAddress direc = IPAddress.Parse(servidor_ip); // Creamos un IPEndPoint con el ip del servidor y puerto del servidor 
            IPEndPoint ipep = new IPEndPoint(direc, puerto_tcp);
            label1.Text = "IP: " + servidor_ip;

            server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp); // Creamos Socket
            try
            {
                server.Connect(ipep); // Intentamos conectar el socket (verde si funciona)
                MessageBox.Show("Conectado", "Conexión exitosa", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al cerrar la conexión: {ex.Message}", "Error de conexión", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            // Configurar el TextBox para ocultar la contraseña al iniciar
            contraseñaTextBox.PasswordChar = '*';
            checkBox1.Text = "Mostrar contraseña"; // Texto inicial del CheckBox
        }

        private void entrarButton_Click(object sender, EventArgs e)
        {
            try
            {
                if (!string.IsNullOrEmpty(usuarioTextBox.Text) && !string.IsNullOrEmpty(contraseñaTextBox.Text))
                {
                    string msgUsuario = usuarioTextBox.Text;                                    //Cogemos el usuario del textBox
                    string msgContraseña = contraseñaTextBox.Text;                              //Cogemos la contraseña del textBox
                    string mensaje = "AUTENTICAR/" + msgUsuario + "/" + msgContraseña;          // Mensaje al servidor

                    byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                    server.Send(msg);
                    this.BackColor = Color.Blue;                                                //Cambiamos el color de fondo para confirmar el envio

                    byte[] buffer = new byte[256]; // Recibimos la respuesta del servidor
                    int bytesRecibidos = server.Receive(buffer);
                    string respuesta = System.Text.Encoding.ASCII.GetString(buffer, 0, bytesRecibidos);

                    if (respuesta == "OK")
                    {
                        MessageBox.Show("Autenticación correcta. Bienvenido.", "Inicio de sesión correcto", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        FormInterfaz formInterfaz = new FormInterfaz(server, msgUsuario);
                        formInterfaz.ShowDialog();
                    }

                    else if (respuesta == "Online")
                    {
                        this.BackColor = Color.Brown;
                        MessageBox.Show("El usuario ya está en línea.", "Estado en línea", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    }
                    else if (respuesta == "ERROR")
                    {
                        this.BackColor = Color.Brown;
                        MessageBox.Show("Usuario o contraseña incorrectos. Inténtalo de nuevo.", "Error de autenticación", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
                else
                {
                    MessageBox.Show("Introduce usuario y contraseña", "Campos requeridos", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                }
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al cerrar la conexión: {ex.Message}", "Error de conexión", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void crearusuarioButton_Click(object sender, EventArgs e)
        {
            FormCrearUsuario formCrearUsuario = new FormCrearUsuario(server); // Para no tener que volver a establecer conexion nos la llevamos
            formCrearUsuario.ShowDialog();
        }

        private void contar_usuariosButton_Click(object sender, EventArgs e)
        {
            try
            {
                string mensaje = "CONTAR_USERS"; // Mensaje al server para que sepa que hacer

                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje); // Enviamos al servidor el usuario (azul)
                server.Send(msg);
                this.BackColor = Color.Blue;

                byte[] buffer = new byte[256]; // Recibimos la respuesta del servidor
                int bytesRecibidos = server.Receive(buffer);
                string respuesta = System.Text.Encoding.ASCII.GetString(buffer, 0, bytesRecibidos);
                MessageBox.Show("Respuesta recibida del servidor: " + respuesta, "Respuesta del servidor", MessageBoxButtons.OK, MessageBoxIcon.Information);

                if (respuesta == "OK")
                {
                    return;
                }
                if (respuesta == "ERROR")
                {
                    this.BackColor = Color.Brown;
                    MessageBox.Show("No se han podido contar los usuarios.", "Error al contar usuarios", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                else
                {
                    MessageBox.Show("Introduce usuario y contraseña", "Campos requeridos", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                }
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al cerrar la conexión: {ex.Message}", "Error de conexión", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void pass_olvidadaButton_Click(object sender, EventArgs e)
        {
            FormCambiarPass form1 = new FormCambiarPass(server);
            form1.ShowDialog();
        }

        private void checkBox1_CheckedChanged_1(object sender, EventArgs e)
        {
            if (checkBox1.Checked)
            {
                // Mostrar la contraseña en texto plano
                contraseñaTextBox.PasswordChar = '\0';
                checkBox1.Text = "Ocultar contraseña";
            }
            else
            {
                // Ocultar la contraseña con asteriscos
                contraseñaTextBox.PasswordChar = '*';
                checkBox1.Text = "Mostrar contraseña";
            }
        }

        private void offlineButton_Click(object sender, EventArgs e)
        {
            FormGame formGame = new FormGame("offline");
            formGame.ShowDialog();
        }
    }
}

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

            // Asociar el evento de MouseDown al DataGridView
            dataGridViewJugadores.MouseDown += DataGridViewJugadores_MouseDown;

            // Asociar el evento Click del menu de invitación
            //invitarToolStripMenuItem.Click += invitarToolStripMenuItem_Click;
        }
        private void DataGridViewJugadores_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                var hitTestInfo = dataGridViewJugadores.HitTest(e.X, e.Y);
                if (hitTestInfo.RowIndex >= 0)
                {
                    dataGridViewJugadores.ClearSelection();
                    dataGridViewJugadores.Rows[hitTestInfo.RowIndex].Selected = true;
                    contextMenuStripInvitar.Show(dataGridViewJugadores, e.Location);
                }
            }
        }


        private void playButton_Click(object sender, EventArgs e)
        {
            // Lógica del juego (comentada para ser implementada más tarde)
            FormGame formGame = new FormGame(usuario);
            formGame.ShowDialog();
            this.Close();
        }

        private async void FormInterfaz_Load(object sender, EventArgs e)
        {
            try
            {
                // Cargar la imagen de fondo
                Image background = Image.FromFile("imagen3.png");
                this.BackgroundImage = background;

                // Ajustar el modo de visualización de la imagen
                this.BackgroundImageLayout = ImageLayout.Stretch; // Puedes usar otros modos como Tile, Center, Zoom, etc.

                // Iniciar la escucha de mensajes
                await Task.Run(() => EscucharMensajesServidor());
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error al cargar el formulario: {ex.Message}");
            }
        }

        private void EscucharMensajesServidor()
        {
            try
            {
                byte[] buffer = new byte[4096]; // Aumenta el tamaño si es necesario
                int bytesRecibidos;

                while ((bytesRecibidos = server.Receive(buffer)) > 0)
                {
                    string respuesta = Encoding.ASCII.GetString(buffer, 0, bytesRecibidos).Trim();
                    Console.WriteLine($"Mensaje recibido: {respuesta}");

                    if (respuesta.StartsWith("USUARIOS_CONECTADOS/"))
                    {
                        // Manejar la lista completa de usuarios conectados
                        string lista = respuesta.Substring("USUARIOS_CONECTADOS/".Length).Trim();
                        Console.WriteLine("Lista de usuarios conectados recibida: " + lista);
                        ActualizarListaJugadores(lista);
                    }
                    else if (respuesta.StartsWith("INVITACION:"))
                    {
                        // Manejar invitación recibida
                        string source_username = respuesta.Substring("INVITACION:".Length).Trim();
                        Console.WriteLine($"Recibida invitación de: {source_username}");

                        // Invocar el MessageBox en el hilo de la interfaz de usuario
                        this.Invoke((MethodInvoker)delegate
                        {
                            DialogResult result = MessageBox.Show($"{source_username} te ha invitado a jugar. ¿Aceptar?", "Invitación a jugar", MessageBoxButtons.YesNo, MessageBoxIcon.Question);

                            if (result == DialogResult.Yes)
                            {
                                // Enviar "INVITACION_ACEPTADA/<source_username>/<target_username>" al servidor
                                string mensaje = $"INVITACION_ACEPTADA/{source_username}/{this.usuario}";
                                byte[] msg = Encoding.ASCII.GetBytes(mensaje);
                                try
                                {
                                    server.Send(msg);
                                }
                                catch (SocketException ex)
                                {
                                    MessageBox.Show($"Error al enviar la aceptación: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                }

                                // Unir a la partida
                                MessageBox.Show("Has aceptado la invitación. Iniciando partida...", "Partida iniciada", MessageBoxButtons.OK, MessageBoxIcon.Information);
                                //FormGame formGame = new FormGame(server, usuario, source_username);
                                //formGame.ShowDialog();
                            }
                            else
                            {
                                // Enviar "INVITACION_RECHAZADA/<source_username>/<target_username>" al servidor
                                string mensaje = $"INVITACION_RECHAZADA/{source_username}/{this.usuario}";
                                byte[] msg = Encoding.ASCII.GetBytes(mensaje);
                                try
                                {
                                    server.Send(msg);
                                }
                                catch (SocketException ex)
                                {
                                    MessageBox.Show($"Error al enviar el rechazo: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                }
                            }
                        });
                    }
                    else if (respuesta.StartsWith("INVITACION_ACEPTADA:"))
                    {
                        // Manejar confirmación de invitación aceptada
                        string target_username = respuesta.Substring("INVITACION_ACEPTADA:".Length).Trim();

                        // Invocar el MessageBox en el hilo de la interfaz de usuario
                        this.Invoke((MethodInvoker)delegate
                        {
                            MessageBox.Show($"{target_username} ha aceptado tu invitación. Iniciando partida...", "Invitación aceptada", MessageBoxButtons.OK, MessageBoxIcon.Information);

                            // Unir a la partida
                            //FormGame formGame = new FormGame(server, usuario, target_username);
                            //formGame.ShowDialog();
                        });
                    }
                    else if (respuesta.StartsWith("INVITACION_RECHAZADA:"))
                    {
                        // Manejar confirmación de invitación rechazada
                        string target_username = respuesta.Substring("INVITACION_RECHAZADA:".Length).Trim();

                        // Invocar el MessageBox en el hilo de la interfaz de usuario
                        this.Invoke((MethodInvoker)delegate
                        {
                            MessageBox.Show($"{target_username} ha rechazado tu invitación.", "Invitación rechazada", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        });
                    }
                    else
                    {
                        // Manejar otros tipos de respuestas si es necesario
                        Console.WriteLine($"Respuesta no manejada: {respuesta}");
                    }
                }
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al comunicar con el servidor: {ex.Message}");
            }
            catch (ObjectDisposedException)
            {
                // El socket ha sido cerrado, probablemente la aplicación se está cerrando
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error inesperado: {ex.Message}");
            }
        }


        private void ActualizarListaJugadores(string lista)
        {
            if (InvokeRequired)
            {
                this.Invoke(new Action<string>(ActualizarListaJugadores), new object[] { lista });
                return;
            }

            // Limpiar la lista actual
            dataGridViewJugadores.Rows.Clear();

            // Separar los nombres de usuario por salto de línea
            string[] usuarios = lista.Split(new[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);

            // Añadir cada usuario al DataGridView
            foreach (string usuario in usuarios)
            {
                dataGridViewJugadores.Rows.Add(usuario.Trim());
            }
        }

        private void invitarToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (dataGridViewJugadores.SelectedRows.Count > 0)
            {
                string target_username = dataGridViewJugadores.SelectedRows[0].Cells["NombreJugador"].Value.ToString().Trim();
                string source_username = this.usuario.Trim();

                // Validar que no estás intentando invitarte a ti mismo
                if (string.Equals(target_username, source_username, StringComparison.OrdinalIgnoreCase))
                {
                    MessageBox.Show("No puedes invitarte a ti mismo.", "Invitación inválida", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return;
                }

                // Enviar mensaje "INVITAR/<target_username>/<source_username>" al servidor
                string mensaje = $"INVITAR/{target_username}/{source_username}";
                byte[] msg = Encoding.ASCII.GetBytes(mensaje);
                try
                {
                    server.Send(msg);
                    Console.WriteLine($"Enviada invitación a: {target_username}");
                    MessageBox.Show($"Has invitado a {target_username} a jugar.", "Invitación enviada", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                catch (SocketException ex)
                {
                    MessageBox.Show($"Error al enviar la invitación: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }
    }
}

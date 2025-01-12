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

        Socket server; // Socket para la conexión con el servidor
        private string usuario; // Nombre del usuario que juega
        private List<string> lista_mensajes = new List<string>();


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
            // Enviar mensaje "BUSCAR_PARTIDA" al servidor
            string mensaje = "BUSCAR_PARTIDA";
            byte[] msg = Encoding.ASCII.GetBytes(mensaje);
            try
            {
                server.Send(msg);


            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al enviar el mensaje de matchmaking: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private async void FormInterfaz_Load(object sender, EventArgs e)
        {
            try
            {
                // Cargar la imagen de fondo
                Image background = Image.FromFile("imagen3.png");
                this.BackgroundImage = background;

                // Ajustar el datagrid
                dataGridViewJugadores.RowHeadersVisible = false;

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

                    else if (respuesta == "PARTIDA_INICIADA")
                    {
                        this.Invoke((MethodInvoker)delegate
                        {
                            // Iniciar el juego
                            FormGame formGame = new FormGame(server, usuario);
                            formGame.Show();
                        });
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
                            MessageBox.Show($"{target_username} ha aceptado tu invitación. Uniéndote a la sala...", "Invitación aceptada", MessageBoxButtons.OK, MessageBoxIcon.Information);

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
                    else if (respuesta == "HAS_SIDO_ASIGNADO_A_PARTIDA")
                    {
                        // Notificar al usuario que ha sido asignado a una partida
                        this.Invoke((MethodInvoker)delegate
                        {
                            MessageBox.Show("Has sido asignado a una partida. La partida está iniciando...", "Asignado a Partida", MessageBoxButtons.OK, MessageBoxIcon.Information);

                        });
                    }
                    else if (respuesta.StartsWith("CHAT/"))
                    {
                        Console.WriteLine("Respuesta de chat recibida: " + respuesta);
                        // Manejar mensajes de chat
                        string[] partes = respuesta.Split(new[] { '/' }, 3);
                        if (partes.Length >= 3)
                        {
                            string emisor = partes[1];
                            string mensajeChat = partes[2];
                            // Invocar el método para actualizar el chat en el hilo de la interfaz de usuario
                            this.Invoke((MethodInvoker)delegate
                            {
                                // Añadir el mensaje al PictureBox
                                lista_mensajes.Add($"{emisor}: {mensajeChat}");
                                ActualizarChat();
                            });
                        }
                    }
                    else if (respuesta.StartsWith("CUENTA_ELIMINADA_OK"))
                    {
                        // Cerrar la ventana de la interfaz y mostrar el formulario de inicio de sesión
                        this.Invoke((MethodInvoker)delegate
                        {
                            MessageBox.Show("Cuenta eliminada correctamente. Reinicia el programa.", "Cuenta eliminada", MessageBoxButtons.OK, MessageBoxIcon.Information);
                            this.Close();
                        });
                    }
                    else if (respuesta.StartsWith("SALA_LLENA"))
                    {
                        // Notificar al usuario que la sala está llena
                        this.Invoke((MethodInvoker)delegate
                        {
                            MessageBox.Show("La sala a la que intentas invitar está llena.", "Sala Llena", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        });
                    }
                    /*else if (respuesta == "PARTIDA_GANADA")
                    {
                        this.Invoke((MethodInvoker)delegate
                        {
                            MessageBox.Show("¡Has ganado la partida!",
                                            "Victoria",
                                            MessageBoxButtons.OK,
                                            MessageBoxIcon.Information);

                            // Cerrar el FormGame si está abierto
                            foreach (Form frm in Application.OpenForms)
                            {
                                if (frm is FormGame)
                                {
                                    frm.Close();
                                    break;
                                }
                            }
                        });
                    }*/

                    else if (respuesta.StartsWith("JUGANDO"))
                    {
                        // Notificar al usuario que la sala está llena
                        this.Invoke((MethodInvoker)delegate
                        {
                            MessageBox.Show("El jugador esta en partida", "Jugando", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        });
                    }
                    else if (respuesta.StartsWith("CUENTA_ELIMINADA_ERROR"))
                    {
                        // Mostrar mensaje de error al eliminar la cuenta
                        this.Invoke((MethodInvoker)delegate
                        {
                            MessageBox.Show("Error al eliminar la cuenta. Inténtalo de nuevo.", "Error al eliminar cuenta", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        });
                    }
                    else if (respuesta.StartsWith("PARTIDA_ABANDONADA_OK"))
                    {
                        // Mostrar mensaje de partida abandonada
                        this.Invoke((MethodInvoker)delegate
                        {
                            MessageBox.Show("Has abandonado la partida.", "Partida abandonada", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        });
                    }
                    else if (respuesta.StartsWith("HISTORIAL/"))
                    {
                        // El resto son líneas "id,fecha\nid,fecha\n..."
                        string raw = respuesta.Substring("HISTORIAL/".Length);
                        // Separar por líneas
                        string[] lineas = raw.Split(new[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);

                        this.Invoke((MethodInvoker)delegate
                        {
                            // Limpiar dataGridViewPartidas
                            dataGridViewPartidas.Rows.Clear();

                            // Suponiendo que dataGridViewPartidas tiene 2 columnas: "Partida" y "Fecha"
                            // Recorremos cada línea
                            foreach (string linea in lineas)
                            {
                                // linea => "id,fecha"
                                string[] partes = linea.Split(',');
                                if (partes.Length == 2)
                                {
                                    string pid = partes[0];
                                    string fecha = partes[1];
                                    dataGridViewPartidas.Rows.Add(pid, fecha);
                                }
                            }
                        });
                    }
                    else if (respuesta.StartsWith("GANADOR/"))
                    {
                        // Formato => "GANADOR/nombre/puntuacion"
                        string[] partes = respuesta.Split('/');
                        if (partes.Length == 3)
                        {
                            string ganadorNombre = partes[1];
                            string ganadorPunt = partes[2];

                            this.Invoke((MethodInvoker)delegate
                            {
                                MessageBox.Show($"El ganador es {ganadorNombre} con {ganadorPunt} puntos",
                                                "Ganador de la partida",
                                                MessageBoxButtons.OK, MessageBoxIcon.Information);
                            });
                        }
                    }

                    else if (respuesta.StartsWith("RESULTADOS/"))
                    {
                        string raw = respuesta.Substring("RESULTADOS/".Length).Trim();
                        string[] lineas = raw.Split(new[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);

                        // Construir un string para mostrar en MessageBox
                        // Ej: "Jugador1 - 120\nJugador2 - 80\n..."
                        StringBuilder sb = new StringBuilder();
                        foreach (string linea in lineas)
                        {
                            // linea => "nombre,puntuacion"
                            string[] partes = linea.Split(',');
                            if (partes.Length == 2)
                            {
                                sb.AppendLine($"{partes[0]} - {partes[1]} puntos");
                            }
                        }
                        string mensajeMostrar = $"Resultados de la partida:\n\n{sb.ToString()}";
                        this.Invoke((MethodInvoker)delegate
                        {
                            MessageBox.Show(mensajeMostrar, "Resultados de la Partida", MessageBoxButtons.OK, MessageBoxIcon.Information);
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

        private void ActualizarChat()
        {

            // Formato del string del mensaje
            StringFormat string_formato = new StringFormat();
            string_formato.LineAlignment = StringAlignment.Center;
            string_formato.Alignment = StringAlignment.Center;
            Font fuente = this.Font;
            Brush color = Brushes.Black;

            // Graphics del picturebox
            Graphics g = chat_picturebox.CreateGraphics();
            g.Clear(Color.White);

            // Redibujar todos los strings
            for (int i = 0; i < this.lista_mensajes.Count(); i++)
            {

                int last_element = this.lista_mensajes.Count() - 1;
                string mensaje = this.lista_mensajes[last_element - i];
                Size tamaño_texto = TextRenderer.MeasureText(mensaje, fuente);
                g.DrawString(mensaje, fuente, color, ((tamaño_texto.Width / 2) + 5), (chat_picturebox.Height - 10 - 12 * i), string_formato);
            }

        }

        private void send_button_Click(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(chat_textbox.Text))
            {
                // Enviar mensaje al servidor en formato "CHAT/<mensaje>"
                string mensaje = $"CHAT/{this.usuario}/{chat_textbox.Text}";
                byte[] msg = Encoding.ASCII.GetBytes(mensaje);
                try
                {
                    server.Send(msg);
                    // Añadir el mensaje al chat local
                    //this.lista_mensajes.Add($"{usuario}: {chat_textbox.Text}");
                    ActualizarChat();
                    chat_textbox.Clear();
                }
                catch (SocketException ex)
                {
                    MessageBox.Show($"Error al enviar el mensaje de chat: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void chat_textbox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                e.SuppressKeyPress = true; // Evitar que se agregue un salto de línea
                send_button.PerformClick(); // Simula el clic en el botón de envío
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
                    MessageBox.Show($"Has invitado a {target_username} a jugar.", "Invitación enviada", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                catch (SocketException ex)
                {
                    MessageBox.Show($"Error al enviar la invitación: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void eliminarCuentaToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string mensaje = "ELIMINAR_CUENTA";
            byte[] msg = Encoding.ASCII.GetBytes(mensaje);
            try
            {
                server.Send(msg);
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al enviar el mensaje de eliminación de cuenta: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void abandonarSalaToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Enviar mensaje para salir de la partida
            string mensaje = "SALIR_PARTIDA";
            byte[] msg = Encoding.ASCII.GetBytes(mensaje);
            try
            {
                server.Send(msg);
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al enviar el mensaje para salir de la partida: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void FormInterfaz_FormClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                string mensaje = "SALIR_PARTIDA";
                byte[] msg = Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al enviar mensaje de salir partida: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            try
            {
                // Enviar el mensaje para que el servidor quite al usuario de la lista
                string mensaje = "QUITAR_USUARIO";
                byte[] msg = Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error al enviar mensaje para quitar usuario: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void dataGridViewPartidas_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                var hitTestInfo = dataGridViewPartidas.HitTest(e.X, e.Y);
                if (hitTestInfo.RowIndex >= 0)
                {
                    dataGridViewPartidas.ClearSelection();
                    dataGridViewPartidas.Rows[hitTestInfo.RowIndex].Selected = true;
                    contextMenuStripPartidas.Show(dataGridViewPartidas, e.Location);
                }
            }
        }

        private void verResultadosStripMenuItem_Click(object sender, EventArgs e)
        {
            if (dataGridViewPartidas.SelectedRows.Count > 0)
            {
                string partida = dataGridViewPartidas.SelectedRows[0].Cells["Partida"].Value.ToString().Trim();  // ID de la partida

                // Enviar "GET_RESULTADOS/<partida>"
                string mensaje = $"GET_RESULTADOS/{partida}";
                byte[] msg = Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
            }
        }

        private void verGanadorStripMenuItem_Click(object sender, EventArgs e)
        {
            if (dataGridViewPartidas.SelectedRows.Count > 0)
            {
                string partida = dataGridViewPartidas.SelectedRows[0].Cells["Partida"].Value.ToString().Trim();  // ID de la partida

                // Enviar "GET_GANADOR/<partida>"
                string mensaje = $"GET_GANADOR/{partida}";
                byte[] msg = Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
            }
        }

        private void historialDePartidasToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string mensaje = $"GET_HISTORIAL/{this.usuario}";
            byte[] msg = Encoding.ASCII.GetBytes(mensaje);
            server.Send(msg);

        }
    }
}


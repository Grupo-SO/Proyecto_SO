using Biblioteca_Clases;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class FormGame : Form
    {
        // =================================================== ATRIBUTOS =================================================== \\

        System.Windows.Forms.Timer clock = new System.Windows.Forms.Timer();
        Image fondo = Image.FromFile("fondo_juego.png");
        Image burbuja_sprite;
        Socket server;

        int box_ancho = 0;
        int box_alto = 0;

        Point origen = new Point();
        bool cursor_over_box = false;

        float fondo_x = 0;
        float fondo_y = 0;
        bool block_left = false;
        bool block_right = false;
        bool block_up = false;
        bool block_down = false;

        Burbuja jugador = new Burbuja();

        private Thread threadEscuchar;

        private Dictionary<string, Burbuja> jugadores = new Dictionary<string, Burbuja>();
        private Dictionary<int, StaticBubble> staticBubbles = new Dictionary<int, StaticBubble>();

        char estado = 'A';

        private List<string> jugadores_comidos = new List<string>();
        private List<int> burbujasEstaticasComidas = new List<int>();

        private Random rnd = new Random();

        bool modoOffline = false; // Nueva variable para indicar el modo offline
        bool todos_in = false; // True cuando todos los jugadores han entrado ya

        private bool isClosing = false; // Para cerrar el juego cuando el clock
        private bool cancelarEscucha = false;

        private UdpClient cliente_udp;
        private IPEndPoint ipep_udp;
        private Thread threadEscuchar_udp;
        private bool cancelarEscucha_udp = false;
        int puerto_udp = 50013;


        // =================================================== METODOS =================================================== \\

        // Constructor para modo online
        public FormGame(Socket serverSocket, string usuario)
        {
            InitializeComponent();
            this.jugador.nombre = usuario;
            this.server = serverSocket;
            Inicializar_UDP();
        }

        // Constructor para modo offline
        public FormGame(string usuario)
        {
            InitializeComponent();
            this.jugador.nombre = usuario;
            this.server = null; // No hay servidor en modo offline
            this.modoOffline = true;
        }

        private void FormGame_Load(object sender, EventArgs e)
        {
            this.DoubleBuffered = true;

            // Elegir color y posición inicial aleatoria
            jugador.color = (int)rnd.Next(1, 9);
            jugador.size = 80;
            jugador.velocidad = 7;

            // Posición inicial aleatoria dentro de los límites del mapa
            int mapWidth = fondo.Width;
            int mapHeight = fondo.Height;
            int startX = rnd.Next(0, mapWidth - jugador.size);
            int startY = rnd.Next(0, mapHeight - jugador.size);

            jugador.posicion_x = startX;
            jugador.posicion_y = startY;

            string sprite_model = "burbuja_sprite_" + (jugador.color).ToString() + ".png";
            burbuja_sprite = Image.FromFile(sprite_model);

            this.MinimumSize = new Size(866, 539);
            this.MaximumSize = new Size(866, 539);
            this.CenterToScreen();

            fondo_box.BackColor = Color.Transparent;
            origen = fondo_box.Location;
            this.box_ancho = fondo_box.Width;
            this.box_alto = fondo_box.Height;
            this.Controls.Add(this.fondo_box);

            clock.Interval = 41;
            clock.Tick += new EventHandler(clock_Tick);

            // Inicializar las burbujas estáticas
            InicializarBurbujasEstaticas();


            if (!modoOffline)
            {
                Inicializar_TCP();
                Enviar_Actualizacion_Estado();
            }
            else
            {
                // En modo offline, no hay comunicación con el servidor
                // Puedes inicializar cualquier lógica adicional aquí si es necesario
            }
            clock.Enabled = true;
            cursor_over_box = true;
        }

        private void Detectar_Ganador()
        {
            if ((jugadores.Count == 0) && (this.todos_in == true))
            {
                clock.Enabled = false;
                if (!modoOffline && server != null)
                {
                    string mensaje = "SALIR_PARTIDA2";
                    byte[] msg = Encoding.ASCII.GetBytes(mensaje);
                    try
                    {
                        MessageBox.Show("¡Has ganado! Bien jugado");
                        server.Send(msg);
                    }
                    catch (SocketException ex)
                    {
                        MessageBox.Show($"Error al enviar el mensaje para salir de la partida: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
                this.Close();
            }
        }

        // Envia por TCP una actualizacion sobre el estado del jugador
        private void Enviar_Actualizacion_Estado()
        {
            if (!modoOffline && server != null)
            {
                // Serializar la información del jugador y las burbujas comidas
                string jugadoresComidosStr = String.Join(",", jugadores_comidos.ToArray());
                string burbujasEstaticasComidasStr = String.Join(",", burbujasEstaticasComidas.ToArray());

                // Verificar si los strings están vacíos o nulos y asignar un guion si es el caso
                if (string.IsNullOrEmpty(jugadoresComidosStr))
                    jugadoresComidosStr = "-";
                if (string.IsNullOrEmpty(burbujasEstaticasComidasStr))
                    burbujasEstaticasComidasStr = "-";

                string estadoJugador = $"{jugador.nombre}/{jugador.size},{jugador.color},{this.estado}";
                string mensaje = $"UPDATE_GAME_STATE/{estadoJugador}/{jugadoresComidosStr}/{burbujasEstaticasComidasStr}";
                byte[] msg = Encoding.ASCII.GetBytes(mensaje);

                try
                {
                    server.Send(msg);
                }
                catch (SocketException ex)
                {
                    MessageBox.Show($"Error al enviar actualización de estado: {ex.Message}", "Error de envío", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                // Limpiar las listas de burbujas comidas para el siguiente tick
                jugadores_comidos.Clear();
                burbujasEstaticasComidas.Clear();
            }
        }

        // Envia por UDP una actualización sobre la posicion del jugador
        private void Enviar_Actualizacion_Posicion()
        {
            if (!modoOffline && cliente_udp != null)
            {
                string estadoJugador = $"{jugador.nombre}/{(int)jugador.posicion_x},{(int)jugador.posicion_y}";
                string mensaje = $"UPDATE_GAME_POS/{estadoJugador}";
                byte[] msg = Encoding.ASCII.GetBytes(mensaje);

                try
                {
                    cliente_udp.Send(msg, msg.Length);
                }
                catch (SocketException ex)
                {
                    MessageBox.Show($"Error al enviar actualización de posición: {ex.Message}", "Error de envío", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        // Función para procesar los mensajes recibidos del servidor
        private void Procesar_Mensaje_Servidor(string mensaje)
        {
            if (mensaje.StartsWith("UPDATE_GAME_STATE/"))
            {
                // Formato recibido:
                // UPDATE_GAME_STATE/nombre/size,color,estado/jugadoresComidos/burbujasEstaticasComidas
                // jugadoresComidos = "jug1,jug2"
                // burbujasEstaticasComidas = "1,2,3"
                string datos = mensaje.Substring("UPDATE_GAME_STATE/".Length);
                string[] partes = datos.Split('/');
                if (partes.Length < 4)
                    return;

                // partes[0] = nombre
                // partes[1] = size,color,estado
                // partes[2] = jugadoresComidosStr
                // partes[3] = burbujasComidasStr

                string nombreJugador = partes[0];
                string stateData = partes[1];
                string jugadoresComidosStr = partes[2];
                string burbujasComidasStr = partes[3];

                // Ahora parsea stateData por comas:
                string[] datosPartes = stateData.Split(',');
                if (datosPartes.Length != 3)
                    return;

                int size = int.Parse(datosPartes[0]);
                int color = int.Parse(datosPartes[1]);
                char estado = char.Parse(datosPartes[2]);

                lock (jugadores)
                {
                    if (jugadores.ContainsKey(nombreJugador))
                    {
                        if (estado == 'A')
                        {
                            // Actualizar datos
                            jugadores[nombreJugador].size = size;
                            jugadores[nombreJugador].color = color;
                        }
                    }
                    else
                    {
                        if (estado == 'A')
                        {
                            Burbuja nuevoJugador = new Burbuja()
                            {
                                nombre = nombreJugador,
                                size = size,
                                color = color
                            };
                            jugadores.Add(nombreJugador, nuevoJugador);
                            this.todos_in = true;
                        }
                    }

                    // Procesar jugadores comidos
                    if (!string.IsNullOrEmpty(jugadoresComidosStr))
                    {
                        string[] jComidos = jugadoresComidosStr.Split(',');
                        foreach (string jc in jComidos)
                        {
                            if (jugadores.ContainsKey(jc))
                            {
                                jugadores.Remove(jc);
                            }
                            if (jc == jugador.nombre)
                            {
                                clock.Enabled = false;
                                this.estado = 'D';
                                Enviar_Actualizacion_Estado();

                                if (!modoOffline && server != null)
                                {
                                    string message = "SALIR_PARTIDA2";
                                    byte[] msg = Encoding.ASCII.GetBytes(message);
                                    try
                                    {
                                        MessageBox.Show("Has perdido... Otra vez será");
                                        server.Send(msg);
                                    }
                                    catch (SocketException ex)
                                    {
                                        MessageBox.Show($"Error al enviar el mensaje para salir de la partida: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                    }
                                }

                                if (this.IsHandleCreated && !this.IsDisposed)
                                {
                                    this.Invoke((MethodInvoker)delegate
                                    {
                                        this.Close();
                                    });
                                }
                                break;
                            }
                        }
                    }

                    // Procesar burbujas estáticas comidas
                    if (!string.IsNullOrEmpty(burbujasComidasStr))
                    {
                        string[] bComidas = burbujasComidasStr.Split(',');
                        foreach (string bc in bComidas)
                        {
                            if (int.TryParse(bc, out int id))
                            {
                                if (staticBubbles.ContainsKey(id))
                                {
                                    // Poner la burbujita invisible y lanzar un temporizador local de reaparición
                                    StaticBubble sb = staticBubbles[id];
                                    sb.Visible = false;
                                    sb.LastEatenTime = DateTime.Now;
                                    staticBubbles[id] = sb;
                                }
                            }
                        }
                    }
                }
            }
            else if (mensaje.StartsWith("UPDATE_GAME_POS/"))
            {
                // Formato recibido:
                // UPDATE_GAME_POS/nombre/x,y
                string datos = mensaje.Substring("UPDATE_GAME_POS/".Length);
                string[] partes = datos.Split('/');
                if (partes.Length < 2)
                    return;

                // partes[0] = nombre
                // partes[1] = x,y

                string nombreJugador = partes[0];
                string posData = partes[1];

                // Ahora parsea posData por comas:
                string[] datosPartes = posData.Split(',');
                if (datosPartes.Length != 2)
                    return;

                int x = int.Parse(datosPartes[0]);
                int y = int.Parse(datosPartes[1]);

                lock (jugadores)
                {
                    if (jugadores.ContainsKey(nombreJugador))
                    {
                        // Actualizar datos
                        jugadores[nombreJugador].posicion_x = x;
                        jugadores[nombreJugador].posicion_y = y;
                    }
                }
            }
        }

        // Función para iniciar la escucha tcp
        private void Inicializar_TCP()
        {
            if (modoOffline || server == null)
                return;

            // Escucha del thread principal
            threadEscuchar = new Thread(Escuchar_Servidor);
            threadEscuchar.IsBackground = true;
            threadEscuchar.Start();
        }

        // Inicializa la connexión UDP
        private void Inicializar_UDP()
        {
            try
            {
                // Obtener la IP del servidor a partir del socket TCP
                IPEndPoint servidorTcpEP = (IPEndPoint)this.server.RemoteEndPoint;
                string servidor_ip = servidorTcpEP.Address.ToString();

                cliente_udp = new UdpClient();
                cliente_udp.Connect(servidor_ip, this.puerto_udp);

                // Enviar mensaje de registro UDP
                string mensajeInicial = $"REGISTER_UDP/{this.jugador.nombre}";
                byte[] data = Encoding.ASCII.GetBytes(mensajeInicial);
                cliente_udp.Send(data, data.Length);

                // Iniciar el hilo para escuchar mensajes UDP
                threadEscuchar_udp = new Thread(Escuchar_Servidor_UDP);
                threadEscuchar_udp.IsBackground = true;
                threadEscuchar_udp.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error al inicializar UDP: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void Escuchar_Servidor()
        {
            // Verificar si el formulario se está cerrando
            if (isClosing)
                return;

            try
            {
                while (!cancelarEscucha)
                {
                    byte[] buffer = new byte[512];
                    int bytesRecibidos = server.Receive(buffer);
                    if (bytesRecibidos > 0)
                    {
                        string mensaje = Encoding.ASCII.GetString(buffer, 0, bytesRecibidos).Trim();
                        Procesar_Mensaje_Servidor(mensaje);
                    }
                }
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error en la comunicación con el servidor: {ex.Message}", "Error de comunicación", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // Función que escucha al socket UDP
        private void Escuchar_Servidor_UDP()
        {
            // Verificar si el formulario se está cerrando
            if (isClosing)
                return;

            try
            {
                IPEndPoint remoteEP = new IPEndPoint(IPAddress.Any, 0);
                while (!cancelarEscucha)
                {

                    byte[] data = cliente_udp.Receive(ref remoteEP);
                    string mensaje = Encoding.ASCII.GetString(data).Trim();

                    Procesar_Mensaje_Servidor(mensaje);

                }
            }
            catch (SocketException ex)
            {
                MessageBox.Show($"Error en la comunicación con el servidor: {ex.Message}", "Error de comunicación", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            catch (ObjectDisposedException)
            {
                // El UdpClient ha sido cerrado
            }
        }

        private void Movimiento_Fondo(Point cursor)
        {
            float deltaX = cursor.X - jugador.X - (jugador.size / 2.0f);
            float deltaY = cursor.Y - jugador.Y - (jugador.size / 2.0f);

            float distancia = (float)Math.Sqrt(deltaX * deltaX + deltaY * deltaY);

            if ((distancia > 5.0f) && (cursor_over_box == true))
            {
                float direccionX = deltaX / distancia;
                float direccionY = deltaY / distancia;

                float movimientoFondoX = direccionX * jugador.velocidad;
                float movimientoFondoY = direccionY * jugador.velocidad;

                // Eje X
                if ((fondo_x + movimientoFondoX) < 0)
                {
                    this.block_left = true;
                }
                else if ((fondo_x + movimientoFondoX) > (fondo.Width - box_ancho))
                {
                    this.block_right = true;
                }

                if (this.block_left == true)
                {
                    if ((jugador.X + movimientoFondoX) <= 0)
                    {
                        jugador.X = 0;
                    }
                    else if ((jugador.X + movimientoFondoX) >= (box_ancho / 2 - jugador.size / 2))
                    {
                        jugador.X = box_ancho / 2 - jugador.size / 2;
                        this.block_left = false;
                    }
                    else
                    {
                        jugador.X += movimientoFondoX;
                    }
                }
                else if (this.block_right == true)
                {
                    if ((jugador.X + movimientoFondoX) >= (box_ancho - jugador.size))
                    {
                        jugador.X = (box_ancho - jugador.size);
                    }
                    else if ((jugador.X + movimientoFondoX) <= (box_ancho / 2 - jugador.size / 2))
                    {
                        jugador.X = box_ancho / 2 - jugador.size / 2;
                        this.block_right = false;
                    }
                    else
                    {
                        jugador.X += movimientoFondoX;
                    }
                }
                else
                {
                    jugador.X = box_ancho / 2 - jugador.size / 2;
                    fondo_x += movimientoFondoX;
                }

                // Eje Y
                if ((fondo_y + movimientoFondoY) < 0)
                {
                    this.block_up = true;
                }
                else if ((fondo_y + movimientoFondoY) > (fondo.Height - box_alto))
                {
                    this.block_down = true;
                }

                if (this.block_up == true)
                {
                    if ((jugador.Y + movimientoFondoY) <= 0)
                    {
                        jugador.Y = 0;
                    }
                    else if ((jugador.Y + movimientoFondoY) >= (box_alto / 2 - jugador.size / 2))
                    {
                        jugador.Y = box_alto / 2 - jugador.size / 2;
                        this.block_up = false;
                    }
                    else
                    {
                        jugador.Y += movimientoFondoY;
                    }
                }
                else if (this.block_down == true)
                {
                    if ((jugador.Y + movimientoFondoY) >= (box_alto - jugador.size))
                    {
                        jugador.Y = (box_alto - jugador.size);
                    }
                    else if ((jugador.Y + movimientoFondoY) <= (box_alto / 2 - jugador.size / 2))
                    {
                        jugador.Y = box_alto / 2 - jugador.size / 2;
                        this.block_down = false;
                    }
                    else
                    {
                        jugador.Y += movimientoFondoY;
                    }
                }
                else
                {
                    jugador.Y = box_alto / 2 - jugador.size / 2;
                    fondo_y += movimientoFondoY;
                }

                jugador.posicion_x = fondo_x + jugador.X;
                jugador.posicion_y = fondo_y + jugador.Y;
            }
        }

        private void Detectar_Proximidad()
        {
            int tolerance = 15;

            // Comprobar burbujas estáticas primero
            var keys = new List<int>(staticBubbles.Keys); // Evitar modificar durante enumeración

            foreach (var id in keys)
            {
                StaticBubble sb = staticBubbles[id];
                if (!sb.Visible)
                {
                    // Comprobar si han pasado 10 segundos desde que se comió
                    if ((DateTime.Now - sb.LastEatenTime).TotalSeconds >= sb.RespawnTime)
                    {
                        sb.Visible = true;
                        staticBubbles[id] = sb;
                    }
                    continue;
                }

                // Centro de la burbujita
                int sb_x = (int)sb.X + (sb.Size / 2);
                int sb_y = (int)sb.Y + (sb.Size / 2);

                // Centro de mi burbuja
                int j_x = (int)jugador.posicion_x + (jugador.size / 2);
                int j_y = (int)jugador.posicion_y + (jugador.size / 2);

                if ((Math.Abs(j_x - sb_x) <= tolerance) && (Math.Abs(j_y - sb_y) <= tolerance))
                {
                    // Me como la burbujita si soy mas grande que 0 (siempre)
                    jugador.size += sb.Size;
                    sb.Visible = false;
                    sb.LastEatenTime = DateTime.Now;
                    staticBubbles[id] = sb;
                    burbujasEstaticasComidas.Add(id);
                    Enviar_Actualizacion_Estado();
                }
            }

            if (!modoOffline)
            {
                lock (jugadores)
                {
                    List<string> aEliminar = new List<string>();

                    foreach (var kvp in jugadores)
                    {
                        string nombre_jugador = kvp.Key;
                        Burbuja otro_jugador = kvp.Value;

                        if (nombre_jugador != jugador.nombre)
                        {
                            // Coordenadas del centro del jugador local
                            float este_jugador_x = jugador.posicion_x + (jugador.size / 2f);
                            float este_jugador_y = jugador.posicion_y + (jugador.size / 2f);

                            // Coordenadas del centro del otro jugador
                            float otro_jugador_x = otro_jugador.posicion_x + (otro_jugador.size / 2f);
                            float otro_jugador_y = otro_jugador.posicion_y + (otro_jugador.size / 2f);

                            // Calcular distancia entre los centros
                            float dx = este_jugador_x - otro_jugador_x;
                            float dy = este_jugador_y - otro_jugador_y;
                            float distance = (float)Math.Sqrt(dx * dx + dy * dy);

                            // Suma de radios
                            float sumRadii = (jugador.size / 2f) + (otro_jugador.size / 2f);

                            if (distance < sumRadii)
                            {
                                if (otro_jugador.size < jugador.size)
                                {
                                    // El jugador actual es más grande, se come al otro
                                    jugador.size += otro_jugador.size;
                                    aEliminar.Add(nombre_jugador);
                                }
                            }
                        }
                    }

                    // Eliminar jugadores que han sido comidos
                    foreach (string nj in aEliminar)
                    {
                        jugadores_comidos.Add(nj);
                        jugadores.Remove(nj);
                        Enviar_Actualizacion_Estado();

                    }
                }
            }
            else
            {
                //Añadir algo mas para el offline
            }
        }

        void clock_Tick(object sender, EventArgs e)
        {
            // Verificar si el formulario se está cerrando
            if (isClosing)
                return;

            Point cursor = fondo_box.PointToClient(Cursor.Position);
            Movimiento_Fondo(cursor);
            Detectar_Proximidad();
            Detectar_Ganador();
            Enviar_Actualizacion_Posicion();
            Invalidate();
        }

        private void FormGame_Paint(object sender, PaintEventArgs e)
        {
            StringFormat tag_formato = new StringFormat();
            tag_formato.LineAlignment = StringAlignment.Center;
            tag_formato.Alignment = StringAlignment.Center;
            Font fuente = this.Font;
            Brush color = Brushes.Black;

            e.Graphics.TranslateTransform(this.origen.X, this.origen.Y);

            e.Graphics.DrawImage(this.fondo,
                new Rectangle(0, 0, this.box_ancho, this.box_alto),
                new Rectangle((int)this.fondo_x, (int)this.fondo_y, this.box_ancho, this.box_alto),
                GraphicsUnit.Pixel);

            // Dibujar burbujas estáticas
            foreach (var kvp in staticBubbles)
            {
                StaticBubble sb = kvp.Value;
                if (sb.Visible)
                {
                    int drawX = (int)(sb.X - fondo_x);
                    int drawY = (int)(sb.Y - fondo_y);
                    e.Graphics.FillEllipse(Brushes.LightBlue, drawX, drawY, sb.Size, sb.Size);
                }
            }

            if (clock.Enabled == true && this.estado == 'A')
            {
                Point tag_location = new Point(((int)jugador.X + (jugador.size / 2)), ((int)jugador.Y + jugador.size + 10));
                e.Graphics.DrawImage(burbuja_sprite, jugador.X, jugador.Y, jugador.size, jugador.size);
                e.Graphics.DrawString(jugador.nombre, fuente, color, tag_location.X, tag_location.Y, tag_formato);
            }

            lock (jugadores)
            {
                foreach (var kvp in jugadores)
                {
                    string tag_jugador = kvp.Key;
                    Burbuja otro_jugador = kvp.Value;

                    if (tag_jugador != jugador.nombre)
                    {
                        string sprite_model = $"burbuja_sprite_{otro_jugador.color}.png";
                        Image spriteOtroJugador = Image.FromFile(sprite_model);

                        Point posicion_relativa = new Point((int)(otro_jugador.posicion_x - fondo_x), (int)(otro_jugador.posicion_y - fondo_y));
                        Point tag_location_otro = new Point((posicion_relativa.X + (otro_jugador.size / 2)), (posicion_relativa.Y + otro_jugador.size + 10));
                        e.Graphics.DrawImage(spriteOtroJugador, posicion_relativa.X, posicion_relativa.Y, otro_jugador.size, otro_jugador.size);
                        e.Graphics.DrawString(otro_jugador.nombre, fuente, color, tag_location_otro.X, tag_location_otro.Y, tag_formato);
                    }
                }
            }
        }

        private void fondo_box_MouseEnter(object sender, EventArgs e)
        {
            if (clock.Enabled == true)
            {
                cursor_over_box = true;
            }
        }

        private void fondo_box_MouseLeave(object sender, EventArgs e)
        {
            if (clock.Enabled == true)
            {
                cursor_over_box = false;
            }
        }
        private void InicializarBurbujasEstaticas()
        {
            int numBurbujas = 100;
            float minDistance = 150f; // Distancia mínima entre burbujas
            Random rand = new Random();

            for (int i = 0; i < numBurbujas; i++)
            {
                bool positionFound = false;
                float x = 0, y = 0;

                while (!positionFound)
                {
                    x = (float)(rand.NextDouble() * fondo.Width);
                    y = (float)(rand.NextDouble() * fondo.Height);

                    positionFound = true;

                    foreach (var bubble in staticBubbles.Values)
                    {
                        float dx = bubble.X - x;
                        float dy = bubble.Y - y;
                        float distance = (float)Math.Sqrt(dx * dx + dy * dy);

                        if (distance < minDistance)
                        {
                            positionFound = false;
                            break;
                        }
                    }
                }

                StaticBubble sb = new StaticBubble()
                {
                    X = x,
                    Y = y,
                    Size = 10,
                    Visible = true,
                    RespawnTime = 10,
                    LastEatenTime = DateTime.MinValue
                };

                staticBubbles.Add(i, sb);
            }
        }

        private void FormGame_FormClosing(object sender, FormClosingEventArgs e)
        {
            // Indicar que el formulario se está cerrando
            isClosing = true;

            // Detener y deshabilitar el clock
            if (clock != null)
            {
                clock.Enabled = false;
                clock.Tick -= clock_Tick;
                // No es necesario llamar a clock.Dispose() aquí
            }

            // Indicar al hilo de escucha que debe detenerse
            cancelarEscucha = true;

            // Solo enviar mensaje si NO estamos en modo offline
            if (!modoOffline && server != null)
            {
                string mensaje = "SALIR_PARTIDA2";
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

            // Si estamos en modo offline, volver al formulario de inicio de sesión
            if (modoOffline)
            {

            }

        }
    }

    // Estructura para las burbujas estáticas
    public struct StaticBubble
    {
        public float X;
        public float Y;
        public int Size;
        public bool Visible;
        public double RespawnTime; // Segundos para reaparecer
        public DateTime LastEatenTime;
    }

}

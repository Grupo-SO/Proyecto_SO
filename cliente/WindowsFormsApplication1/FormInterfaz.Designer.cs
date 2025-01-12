namespace WindowsFormsApplication1
{
    partial class FormInterfaz
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.chat_picturebox = new System.Windows.Forms.PictureBox();
            this.dataGridViewJugadores = new System.Windows.Forms.DataGridView();
            this.NombreJugador = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.playButton = new System.Windows.Forms.Button();
            this.send_button = new System.Windows.Forms.Button();
            this.chat_textbox = new System.Windows.Forms.TextBox();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.Menu = new System.Windows.Forms.ToolStripMenuItem();
            this.abandonarSalaToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.historialDePartidasToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.eliminarCuentaToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.contextMenuStripInvitar = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.invitarToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.dataGridViewPartidas = new System.Windows.Forms.DataGridView();
            this.Partida = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Fecha = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.contextMenuStripPartidas = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.verResultadosStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.verGanadorStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.chat_picturebox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewJugadores)).BeginInit();
            this.menuStrip1.SuspendLayout();
            this.contextMenuStripInvitar.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewPartidas)).BeginInit();
            this.contextMenuStripPartidas.SuspendLayout();
            this.SuspendLayout();
            // 
            // chat_picturebox
            // 
            this.chat_picturebox.BackColor = System.Drawing.Color.White;
            this.chat_picturebox.Location = new System.Drawing.Point(940, 81);
            this.chat_picturebox.Margin = new System.Windows.Forms.Padding(8);
            this.chat_picturebox.Name = "chat_picturebox";
            this.chat_picturebox.Size = new System.Drawing.Size(400, 535);
            this.chat_picturebox.TabIndex = 6;
            this.chat_picturebox.TabStop = false;
            // 
            // dataGridViewJugadores
            // 
            this.dataGridViewJugadores.AllowUserToAddRows = false;
            this.dataGridViewJugadores.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            this.dataGridViewJugadores.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewJugadores.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.NombreJugador});
            this.dataGridViewJugadores.Location = new System.Drawing.Point(59, 424);
            this.dataGridViewJugadores.Margin = new System.Windows.Forms.Padding(4, 2, 4, 2);
            this.dataGridViewJugadores.Name = "dataGridViewJugadores";
            this.dataGridViewJugadores.ReadOnly = true;
            this.dataGridViewJugadores.RowHeadersWidth = 102;
            this.dataGridViewJugadores.RowTemplate.Height = 40;
            this.dataGridViewJugadores.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewJugadores.Size = new System.Drawing.Size(344, 327);
            this.dataGridViewJugadores.TabIndex = 5;
            // 
            // NombreJugador
            // 
            this.NombreJugador.HeaderText = "Nombre del Jugador";
            this.NombreJugador.MinimumWidth = 12;
            this.NombreJugador.Name = "NombreJugador";
            this.NombreJugador.ReadOnly = true;
            // 
            // playButton
            // 
            this.playButton.BackColor = System.Drawing.Color.Red;
            this.playButton.Location = new System.Drawing.Point(563, 540);
            this.playButton.Margin = new System.Windows.Forms.Padding(4, 6, 4, 6);
            this.playButton.Name = "playButton";
            this.playButton.Size = new System.Drawing.Size(210, 76);
            this.playButton.TabIndex = 0;
            this.playButton.Text = "Play";
            this.playButton.UseVisualStyleBackColor = false;
            this.playButton.Click += new System.EventHandler(this.playButton_Click);
            // 
            // send_button
            // 
            this.send_button.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(0)))));
            this.send_button.Location = new System.Drawing.Point(1284, 665);
            this.send_button.Margin = new System.Windows.Forms.Padding(8);
            this.send_button.Name = "send_button";
            this.send_button.Size = new System.Drawing.Size(56, 48);
            this.send_button.TabIndex = 7;
            this.send_button.UseVisualStyleBackColor = false;
            this.send_button.Click += new System.EventHandler(this.send_button_Click);
            // 
            // chat_textbox
            // 
            this.chat_textbox.Location = new System.Drawing.Point(940, 665);
            this.chat_textbox.Margin = new System.Windows.Forms.Padding(8);
            this.chat_textbox.Name = "chat_textbox";
            this.chat_textbox.Size = new System.Drawing.Size(320, 38);
            this.chat_textbox.TabIndex = 8;
            this.chat_textbox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.chat_textbox_KeyDown);
            // 
            // menuStrip1
            // 
            this.menuStrip1.GripMargin = new System.Windows.Forms.Padding(2, 2, 0, 2);
            this.menuStrip1.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.Menu});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Padding = new System.Windows.Forms.Padding(12, 4, 0, 4);
            this.menuStrip1.Size = new System.Drawing.Size(1476, 56);
            this.menuStrip1.TabIndex = 10;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // Menu
            // 
            this.Menu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.abandonarSalaToolStripMenuItem,
            this.historialDePartidasToolStripMenuItem,
            this.eliminarCuentaToolStripMenuItem});
            this.Menu.Name = "Menu";
            this.Menu.Size = new System.Drawing.Size(218, 48);
            this.Menu.Text = "Menu ajustes";
            // 
            // abandonarSalaToolStripMenuItem
            // 
            this.abandonarSalaToolStripMenuItem.Name = "abandonarSalaToolStripMenuItem";
            this.abandonarSalaToolStripMenuItem.Size = new System.Drawing.Size(448, 54);
            this.abandonarSalaToolStripMenuItem.Text = "Abandonar sala";
            this.abandonarSalaToolStripMenuItem.Click += new System.EventHandler(this.abandonarSalaToolStripMenuItem_Click);
            // 
            // historialDePartidasToolStripMenuItem
            // 
            this.historialDePartidasToolStripMenuItem.Name = "historialDePartidasToolStripMenuItem";
            this.historialDePartidasToolStripMenuItem.Size = new System.Drawing.Size(448, 54);
            this.historialDePartidasToolStripMenuItem.Text = "Historial de partidas";
            this.historialDePartidasToolStripMenuItem.Click += new System.EventHandler(this.historialDePartidasToolStripMenuItem_Click);
            // 
            // eliminarCuentaToolStripMenuItem
            // 
            this.eliminarCuentaToolStripMenuItem.Name = "eliminarCuentaToolStripMenuItem";
            this.eliminarCuentaToolStripMenuItem.Size = new System.Drawing.Size(448, 54);
            this.eliminarCuentaToolStripMenuItem.Text = "Eliminar cuenta ";
            this.eliminarCuentaToolStripMenuItem.Click += new System.EventHandler(this.eliminarCuentaToolStripMenuItem_Click);
            // 
            // contextMenuStripInvitar
            // 
            this.contextMenuStripInvitar.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.contextMenuStripInvitar.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.invitarToolStripMenuItem});
            this.contextMenuStripInvitar.Name = "invitatToolStripMenuItem";
            this.contextMenuStripInvitar.Size = new System.Drawing.Size(178, 52);
            // 
            // invitarToolStripMenuItem
            // 
            this.invitarToolStripMenuItem.Name = "invitarToolStripMenuItem";
            this.invitarToolStripMenuItem.Size = new System.Drawing.Size(177, 48);
            this.invitarToolStripMenuItem.Text = "Invitar";
            this.invitarToolStripMenuItem.Click += new System.EventHandler(this.invitarToolStripMenuItem_Click);
            // 
            // dataGridViewPartidas
            // 
            this.dataGridViewPartidas.AllowUserToAddRows = false;
            this.dataGridViewPartidas.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            this.dataGridViewPartidas.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewPartidas.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Partida,
            this.Fecha});
            this.dataGridViewPartidas.ContextMenuStrip = this.contextMenuStripPartidas;
            this.dataGridViewPartidas.Location = new System.Drawing.Point(59, 81);
            this.dataGridViewPartidas.Margin = new System.Windows.Forms.Padding(4, 2, 4, 2);
            this.dataGridViewPartidas.Name = "dataGridViewPartidas";
            this.dataGridViewPartidas.ReadOnly = true;
            this.dataGridViewPartidas.RowHeadersWidth = 102;
            this.dataGridViewPartidas.RowTemplate.Height = 40;
            this.dataGridViewPartidas.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewPartidas.Size = new System.Drawing.Size(539, 302);
            this.dataGridViewPartidas.TabIndex = 11;
            this.dataGridViewPartidas.MouseDown += new System.Windows.Forms.MouseEventHandler(this.dataGridViewPartidas_MouseDown);
            // 
            // Partida
            // 
            this.Partida.HeaderText = "Partida";
            this.Partida.MinimumWidth = 12;
            this.Partida.Name = "Partida";
            this.Partida.ReadOnly = true;
            // 
            // Fecha
            // 
            this.Fecha.HeaderText = "Fecha";
            this.Fecha.MinimumWidth = 12;
            this.Fecha.Name = "Fecha";
            this.Fecha.ReadOnly = true;
            // 
            // contextMenuStripPartidas
            // 
            this.contextMenuStripPartidas.ImageScalingSize = new System.Drawing.Size(40, 40);
            this.contextMenuStripPartidas.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.verResultadosStripMenuItem,
            this.verGanadorStripMenuItem});
            this.contextMenuStripPartidas.Name = "contextMenuStripPartidas";
            this.contextMenuStripPartidas.Size = new System.Drawing.Size(285, 100);
            // 
            // verResultadosStripMenuItem
            // 
            this.verResultadosStripMenuItem.Name = "verResultadosStripMenuItem";
            this.verResultadosStripMenuItem.Size = new System.Drawing.Size(360, 48);
            this.verResultadosStripMenuItem.Text = "Ver resultados";
            this.verResultadosStripMenuItem.Click += new System.EventHandler(this.verResultadosStripMenuItem_Click);
            // 
            // verGanadorStripMenuItem
            // 
            this.verGanadorStripMenuItem.Name = "verGanadorStripMenuItem";
            this.verGanadorStripMenuItem.Size = new System.Drawing.Size(360, 48);
            this.verGanadorStripMenuItem.Text = "Ver ganador";
            this.verGanadorStripMenuItem.Click += new System.EventHandler(this.verGanadorStripMenuItem_Click);
            // 
            // FormInterfaz
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(16F, 31F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1476, 790);
            this.Controls.Add(this.dataGridViewPartidas);
            this.Controls.Add(this.chat_textbox);
            this.Controls.Add(this.send_button);
            this.Controls.Add(this.dataGridViewJugadores);
            this.Controls.Add(this.playButton);
            this.Controls.Add(this.chat_picturebox);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Margin = new System.Windows.Forms.Padding(4, 6, 4, 6);
            this.Name = "FormInterfaz";
            this.Text = "FormInterfaz";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FormInterfaz_FormClosing);
            this.Load += new System.EventHandler(this.FormInterfaz_Load);
            ((System.ComponentModel.ISupportInitialize)(this.chat_picturebox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewJugadores)).EndInit();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.contextMenuStripInvitar.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewPartidas)).EndInit();
            this.contextMenuStripPartidas.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.PictureBox chat_picturebox;
        private System.Windows.Forms.DataGridView dataGridViewJugadores;
        private System.Windows.Forms.DataGridViewTextBoxColumn NombreJugador;
        private System.Windows.Forms.Button playButton;
        private System.Windows.Forms.Button send_button;
        private System.Windows.Forms.TextBox chat_textbox;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem Menu;
        private System.Windows.Forms.ToolStripMenuItem abandonarSalaToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem historialDePartidasToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem eliminarCuentaToolStripMenuItem;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripInvitar;
        private System.Windows.Forms.ToolStripMenuItem invitarToolStripMenuItem;
        private System.Windows.Forms.DataGridView dataGridViewPartidas;
        private System.Windows.Forms.DataGridViewTextBoxColumn Partida;
        private System.Windows.Forms.DataGridViewTextBoxColumn Fecha;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripPartidas;
        private System.Windows.Forms.ToolStripMenuItem verResultadosStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem verGanadorStripMenuItem;
    }
}
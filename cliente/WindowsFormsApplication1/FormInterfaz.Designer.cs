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
            this.contextMenuStripInvitar = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.invitarToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.chat_picturebox = new System.Windows.Forms.PictureBox();
            this.dataGridViewJugadores = new System.Windows.Forms.DataGridView();
            this.NombreJugador = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.button5 = new System.Windows.Forms.Button();
            this.playButton = new System.Windows.Forms.Button();
            this.send_button = new System.Windows.Forms.Button();
            this.chat_textbox = new System.Windows.Forms.TextBox();
            this.contextMenuStripInvitar.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.chat_picturebox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewJugadores)).BeginInit();
            this.SuspendLayout();
            // 
            // contextMenuStripInvitar
            // 
            this.contextMenuStripInvitar.ImageScalingSize = new System.Drawing.Size(40, 40);
            this.contextMenuStripInvitar.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.invitarToolStripMenuItem});
            this.contextMenuStripInvitar.Name = "contextMenuStripInvite";
            this.contextMenuStripInvitar.Size = new System.Drawing.Size(178, 52);
            // 
            // invitarToolStripMenuItem
            // 
            this.invitarToolStripMenuItem.Name = "invitarToolStripMenuItem";
            this.invitarToolStripMenuItem.Size = new System.Drawing.Size(177, 48);
            this.invitarToolStripMenuItem.Text = "Invitar";
            this.invitarToolStripMenuItem.Click += new System.EventHandler(this.invitarToolStripMenuItem_Click);
            // 
            // chat_picturebox
            // 
            this.chat_picturebox.BackColor = System.Drawing.Color.White;
            this.chat_picturebox.Location = new System.Drawing.Point(941, 29);
            this.chat_picturebox.Margin = new System.Windows.Forms.Padding(8, 7, 8, 7);
            this.chat_picturebox.Name = "chat_picturebox";
            this.chat_picturebox.Size = new System.Drawing.Size(397, 587);
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
            this.dataGridViewJugadores.ContextMenuStrip = this.contextMenuStripInvitar;
            this.dataGridViewJugadores.Location = new System.Drawing.Point(27, 188);
            this.dataGridViewJugadores.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.dataGridViewJugadores.Name = "dataGridViewJugadores";
            this.dataGridViewJugadores.ReadOnly = true;
            this.dataGridViewJugadores.RowHeadersWidth = 102;
            this.dataGridViewJugadores.RowTemplate.Height = 40;
            this.dataGridViewJugadores.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewJugadores.Size = new System.Drawing.Size(344, 427);
            this.dataGridViewJugadores.TabIndex = 5;
            // 
            // NombreJugador
            // 
            this.NombreJugador.HeaderText = "Nombre del Jugador";
            this.NombreJugador.MinimumWidth = 12;
            this.NombreJugador.Name = "NombreJugador";
            this.NombreJugador.ReadOnly = true;
            // 
            // button5
            // 
            this.button5.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.button5.Location = new System.Drawing.Point(64, 29);
            this.button5.Margin = new System.Windows.Forms.Padding(5, 5, 5, 5);
            this.button5.Name = "button5";
            this.button5.Size = new System.Drawing.Size(269, 103);
            this.button5.TabIndex = 4;
            this.button5.Text = "Settings";
            this.button5.UseVisualStyleBackColor = false;
            // 
            // playButton
            // 
            this.playButton.BackColor = System.Drawing.Color.Red;
            this.playButton.Location = new System.Drawing.Point(525, 506);
            this.playButton.Margin = new System.Windows.Forms.Padding(5, 5, 5, 5);
            this.playButton.Name = "playButton";
            this.playButton.Size = new System.Drawing.Size(269, 110);
            this.playButton.TabIndex = 0;
            this.playButton.Text = "Play";
            this.playButton.UseVisualStyleBackColor = false;
            this.playButton.Click += new System.EventHandler(this.playButton_Click);
            // 
            // send_button
            // 
            this.send_button.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(0)))));
            this.send_button.Location = new System.Drawing.Point(1283, 665);
            this.send_button.Margin = new System.Windows.Forms.Padding(8, 7, 8, 7);
            this.send_button.Name = "send_button";
            this.send_button.Size = new System.Drawing.Size(56, 48);
            this.send_button.TabIndex = 7;
            this.send_button.UseVisualStyleBackColor = false;
            this.send_button.Click += new System.EventHandler(this.send_button_Click);
            // 
            // chat_textbox
            // 
            this.chat_textbox.Location = new System.Drawing.Point(941, 665);
            this.chat_textbox.Margin = new System.Windows.Forms.Padding(8, 7, 8, 7);
            this.chat_textbox.Name = "chat_textbox";
            this.chat_textbox.Size = new System.Drawing.Size(319, 38);
            this.chat_textbox.TabIndex = 8;
            // 
            // FormInterfaz
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(16F, 31F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1371, 749);
            this.Controls.Add(this.chat_textbox);
            this.Controls.Add(this.send_button);
            this.Controls.Add(this.dataGridViewJugadores);
            this.Controls.Add(this.button5);
            this.Controls.Add(this.playButton);
            this.Controls.Add(this.chat_picturebox);
            this.Margin = new System.Windows.Forms.Padding(5, 5, 5, 5);
            this.Name = "FormInterfaz";
            this.Text = "FormInterfaz";
            this.Load += new System.EventHandler(this.FormInterfaz_Load);
            this.contextMenuStripInvitar.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.chat_picturebox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewJugadores)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.ContextMenuStrip contextMenuStripInvitar;
        private System.Windows.Forms.ToolStripMenuItem invitarToolStripMenuItem;
        private System.Windows.Forms.PictureBox chat_picturebox;
        private System.Windows.Forms.DataGridView dataGridViewJugadores;
        private System.Windows.Forms.DataGridViewTextBoxColumn NombreJugador;
        private System.Windows.Forms.Button button5;
        private System.Windows.Forms.Button playButton;
        private System.Windows.Forms.Button send_button;
        private System.Windows.Forms.TextBox chat_textbox;
    }
}
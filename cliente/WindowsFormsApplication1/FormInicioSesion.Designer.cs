namespace WindowsFormsApplication1
{
    partial class FormInicioSesion
    {
        /// <summary>
        /// Variable del diseñador requerida.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Limpiar los recursos que se estén utilizando.
        /// </summary>
        /// <param name="disposing">true si los recursos administrados se deben eliminar; false en caso contrario, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Código generado por el Diseñador de Windows Forms

        /// <summary>
        /// Método necesario para admitir el Diseñador. No se puede modificar
        /// el contenido del método con el editor de código.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.usuarioTextBox = new System.Windows.Forms.TextBox();
            this.entrarButton = new System.Windows.Forms.Button();
            this.crearusuarioButton = new System.Windows.Forms.Button();
            this.contraseñaTextBox = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.panel1 = new System.Windows.Forms.Panel();
            this.label3 = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.contar_usuariosButton = new System.Windows.Forms.Button();
            this.pass_olvidadaButton = new System.Windows.Forms.Button();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.label1.Location = new System.Drawing.Point(3, 24);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(193, 25);
            this.label1.TabIndex = 0;
            this.label1.Text = "IP: 192.168.56.102";
            // 
            // usuarioTextBox
            // 
            this.usuarioTextBox.BackColor = System.Drawing.Color.LightGray;
            this.usuarioTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.900001F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.usuarioTextBox.ForeColor = System.Drawing.SystemColors.WindowText;
            this.usuarioTextBox.Location = new System.Drawing.Point(336, 110);
            this.usuarioTextBox.Name = "usuarioTextBox";
            this.usuarioTextBox.Size = new System.Drawing.Size(282, 22);
            this.usuarioTextBox.TabIndex = 3;
            this.usuarioTextBox.Text = "Ingresar usuario";
            // 
            // entrarButton
            // 
            this.entrarButton.BackColor = System.Drawing.Color.SkyBlue;
            this.entrarButton.FlatAppearance.BorderColor = System.Drawing.Color.Blue;
            this.entrarButton.FlatAppearance.BorderSize = 0;
            this.entrarButton.FlatAppearance.MouseDownBackColor = System.Drawing.Color.Blue;
            this.entrarButton.FlatAppearance.MouseOverBackColor = System.Drawing.Color.Blue;
            this.entrarButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.entrarButton.ForeColor = System.Drawing.SystemColors.WindowText;
            this.entrarButton.Location = new System.Drawing.Point(450, 241);
            this.entrarButton.Name = "entrarButton";
            this.entrarButton.Size = new System.Drawing.Size(168, 27);
            this.entrarButton.TabIndex = 9;
            this.entrarButton.Text = "ACCEDER";
            this.entrarButton.UseVisualStyleBackColor = false;
            this.entrarButton.Click += new System.EventHandler(this.entrarButton_Click);
            // 
            // crearusuarioButton
            // 
            this.crearusuarioButton.BackColor = System.Drawing.Color.SkyBlue;
            this.crearusuarioButton.FlatAppearance.BorderColor = System.Drawing.Color.Blue;
            this.crearusuarioButton.FlatAppearance.BorderSize = 0;
            this.crearusuarioButton.FlatAppearance.MouseDownBackColor = System.Drawing.Color.Blue;
            this.crearusuarioButton.FlatAppearance.MouseOverBackColor = System.Drawing.Color.Blue;
            this.crearusuarioButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.crearusuarioButton.ForeColor = System.Drawing.SystemColors.WindowText;
            this.crearusuarioButton.Location = new System.Drawing.Point(209, 241);
            this.crearusuarioButton.Name = "crearusuarioButton";
            this.crearusuarioButton.Size = new System.Drawing.Size(186, 27);
            this.crearusuarioButton.TabIndex = 8;
            this.crearusuarioButton.Text = "NUEVO USUARIO";
            this.crearusuarioButton.UseVisualStyleBackColor = false;
            this.crearusuarioButton.Click += new System.EventHandler(this.crearusuarioButton_Click);
            // 
            // contraseñaTextBox
            // 
            this.contraseñaTextBox.BackColor = System.Drawing.Color.LightGray;
            this.contraseñaTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.900001F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.contraseñaTextBox.ForeColor = System.Drawing.SystemColors.WindowText;
            this.contraseñaTextBox.Location = new System.Drawing.Point(336, 150);
            this.contraseñaTextBox.Name = "contraseñaTextBox";
            this.contraseñaTextBox.Size = new System.Drawing.Size(282, 22);
            this.contraseñaTextBox.TabIndex = 7;
            this.contraseñaTextBox.Tag = "";
            this.contraseñaTextBox.Text = "Ingresar contraseña";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.ForeColor = System.Drawing.Color.Black;
            this.label2.Location = new System.Drawing.Point(331, 24);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(169, 26);
            this.label2.TabIndex = 11;
            this.label2.Text = "INICIO SESIÓN";
            // 
            // panel1
            // 
            this.panel1.BackColor = System.Drawing.Color.SteelBlue;
            this.panel1.Controls.Add(this.label3);
            this.panel1.Controls.Add(this.label1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Left;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Margin = new System.Windows.Forms.Padding(1);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(195, 386);
            this.panel1.TabIndex = 12;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.label3.Location = new System.Drawing.Point(3, 302);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(186, 25);
            this.label3.TabIndex = 1;
            this.label3.Text = "Player code: 7733";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = global::WindowsFormsApplication1.Properties.Resources.logo;
            this.pictureBox1.Location = new System.Drawing.Point(209, 87);
            this.pictureBox1.Margin = new System.Windows.Forms.Padding(1);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(106, 107);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox1.TabIndex = 13;
            this.pictureBox1.TabStop = false;
            // 
            // contar_usuariosButton
            // 
            this.contar_usuariosButton.Location = new System.Drawing.Point(233, 285);
            this.contar_usuariosButton.Margin = new System.Windows.Forms.Padding(1);
            this.contar_usuariosButton.Name = "contar_usuariosButton";
            this.contar_usuariosButton.Size = new System.Drawing.Size(132, 36);
            this.contar_usuariosButton.TabIndex = 14;
            this.contar_usuariosButton.Text = "ContarUsuarios";
            this.contar_usuariosButton.UseVisualStyleBackColor = true;
            this.contar_usuariosButton.Click += new System.EventHandler(this.contar_usuariosButton_Click);
            // 
            // pass_olvidadaButton
            // 
            this.pass_olvidadaButton.Location = new System.Drawing.Point(469, 285);
            this.pass_olvidadaButton.Margin = new System.Windows.Forms.Padding(1);
            this.pass_olvidadaButton.Name = "pass_olvidadaButton";
            this.pass_olvidadaButton.Size = new System.Drawing.Size(128, 36);
            this.pass_olvidadaButton.TabIndex = 15;
            this.pass_olvidadaButton.Text = "¿Contraseña olvidada?";
            this.pass_olvidadaButton.UseVisualStyleBackColor = true;
            this.pass_olvidadaButton.Click += new System.EventHandler(this.pass_olvidadaButton_Click);
            // 
            // FormInicioSesion
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(630, 386);
            this.Controls.Add(this.pass_olvidadaButton);
            this.Controls.Add(this.contar_usuariosButton);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.entrarButton);
            this.Controls.Add(this.crearusuarioButton);
            this.Controls.Add(this.contraseñaTextBox);
            this.Controls.Add(this.usuarioTextBox);
            this.Name = "FormInicioSesion";
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox usuarioTextBox;
        private System.Windows.Forms.TextBox contraseñaTextBox;
        private System.Windows.Forms.Button crearusuarioButton;
        private System.Windows.Forms.Button entrarButton;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Button contar_usuariosButton;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button pass_olvidadaButton;
    }
}


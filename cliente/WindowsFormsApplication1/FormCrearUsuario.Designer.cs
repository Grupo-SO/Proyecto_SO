namespace WindowsFormsApplication1
{
    partial class FormCrearUsuario
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
            this.guardarButton = new System.Windows.Forms.Button();
            this.creaUsuarioText = new System.Windows.Forms.Label();
            this.inserta_usuarioTextBox = new System.Windows.Forms.TextBox();
            this.creaContraseñaText = new System.Windows.Forms.Label();
            this.inserta_contraseñaTextBox = new System.Windows.Forms.TextBox();
            this.CrearBDButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // guardarButton
            // 
            this.guardarButton.Location = new System.Drawing.Point(102, 175);
            this.guardarButton.Margin = new System.Windows.Forms.Padding(1);
            this.guardarButton.Name = "guardarButton";
            this.guardarButton.Size = new System.Drawing.Size(88, 35);
            this.guardarButton.TabIndex = 0;
            this.guardarButton.Text = "Guardar";
            this.guardarButton.UseVisualStyleBackColor = true;
            this.guardarButton.Click += new System.EventHandler(this.guardarButton_Click);
            // 
            // creaUsuarioText
            // 
            this.creaUsuarioText.AutoSize = true;
            this.creaUsuarioText.Location = new System.Drawing.Point(28, 34);
            this.creaUsuarioText.Margin = new System.Windows.Forms.Padding(1, 0, 1, 0);
            this.creaUsuarioText.Name = "creaUsuarioText";
            this.creaUsuarioText.Size = new System.Drawing.Size(68, 13);
            this.creaUsuarioText.TabIndex = 1;
            this.creaUsuarioText.Text = "Crea Usuario";
            // 
            // inserta_usuarioTextBox
            // 
            this.inserta_usuarioTextBox.Location = new System.Drawing.Point(153, 34);
            this.inserta_usuarioTextBox.Margin = new System.Windows.Forms.Padding(1);
            this.inserta_usuarioTextBox.Name = "inserta_usuarioTextBox";
            this.inserta_usuarioTextBox.Size = new System.Drawing.Size(124, 20);
            this.inserta_usuarioTextBox.TabIndex = 2;
            this.inserta_usuarioTextBox.Text = "Ex: Jugador1234";
            // 
            // creaContraseñaText
            // 
            this.creaContraseñaText.AutoSize = true;
            this.creaContraseñaText.Location = new System.Drawing.Point(28, 78);
            this.creaContraseñaText.Margin = new System.Windows.Forms.Padding(1, 0, 1, 0);
            this.creaContraseñaText.Name = "creaContraseñaText";
            this.creaContraseñaText.Size = new System.Drawing.Size(86, 13);
            this.creaContraseñaText.TabIndex = 3;
            this.creaContraseñaText.Text = "Crea Contraseña";
            // 
            // inserta_contraseñaTextBox
            // 
            this.inserta_contraseñaTextBox.Location = new System.Drawing.Point(153, 75);
            this.inserta_contraseñaTextBox.Margin = new System.Windows.Forms.Padding(1);
            this.inserta_contraseñaTextBox.Name = "inserta_contraseñaTextBox";
            this.inserta_contraseñaTextBox.Size = new System.Drawing.Size(124, 20);
            this.inserta_contraseñaTextBox.TabIndex = 4;
            // 
            // CrearBDButton
            // 
            this.CrearBDButton.Location = new System.Drawing.Point(102, 121);
            this.CrearBDButton.Margin = new System.Windows.Forms.Padding(1);
            this.CrearBDButton.Name = "CrearBDButton";
            this.CrearBDButton.Size = new System.Drawing.Size(88, 35);
            this.CrearBDButton.TabIndex = 5;
            this.CrearBDButton.Text = "CrearBD";
            this.CrearBDButton.UseVisualStyleBackColor = true;
            this.CrearBDButton.Click += new System.EventHandler(this.CrearBDButton_Click);
            // 
            // FormCrearUsuario
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(312, 220);
            this.Controls.Add(this.CrearBDButton);
            this.Controls.Add(this.inserta_contraseñaTextBox);
            this.Controls.Add(this.creaContraseñaText);
            this.Controls.Add(this.inserta_usuarioTextBox);
            this.Controls.Add(this.creaUsuarioText);
            this.Controls.Add(this.guardarButton);
            this.Margin = new System.Windows.Forms.Padding(1);
            this.Name = "FormCrearUsuario";
            this.Text = "FormCrearUsuario";
            this.Load += new System.EventHandler(this.FormCrearUsuario_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button guardarButton;
        private System.Windows.Forms.Label creaUsuarioText;
        private System.Windows.Forms.TextBox inserta_usuarioTextBox;
        private System.Windows.Forms.Label creaContraseñaText;
        private System.Windows.Forms.TextBox inserta_contraseñaTextBox;
        private System.Windows.Forms.Button CrearBDButton;
    }
}
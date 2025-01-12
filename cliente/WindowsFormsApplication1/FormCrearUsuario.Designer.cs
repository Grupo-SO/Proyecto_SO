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
            this.SuspendLayout();
            // 
            // guardarButton
            // 
            this.guardarButton.Location = new System.Drawing.Point(273, 357);
            this.guardarButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.guardarButton.Name = "guardarButton";
            this.guardarButton.Size = new System.Drawing.Size(234, 83);
            this.guardarButton.TabIndex = 0;
            this.guardarButton.Text = "Guardar";
            this.guardarButton.UseVisualStyleBackColor = true;
            this.guardarButton.Click += new System.EventHandler(this.guardarButton_Click);
            // 
            // creaUsuarioText
            // 
            this.creaUsuarioText.AutoSize = true;
            this.creaUsuarioText.Location = new System.Drawing.Point(74, 81);
            this.creaUsuarioText.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.creaUsuarioText.Name = "creaUsuarioText";
            this.creaUsuarioText.Size = new System.Drawing.Size(180, 32);
            this.creaUsuarioText.TabIndex = 1;
            this.creaUsuarioText.Text = "Crea Usuario";
            // 
            // inserta_usuarioTextBox
            // 
            this.inserta_usuarioTextBox.Location = new System.Drawing.Point(408, 81);
            this.inserta_usuarioTextBox.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.inserta_usuarioTextBox.Name = "inserta_usuarioTextBox";
            this.inserta_usuarioTextBox.Size = new System.Drawing.Size(324, 38);
            this.inserta_usuarioTextBox.TabIndex = 2;
            // 
            // creaContraseñaText
            // 
            this.creaContraseñaText.AutoSize = true;
            this.creaContraseñaText.Location = new System.Drawing.Point(74, 186);
            this.creaContraseñaText.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.creaContraseñaText.Name = "creaContraseñaText";
            this.creaContraseñaText.Size = new System.Drawing.Size(229, 32);
            this.creaContraseñaText.TabIndex = 3;
            this.creaContraseñaText.Text = "Crea Contraseña";
            // 
            // inserta_contraseñaTextBox
            // 
            this.inserta_contraseñaTextBox.Location = new System.Drawing.Point(408, 178);
            this.inserta_contraseñaTextBox.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.inserta_contraseñaTextBox.Name = "inserta_contraseñaTextBox";
            this.inserta_contraseñaTextBox.Size = new System.Drawing.Size(324, 38);
            this.inserta_contraseñaTextBox.TabIndex = 4;
            // 
            // FormCrearUsuario
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(16F, 31F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(832, 525);
            this.Controls.Add(this.inserta_contraseñaTextBox);
            this.Controls.Add(this.creaContraseñaText);
            this.Controls.Add(this.inserta_usuarioTextBox);
            this.Controls.Add(this.creaUsuarioText);
            this.Controls.Add(this.guardarButton);
            this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
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
    }
}
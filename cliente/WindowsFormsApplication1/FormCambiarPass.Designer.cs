namespace WindowsFormsApplication1
{
    partial class FormCambiarPass
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
            this.NewUsertextBox1 = new System.Windows.Forms.TextBox();
            this.PlayerCodetextBox2 = new System.Windows.Forms.TextBox();
            this.NewPasstextBox3 = new System.Windows.Forms.TextBox();
            this.ConfirmarButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // NewUsertextBox1
            // 
            this.NewUsertextBox1.Location = new System.Drawing.Point(149, 80);
            this.NewUsertextBox1.Name = "NewUsertextBox1";
            this.NewUsertextBox1.Size = new System.Drawing.Size(621, 38);
            this.NewUsertextBox1.TabIndex = 0;
            this.NewUsertextBox1.Text = "Ingrese usuario:";
            // 
            // PlayerCodetextBox2
            // 
            this.PlayerCodetextBox2.Location = new System.Drawing.Point(149, 274);
            this.PlayerCodetextBox2.Name = "PlayerCodetextBox2";
            this.PlayerCodetextBox2.Size = new System.Drawing.Size(621, 38);
            this.PlayerCodetextBox2.TabIndex = 1;
            this.PlayerCodetextBox2.Text = "Ingrese player code:";
            // 
            // NewPasstextBox3
            // 
            this.NewPasstextBox3.Location = new System.Drawing.Point(149, 184);
            this.NewPasstextBox3.Name = "NewPasstextBox3";
            this.NewPasstextBox3.Size = new System.Drawing.Size(621, 38);
            this.NewPasstextBox3.TabIndex = 2;
            this.NewPasstextBox3.Text = "Ingrese nueva contraseña:";
            // 
            // ConfirmarButton
            // 
            this.ConfirmarButton.Location = new System.Drawing.Point(289, 416);
            this.ConfirmarButton.Name = "ConfirmarButton";
            this.ConfirmarButton.Size = new System.Drawing.Size(230, 76);
            this.ConfirmarButton.TabIndex = 3;
            this.ConfirmarButton.Text = "Confirmar";
            this.ConfirmarButton.UseVisualStyleBackColor = true;
            this.ConfirmarButton.Click += new System.EventHandler(this.ConfirmarButton_Click);
            // 
            // FormCambiarPass
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(16F, 31F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(833, 524);
            this.Controls.Add(this.ConfirmarButton);
            this.Controls.Add(this.NewPasstextBox3);
            this.Controls.Add(this.PlayerCodetextBox2);
            this.Controls.Add(this.NewUsertextBox1);
            this.Name = "FormCambiarPass";
            this.Text = "FormCambiarPass";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox NewUsertextBox1;
        private System.Windows.Forms.TextBox PlayerCodetextBox2;
        private System.Windows.Forms.TextBox NewPasstextBox3;
        private System.Windows.Forms.Button ConfirmarButton;
    }
}
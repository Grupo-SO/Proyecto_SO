namespace WindowsFormsApplication1
{
    partial class FormGame
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
            this.fondo_box = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.fondo_box)).BeginInit();
            this.SuspendLayout();
            // 
            // fondo_box
            // 
            this.fondo_box.Location = new System.Drawing.Point(0, 0);
            this.fondo_box.Name = "fondo_box";
            this.fondo_box.Size = new System.Drawing.Size(850, 500);
            this.fondo_box.TabIndex = 0;
            this.fondo_box.TabStop = false;
            this.fondo_box.Click += new System.EventHandler(this.fondo_box_Click);
            this.fondo_box.MouseEnter += new System.EventHandler(this.fondo_box_MouseEnter);
            this.fondo_box.MouseLeave += new System.EventHandler(this.fondo_box_MouseLeave);
            // 
            // FormGame
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(850, 500);
            this.Controls.Add(this.fondo_box);
            this.Margin = new System.Windows.Forms.Padding(1);
            this.Name = "FormGame";
            this.Text = "FormGame";
            this.Load += new System.EventHandler(this.FormGame_Load);
            this.Paint += new System.Windows.Forms.PaintEventHandler(this.FormGame_Paint);
            ((System.ComponentModel.ISupportInitialize)(this.fondo_box)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox fondo_box;
    }
}
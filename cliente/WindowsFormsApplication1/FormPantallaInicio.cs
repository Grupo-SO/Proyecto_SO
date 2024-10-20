using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms; 

namespace WindowsFormsApplication1
{
    public partial class FormPantallaInicio : Form
    {
        //=========================================================================================================================\\
        //======================================================= ATRIBUTOS =======================================================\\

        //=========================================================================================================================\\
        //======================================================== MÉTODOS ========================================================\\
        public FormPantallaInicio()
        {
            InitializeComponent();
        }

        private void FormPantallaInicio_Load(object sender, EventArgs e)
        {
            // Cargar la imagen de fondo
            string backgroundImagePath = System.IO.Path.Combine(Application.StartupPath, "image2.png");
            this.BackgroundImage = Image.FromFile(backgroundImagePath);

            // Ajustar el modo de visualización de la imagen
            this.BackgroundImageLayout = ImageLayout.Stretch; // Puedes usar otros modos como Tile, Center, Zoom, etc.

            RoundedButton roundedButton = new RoundedButton();
            roundedButton.Size = new Size(90, 90); // Tamaño cuadrado para mantener la forma redonda

            // Ruta de la imagen
            string imagePath = System.IO.Path.Combine(Application.StartupPath, "imagen1.png");
            roundedButton.ButtonImage = Image.FromFile(imagePath); // Cargar la imagen
            roundedButton.ButtonText = ""; // Texto en el botón

            // Centramos el botón en el formulario
            roundedButton.Left = (this.ClientSize.Width - roundedButton.Width) / 2;
            roundedButton.Top = (int)((this.ClientSize.Height - roundedButton.Height) / 1.3);

            // Asignar el evento Click al botón
            roundedButton.Click += new EventHandler(roundedButton_Click);

            this.Controls.Add(roundedButton);
        }
        private void roundedButton_Click(object sender, EventArgs e)
        {   
            FormInicioSesion FormInicioSesion = new FormInicioSesion();
            FormInicioSesion.ShowDialog();
            this.Close();
        }
    }
}

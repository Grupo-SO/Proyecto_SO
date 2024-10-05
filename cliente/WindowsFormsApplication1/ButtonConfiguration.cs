using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;

public class RoundedButton : Button
{
    public Image ButtonImage { get; set; }
    public string ButtonText { get; set; } = "Empezar";

    public RoundedButton()
    {
        this.FlatStyle = FlatStyle.Flat;
        this.FlatAppearance.BorderSize = 0;
        this.TextAlign = ContentAlignment.MiddleCenter;
    }

    protected override void OnPaint(PaintEventArgs pevent)
    {
        base.OnPaint(pevent);

        Graphics graphics = pevent.Graphics;
        graphics.SmoothingMode = SmoothingMode.AntiAlias;

        // Hacer que el botón sea completamente redondo
        Rectangle rectSurface = this.ClientRectangle;

        using (GraphicsPath pathSurface = new GraphicsPath())
        {
            // Círculo para la superficie del botón
            pathSurface.AddEllipse(0, 0, this.Width, this.Height);
            this.Region = new Region(pathSurface);

            // Dibujar la imagen de fondo si está presente
            if (ButtonImage != null)
            {
                graphics.DrawImage(ButtonImage, 0, 0, this.Width, this.Height);
            }

            // Dibujar el texto del botón
            if (!string.IsNullOrEmpty(ButtonText))
            {
                using (Font font = new Font("Arial", 16, FontStyle.Bold))
                using (Brush textBrush = new SolidBrush(Color.White)) // Cambia el color del texto según tu necesidad
                {
                    SizeF textSize = graphics.MeasureString(ButtonText, font);
                    PointF textLocation = new PointF(
                        (this.Width - textSize.Width) / 2,
                        (this.Height - textSize.Height) / 2
                    );
                    graphics.DrawString(ButtonText, font, textBrush, textLocation);
                }
            }
        }
    }
    protected override void OnResize(EventArgs e)
    {
        base.OnResize(e);
        // Asegurarse de que el botón sea siempre redondo
        if (this.Width != this.Height)
        {
            this.Width = this.Height;
        }
    }
    protected override void OnMouseDown(MouseEventArgs mevent)
    {
        base.OnMouseDown(mevent);
        this.ButtonImage = null; // Cambiar imagen o efecto
        this.Invalidate(); // Redibujar el botón
    }

    protected override void OnMouseUp(MouseEventArgs mevent)
    {
        base.OnMouseUp(mevent);
        this.ButtonImage = Image.FromFile("imagen1.png"); // Restaurar imagen o efecto
        this.Invalidate(); // Redibujar el botón
    }

}

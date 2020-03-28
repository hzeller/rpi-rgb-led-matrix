using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace rpi_rgb_led_matrix_sharp
{
    // provides nodejs binding interface via edge
	public class Startup
	{
        public Task<object> Invoke(dynamic input)
        {
            string command = input.command;
            switch (command) {

                case "init":
                    {
                        int rows = input.rows;
                        int chained = input.chained;
                        int parallel = input.parallel;
                        matrix = new RGBLedMatrix(rows, chained, parallel);
                        canvas = matrix.CreateOffscreenCanvas();
                        break;
                    }
                case "render":
                    {
                        canvas = matrix.SwapOnVsync(canvas);
                        canvas.Clear();
                        break;
                    }
                case "setPixel":
                    {
                        int x = input.x;
                        int y = input.y;
                        int r = input.r;
                        int g = input.g;
                        int b = input.b;
                        canvas.SetPixel(x, y, new Color(r, g, b));
                        break;
                    }
                case "fill":
                    {
                        int r = input.r;
                        int g = input.g;
                        int b = input.b;
                        canvas.Fill(new Color(r, g, b));
                        break;
                    }
                case "drawCircle":
                    {
                        int x = input.x;
                        int y = input.y;
                        int radius = input.radius;
                        int r = input.r;
                        int g = input.g;
                        int b = input.b;
                        canvas.DrawCircle(x, y, radius, new Color(r, g, b));
                        break;
                    }
                case "drawLine":
                    {
                        int x = input.x;
                        int y = input.y;
                        int x1 = input.x1;
                        int y1 = input.y1;
                        int r = input.r;
                        int g = input.g;
                        int b = input.b;
                        canvas.DrawLine(x, y, x1, y1, new Color(r, g, b));
                        break;
                    }
                case "drawText":
                    {
                        string font = input.font;
                        int x = input.x;
                        int y = input.y;
                        int r = input.r;
                        int g = input.g;
                        int b = input.b;
                        int spacing = input.spacing ?? 0;
                        bool vertical = input.vertical ?? false;
                        string text = input.text;
                        canvas.DrawText(fonts[font], x, y, new Color(r, g, b), text, spacing, vertical);
                        break;
                    }
                case "loadFont":
                    {
                        string fontPath = input.fontPath;
                        string name = input.name ?? fontPath;
                        fonts.Add(name, new RGBLedFont(fontPath));
                        break;
                    }
            }
            return Task.FromResult<object>(true);
        }

        private Dictionary<string, RGBLedFont> fonts = new Dictionary<string, RGBLedFont>();
        private RGBLedMatrix matrix;
        private RGBLedCanvas canvas;
    }
}

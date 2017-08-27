using rpi_rgb_led_matrix_sharp;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace lightning
{
    class Program
    {
        
        static float time;
        static void Main(string[] args)
        {
            Console.WriteLine("lightning.exe <num_frames>");
            Console.WriteLine();

            int TOTAL_FRAMES = 10;
            if (args.Length > 0)
            {
                TOTAL_FRAMES = int.Parse(args[0]);
            }

            var matrix = new RGBLedMatrix(32, 2, 1);
            var canvas = matrix.CreateOffscreenCanvas();            
            var resolution = new vec2(canvas.Height, canvas.Width);
            var sw = new Stopwatch();
            sw.Start();

            var frames = new List<Color[]>();

            // pre-calculate frames of animation            
            int i = 0;
            Console.WriteLine("Calculating " + TOTAL_FRAMES +" Frames. This may take a while.");
            for (i=0; i<TOTAL_FRAMES; i++)
            {
                Console.Write(".");
                var frame = new Color[canvas.Width * canvas.Height];
                frames.Add(frame);
                time += .08f;
                int o = 0;
                for (var y = 0; y < canvas.Height; ++y)
                {
                    for (var x = 0; x < canvas.Width; ++x)
                    {
                        frame[o++] = fragment(new vec2(y, x), resolution);
                    }
                }
            }

            // render
            Console.WriteLine("Done!");
            Console.WriteLine("Rendering");
            i = -1;
            while (!Console.KeyAvailable)
            {
                sw.Restart();

                if (++i == TOTAL_FRAMES) i = 0;
                int j = 0;
                for (var y = 0; y < canvas.Height; ++y)
                {
                    for (var x = 0; x < canvas.Width; ++x)
                    {
                        canvas.SetPixel(x,y,frames[i][j++]);
                    }
                }

                canvas = matrix.SwapOnVsync(canvas);

                // force 15 FPS
                var elapsed = sw.ElapsedMilliseconds;
                if (elapsed < 66)
                {
                    Thread.Sleep(66 - (int)elapsed);
                }
            }         
        }

        static Color fragment(vec2 coord, vec2 resolution)
        {
            vec2 uv = (coord.xy / resolution.xy) * 2.0f - 1.75f;
            uv.x *= resolution.x / resolution.y;


            vec3 finalColor = new vec3();
            for (int i = 1; i < 4; ++i)
            {
                float hh = (float)i * 0.1f;
                float t = abs(.750f / ((uv.x + fbm(uv + (time * 5.75f) / (float)i)) * 65f));
                finalColor += t * new vec3(hh + 0.1f, hh + 0.5f, 2.0f);
            }

            return new Color(
                (int)max(finalColor.x * 255,255), 
                (int)max(finalColor.y * 255,255), 
                (int)max(finalColor.z * 255,255));
        }


        static float hash(vec2 p)
        {
            vec3 p2 = new vec3(p.xy, 1.0f);
            return fract(sin(dot(p2, new vec3(37.1f, 61.7f, 12.4f))) * 3758.5453123f);
        }

        static float noise(vec2 p)
        {
            vec2 i = floor(p);
            vec2 f = fract(p);
            f *= f * (1.5f - .5f * f);

            return mix(mix(hash(i + new vec2(0f, 0f)), hash(i + new vec2(1f, 0f)), f.x),
                       mix(hash(i + new vec2(0f, 1f)), hash(i + new vec2(1f, 1f)), f.x),
                       f.y);
        }

        static float fbm(vec2 p)
        {
            float v = 0.0f;
            v += noise(p * 1f) * .5f;
            v += noise(p * 2f) * .25f;
            v += noise(p * 4f) * .125f;
            v += noise(p * 8f) * .0625f;

            return v * 1.0f;
        }


        

        static float pow(float a, float e)
        {
            return (float)Math.Pow(a, e);
        }
        static float length(vec2 v)
        {
            return (float)Math.Sqrt(v.x * v.x + v.y * v.y);
        }
        static float mod(float a, float b)
        {
            return a % b;
        }
        static float min(float val, float min)
        {
            if (val < min) return min;
            return val;
        }
        static float max(float val, float max)
        {
            if (val > max) return max;
            return val;
        }
        static float smoothstep(float edge0, float edge1, float x)
        {
            float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
            return t * t * (3.0f - 2.0f * t);
        }
        static float clamp (float val, float min, float max)
        {
            if (val < min) return min;
            else if (val > max) return max;
            else return val;
        }
        
        struct vec2
        {
            public vec2(float x, float y) { this.x = x; this.y = y; }
            public float x;
            public float y;            
            public static vec2 operator +(vec2 a, vec2 v) { return new vec2(v.x + a.x, v.y + a.y); }
            public static vec2 operator *(vec2 a, vec2 v) { return new vec2(v.x * a.x, v.y * a.y); }
            public static vec2 operator *(float a, vec2 v) { return new vec2(v.x * a, v.y * a); }
            public static vec2 operator *(vec2 v, float a) { return new vec2(v.x * a, v.y * a); }
            public static vec2 operator +(float a, vec2 v ) { return new vec2(v.x + a, v.y + a); }
            public static vec2 operator +(vec2 v, float a) { return new vec2(v.x + a, v.y + a); }
            public static vec2 operator -(vec2 a, vec2 b) { return new vec2(a.x - b.x, a.y - b.y); }
            public static vec2 operator -(vec2 a, float b) { return new vec2(a.x - b, a.y - b); }
            public static vec2 operator -(float a, vec2 b) { return new vec2(b.x + a, b.y + a); }
            public static vec2 operator /(vec2 a, vec2 b) { return new vec2(a.x / b.x, a.y / b.y); }
            public static vec2 operator /(vec2 a, float b) { return new vec2(a.x / b, a.y / b); }
            public vec2 xy => this;
            public vec2 yx => new vec2(y, x);
        }
        struct vec3
        {
            public vec3(float x, float y, float z) { this.x = x; this.y = y; this.z = z; }
            public vec3(vec2 v, float z) { this.x = v.x; this.y = v.y; this.z = z; }
            public float x;
            public float y;
            public float z;
            public static vec3 operator *(float a, vec3 v) { return new vec3(v.x * a, v.y * a, v.z *a); }
            public static vec3 operator +(vec3 v, float a) { return new vec3(v.x + a, v.y + a, v.z +a); }
            public static vec3 operator +(vec3 v, vec3 a) { return new vec3(v.x + a.x, v.y + a.y, v.z + a.z); }
            public vec2 xy => new vec2(x , y);
        }
        static vec3 sqrt(vec3 a )
        {
            return new vec3((float)Math.Sqrt(a.x), (float)Math.Sqrt(a.y), (float)Math.Sqrt(a.z));
        }
        static float abs(float a)
        {
            return Math.Abs(a);
        }
        static float dot(vec2 a, vec2 b)
        {
            return a.x * a.y + b.x * b.y;
        }
        static float dot(vec3 a, vec3 b)
        {
            return a.x * a.y * a.z + b.x * b.y * b.z;
        }
        static float sin(float x)
        {
            return (float)Math.Sin(x);
        }
        static float cos(float x)
        {
            return (float)Math.Cos(x);
        }
    
        static float fract(float a)
        {
            return a - (float)Math.Floor(a);
        }
        static vec2 fract(vec2 a)
        {
            return new vec2(fract(a.x), fract(a.y));
        }
        
        static float floor(float a)
        {
            return (float)Math.Floor(a);
        }
        static vec2 floor(vec2 a)
        {
            return new vec2(floor(a.x), floor(a.y));
        }
        static float mix(float x, float y, float a)
        {
            return x * (1 - a) + y * a;
        }        
    }
}

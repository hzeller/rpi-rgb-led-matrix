using RPiRgbLEDMatrix;
using System.Numerics;

const float MaxModuleSpeed = 0.1f;
const float FOV = 60f;
const float Scale = 1.1f;
const float LerpPow = 0.002f;
const int ChangePerFrames = 50;

using var leds = new RGBLedMatrix(32, 1, 1);
var canvas = leds.CreateOffscreenCanvas();

var (centerX, centerY) = (canvas.Width / 2, canvas.Height / 2);

var rnd = new Random();
var angleSpeed = new Vector3();
var nextAngleSpeed = new Vector3();
var frame = -1;

var rotateMatrix = Matrix4x4.Identity;
var scaleMatrix = Matrix4x4.CreateScale(Scale);
var projectMatrix = Matrix4x4.CreatePerspectiveFieldOfView(FOV / 180 * MathF.PI, 1, 0.1f, 100f);
var cameraMatrix = Matrix4x4.CreateLookAt(new(0, 0, 4), new(0, 0, 0), new(0, 1, 0));

// run until user presses Ctrl+C
var running = true;
Console.CancelKeyPress += (_, e) =>
{
    running = false;
    e.Cancel = true; // do not terminate program with Ctrl+C, we need to dispose
};
while (running)
{
    var frameStart = Environment.TickCount64;

    // update angle speed
    frame = (frame + 1) % ChangePerFrames;
    if(frame == 0)
        nextAngleSpeed = new Vector3(
            (rnd.NextSingle() * 2 - 1) * MaxModuleSpeed,
            (rnd.NextSingle() * 2 - 1) * MaxModuleSpeed,
            (rnd.NextSingle() * 2 - 1) * MaxModuleSpeed
        );

    angleSpeed = Vector3.Lerp(angleSpeed, nextAngleSpeed, LerpPow);

    // update matrices
    rotateMatrix *= Matrix4x4.CreateRotationX(angleSpeed.X);
    rotateMatrix *= Matrix4x4.CreateRotationY(angleSpeed.Y);
    rotateMatrix *= Matrix4x4.CreateRotationZ(angleSpeed.Z);
    var matrix = scaleMatrix * rotateMatrix * cameraMatrix * projectMatrix;

    // calculate points
    var top1 = Vector4.Transform(new Vector3( 1,  1,  1), matrix);
    var top2 = Vector4.Transform(new Vector3(-1,  1,  1), matrix);
    var top3 = Vector4.Transform(new Vector3(-1,  1, -1), matrix);
    var top4 = Vector4.Transform(new Vector3( 1,  1, -1), matrix);

    var bot1 = Vector4.Transform(new Vector3( 1, -1,  1), matrix);
    var bot2 = Vector4.Transform(new Vector3(-1, -1,  1), matrix);
    var bot3 = Vector4.Transform(new Vector3(-1, -1, -1), matrix);
    var bot4 = Vector4.Transform(new Vector3( 1, -1, -1), matrix);

    // draw
    canvas.Fill(new(0, 0, 0));
    DrawLine(top1, top2);
    DrawLine(top2, top3);
    DrawLine(top3, top4);
    DrawLine(top4, top1);

    DrawLine(bot1, bot2);
    DrawLine(bot2, bot3);
    DrawLine(bot3, bot4);
    DrawLine(bot4, bot1);

    DrawLine(top1, bot1);
    DrawLine(top2, bot2);
    DrawLine(top3, bot3);
    DrawLine(top4, bot4);

    leds.SwapOnVsync(canvas);
    // force 30 FPS
    var elapsed = Environment.TickCount64 - frameStart;
    if (elapsed < 33) Thread.Sleep(33 - (int)elapsed);
}

void DrawLine(Vector4 a, Vector4 b) => canvas.DrawLine(
    (int)(a.X * a.W + centerX), (int)(a.Y * a.W + centerY),
    (int)(b.X * b.W + centerX), (int)(b.Y * b.W + centerY),
    new(255, 255, 255));
